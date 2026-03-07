# Etmr_Pwm_Complementary_Demo Example Code

## Project Name: Etmr_Pwm_Complementary_Demo

---

## Function description

This project is designed to demonstrate the eTMR module complementary pwm configuration and usage:
- **Etmr_Pwm_Complementary_Demo**: 
  - This demo chooses eTMR0_CH2 and eTMR0_CH3 as pwm signal.
  - The frequency is 10KHz.
  - The frequency of dead time is the half of fast bus clock, even channel dead time counter ticks is 8, odd channel dead time counter ticks is 10.
  - The duty cycle will be changed each cycle into overflow interrupt. The duty cycle is 0 -> 12.5% -> 25% -> 37.5% -> 50% -> 62.5% -> 75% -> 87.5% -> 0, repeatedly.

---

## HW requirement
+ **EVB**: YTM32B1MC0-EVB-Q64
+ **Debugger**: J-LINK v9.2 or later with YUNTU YTM32B1MC0 patch(support package) installed.

---

## SW requirement

+ **Config & Code Generation Tool**
  + **SDK**:       Yuntu Config Tool(YCT) v2.6.3 or later version
+ **Compiler Toolchain**
  + **GCC***     : GNU GCC v10.3 or later version
  + **Keil/MDK***: ARM Keil/MDK v5.36 or later version
  + **IAR***     : IAR v8.4 or later version
  + **VS Code**  : Microsoft Visual Studio Code IDE v1.76 or later version(C/C++ support, GNU toolchain, CMake v3.25, ninja, Cortex-Debug extensions installation required)
+ **Debugger driver**
    + **J-LINK***              : SEGGER JLINK v6.96 or later version, YUNTU YTM32B1MC0 patch(support package) installation required.
    + **Ozone Debugger tool**  : SEGGER Ozone v3.28 or later version.

    ***Note***: * **YTM32B1MC0** patch install required which can be obtained via **YCT** or contact ***YUNTU sales/FAE team***.

---

## Setup

+ ***Step1***: Open the project.yct with the YCT tool, config the toolchain per your required(by default, the VS code + GCC are used)
+ ***Step2***: Generate the codes, you can change the configurations if needed(to adapt to your own HW design), if so, please ensure all the modifications are OK with no any errors exist before generating the code.
+ ***Step3***: Open the generated project with VS code/IAR/Keil IDE per your selected.
+ ***Step4***: Compile the project, power on the EVB and download it to the EVB via the available debugger, such as a J-LINK debugger probe.
+ ***Step5***: At last, enjoy the demo example.
  
---

1. Connect oscilloscope or logic analyzer to PWM0 and PWM1 which are on EVB.
2. These two channels will output complementary pwm signal with deadtime.
  
---