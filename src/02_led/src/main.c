#include <zephyr/app_version.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* 每个 LED 点亮后保持的时间，单位是毫秒。 */
#define LED_BLINK_DELAY_MS 200

/*
 * 从设备树 alias 中取得 LED 的 GPIO 配置。
 *
 * 例如 LED_SPEC(led0) 会展开为：
 *   GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios)
 *
 * 它会从板级 dts 的 aliases 节点找到 led0 对应的设备树节点，
 * 再读取这个节点的 gpios 属性，生成 struct gpio_dt_spec。
 */
#define LED_SPEC(alias) GPIO_DT_SPEC_GET(DT_ALIAS(alias), gpios)

/*
 * 保存所有 LED 的 GPIO 信息。
 *
 * struct gpio_dt_spec 里面包含：
 *   - port：GPIO 控制器设备
 *   - pin：GPIO 引脚编号
 *   - dt_flags：设备树里配置的 GPIO 标志，例如 GPIO_ACTIVE_LOW
 */
static const struct gpio_dt_spec leds[] = {
	LED_SPEC(led0),
	LED_SPEC(led1),
	LED_SPEC(led2),
	LED_SPEC(led3),
};

int main(void)
{
	int ret;

	printk("02 LED example start\r\n");
	printk("App version: %s\r\n", APP_VERSION_STRING);

	/* 初始化每一个 LED 对应的 GPIO 引脚。 */
	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		/*
		 * gpio_is_ready_dt() 检查 GPIO 控制器是否已经初始化完成。
		 * 如果控制器没有 ready，后续配置或读写这个引脚都会失败。
		 */
		if (!gpio_is_ready_dt(&leds[i])) {
			printk("LED%u GPIO controller is not ready\r\n", i);
			return 0;
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
			return 0;
		}
	}

	while (1) {
		/* 依次点亮每个 LED，形成流水灯效果。 */
		for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
			/*
			 * gpio_pin_set_dt() 写的是逻辑值：
			 *   1 表示激活 LED
			 *   0 表示关闭 LED
			 *
			 * 如果设备树配置了 GPIO_ACTIVE_LOW，驱动会自动转换
			 * 成实际的低电平点亮。
			 */
			gpio_pin_set_dt(&leds[i], 1);
			k_msleep(LED_BLINK_DELAY_MS);
			gpio_pin_set_dt(&leds[i], 0);
		}
	}

	return 0;
}
