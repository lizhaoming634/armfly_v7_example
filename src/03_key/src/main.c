#include <stdbool.h>

#include <zephyr/app_version.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

/* 按键轮询周期，单位是毫秒。数值越小响应越快，但 CPU 唤醒越频繁。 */
#define KEY_POLL_INTERVAL_MS 20

/*
 * 从设备树 alias 中取得按键 GPIO 配置。
 *
 * 例如 KEY_SPEC(sw0) 会先通过 DT_ALIAS(sw0) 找到 sw0 指向的节点，
 * 再读取这个节点的 gpios 属性，生成 struct gpio_dt_spec。
 */
#define KEY_SPEC(alias) GPIO_DT_SPEC_GET(DT_ALIAS(alias), gpios)

/*
 * 取得设备树节点名称，用来打印按键名字。
 *
 * 注意：这里拿到的是节点名，例如 "joy-down"；如果想打印
 * "JOY_DOWN" 这种自定义名称，可以在 dts 子节点里增加 label
 * 属性，然后改用 DT_PROP(DT_ALIAS(alias), label)。
 */
#define KEY_NAME(alias) DT_NODE_FULL_NAME(DT_ALIAS(alias))

/* 保存所有按键的 GPIO 信息，顺序和 key_names[] 保持一致。 */
static const struct gpio_dt_spec keys[] = {
	KEY_SPEC(sw0),
	KEY_SPEC(sw1),
	KEY_SPEC(sw2),
	KEY_SPEC(sw3),
	KEY_SPEC(sw4),
	KEY_SPEC(sw5),
	KEY_SPEC(sw6),
	KEY_SPEC(sw7),
};

/* 每个按键对应的显示名称，用于 printk 输出。 */
static const char *const key_names[] = {
	KEY_NAME(sw0),
	KEY_NAME(sw1),
	KEY_NAME(sw2),
	KEY_NAME(sw3),
	KEY_NAME(sw4),
	KEY_NAME(sw5),
	KEY_NAME(sw6),
	KEY_NAME(sw7),
};

/*
 * 记录每个按键上一次扫描时是否处于按下状态。
 *
 * gpio_pin_get_dt() 只能告诉我们“当前是否按下”。如果不保存历史状态，
 * 按住按键时每次轮询都会打印一次。key_pressed[] 用来实现“只在刚按下
 * 的那一刻打印一次”。
 */
static bool key_pressed[ARRAY_SIZE(keys)] = { 0 };

int main(void)
{
	int ret;

	printk("03 KEY example start\r\n");
	printk("App version: %s\r\n", APP_VERSION_STRING);

	/* 初始化每一个按键对应的 GPIO 引脚。 */
	for (size_t i = 0; i < ARRAY_SIZE(keys); i++) {
		/* 检查 GPIO 控制器是否 ready。 */
		if (!gpio_is_ready_dt(&keys[i])) {
			printk("KEY%u GPIO controller is not ready\r\n", i);
			return 0;
		}

		/* 将按键引脚配置为输入模式。 */
		ret = gpio_pin_configure_dt(&keys[i], GPIO_INPUT);
		if (ret < 0) {
			printk("Failed to configure %s: %d\r\n", key_names[i], ret);
			return 0;
		}
	}

	while (1) {
		/* 轮询读取每一个按键的当前状态。 */
		for (size_t i = 0; i < ARRAY_SIZE(keys); i++) {
			/*
			 * gpio_pin_get_dt() 返回逻辑值：
			 *   ret > 0  表示按键处于激活状态，也就是“按下”
			 *   ret == 0 表示按键未激活，也就是“松开”
			 *   ret < 0  表示读取失败，返回值是错误码
			 *
			 * 如果设备树里配置了 GPIO_ACTIVE_LOW，函数会自动把
			 * 物理低电平转换成逻辑 1，因此应用层仍然可以用
			 * ret != 0 判断“按下”。
			 */
			ret = gpio_pin_get_dt(&keys[i]);
			if (ret < 0) {
				printk("Failed to read %s: %d\r\n", key_names[i], ret);
				continue;
			}

			/*
			 * 检测“松开 -> 按下”的状态变化。
			 *
			 * ret != 0 表示当前按下；
			 * !key_pressed[i] 表示上一次扫描时还没有按下。
			 * 两个条件同时成立，说明这是一次新的按下事件。
			 */
			if ((ret != 0) && !key_pressed[i]) {
				printk("%s pressed\r\n", key_names[i]);
				key_pressed[i] = true;
			} else if (ret == 0) {
				/*
				 * 当前已经松开，清除历史状态。
				 * 这样下次再次按下时，才能重新打印 pressed。
				 */
				key_pressed[i] = false;
			}
		}

		/* 等待一段时间再扫描下一轮，避免一直占用 CPU。 */
		k_msleep(KEY_POLL_INTERVAL_MS);
	}

	return 0;
}
