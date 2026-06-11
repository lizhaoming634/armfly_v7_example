# 00. 环境搭建

本文使用 Windows PowerShell，并假设 Zephyr 工作目录位于：

```text
D:\zephyrproject
```

建议将 `zephyr` 和 `zephyr-sdk` 文件夹放在同一个工作目录下面。本文后续示例都会以 `D:\zephyrproject` 作为工作目录进行讲解；你也可以设置成自己喜欢的目录，只需要在命令中替换为对应路径即可。

如果你还没有安装 Zephyr，建议先参考 Zephyr 官方入门文档完成基础环境安装：

https://docs.zephyrproject.org/latest/develop/getting_started/index.html

本文不会完整替代官方安装文档，只记录 Armfly STM32H743XIH6 V7 开发板示例
常用的构建、烧录和调试命令。

## 1. 安装基础工具

需要先安装以下工具：

- Git
- Python 3
- CMake
- Ninja
- Zephyr SDK
- 与调试器对应的烧录工具

常用烧录工具：

- DAPLink 或 CMSIS-DAP：pyOCD
- ST-Link：STM32CubeProgrammer
- J-Link：SEGGER J-Link Software

## 2. 进入 Zephyr 工作目录

打开 PowerShell，进入 Zephyr 工作目录：

```powershell
cd D:\zephyrproject
```

如果你使用 Python 虚拟环境，先激活它：

```powershell
.\.venv\Scripts\activate
```

如果提示找不到 `west`、`ninja` 或 `pyocd`，可以把虚拟环境目录临时加入
`PATH`：

```powershell
$env:PATH = "D:\zephyrproject\.venv\Scripts;$env:PATH"
```

检查基础工具是否可用：

```powershell
west --version
cmake --version
ninja --version
python --version
```

## 3. 检查开发板名称

Armfly STM32H743XIH6 V7 在 Zephyr 中的板级名称是：

```text
armfly_stm32h743xih6
```

可以用下面的命令检查 Zephyr 是否识别到这个板子：

```powershell
west boards | findstr armfly
```

## 4. 编译 Hello World

在 `D:\zephyrproject` 目录下执行：

```powershell
cd D:\zephyrproject
```

通用格式：

```powershell
west build -b <开发板名称> <代码目录> -d <本工程构建输出目录> -p always
```

本示例命令：

```powershell
west build -b armfly_stm32h743xih6 .\armfly_v7_example\src\01_helloworld -d .\armfly_v7_example\build\01_helloworld -p always
```

生成的固件位于：

```text
D:\zephyrproject\armfly_v7_example\build\01_helloworld\zephyr
```

## 5. 使用 DAPLink 或 CMSIS-DAP 烧录

先检查 pyOCD 是否能识别调试器：

```powershell
pyocd list
```

使用 pyOCD 烧录：

通用格式：

```powershell
west flash -d <本工程构建输出目录> -r <runner> [runner参数]
```

本示例命令：

```powershell
west flash -d .\armfly_v7_example\build\01_helloworld -r pyocd
```

如果 pyOCD 无法自动识别芯片型号，可以手动指定 target：

```powershell
west flash -d .\armfly_v7_example\build\01_helloworld -r pyocd --target stm32h743xx
```

## 6. 使用 ST-Link 烧录

通过 SWD 连接 ST-Link 后执行：

```powershell
west flash -d .\armfly_v7_example\build\01_helloworld -r stm32cubeprogrammer
```

## 7. 使用 J-Link 烧录

通过 SWD 连接 J-Link 后执行：

```powershell
west flash -d .\armfly_v7_example\build\01_helloworld -r jlink --speed 1000
```

## 8. 串口输出

默认 Zephyr 控制台使用 `USART1`，串口参数为：

```text
115200 8N1
```

打开串口终端后复位开发板，Hello World 示例应当会在串口输出信息。

## 9. 板载 LED 注意事项

Armfly STM32H743XIH6 V7 的板载 LED 通过 STM32 FMC Bank1 外部锁存器连接。
因为访问锁存器之前必须先初始化 FMC 控制器，所以 Zephyr 上游板级设备树默认
禁用了这些 LED 相关节点。

使用板载 LED 的示例需要：

- 在 `prj.conf` 中启用 `CONFIG_MEMC=y`,`CONFIG_MEMC_STM32=y`
- 在应用 overlay 中启用 `latch_gpio`
- 在应用 overlay 中启用 `leds` 节点，并根据需要提供 `led0` 等 aliases

这样可以避免程序在 FMC 地址空间尚未初始化时访问锁存器，从而导致 HardFault。

## 10. 常见问题

如果 `west build` 提示找不到 Ninja，先确认虚拟环境已激活，或者手动设置
`PATH`：

```powershell
$env:PATH = "D:\zephyrproject\.venv\Scripts;$env:PATH"
```

如果烧录失败，检查以下内容：

- 开发板是否已经供电
- SWD 接线是否正确
- 杜邦线是否过长，试试降低速度
- 是否有其他软件正在占用同一个调试器
- `west flash` 选择的 runner 是否和当前调试器匹配
