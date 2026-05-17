# Power_Mode_Demo

---

## 功能描述

该示例演示了芯片可以进入各种低功耗模式：

- 当 MCU 进入非 Power Down 模式时，例如 Standby 模式，用户可通过按键（PTC1），通过 GPIO 中断来唤醒 MCU，此时程序会接着往下运行
- 当 MCU 进入 Power Down 模式时，用户需要配置 WKU，同样通过按键（PTC1），通过 WKU 来唤醒 MCU，此时 MCU 会复位

---

## 硬件需求

+ **开发板**: YTM32B1MD1-EVB-Q100
+ **调试器**: J-LINK v9.2 或更高版本

---

## 软件需求

+ **SDK**     : YTM32B1MD1_1_4_0 或更高版本
+ **GCC**     : GNU GCC v10.3 或更高版本
+ **Keil/MDK**: ARM Keil/MDK v5.36 或更高版本
+ **IAR**     : IAR v8.40 或更高版本
+ **VS Code** : Microsoft Visual Studio Code IDE v1.76 或更高版本（GNU GCC 工具链，CMake v3.25，ninja，Cortex-Debug 扩展安装）
+ **J-LINK**  : SEGGER JLINK v6.96 或更高版本，需要安装 YTM32B1MD1 补丁（支持包）
+ **Ozone**   : SEGGER Ozone v3.28 或更高版本

---