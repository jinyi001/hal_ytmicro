# Etmr_Trigger_Adc_Demo

---

## 功能描述

本例程实现了输出三组互补对称的PWM波形，对齐方式为中心对齐。并在eTMR0_CH3的VAL0匹配时触发ADC采集。

1. 将逻辑分析仪或示波器接入J21-12(eTMR0_CH2)，J21-10(eTMR0_CH3)，J21-8(eTMR0_CH4), J21-6(eTMR0_CH5), J21-4(eTMR0_CH6), J21-2(eTMR0_CH7)，J17-2(TMU_OUT1)
2. 现象如下:
   - 逻辑分析仪或示波器中会显示三路互补输出的中心对齐PWM，占空比为50%，频率为20KHz。对应关系为J21-12端（正相）与J21-10端（反相）输出的PWM互补，J21-8端（正相）与J21-6端（反相）输出的PWM互补，J21-4端（正相）与J21-2端（反相）输出的PWM互补。
   - J17-2会观测到eTMR在中心点输出脉冲。
   - 8路ADC的采样值会存入adc0ChannelData[8]中。

---

## EVB硬件设置

以下表格是例程中使用到的外部引脚配置：

|  引脚        |    功能       |
|  ------     |    -----      |
|  PTC_2      |    eTMR0_CH2  |
|  PTC_3      |    eTMR0_CH3  |
|  PTB_4      |    eTMR0_CH4  |
|  PTB_5      |    eTMR0_CH5  |
|  PTE_8      |    eTMR0_CH6  |
|  PTE_9      |    eTMR0_CH7  |
|  PTD_0      |    TMU_OUT1   |
|  PTA_0      |    ADC0_SE0   |
|  PTA_1      |    ADC0_SE1   |
|  PTA_6      |    ADC0_SE2   |
|  PTA_7      |    ADC0_SE3   |
|  PTB_0      |    ADC0_SE4   |
|  PTB_1      |    ADC0_SE5   |
|  PTB_2      |    ADC0_SE6   |
|  PTB_3      |    ADC0_SE7   |

以下表格是EVB硬件连接设置，左端和右端短接：

|  左端        |    右端       |     描述        |
|  ------      |    -----     |     -----      |
|  J1-2        |    5VUSB     |     USB 5V供电  |
|  J2-2        |    5V        |     VDD 5V      |
|  J3-1        |    J3-2      |     单片机工作    |
|  J4-1        |    J4-2      |     VDDA 5V     |
|  J21-12      |     -        |     eTMR0_CH2   |
|  J21-10      |     -        |     eTMR0_CH3   |
|  J21-8       |     -        |     eTMR0_CH4   |
|  J21-6       |     -        |     eTMR0_CH5   |
|  J21-4       |     -        |     eTMR0_CH6   |
|  J21-2       |     -        |     eTMR0_CH7   |
|  J17-2       |     -        |     TMU_OUT1    |
|  J20-2       |     -        |     ADC0_SE0    |
|  J20-1       |     -        |     ADC0_SE1    |
|  J19-6       |     -        |     ADC0_SE2    |
|  J19-5       |     -        |     ADC0_SE3    |
|  J19-2       |     -        |     ADC0_SE4    |
|  J19-1       |     -        |     ADC0_SE5    |
|  J18-16      |     -        |     ADC0_SE6    |
|  J18-15      |     -        |     ADC0_SE7    |

---

## 硬件要求

+ **开发板**: YTM32B1MC0-EVB-Q64
+ **调试器**: J-LINK v9.2 或更高版本
  
---

## 软件要求

+ **SDK**     : YTM32B1MC0_1_3_1_HF001 或更高版本
+ **GCC**     : GNU GCC v10.3 或更高版本
+ **Keil/MDK**: ARM Keil/MDK v5.36 或更高版本
+ **IAR**     : IAR v8.40 或更高版本
+ **VS Code** : Microsoft Visual Studio Code IDE v1.76 或更高版本（GNU GCC 工具链，CMake v3.25，ninja，Cortex-Debug 扩展安装）
+ **J-LINK**  : SEGGER JLINK v6.96 或更高版本，需要安装 YTM32B1LE0 补丁（支持包）
+ **Ozone**   : SEGGER Ozone v3.28 或更高版本

---