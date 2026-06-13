/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/app_version.h>
#include <zephyr/kernel.h>

int main(void)
{
	/*
	 * key_thread 和 led_thread 都通过 K_THREAD_DEFINE() 自动创建。
	 * main() 这里只负责打印启动信息，实际按键和 LED 逻辑在线程中运行。
	 */
	printk("04_key_control_led example start\r\n");
	printk("App version: %s\r\n", APP_VERSION_STRING);

	return 0;
}
