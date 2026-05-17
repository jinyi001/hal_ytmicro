# Cmu_Demo

---

## 功能描述

该示例演示了 CMU 功能以及自测试：

- 时钟丢失时系统复位，时钟超出范围时产生中断。
- 自测试会手动修改 CMU 中 FXOSC 的频率范围，制造时钟超出范围的场景
- 时钟超出范围时立即产生中断，随机软件复位
- 复位后查看 CMU 的状态，确认状态

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