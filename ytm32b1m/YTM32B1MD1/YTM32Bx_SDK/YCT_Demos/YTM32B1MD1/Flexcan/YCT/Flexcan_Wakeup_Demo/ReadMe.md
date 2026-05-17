# Flexcan_Wakeup_Demo

---

## Function description

This project is flexcan wake up chip demo:
- **Flexcan_Wakeup_Demo**: 
  - Standby mode.
  - Powerdown mode.
  - Active mode.
1. Config alt value of PTE_12 Pin with LINFlexD2_TX as TX pin and PTD_17 with LINFlexD2_RX as RX pin.
2. Enable and config LINFlexD2_CLK.
3. Enable and config Linflexd Uart and Utility Print module.
4. Config alt value of PTB_4 Pin with LED5, PTB_5 Pin with LED6, PTE_8 Pin with LED7, PTD_12 Pin with CAN0_STB.
5. Enable and config GPIO_CLK.
6. Config alt value of PTE_5 Pin with CAN0_TX, PTE_4 Pin with CAN0_RX.
8. Enable and config FlexCAN0_CLK.
9. Enable and config CAN module.
10. Enable and config WKU_CLK.
11. Enable and config WKUP module.
12. Enable and config POWER module.
13. Use **TX_MAILBOX** defined mailbox number(mailbox-1) to send a standard frame message with id = **TX_MSG_ID**(0x003) every 1s with polling, once send the message success the LED5 will be toggled.
14. Use **RX_STD_MAILBOX** defined mailbox number(mailbox-0) to receive a message id = **RX_STD_MSG_ID**(0x002) with can0 event interupt(FLEXCAN_EVENT_RX_COMPLETE), once received the message the LED6 will be toggled.
15. Use **RX_EXT_MAILBOX** defined mailbox number(mailbox-2) to receive a message id = **RX_EXTMSG_ID**(0x70012) with can0 event interupt(FLEXCAN_EVENT_RX_COMPLETE), once received the message the LED7 will be toggled.
16. User can choose the standby or powerdown mode by standard can message received. message data[0] == 0x01 will enter standby mode, data[0] == 0x02 will enter powerdown mode, others data don't care;
17. When selecting Powerdown mode, CAN0_RX(pte4) will be changed to GPIO input mode as wakeup source.
18. Any frame can wake up the chip to active mode.
  - The code continues to run after entering the standby statement (wake up from Standby mode).
  - System will reset(wake up from Powerdown mode) .
  
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