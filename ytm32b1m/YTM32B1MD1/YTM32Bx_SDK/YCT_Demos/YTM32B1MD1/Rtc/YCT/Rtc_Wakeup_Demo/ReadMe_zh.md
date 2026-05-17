# Rtc_Wakeup_Demo

---

## 功能描述

该工程演示了 RTC 唤醒从待机模式唤醒的使用。

- 由于自测试需求，当前例程选用 SIRC 做 RTC 计数时钟，但无法分频，所以秒中断与闹钟中断均异常，用户可外部 RTC 引脚灌入 32.768kHz 的时钟信号完成精准定时
- 每秒发生一次秒中断，每 5 秒发生一次闹钟中断
- 在唤醒循环中，RTC 中断将唤醒芯片，如果唤醒计数达到 10，则循环将停止
- 在 RTC 唤醒循环后，如果秒中断计数达到 6，则整个演示例程将停止

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