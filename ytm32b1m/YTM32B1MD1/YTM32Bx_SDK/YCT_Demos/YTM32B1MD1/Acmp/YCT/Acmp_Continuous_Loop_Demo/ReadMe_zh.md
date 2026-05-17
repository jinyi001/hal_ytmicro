# Acmp_Continuous_Loop_Demo

---

## 功能描述

本示例用于演示 SDK ACMP 模块的配置和使用，以及自测试:

1. 旋转电位器以调整通道 0 的电压为 4V
2. 旋转电位器以调整通道 1 的电压为 3V
3. 程序先设置内部 DAC 电压为 2.5V，此时通道 0 与通道 1 电压均大于 DAC 电压，对应红灯与绿灯亮起
4. 随后设置内部 DAC 电压为 3.5V，此时只有通道 0 电压大于 DAC 电压，对应红灯亮起
5. 最后设置内部 DAC 电压为 4.5V，通道电压均小于 DAC 电压，没有灯会亮起
6. 每一次修改 DAC 电压都会去判断比较结果是否与预期一致

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