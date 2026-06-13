#include <stdbool.h>

#include <zephyr/app_version.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include "key_led_msg.h"

/*
 * 从设备树 alias 中取得按键 GPIO 配置。
 *
 * 例如 KEY_SPEC(sw0) 会先通过 DT_ALIAS(sw0) 找到 sw0 指向的节点，
 * 再读取这个节点的 gpios 属性，生成 struct gpio_dt_spec。
 */
#define KEY_SPEC(alias) GPIO_DT_SPEC_GET(DT_ALIAS(alias), gpios)

/* 取得设备树节点名称，用来打印按键名字。 */
#define KEY_NAME(alias) DT_NODE_FULL_NAME(DT_ALIAS(alias))

#define KEY_THREAD_STACK_SIZE 1024
#define KEY_THREAD_PRIORITY 5

/* 保存所有按键的 GPIO 信息，顺序要和 key_names[] 保持一致。 */
static const struct gpio_dt_spec keys[] = {
	KEY_SPEC(sw0),
	KEY_SPEC(sw1),
	KEY_SPEC(sw2),
	KEY_SPEC(sw3),
};

/* 每个按键对应的显示名称，用于 printk 输出。 */
static const char *const key_names[] = {
	KEY_NAME(sw0),
	KEY_NAME(sw1),
	KEY_NAME(sw2),
	KEY_NAME(sw3),
};

static struct k_work_delayable key_work;

/*
 * 已经确认稳定后的按键状态。
 *
 * 每个 bit 表示一个按键：bit=1 表示按下，bit=0 表示松开。
 */
static uint32_t stable_key_state;
static struct gpio_callback key_cb_data[ARRAY_SIZE(keys)];

static uint32_t read_key_state(void)
{
	uint32_t state = 0;

	for (size_t i = 0; i < ARRAY_SIZE(keys); i++) {
		/* gpio_pin_get_dt() 会考虑设备树里的 GPIO_ACTIVE_LOW/HIGH。 */
		int ret = gpio_pin_get_dt(&keys[i]);

		if (ret > 0) {
			state |= BIT(i);
		}
	}

	return state;
}

static void key_work_handler(struct k_work *work)
{
	uint32_t new_state;
	uint32_t changed;

	ARG_UNUSED(work);

	new_state = read_key_state();
	changed = new_state ^ stable_key_state;

	/* 没有变化说明可能只是抖动，不需要发送 LED 控制消息。 */
	if (changed == 0) {
		return;
	}

	for (size_t i = 0; i < ARRAY_SIZE(keys); i++) {
		if ((changed & BIT(i)) == 0) {
			continue;
		}

		/*
		 * 按键线程不直接控制 LED，而是把命令放进消息队列。
		 * LED 线程收到消息后再操作 GPIO，这样两个线程职责更清楚。
		 */
		if ((new_state & BIT(i)) != 0) {
			struct led_msg msg = {
				.led_id = i,
				.cmd = LED_CMD_ON,
			};

			printk("%s pressed\r\n", key_names[i]);
			k_msgq_put(&led_msgq, &msg, K_NO_WAIT);
		} else {
			struct led_msg msg = {
				.led_id = i,
				.cmd = LED_CMD_OFF,
			};

			printk("%s released\r\n", key_names[i]);
			k_msgq_put(&led_msgq, &msg, K_NO_WAIT);
		}
	}

	stable_key_state = new_state;
}

static void key_isr(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	/*
	 * 按键机械触点会抖动，所以中断里不马上读取按键。
	 * 每次中断都把延迟 work 重新安排到 20 ms 后执行，
	 * 只有按键状态稳定下来后才会进入 key_work_handler()。
	 */
	k_work_reschedule(&key_work, K_MSEC(20));
}

void key_thread(void *p1, void *p2, void *p3)
{
	int ret;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	/* 初始化每一个按键对应的 GPIO 引脚。 */
	for (size_t i = 0; i < ARRAY_SIZE(keys); i++) {
		/* 检查 GPIO 控制器是否已经初始化完成。 */
		if (!gpio_is_ready_dt(&keys[i])) {
			printk("KEY%u GPIO controller is not ready\r\n", i);
			return;
		}

		/* 将按键引脚配置为输入模式。 */
		ret = gpio_pin_configure_dt(&keys[i], GPIO_INPUT);
		if (ret < 0) {
			printk("Failed to configure %s: %d\r\n", key_names[i], ret);
			return;
		}
	}

	/* 准备延迟 work，并记录启动时的初始按键状态。 */
	k_work_init_delayable(&key_work, key_work_handler);
	stable_key_state = read_key_state();

	/* 注册 GPIO 中断回调函数。 */
	for (size_t i = 0; i < ARRAY_SIZE(keys); i++) {
		gpio_init_callback(&key_cb_data[i], key_isr, BIT(keys[i].pin));

		ret = gpio_add_callback(keys[i].port, &key_cb_data[i]);
		if (ret < 0) {
			printk("Failed to add callback for %s: %d\r\n", key_names[i], ret);
			return;
		}
	}

	/* 打开双边沿中断：按下和松开都会触发。 */
	for (size_t i = 0; i < ARRAY_SIZE(keys); i++) {
		ret = gpio_pin_interrupt_configure_dt(&keys[i], GPIO_INT_EDGE_BOTH);
		if (ret < 0) {
			printk("Failed to configure interrupt for %s: %d\r\n", key_names[i], ret);
			return;
		}
	}
}

/*
 * 定义并启动按键线程。
 *
 * 最后一个参数 0 表示系统启动后立即运行；如果填 K_MSEC(x)，
 * 就会延迟 x 毫秒再启动线程。
 */
K_THREAD_DEFINE(key_thread_id, KEY_THREAD_STACK_SIZE,
		key_thread, NULL, NULL, NULL,
		KEY_THREAD_PRIORITY, 0, 0);
