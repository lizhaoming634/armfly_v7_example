# Armfly V7 Zephyr 示例

这个仓库用于记录 Armfly STM32H743XIH6 V7 开发板的 Zephyr 示例代码和学习笔记。

## 开发板信息

- 开发板：Armfly STM32H743XIH6 V7
- Zephyr 板级名称：`armfly_stm32h743xih6`
- MCU：STM32H743XIH6

## 目录结构

```text
doc/
  00_start/        环境搭建和基础使用
  01_helloworld/   Hello World 示例说明

src/
  01_helloworld/   Hello World 示例工程
```

## 快速开始

第一次使用建议先阅读环境搭建文档：

- [00. 环境搭建](doc/00_start/00_start.md)

## 注意事项

开发板上的部分外设通过 STM32 FMC Bank1 外部锁存器连接，例如板载 LED。
这些外设默认不会在 Zephyr 板级设备树中启用。使用相关示例时，需要在应用
里显式启用 MEMC，并通过 overlay 启用对应的设备树节点。

## 许可证

见 [LICENSE](LICENSE)。
