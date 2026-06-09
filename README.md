# Armfly V7 Zephyr Examples

This repository contains Zephyr examples and notes for the Armfly STM32H743XIH6
V7 development board.

## Board

- Board: Armfly STM32H743XIH6 V7
- Zephyr board target: `armfly_stm32h743xih6`
- MCU: STM32H743XIH6

## Repository Layout

```text
doc/
  00_start/        Basic setup notes
  01_helloworld/   Hello World notes

src/
  01_helloworld/   Hello World example project
```

## Build

Run commands from the Zephyr workspace root:

```powershell
cd D:\zephyrproject
west build -b armfly_stm32h743xih6 .\armfly_v7_example\src\01_helloworld -d .\armfly_v7_example\build\01_helloworld -p always
```

## Flash

Using DAPLink or another CMSIS-DAP probe:

```powershell
west flash -d .\armfly_v7_example\build\01_helloworld -r pyocd
```

Using ST-Link:

```powershell
west flash -d .\armfly_v7_example\build\01_helloworld -r stm32cubeprogrammer
```

## Notes

Some onboard peripherals, such as the LEDs, are connected through an external
latch on STM32 FMC Bank1. Examples that use these peripherals need to enable
MEMC and the related board devicetree overlay explicitly.

## License

MIT License
