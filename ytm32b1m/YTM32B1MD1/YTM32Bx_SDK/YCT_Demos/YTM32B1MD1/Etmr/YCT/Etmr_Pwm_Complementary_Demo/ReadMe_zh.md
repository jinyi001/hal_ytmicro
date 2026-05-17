# Etmr_Pwm_Complementary_Demo

---

## 功能描述

该例程展示了eTMR的互补PWM输出功能，并通过输入捕获功能进行自测试。

- 可以短接 eTMR0 与 eTMR3 的四组通道，进行自测试
- eTMR0_CH0 和 eTMR0_CH1 将输出互补PWM信号，带有死区时间，占空比为 25% 与 75%。
- eTMR0_CH2 和 eTMR0_CH3 将输出互补PWM信号，带有死区时间，占空比为 62.5% 与 37.5%。
- 频率为10kHz。
- 死区时间的频率是快速总线时钟，偶数通道死区时间计数器周期为8，奇数通道死区时间计数器周期为10。
- eTMR3 通道0，1，2，3 分别去捕获 eTMR0 的四个通道的占空比与频率，并检查结果是否与配置一致

---

## EVB硬件设置

以下表格是例程中使用到的外部引脚配置：

|  引脚        |    功能       |
|  ------     |    -----      |
|  PTB_12      |    eTMR0_CH0  |
|  PTB_13      |    eTMR0_CH1  |
|  PTB_14      |    eTMR0_CH2  |
|  PTB_15      |    eTMR0_CH3  |
|  PTB_8       |    eTMR3_CH0  |
|  PTB_9       |    eTMR3_CH1  |
|  PTC_6       |    eTMR3_CH2  |
|  PTC_7       |    eTMR3_CH3  |

---

## 硬件要求

+ **开发板**: YTM32B1MD1-EVB-Q100
+ **调试器**: J-LINK v9.2 或更高版本

---

## 软件要求

+ **SDK**     : YTM32B1MD1_1_4_0 或更高版本
+ **GCC**     : GNU GCC v10.3 或更高版本
+ **Keil/MDK**: ARM Keil/MDK v5.36 或更高版本
+ **IAR**     : IAR v8.40 或更高版本
+ **VS Code** : Microsoft Visual Studio Code IDE v1.76 或更高版本（GNU GCC 工具链，CMake v3.25，ninja，Cortex-Debug 扩展安装）
+ **J-LINK**  : SEGGER JLINK v6.96 或更高版本，需要安装 YTM32B1MD1 补丁（支持包）
+ **Ozone**   : SEGGER Ozone v3.28 或更高版本

---