#include <stdbool.h>

#include <zephyr/app_version.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include "key_led_msg.h"

#define LED_THREAD_STACK_SIZE 1024
#define LED_THREAD_PRIORITY 5

/*
 * 从设备树 alias 中取得 LED GPIO 配置。
 *
 * app.overlay 或开发板 dts 中通常会定义 led0、led1 等 alias。
 */
#define LED_SPEC(alias) GPIO_DT_SPEC_GET(DT_ALIAS(alias), gpios)

/*
 * 保存所有 LED 的 GPIO 信息。
 *
 * struct gpio_dt_spec 里面包含：
 * - port：GPIO 控制器设备
 * - pin：GPIO 引脚编号
 * - dt_flags：设备树里的 GPIO 标志，例如 GPIO_ACTIVE_LOW
 */
static const struct gpio_dt_spec leds[] = {
	LED_SPEC(led0),
	LED_SPEC(led1),
	LED_SPEC(led2),
	LED_SPEC(led3),
};

static struct led_msg msg;

/*
 * 定义一个消息队列，用来在按键线程和 LED 线程之间传递命令。
 *
 * 参数含义：
 * - led_msgq：队列名字
 * - sizeof(struct led_msg)：每条消息的大小
 * - 8：队列最多缓存 8 条消息
 * - 4：队列内存按 4 字节对齐
 */
K_MSGQ_DEFINE(led_msgq, sizeof(struct led_msg), 8, 4);

void led_thread(void *p1, void *p2, void *p3)
{
	int ret;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	/* 初始化每一个 LED 对应的 GPIO 引脚。 */
	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		/*
		 * gpio_is_ready_dt() 检查 GPIO 控制器是否已经初始化完成。
		 * 如果控制器没有 ready，后续配置或读写这个引脚都会失败。
		 */
		if (!gpio_is_ready_dt(&leds[i])) {
			printk("LED%u GPIO controller is not ready\r\n", i);
			return;
		}

		/*
		 * 将 LED 引脚配置为输出，并设置为“非激活”状态。
		 *
		 * GPIO_OUTPUT_INACTIVE 会结合设备树里的 GPIO_ACTIVE_LOW/
		 * GPIO_ACTIVE_HIGH 自动决定实际输出电平。因此这里表示
		 * “先让 LED 熄灭”，而不是固定输出高电平或低电平。
		 */
		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		if (ret < 0) {
			printk("Failed to configure LED%u: %d\r\n", i, ret);
			return;
		}
	}

	while (1) {
		/*
		 * K_FOREVER 表示如果队列为空，当前线程会一直等待。
		 * 等到按键线程放入消息后，这里才继续往下执行。
		 */
		k_msgq_get(&led_msgq, &msg, K_FOREVER);

		if (msg.led_id >= ARRAY_SIZE(leds)) {
			continue;
		}

		switch (msg.cmd) {
		case LED_CMD_ON:
			gpio_pin_set_dt(&leds[msg.led_id], 1);
			break;

		case LED_CMD_OFF:
			gpio_pin_set_dt(&leds[msg.led_id], 0);
			break;

		default:
			break;
		}
	}
}

/*
 * 定义并启动 LED 线程。
 *
 * 这个线程启动后会先初始化 LED，然后阻塞等待 led_msgq 中的消息。
 */
K_THREAD_DEFINE(led_thread_id, LED_THREAD_STACK_SIZE,
		led_thread, NULL, NULL, NULL,
		LED_THREAD_PRIORITY, 0, 0);
