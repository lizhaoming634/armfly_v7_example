#ifndef KEY_LED_MSG_H
#define KEY_LED_MSG_H

#include <stdint.h>
#include <zephyr/kernel.h>

/* LED 线程支持的控制命令。 */
enum led_cmd_type {
	LED_CMD_OFF,
	LED_CMD_ON,
};

/*
 * 按键线程发送给 LED 线程的消息。
 *
 * led_id 对应 leds[] 数组下标：
 * - 0 表示 led0
 * - 1 表示 led1
 * 以此类推。
 */
struct led_msg {
	uint8_t led_id;
	enum led_cmd_type cmd;
};

/*
 * 消息队列在 led_thread.c 中用 K_MSGQ_DEFINE() 定义。
 * 这里用 extern 声明，让 key_thread.c 也可以访问同一个队列。
 */
extern struct k_msgq led_msgq;

#endif
