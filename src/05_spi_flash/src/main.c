/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>

#include <stdio.h>
#include <string.h>

#if DT_HAS_COMPAT_STATUS_OKAY(jedec_spi_nor)
#define SPI_FLASH_COMPAT jedec_spi_nor
#else
#error No jedec,spi-nor flash node enabled
#endif

#define SPI_FLASH_TEST_REGION_OFFSET 0xff000
#define SPI_FLASH_SECTOR_SIZE        4096

static const uint8_t erased[] = {0xff, 0xff, 0xff, 0xff};
static const uint8_t expected[] = {0x55, 0xaa, 0x66, 0x99};

static void single_sector_test(const struct device *flash_dev)
{
	uint8_t buf[sizeof(expected)];
	int ret;

	printf("\nTest 1: Flash erase\n");
	ret = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			  SPI_FLASH_SECTOR_SIZE);
	if (ret != 0) {
		printf("flash_erase failed: %d\n", ret);
		return;
	}

	memset(buf, 0, sizeof(buf));
	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, sizeof(buf));
	if (ret != 0) {
		printf("flash_read failed: %d\n", ret);
		return;
	}

	if (memcmp(erased, buf, sizeof(erased)) != 0) {
		printf("Flash erase failed at offset 0x%x\n",
		       SPI_FLASH_TEST_REGION_OFFSET);
		return;
	}

	printf("Flash erase succeeded!\n");

	printf("\nTest 2: Flash write\n");
	printf("Attempting to write %u bytes\n", (unsigned int)sizeof(expected));
	ret = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, expected,
			  sizeof(expected));
	if (ret != 0) {
		printf("flash_write failed: %d\n", ret);
		return;
	}

	memset(buf, 0, sizeof(buf));
	ret = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, sizeof(buf));
	if (ret != 0) {
		printf("flash_read failed: %d\n", ret);
		return;
	}

	if (memcmp(expected, buf, sizeof(expected)) == 0) {
		printf("Data read matches data written. Good!!\n");
		return;
	}

	printf("Data read does not match data written!!\n");
	for (size_t i = 0; i < sizeof(expected); i++) {
		printf("%08x wrote %02x read %02x %s\n",
		       (uint32_t)(SPI_FLASH_TEST_REGION_OFFSET + i),
		       expected[i], buf[i], expected[i] == buf[i] ? "match" : "MISMATCH");
	}
}

int main(void)
{
	const struct device *flash_dev =
		DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(SPI_FLASH_COMPAT));

	printf("05_spi_flash example start\r\n");
	printf("App version: %s\r\n", APP_VERSION_STRING);

	printf("JEDEC SPI-NOR flash testing\n");
	printf("===========================\n");
	printf("Device: %s\n", flash_dev->name);

	if (!device_is_ready(flash_dev)) {
		printf("%s: device not ready\n", flash_dev->name);
		return 0;
	}

	single_sector_test(flash_dev);

	return 0;
}
