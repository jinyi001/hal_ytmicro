# Ewdg_Demo

---

## 功能描述

本例程体现了 EWDG 的两个功能：
- 当 EWDG_IN 与配置的不一致时，可以正常喂狗，超时后 EWDG_OUT 会拉低
- 当 EWDG_IN 与配置的一致时，无法喂狗，且一喂狗，EWDG_OUT 会拉低
- 由于 EWDG 模块只能初始化一次，所以自测程序通过复位去实现另一个功能
- 用户可短接 PTE17 与 EWDG_OUT，PTE13 与 EWDG_IN 来完成自测试

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