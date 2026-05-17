# Lin_Master_Slave_Demo

---

## 功能描述

该示例演示了 lin 主机模式与从机模式的传输，并自测试。

- LIN1 为 LIN 主机，LIN2 为 LIN 从机
- 短接 LIN1 与 LIN2 的 LIN 线完成自测试
- 为 LIN1 与 LIN2 的 VBAT 供 12V 电压
- 主机发送 ID 为 0x23 帧头，发送完帧头后，从机开启接收主机的帧数据，随后主机接着发送剩下的报文内容
- 主机发送 ID 为 0x32 帧头，发送完帧头后，从机开启回复主机，随后主机接收从机发送的内容
- 通过 UART 打印主机与从机发送的，接收的数据。

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