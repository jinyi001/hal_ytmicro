# Flexcan_Canfd_Demo

---

## 功能描述

该示例演示了 FlexCAN CANFD 模式的使用，以及自测试。

- 短接板子上的 CAN1_H 与 CAN2_H，CAN1_L 与 CAN2_L 来完成自测试
- 波特率（标称）为 500Kbps，波特率（数据）为 2000Kbps
- CAN1 用邮箱-1 发送 ID 为 0x002 的 CAN-FD 报文，用邮箱-0 接收 ID 为 0x003 的 CAN-FD 报文
- CAN2 用邮箱-1 发送 ID 为 0x003 的 CAN-FD 报文，用邮箱-0 接收 ID 为 0x002 的 CAN-FD 报文
- CAN1 与 CAN2 均会打印当前收到的报文内容
- 当 CAN1 发送完 10 条报文后，结束自测试

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