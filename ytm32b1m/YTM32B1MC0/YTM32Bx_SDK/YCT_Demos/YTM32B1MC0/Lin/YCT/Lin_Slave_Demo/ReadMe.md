# Lin Slave Example Code

## Project Name: Lin_Slave_Demo

---

## Function description

This project is designed to demonstrate the Lin module config and usage:

**Note:** 
  1. Connect J10 jumper caps.
  2. J11-1 connectes to 12V and J12-3 connectes to GND.
  3. J11-2 connectes to lin bus.
  4. The baud rate of Lin is **19200**.

- **Lin_Slave_Demo**: 
  - The slave waits for the master to send the frame header with id 1 and accepts data from the master.
  - The slave waits for the master to send the frame header with id 2 and sends the response.
  - The slave waits for the master to send the frame header with ID 3, accepts the data, and enter sleep mode.
---

## HW requirement
+ **EVB**: YTM32B1MC0-EVB-Q64
+ **Debugger**: J-LINK v9.2 or later with YUNTU YTM32B1MC0 patch(support package) installed.

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

1. Config alt value of PTA_2 Pin with UART0_RX as RX pin and PTA_3 with UART0_TX as TX pin.
2. Enable and config UART0_CLK.
3. Enable and config LIN module as LIN slave.
  
---