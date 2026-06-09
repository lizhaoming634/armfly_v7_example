# 01. Hello World

本节用于验证 Zephyr 工程是否可以在 Armfly STM32H743XIH6 V7 开发板上正常
编译、烧录和运行。

## 编译

在 `D:\zephyrproject` 目录下执行：

通用格式：

```powershell
west build -b <开发板名称> <代码目录> -d <本工程构建输出目录> -p always
```

本示例命令：

```powershell
west build -b armfly_stm32h743xih6 .\armfly_v7_example\src\01_helloworld -d .\armfly_v7_example\build\01_helloworld -p always
```

## 烧录

根据你使用的调试器选择一种方式。

DAPLink 或 CMSIS-DAP：

通用格式：

```powershell
west flash -d <本工程构建输出目录> -r <runner> [runner参数]
```

本示例命令：

```powershell
west flash -d .\armfly_v7_example\build\01_helloworld -r pyocd
```

ST-Link：

```powershell
west flash -d .\armfly_v7_example\build\01_helloworld -r stm32cubeprogrammer
```

J-Link：

```powershell
west flash -d .\armfly_v7_example\build\01_helloworld -r jlink --speed 1000
```

## VERSION 文件

`src/01_helloworld/VERSION` 用于描述这个示例应用自己的版本号。建议只保留
简单的 `KEY = VALUE` 格式，不要在 `VERSION` 文件里添加 `//` 行尾注释。

字段含义如下：

```text
VERSION_MAJOR = 1
```

主版本号，通常用于较大的功能变化。

```text
VERSION_MINOR = 0
```

次版本号，通常用于新增功能。

```text
PATCHLEVEL = 0
```

修订版本号，通常用于 bug 修复或小改动。

```text
VERSION_TWEAK = 0
```

微调版本号，通常可以保持为 `0`。

```text
EXTRAVERSION = helloworld
```

额外版本信息，常用于标记示例名称、测试版本或开发版本。

## 运行结果

打开 `USART1` 串口，参数为 `115200 8N1`。复位开发板后，串口应当可以看到
Hello World 输出。
