# Etmr_Trigger_Adc_Demo

---

## Function Description

This example shows that the three pairs of complementary pwm waveforms will be output, aligned in central alignment. Additionally, when counter matches val0 of channel 3, the ADC sampling will be triggered.

1. Connect the logic analyzer or oscilloscope to J21-12(eTMR0_CH2)，J21-10(eTMR0_CH3)，J21-8(eTMR0_CH4), J21-6(eTMR0_CH5), J21-4(eTMR0_CH6), J21-2(eTMR0_CH7)，J17-2(TMU_OUT1).
2. The phenomena are as follows.
   - Three pairs of complementary pwm will be displayed in the logic analyzer or oscilloscope, with a duty cycle of 50% and a frequency of 20KHz.
   - The corresponding relationship of pwm is
     - J21-12 terminal (normal phase) - J21-10 terminal (inverted phase);
     - J21-8 terminal (normal phase) - J21-6 terminal (inverted phase);
     - J21-4 terminal (normal phase) - J21-2 terminal (inverted phase).
   - J17-2 is used to observe eTMR trigger output at the center point.
   - The 8 ADC sampling values will be stored in adc0ChannelData[8].

---

## Hardware Setup on EVB

The following table shows the pin configuration of demo:

|  Pin        |    Function   |
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

The following table shows the hardware connection on EVB, short-L is connected to short-R:

|  Short-L     |    Short-R   |      Description     |
|  ------      |    -----     |      -----           |
|  J1-2        |    5VUSB     |      USB 5V supply   |
|  J2-2        |    5V        |      VDD 5V          |
|  J3-1        |    J3-2      |      MCU work        |
|  J4-1        |    J4-2      |      VDDA 5V         |
|  J21-12      |     -        |      eTMR0_CH2       |
|  J21-10      |     -        |      eTMR0_CH3       |
|  J21-8       |     -        |      eTMR0_CH4       |
|  J21-6       |     -        |      eTMR0_CH5       |
|  J21-4       |     -        |      eTMR0_CH6       |
|  J21-2       |     -        |      eTMR0_CH7       |
|  J17-2       |     -        |      TMU_OUT1        |
|  J20-2       |     -        |      ADC0_SE0        |
|  J20-1       |     -        |      ADC0_SE1        |
|  J19-6       |     -        |      ADC0_SE2        |
|  J19-5       |     -        |      ADC0_SE3        |
|  J19-2       |     -        |      ADC0_SE4        |
|  J19-1       |     -        |      ADC0_SE5        |
|  J18-16      |     -        |      ADC0_SE6        |

---

## Hardware Requirements

- **EVB**: YTM32B1MC0-EVB-Q64

---

## Hardware Requirements

+ **EVB**: YTM32B1MC0-EVB-Q64
+ **Debugger**: J-LINK v9.2 or later version

---

## Software Requirements

+ **SDK**      : YTM32B1MC0_1_3_1_HF001 or later version
+ **GCC**      : GNU GCC v10.3 or later version
+ **Keil/MDK** : ARM Keil/MDK v5.36 or later version
+ **IAR***     : IAR v8.4 or later version
+ **VS Code**  : Microsoft Visual Studio Code IDE v1.76 or later version(GNU GCC toolchain, CMake v3.25, ninja, Cortex-Debug extensions installation required)
+ **J-LINK** : SEGGER JLINK v6.96 or later version, YTM32B1LE0 patch(support package) installation required.
+ **Ozone**  : SEGGER Ozone v3.28 or later version.

---