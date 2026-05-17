# Rtc_Wakeup_Demo

---

## Function Description

This project demonstrates the use of the RTC to wake the system from Standby mode.

- For self-test requirements, the current routine uses SIRC as the RTC clock source. However, due to the lack of prescaler support, both the second interrupt and alarm interrupt are inaccurate. Users may feed an external 32.768kHz clock signal into the RTC pin for precise timing.
- Second interrupts occur every second; alarm interrupts occur every 5 seconds.
- During the wake-up cycle, the RTC interrupt wakes the chip. If the wake-up count reaches 10, the cycle stops.
- After the RTC wake-up cycle, if the second interrupt count reaches 6, the entire demo routine stops.

---

## Hardware Requirements

+ **EVB**: YTM32B1MD1-EVB-Q100
+ **Debugger**: J-LINK v9.2 or later version

---

## Software Requirements

+ **SDK**      : YTM32B1MD1_1_4_0 or later version
+ **GCC**      : GNU GCC v10.3 or later version
+ **Keil/MDK** : ARM Keil/MDK v5.36 or later version
+ **IAR***     : IAR v8.4 or later version
+ **VS Code**  : Microsoft Visual Studio Code IDE v1.76 or later version(GNU GCC toolchain, CMake v3.25, ninja, Cortex-Debug extensions installation required)
+ **J-LINK** : SEGGER JLINK v6.96 or later version, YTM32B1MD1 patch(support package) installation required.
+ **Ozone**  : SEGGER Ozone v3.28 or later version.
  
---