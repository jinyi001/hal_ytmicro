;  ---------------------------------------------------------------------------------------
;   @file:    YTM32B1MC0_startup_iar.S
;   @purpose: IAR Startup File
;             YTM32B1MC0
;   @version: 1.3
;   @date:    2024-06-25
;  ---------------------------------------------------------------------------------------
    MODULE  ?cstartup

    ;; Forward declaration of sections.
    SECTION CSTACK:DATA:NOROOT(3)

    SECTION .intvec:CODE:ROOT(2)

    EXTERN  main
    EXTERN  SystemInit
    EXTERN  init_data_bss
    PUBLIC  __vector_table
    PUBLIC  __Vectors
    PUBLIC  __Vectors_End
    PUBLIC  __Vectors_Size

    DATA

__vector_table
    DCD   sfe(CSTACK)
    DCD   Reset_Handler                      ; Reset Handler
    DCD   NMI_Handler                        ; Non Maskable Interrupt
    DCD   HardFault_Handler                  ; Cortex-M33 Hard Fault Interrupt
    DCD   0
    DCD   0
    DCD   0
    DCD   0
    DCD   0
    DCD   0
    DCD   0
    DCD   SVC_Handler                        ; Cortex-M33 SV Call Interrupt
    DCD   0
    DCD   0
    DCD   PendSV_Handler                     ; Cortex-M33 Pend SV Interrupt
    DCD   SysTick_Handler                    ; Cortex-M33 System Tick Interrupt

    DCD   DMA0_IRQHandler                                   ;  DMA0 Handler
    DCD   DMA1_IRQHandler                                   ;  DMA1 Handler
    DCD   DMA2_IRQHandler                                   ;  DMA2 Handler
    DCD   DMA3_IRQHandler                                   ;  DMA3 Handler
    DCD   Reserved5_IRQHandler                              ;  Reserved5 Handler
    DCD   Reserved6_IRQHandler                              ;  Reserved6 Handler
    DCD   Reserved7_IRQHandler                              ;  Reserved7 Handler
    DCD   Reserved8_IRQHandler                              ;  Reserved8 Handler
    DCD   Reserved9_IRQHandler                              ;  Reserved9 Handler
    DCD   Reserved10_IRQHandler                             ;  Reserved10 Handler
    DCD   Reserved11_IRQHandler                             ;  Reserved11 Handler
    DCD   Reserved12_IRQHandler                             ;  Reserved12 Handler
    DCD   Reserved13_IRQHandler                             ;  Reserved13 Handler
    DCD   Reserved14_IRQHandler                             ;  Reserved14 Handler
    DCD   Reserved15_IRQHandler                             ;  Reserved15 Handler
    DCD   Reserved16_IRQHandler                             ;  Reserved16 Handler
    DCD   DMA_Error_IRQHandler                              ;  DMA_Error Handler
    DCD   Reserved17_IRQHandler                             ;  Reserved17 Handler
    DCD   EFM_IRQHandler                                    ;  EFM Handler
    DCD   EFM_Error_IRQHandler                              ;  EFM_Error Handler
    DCD   PCU_IRQHandler                                    ;  PCU Handler
    DCD   EFM_Ecc_IRQHandler                                ;  EFM_Ecc Handler
    DCD   WDG0_IRQHandler                                   ;  WDG0 Handler
    DCD   Reserved18_IRQHandler                             ;  Reserved18 Handler
    DCD   I2C0_Master_IRQHandler                             ;  I2C0_Master Handler
    DCD   I2C0_Slave_IRQHandler                             ;  I2C0_Slave Handler
    DCD   SPI0_IRQHandler                                   ;  SPI0 Handler
    DCD   SPI1_IRQHandler                                   ;  SPI1 Handler
    DCD   SPI2_IRQHandler                                   ;  SPI2 Handler
    DCD   I2C1_Master_IRQHandler                             ;  I2C1_Master Handler
    DCD   Reserved19_IRQHandler                             ;  Reserved19 Handler
    DCD   UART0_IRQHandler                                  ;  UART0 Handler
    DCD   Reserved20_IRQHandler                             ;  Reserved20 Handler
    DCD   UART1_IRQHandler                                  ;  UART1 Handler
    DCD   Reserved21_IRQHandler                             ;  Reserved21 Handler
    DCD   UART2_IRQHandler                                  ;  UART2 Handler
    DCD   Reserved22_IRQHandler                             ;  Reserved22 Handler
    DCD   Reserved23_IRQHandler                             ;  Reserved23 Handler
    DCD   Reserved24_IRQHandler                             ;  Reserved24 Handler
    DCD   ADC0_IRQHandler                                   ;  ADC0 Handler
    DCD   Reserved25_IRQHandler                             ;  Reserved25 Handler
    DCD   ACMP0_IRQHandler                                  ;  ACMP0 Handler
    DCD   Reserved26_IRQHandler                             ;  Reserved26 Handler
    DCD   Reserved27_IRQHandler                             ;  Reserved27 Handler
    DCD   EMU0_SB_IRQHandler                                ;  EMU0_SB Handler
    DCD   EMU0_DB_IRQHandler                                ;  EMU0_DB Handler
    DCD   Reserved28_IRQHandler                             ;  Reserved28 Handler
    DCD   Reserved29_IRQHandler                             ;  Reserved29 Handler
    DCD   pTMR0_Ch0_IRQHandler                              ;  pTMR0_Ch0 Handler
    DCD   pTMR0_Ch1_IRQHandler                              ;  pTMR0_Ch1 Handler
    DCD   pTMR0_Ch2_IRQHandler                              ;  pTMR0_Ch2 Handler
    DCD   pTMR0_Ch3_IRQHandler                              ;  pTMR0_Ch3 Handler
    DCD   Reserved30_IRQHandler                             ;  Reserved30 Handler
    DCD   Reserved31_IRQHandler                             ;  Reserved31 Handler
    DCD   Reserved32_IRQHandler                             ;  Reserved32 Handler
    DCD   Reserved33_IRQHandler                             ;  Reserved33 Handler
    DCD   Reserved34_IRQHandler                             ;  Reserved34 Handler
    DCD   SCU_IRQHandler                                    ;  SCU Handler
    DCD   lpTMR0_IRQHandler                                 ;  lpTMR0 Handler
    DCD   GPIOA_IRQHandler                                  ;  GPIOA Handler
    DCD   GPIOB_IRQHandler                                  ;  GPIOB Handler
    DCD   GPIOC_IRQHandler                                  ;  GPIOC Handler
    DCD   GPIOD_IRQHandler                                  ;  GPIOD Handler
    DCD   GPIOE_IRQHandler                                  ;  GPIOE Handler
    DCD   Reserved35_IRQHandler                             ;  Reserved35 Handler
    DCD   Reserved36_IRQHandler                             ;  Reserved36 Handler
    DCD   Reserved37_IRQHandler                             ;  Reserved37 Handler
    DCD   Reserved38_IRQHandler                             ;  Reserved38 Handler
    DCD   Reserved39_IRQHandler                             ;  Reserved39 Handler
    DCD   Reserved40_IRQHandler                             ;  Reserved40 Handler
    DCD   Reserved41_IRQHandler                             ;  Reserved41 Handler
    DCD   Reserved42_IRQHandler                             ;  Reserved42 Handler
    DCD   Reserved43_IRQHandler                             ;  Reserved43 Handler
    DCD   Reserved44_IRQHandler                             ;  Reserved44 Handler
    DCD   Reserved45_IRQHandler                             ;  Reserved45 Handler
    DCD   Reserved46_IRQHandler                             ;  Reserved46 Handler
    DCD   Reserved47_IRQHandler                             ;  Reserved47 Handler
    DCD   Reserved48_IRQHandler                             ;  Reserved48 Handler
    DCD   CAN0_ORed_IRQHandler                              ;  CAN0_ORed Handler
    DCD   CAN0_Error_IRQHandler                             ;  CAN0_Error Handler
    DCD   CAN0_Wake_Up_IRQHandler                             ;  CAN0_Wake_Up Handler
    DCD   CAN0_ORed_0_15_MB_IRQHandler                             ;  CAN0_ORed_0_15_MB Handler
    DCD   CAN0_ORed_16_31_MB_IRQHandler                             ;  CAN0_ORed_16_31_MB Handler
    DCD   CAN0_ORed_32_47_MB_IRQHandler                             ;  CAN0_ORed_32_47_MB Handler
    DCD   CAN0_ORed_48_63_MB_IRQHandler                             ;  CAN0_ORed_48_63_MB Handler
    DCD   CAN1_ORed_IRQHandler                              ;  CAN1_ORed Handler
    DCD   CAN1_Error_IRQHandler                             ;  CAN1_Error Handler
    DCD   CAN1_Wake_Up_IRQHandler                             ;  CAN1_Wake_Up Handler
    DCD   CAN1_ORed_0_15_MB_IRQHandler                             ;  CAN1_ORed_0_15_MB Handler
    DCD   CAN1_ORed_16_31_MB_IRQHandler                             ;  CAN1_ORed_16_31_MB Handler
    DCD   CAN1_ORed_32_47_MB_IRQHandler                             ;  CAN1_ORed_32_47_MB Handler
    DCD   CAN1_ORed_48_63_MB_IRQHandler                             ;  CAN1_ORed_48_63_MB Handler
    DCD   Reserved49_IRQHandler                             ;  Reserved49 Handler
    DCD   Reserved50_IRQHandler                             ;  Reserved50 Handler
    DCD   Reserved51_IRQHandler                             ;  Reserved51 Handler
    DCD   Reserved52_IRQHandler                             ;  Reserved52 Handler
    DCD   Reserved53_IRQHandler                             ;  Reserved53 Handler
    DCD   Reserved54_IRQHandler                             ;  Reserved54 Handler
    DCD   Reserved55_IRQHandler                             ;  Reserved55 Handler
    DCD   eTMR0_Ch0_Ch1_IRQHandler                             ;  eTMR0_Ch0_Ch1 Handler
    DCD   eTMR0_Ch2_Ch3_IRQHandler                             ;  eTMR0_Ch2_Ch3 Handler
    DCD   eTMR0_Ch4_Ch5_IRQHandler                             ;  eTMR0_Ch4_Ch5 Handler
    DCD   eTMR0_Ch6_Ch7_IRQHandler                             ;  eTMR0_Ch6_Ch7 Handler
    DCD   eTMR0_Fault_IRQHandler                             ;  eTMR0_Fault Handler
    DCD   eTMR0_Ovf_IRQHandler                              ;  eTMR0_Ovf Handler
    DCD   eTMR1_Ch0_Ch1_IRQHandler                             ;  eTMR1_Ch0_Ch1 Handler
    DCD   eTMR1_Ch2_Ch3_IRQHandler                             ;  eTMR1_Ch2_Ch3 Handler
    DCD   eTMR1_Ch4_Ch5_IRQHandler                             ;  eTMR1_Ch4_Ch5 Handler
    DCD   eTMR1_Ch6_Ch7_IRQHandler                             ;  eTMR1_Ch6_Ch7 Handler
    DCD   eTMR1_Fault_IRQHandler                             ;  eTMR1_Fault Handler
    DCD   eTMR1_Ovf_IRQHandler                              ;  eTMR1_Ovf Handler
    DCD   Reserved56_IRQHandler                             ;  Reserved56 Handler
    DCD   Reserved57_IRQHandler                             ;  Reserved57 Handler
    DCD   Reserved58_IRQHandler                             ;  Reserved58 Handler
    DCD   Reserved59_IRQHandler                             ;  Reserved59 Handler
    DCD   Reserved60_IRQHandler                             ;  Reserved60 Handler
    DCD   Reserved61_IRQHandler                             ;  Reserved61 Handler
    DCD   Reserved62_IRQHandler                             ;  Reserved62 Handler
    DCD   Reserved63_IRQHandler                             ;  Reserved63 Handler
    DCD   Reserved64_IRQHandler                             ;  Reserved64 Handler
    DCD   Reserved65_IRQHandler                             ;  Reserved65 Handler
    DCD   Reserved66_IRQHandler                             ;  Reserved66 Handler
    DCD   Reserved67_IRQHandler                             ;  Reserved67 Handler
    DCD   Reserved68_IRQHandler                             ;  Reserved68 Handler
    DCD   Reserved69_IRQHandler                             ;  Reserved69 Handler
    DCD   Reserved70_IRQHandler                             ;  Reserved70 Handler
    DCD   Reserved71_IRQHandler                             ;  Reserved71 Handler
    DCD   Reserved72_IRQHandler                             ;  Reserved72 Handler
    DCD   Reserved73_IRQHandler                             ;  Reserved73 Handler
    DCD   Reserved74_IRQHandler                             ;  Reserved74 Handler
    DCD   Reserved75_IRQHandler                             ;  Reserved75 Handler
    DCD   Reserved76_IRQHandler                             ;  Reserved76 Handler
    DCD   Reserved77_IRQHandler                             ;  Reserved77 Handler
    DCD   Reserved78_IRQHandler                             ;  Reserved78 Handler
    DCD   Reserved79_IRQHandler                             ;  Reserved79 Handler
    DCD   MPWM0_Ch0_Ovf_IRQHandler                             ;  MPWM0_Ch0_Ovf Handler
    DCD   MPWM0_Ch1_Ovf_IRQHandler                             ;  MPWM0_Ch1_Ovf Handler
    DCD   MPWM0_Ch2_Ovf_IRQHandler                             ;  MPWM0_Ch2_Ovf Handler
    DCD   MPWM0_Ch3_Ovf_IRQHandler                             ;  MPWM0_Ch3_Ovf Handler
    DCD   MPWM0_Ch4_Ovf_IRQHandler                             ;  MPWM0_Ch4_Ovf Handler
    DCD   MPWM0_Ch5_Ovf_IRQHandler                             ;  MPWM0_Ch5_Ovf Handler
    DCD   MPWM0_Ch6_Ovf_IRQHandler                             ;  MPWM0_Ch6_Ovf Handler
    DCD   MPWM0_Ch7_Ovf_IRQHandler                             ;  MPWM0_Ch7_Ovf Handler
    DCD   MPWM0_Ch0_IRQHandler                              ;  MPWM0_Ch0 Handler
    DCD   MPWM0_Ch1_IRQHandler                              ;  MPWM0_Ch1 Handler
    DCD   MPWM0_Ch2_IRQHandler                              ;  MPWM0_Ch2 Handler
    DCD   MPWM0_Ch3_IRQHandler                              ;  MPWM0_Ch3 Handler
    DCD   MPWM0_Ch4_IRQHandler                              ;  MPWM0_Ch4 Handler
    DCD   MPWM0_Ch5_IRQHandler                              ;  MPWM0_Ch5 Handler
    DCD   MPWM0_Ch6_IRQHandler                              ;  MPWM0_Ch6 Handler
    DCD   MPWM0_Ch7_IRQHandler                              ;  MPWM0_Ch7 Handler
    DCD   Reserved80_IRQHandler                             ;  Reserved80 Handler
    DCD   Reserved81_IRQHandler                             ;  Reserved81 Handler
    DCD   Reserved82_IRQHandler                             ;  Reserved82 Handler
    DCD   Reserved83_IRQHandler                             ;  Reserved83 Handler
    DCD   Reserved84_IRQHandler                             ;  Reserved84 Handler
    DCD   TRNG_IRQHandler                                   ;  TRNG Handler
    DCD   HCU_IRQHandler                                    ;  HCU Handler

__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size  EQU   __Vectors_End - __Vectors


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
    THUMB

    PUBWEAK Reset_Handler
    SECTION .text:CODE:REORDER:NOROOT(2)
Reset_Handler
    CPSID   I               ; Mask interrupts
    ;; Init the rest of the registers
    LDR     R1,=0
    LDR     R2,=0
    LDR     R3,=0
    LDR     R4,=0
    LDR     R5,=0
    LDR     R6,=0
    LDR     R7,=0
    MOV     R8,R7
    MOV     R9,R7
    MOV     R10,R7
    MOV     R11,R7
    MOV     R12,R7

#ifndef START_NO_ECC_INIT
    IMPORT __RAM_START, __RAM_END
    ;; INIT ECC RAM

    LDR R1, =__RAM_START
    LDR R2, =__RAM_END

    SUBS    R2, R2, R1
    SUBS    R2, #1
    BLE     SRAM_LOOP_END

    MOVS    R0, #0
    MOVS    R3, #4
            
SRAM_LOOP             
    STR     R0, [R1]
    ADD     R1, R1, R3
    SUBS    R2, #4
    BGE     SRAM_LOOP

SRAM_LOOP_END
#endif
    ;; Initialize the stack pointer
    LDR     R0, =sfe(CSTACK)
    MOV     R13,R0

#ifndef __NO_SYSTEM_INIT
    ;; Call the CMSIS system init routine
    LDR     R0, =SystemInit
    BLX     R0
#endif

    ;; Init .data and .bss sections
    LDR     R0, =init_data_bss
    BLX     R0
    CPSIE   I               ; Unmask interrupts

    BL      main
JumpToSelf
    B   JumpToSelf

    PUBWEAK NMI_Handler
    SECTION .text:CODE:REORDER:NOROOT(1)
NMI_Handler
    B .

    PUBWEAK HardFault_Handler
    SECTION .text:CODE:REORDER:NOROOT(1)
HardFault_Handler
    B .


    PUBWEAK SVC_Handler
    SECTION .text:CODE:REORDER:NOROOT(1)
SVC_Handler
    B .

    PUBWEAK PendSV_Handler
    SECTION .text:CODE:REORDER:NOROOT(1)
PendSV_Handler
    B .

    PUBWEAK SysTick_Handler
    SECTION .text:CODE:REORDER:NOROOT(1)
SysTick_Handler
    B .

    PUBWEAK   DMA0_IRQHandler
    PUBWEAK   DMA1_IRQHandler
    PUBWEAK   DMA2_IRQHandler
    PUBWEAK   DMA3_IRQHandler
    PUBWEAK   Reserved5_IRQHandler
    PUBWEAK   Reserved6_IRQHandler
    PUBWEAK   Reserved7_IRQHandler
    PUBWEAK   Reserved8_IRQHandler
    PUBWEAK   Reserved9_IRQHandler
    PUBWEAK   Reserved10_IRQHandler
    PUBWEAK   Reserved11_IRQHandler
    PUBWEAK   Reserved12_IRQHandler
    PUBWEAK   Reserved13_IRQHandler
    PUBWEAK   Reserved14_IRQHandler
    PUBWEAK   Reserved15_IRQHandler
    PUBWEAK   Reserved16_IRQHandler
    PUBWEAK   DMA_Error_IRQHandler
    PUBWEAK   Reserved17_IRQHandler
    PUBWEAK   EFM_IRQHandler
    PUBWEAK   EFM_Error_IRQHandler
    PUBWEAK   PCU_IRQHandler
    PUBWEAK   EFM_Ecc_IRQHandler
    PUBWEAK   WDG0_IRQHandler
    PUBWEAK   Reserved18_IRQHandler
    PUBWEAK   I2C0_Master_IRQHandler
    PUBWEAK   I2C0_Slave_IRQHandler
    PUBWEAK   SPI0_IRQHandler
    PUBWEAK   SPI1_IRQHandler
    PUBWEAK   SPI2_IRQHandler
    PUBWEAK   I2C1_Master_IRQHandler
    PUBWEAK   Reserved19_IRQHandler
    PUBWEAK   UART0_IRQHandler
    PUBWEAK   Reserved20_IRQHandler
    PUBWEAK   UART1_IRQHandler
    PUBWEAK   Reserved21_IRQHandler
    PUBWEAK   UART2_IRQHandler
    PUBWEAK   Reserved22_IRQHandler
    PUBWEAK   Reserved23_IRQHandler
    PUBWEAK   Reserved24_IRQHandler
    PUBWEAK   ADC0_IRQHandler
    PUBWEAK   Reserved25_IRQHandler
    PUBWEAK   ACMP0_IRQHandler
    PUBWEAK   Reserved26_IRQHandler
    PUBWEAK   Reserved27_IRQHandler
    PUBWEAK   EMU0_SB_IRQHandler
    PUBWEAK   EMU0_DB_IRQHandler
    PUBWEAK   Reserved28_IRQHandler
    PUBWEAK   Reserved29_IRQHandler
    PUBWEAK   pTMR0_Ch0_IRQHandler
    PUBWEAK   pTMR0_Ch1_IRQHandler
    PUBWEAK   pTMR0_Ch2_IRQHandler
    PUBWEAK   pTMR0_Ch3_IRQHandler
    PUBWEAK   Reserved30_IRQHandler
    PUBWEAK   Reserved31_IRQHandler
    PUBWEAK   Reserved32_IRQHandler
    PUBWEAK   Reserved33_IRQHandler
    PUBWEAK   Reserved34_IRQHandler
    PUBWEAK   SCU_IRQHandler
    PUBWEAK   lpTMR0_IRQHandler
    PUBWEAK   GPIOA_IRQHandler
    PUBWEAK   GPIOB_IRQHandler
    PUBWEAK   GPIOC_IRQHandler
    PUBWEAK   GPIOD_IRQHandler
    PUBWEAK   GPIOE_IRQHandler
    PUBWEAK   Reserved35_IRQHandler
    PUBWEAK   Reserved36_IRQHandler
    PUBWEAK   Reserved37_IRQHandler
    PUBWEAK   Reserved38_IRQHandler
    PUBWEAK   Reserved39_IRQHandler
    PUBWEAK   Reserved40_IRQHandler
    PUBWEAK   Reserved41_IRQHandler
    PUBWEAK   Reserved42_IRQHandler
    PUBWEAK   Reserved43_IRQHandler
    PUBWEAK   Reserved44_IRQHandler
    PUBWEAK   Reserved45_IRQHandler
    PUBWEAK   Reserved46_IRQHandler
    PUBWEAK   Reserved47_IRQHandler
    PUBWEAK   Reserved48_IRQHandler
    PUBWEAK   CAN0_ORed_IRQHandler
    PUBWEAK   CAN0_Error_IRQHandler
    PUBWEAK   CAN0_Wake_Up_IRQHandler
    PUBWEAK   CAN0_ORed_0_15_MB_IRQHandler
    PUBWEAK   CAN0_ORed_16_31_MB_IRQHandler
    PUBWEAK   CAN0_ORed_32_47_MB_IRQHandler
    PUBWEAK   CAN0_ORed_48_63_MB_IRQHandler
    PUBWEAK   CAN1_ORed_IRQHandler
    PUBWEAK   CAN1_Error_IRQHandler
    PUBWEAK   CAN1_Wake_Up_IRQHandler
    PUBWEAK   CAN1_ORed_0_15_MB_IRQHandler
    PUBWEAK   CAN1_ORed_16_31_MB_IRQHandler
    PUBWEAK   CAN1_ORed_32_47_MB_IRQHandler
    PUBWEAK   CAN1_ORed_48_63_MB_IRQHandler
    PUBWEAK   Reserved49_IRQHandler
    PUBWEAK   Reserved50_IRQHandler
    PUBWEAK   Reserved51_IRQHandler
    PUBWEAK   Reserved52_IRQHandler
    PUBWEAK   Reserved53_IRQHandler
    PUBWEAK   Reserved54_IRQHandler
    PUBWEAK   Reserved55_IRQHandler
    PUBWEAK   eTMR0_Ch0_Ch1_IRQHandler
    PUBWEAK   eTMR0_Ch2_Ch3_IRQHandler
    PUBWEAK   eTMR0_Ch4_Ch5_IRQHandler
    PUBWEAK   eTMR0_Ch6_Ch7_IRQHandler
    PUBWEAK   eTMR0_Fault_IRQHandler
    PUBWEAK   eTMR0_Ovf_IRQHandler
    PUBWEAK   eTMR1_Ch0_Ch1_IRQHandler
    PUBWEAK   eTMR1_Ch2_Ch3_IRQHandler
    PUBWEAK   eTMR1_Ch4_Ch5_IRQHandler
    PUBWEAK   eTMR1_Ch6_Ch7_IRQHandler
    PUBWEAK   eTMR1_Fault_IRQHandler
    PUBWEAK   eTMR1_Ovf_IRQHandler
    PUBWEAK   Reserved56_IRQHandler
    PUBWEAK   Reserved57_IRQHandler
    PUBWEAK   Reserved58_IRQHandler
    PUBWEAK   Reserved59_IRQHandler
    PUBWEAK   Reserved60_IRQHandler
    PUBWEAK   Reserved61_IRQHandler
    PUBWEAK   Reserved62_IRQHandler
    PUBWEAK   Reserved63_IRQHandler
    PUBWEAK   Reserved64_IRQHandler
    PUBWEAK   Reserved65_IRQHandler
    PUBWEAK   Reserved66_IRQHandler
    PUBWEAK   Reserved67_IRQHandler
    PUBWEAK   Reserved68_IRQHandler
    PUBWEAK   Reserved69_IRQHandler
    PUBWEAK   Reserved70_IRQHandler
    PUBWEAK   Reserved71_IRQHandler
    PUBWEAK   Reserved72_IRQHandler
    PUBWEAK   Reserved73_IRQHandler
    PUBWEAK   Reserved74_IRQHandler
    PUBWEAK   Reserved75_IRQHandler
    PUBWEAK   Reserved76_IRQHandler
    PUBWEAK   Reserved77_IRQHandler
    PUBWEAK   Reserved78_IRQHandler
    PUBWEAK   Reserved79_IRQHandler
    PUBWEAK   MPWM0_Ch0_Ovf_IRQHandler
    PUBWEAK   MPWM0_Ch1_Ovf_IRQHandler
    PUBWEAK   MPWM0_Ch2_Ovf_IRQHandler
    PUBWEAK   MPWM0_Ch3_Ovf_IRQHandler
    PUBWEAK   MPWM0_Ch4_Ovf_IRQHandler
    PUBWEAK   MPWM0_Ch5_Ovf_IRQHandler
    PUBWEAK   MPWM0_Ch6_Ovf_IRQHandler
    PUBWEAK   MPWM0_Ch7_Ovf_IRQHandler
    PUBWEAK   MPWM0_Ch0_IRQHandler
    PUBWEAK   MPWM0_Ch1_IRQHandler
    PUBWEAK   MPWM0_Ch2_IRQHandler
    PUBWEAK   MPWM0_Ch3_IRQHandler
    PUBWEAK   MPWM0_Ch4_IRQHandler
    PUBWEAK   MPWM0_Ch5_IRQHandler
    PUBWEAK   MPWM0_Ch6_IRQHandler
    PUBWEAK   MPWM0_Ch7_IRQHandler
    PUBWEAK   Reserved80_IRQHandler
    PUBWEAK   Reserved81_IRQHandler
    PUBWEAK   Reserved82_IRQHandler
    PUBWEAK   Reserved83_IRQHandler
    PUBWEAK   Reserved84_IRQHandler
    PUBWEAK   TRNG_IRQHandler
    PUBWEAK   HCU_IRQHandler
    PUBWEAK DefaultISR
    SECTION .text:CODE:REORDER:NOROOT(1)
NMI_Handler
HardFault_Handler
SVC_Handler
PendSV_Handler
SysTick_Handler
DMA0_IRQHandler
DMA1_IRQHandler
DMA2_IRQHandler
DMA3_IRQHandler
Reserved5_IRQHandler
Reserved6_IRQHandler
Reserved7_IRQHandler
Reserved8_IRQHandler
Reserved9_IRQHandler
Reserved10_IRQHandler
Reserved11_IRQHandler
Reserved12_IRQHandler
Reserved13_IRQHandler
Reserved14_IRQHandler
Reserved15_IRQHandler
Reserved16_IRQHandler
DMA_Error_IRQHandler
Reserved17_IRQHandler
EFM_IRQHandler
EFM_Error_IRQHandler
PCU_IRQHandler
EFM_Ecc_IRQHandler
WDG0_IRQHandler
Reserved18_IRQHandler
I2C0_Master_IRQHandler
I2C0_Slave_IRQHandler
SPI0_IRQHandler
SPI1_IRQHandler
SPI2_IRQHandler
I2C1_Master_IRQHandler
Reserved19_IRQHandler
UART0_IRQHandler
Reserved20_IRQHandler
UART1_IRQHandler
Reserved21_IRQHandler
UART2_IRQHandler
Reserved22_IRQHandler
Reserved23_IRQHandler
Reserved24_IRQHandler
ADC0_IRQHandler
Reserved25_IRQHandler
ACMP0_IRQHandler
Reserved26_IRQHandler
Reserved27_IRQHandler
EMU0_SB_IRQHandler
EMU0_DB_IRQHandler
Reserved28_IRQHandler
Reserved29_IRQHandler
pTMR0_Ch0_IRQHandler
pTMR0_Ch1_IRQHandler
pTMR0_Ch2_IRQHandler
pTMR0_Ch3_IRQHandler
Reserved30_IRQHandler
Reserved31_IRQHandler
Reserved32_IRQHandler
Reserved33_IRQHandler
Reserved34_IRQHandler
SCU_IRQHandler
lpTMR0_IRQHandler
GPIOA_IRQHandler
GPIOB_IRQHandler
GPIOC_IRQHandler
GPIOD_IRQHandler
GPIOE_IRQHandler
Reserved35_IRQHandler
Reserved36_IRQHandler
Reserved37_IRQHandler
Reserved38_IRQHandler
Reserved39_IRQHandler
Reserved40_IRQHandler
Reserved41_IRQHandler
Reserved42_IRQHandler
Reserved43_IRQHandler
Reserved44_IRQHandler
Reserved45_IRQHandler
Reserved46_IRQHandler
Reserved47_IRQHandler
Reserved48_IRQHandler
CAN0_ORed_IRQHandler
CAN0_Error_IRQHandler
CAN0_Wake_Up_IRQHandler
CAN0_ORed_0_15_MB_IRQHandler
CAN0_ORed_16_31_MB_IRQHandler
CAN0_ORed_32_47_MB_IRQHandler
CAN0_ORed_48_63_MB_IRQHandler
CAN1_ORed_IRQHandler
CAN1_Error_IRQHandler
CAN1_Wake_Up_IRQHandler
CAN1_ORed_0_15_MB_IRQHandler
CAN1_ORed_16_31_MB_IRQHandler
CAN1_ORed_32_47_MB_IRQHandler
CAN1_ORed_48_63_MB_IRQHandler
Reserved49_IRQHandler
Reserved50_IRQHandler
Reserved51_IRQHandler
Reserved52_IRQHandler
Reserved53_IRQHandler
Reserved54_IRQHandler
Reserved55_IRQHandler
eTMR0_Ch0_Ch1_IRQHandler
eTMR0_Ch2_Ch3_IRQHandler
eTMR0_Ch4_Ch5_IRQHandler
eTMR0_Ch6_Ch7_IRQHandler
eTMR0_Fault_IRQHandler
eTMR0_Ovf_IRQHandler
eTMR1_Ch0_Ch1_IRQHandler
eTMR1_Ch2_Ch3_IRQHandler
eTMR1_Ch4_Ch5_IRQHandler
eTMR1_Ch6_Ch7_IRQHandler
eTMR1_Fault_IRQHandler
eTMR1_Ovf_IRQHandler
Reserved56_IRQHandler
Reserved57_IRQHandler
Reserved58_IRQHandler
Reserved59_IRQHandler
Reserved60_IRQHandler
Reserved61_IRQHandler
Reserved62_IRQHandler
Reserved63_IRQHandler
Reserved64_IRQHandler
Reserved65_IRQHandler
Reserved66_IRQHandler
Reserved67_IRQHandler
Reserved68_IRQHandler
Reserved69_IRQHandler
Reserved70_IRQHandler
Reserved71_IRQHandler
Reserved72_IRQHandler
Reserved73_IRQHandler
Reserved74_IRQHandler
Reserved75_IRQHandler
Reserved76_IRQHandler
Reserved77_IRQHandler
Reserved78_IRQHandler
Reserved79_IRQHandler
MPWM0_Ch0_Ovf_IRQHandler
MPWM0_Ch1_Ovf_IRQHandler
MPWM0_Ch2_Ovf_IRQHandler
MPWM0_Ch3_Ovf_IRQHandler
MPWM0_Ch4_Ovf_IRQHandler
MPWM0_Ch5_Ovf_IRQHandler
MPWM0_Ch6_Ovf_IRQHandler
MPWM0_Ch7_Ovf_IRQHandler
MPWM0_Ch0_IRQHandler
MPWM0_Ch1_IRQHandler
MPWM0_Ch2_IRQHandler
MPWM0_Ch3_IRQHandler
MPWM0_Ch4_IRQHandler
MPWM0_Ch5_IRQHandler
MPWM0_Ch6_IRQHandler
MPWM0_Ch7_IRQHandler
Reserved80_IRQHandler
Reserved81_IRQHandler
Reserved82_IRQHandler
Reserved83_IRQHandler
Reserved84_IRQHandler
TRNG_IRQHandler
HCU_IRQHandler
DefaultISR
    B DefaultISR

    END