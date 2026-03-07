;/* ---------------------------------------------------------------------------------------
; *  @file:    YTM32B1MC0_startup_armc.S
; *  @purpose: ARM compiler Startup File
; *            YTM32B1MC0
; *  @version: 1.3
; *  @date:    2024-06-25
; * ---------------------------------------------------------------------------------------*/

    PRESERVE8
    THUMB

    AREA    |.text|, CODE, READONLY

    EXPORT  __VECTOR_TABLE
    EXPORT  __Vectors_End
    EXPORT  __Vectors_Size

    EXPORT __BSS_START
    EXPORT __BSS_END

    EXPORT __DATA_ROM
    EXPORT __VECTOR_ROM_START
    EXPORT __VECTOR_ROM_END
    EXPORT __DATA_RAM_START
    EXPORT __DATA_RAM_END
    
    EXPORT __RAM_START
    EXPORT __RAM_END
        
    EXPORT __RAM_VECTOR_TABLE_SIZE
    EXPORT __VECTOR_RAM
    EXPORT __VECTOR_ROM
        
    IMPORT  |Image$$ARM_LIB_STACK$$ZI$$Limit|
    IMPORT  |Image$$ARM_LIB_STACK$$Limit|
    IMPORT  |Image$$ARM_LIB_STACK$$Length|
    IMPORT  |Image$$ARM_LIB_HEAP$$Length|
    
    IMPORT  |Image$$RW_m_data$$ZI$$Base|
    IMPORT  |Image$$RW_m_data$$ZI$$Limit|
    IMPORT  |Image$$RW_m_data$$RW$$Base|
    IMPORT  |Image$$RW_m_data$$RW$$Length|
    IMPORT  |Image$$RW_m_data$$RW$$Limit|

    IMPORT  |Image$$RW_m_data$$Base|
    IMPORT  |Image$$RW_m_data$$Limit|
    IMPORT  |Image$$RW_m_data$$Length|

    IMPORT |Image$$ER_m_text$$Limit|

    IMPORT |Image$$VECTOR_RAM$$Length|
    IMPORT |Image$$VECTOR_RAM$$Base|
        
    IMPORT |Image$$VECTOR_ROM$$Length|
    IMPORT |Image$$VECTOR_ROM$$Base|
    IMPORT |Image$$VECTOR_ROM$$Limit|
                
__RAM_VECTOR_TABLE_SIZE     DCD |Image$$VECTOR_ROM$$Length|
__VECTOR_RAM                DCD |Image$$VECTOR_RAM$$Base|
__VECTOR_ROM                DCD |Image$$VECTOR_ROM$$Base|
__VECTOR_ROM_START          DCD |Image$$VECTOR_ROM$$Base|
__VECTOR_ROM_END            DCD |Image$$VECTOR_ROM$$Limit|

__BSS_START                 DCD  |Image$$RW_m_data$$ZI$$Base|
__BSS_END                   DCD  |Image$$RW_m_data$$ZI$$Limit|

__DATA_RAM_START            DCD  |Image$$RW_m_data$$RW$$Base|
__DATA_RAM_END              DCD  |Image$$RW_m_data$$RW$$Limit|
__DATA_ROM                  DCD  |Image$$ER_m_text$$Limit|

__RAM_START                 DCD  |Image$$RW_m_data$$Base|
__RAM_END                   DCD  |Image$$ARM_LIB_STACK$$ZI$$Limit| - 1

; Vector Table Mapped to Address 0 at Reset
    AREA    RESET, DATA, READONLY
    ALIGN   4
__VECTOR_TABLE
    DCD   |Image$$ARM_LIB_STACK$$ZI$$Limit|     ; top of stack
    DCD   Reset_Handler                         ; Reset Handler
    DCD   NMI_Handler                           ; Non Maskable Interrupt
    DCD   HardFault_Handler                     ; Cortex-M33 Hard Fault Interrupt
    DCD   0
    DCD   0
    DCD   0
    DCD   0
    DCD   0
    DCD   0
    DCD   0
    DCD   SVC_Handler                           ; Cortex-M33 SV Call Interrupt
    DCD   0
    DCD   0
    DCD   PendSV_Handler                        ; Cortex-M33 Pend SV Interrupt
    DCD   SysTick_Handler                       ; Cortex-M33 System Tick Interrupt

    DCD   DMA0_IRQHandler                       ;  DMA0 Handler
    DCD   DMA1_IRQHandler                       ;  DMA1 Handler
    DCD   DMA2_IRQHandler                       ;  DMA2 Handler
    DCD   DMA3_IRQHandler                       ;  DMA3 Handler
    DCD   Reserved5_IRQHandler                  ;  Reserved5 Handler
    DCD   Reserved6_IRQHandler                  ;  Reserved6 Handler
    DCD   Reserved7_IRQHandler                  ;  Reserved7 Handler
    DCD   Reserved8_IRQHandler                  ;  Reserved8 Handler
    DCD   Reserved9_IRQHandler                  ;  Reserved9 Handler
    DCD   Reserved10_IRQHandler                 ;  Reserved10 Handler
    DCD   Reserved11_IRQHandler                 ;  Reserved11 Handler
    DCD   Reserved12_IRQHandler                 ;  Reserved12 Handler
    DCD   Reserved13_IRQHandler                 ;  Reserved13 Handler
    DCD   Reserved14_IRQHandler                 ;  Reserved14 Handler
    DCD   Reserved15_IRQHandler                 ;  Reserved15 Handler
    DCD   Reserved16_IRQHandler                 ;  Reserved16 Handler
    DCD   DMA_Error_IRQHandler                  ;  DMA_Error Handler
    DCD   Reserved17_IRQHandler                 ;  Reserved17 Handler
    DCD   EFM_IRQHandler                        ;  EFM Handler
    DCD   EFM_Error_IRQHandler                  ;  EFM_Error Handler
    DCD   PCU_IRQHandler                        ;  PCU Handler
    DCD   EFM_Ecc_IRQHandler                    ;  EFM_Ecc Handler
    DCD   WDG0_IRQHandler                       ;  WDG0 Handler
    DCD   Reserved18_IRQHandler                 ;  Reserved18 Handler
    DCD   I2C0_Master_IRQHandler                 ;  I2C0_Master Handler
    DCD   I2C0_Slave_IRQHandler                 ;  I2C0_Slave Handler
    DCD   SPI0_IRQHandler                       ;  SPI0 Handler
    DCD   SPI1_IRQHandler                       ;  SPI1 Handler
    DCD   SPI2_IRQHandler                       ;  SPI2 Handler
    DCD   I2C1_Master_IRQHandler                 ;  I2C1_Master Handler
    DCD   Reserved19_IRQHandler                 ;  Reserved19 Handler
    DCD   UART0_IRQHandler                      ;  UART0 Handler
    DCD   Reserved20_IRQHandler                 ;  Reserved20 Handler
    DCD   UART1_IRQHandler                      ;  UART1 Handler
    DCD   Reserved21_IRQHandler                 ;  Reserved21 Handler
    DCD   UART2_IRQHandler                      ;  UART2 Handler
    DCD   Reserved22_IRQHandler                 ;  Reserved22 Handler
    DCD   Reserved23_IRQHandler                 ;  Reserved23 Handler
    DCD   Reserved24_IRQHandler                 ;  Reserved24 Handler
    DCD   ADC0_IRQHandler                       ;  ADC0 Handler
    DCD   Reserved25_IRQHandler                 ;  Reserved25 Handler
    DCD   ACMP0_IRQHandler                      ;  ACMP0 Handler
    DCD   Reserved26_IRQHandler                 ;  Reserved26 Handler
    DCD   Reserved27_IRQHandler                 ;  Reserved27 Handler
    DCD   EMU0_SB_IRQHandler                    ;  EMU0_SB Handler
    DCD   EMU0_DB_IRQHandler                    ;  EMU0_DB Handler
    DCD   Reserved28_IRQHandler                 ;  Reserved28 Handler
    DCD   Reserved29_IRQHandler                 ;  Reserved29 Handler
    DCD   pTMR0_Ch0_IRQHandler                  ;  pTMR0_Ch0 Handler
    DCD   pTMR0_Ch1_IRQHandler                  ;  pTMR0_Ch1 Handler
    DCD   pTMR0_Ch2_IRQHandler                  ;  pTMR0_Ch2 Handler
    DCD   pTMR0_Ch3_IRQHandler                  ;  pTMR0_Ch3 Handler
    DCD   Reserved30_IRQHandler                 ;  Reserved30 Handler
    DCD   Reserved31_IRQHandler                 ;  Reserved31 Handler
    DCD   Reserved32_IRQHandler                 ;  Reserved32 Handler
    DCD   Reserved33_IRQHandler                 ;  Reserved33 Handler
    DCD   Reserved34_IRQHandler                 ;  Reserved34 Handler
    DCD   SCU_IRQHandler                        ;  SCU Handler
    DCD   lpTMR0_IRQHandler                     ;  lpTMR0 Handler
    DCD   GPIOA_IRQHandler                      ;  GPIOA Handler
    DCD   GPIOB_IRQHandler                      ;  GPIOB Handler
    DCD   GPIOC_IRQHandler                      ;  GPIOC Handler
    DCD   GPIOD_IRQHandler                      ;  GPIOD Handler
    DCD   GPIOE_IRQHandler                      ;  GPIOE Handler
    DCD   Reserved35_IRQHandler                 ;  Reserved35 Handler
    DCD   Reserved36_IRQHandler                 ;  Reserved36 Handler
    DCD   Reserved37_IRQHandler                 ;  Reserved37 Handler
    DCD   Reserved38_IRQHandler                 ;  Reserved38 Handler
    DCD   Reserved39_IRQHandler                 ;  Reserved39 Handler
    DCD   Reserved40_IRQHandler                 ;  Reserved40 Handler
    DCD   Reserved41_IRQHandler                 ;  Reserved41 Handler
    DCD   Reserved42_IRQHandler                 ;  Reserved42 Handler
    DCD   Reserved43_IRQHandler                 ;  Reserved43 Handler
    DCD   Reserved44_IRQHandler                 ;  Reserved44 Handler
    DCD   Reserved45_IRQHandler                 ;  Reserved45 Handler
    DCD   Reserved46_IRQHandler                 ;  Reserved46 Handler
    DCD   Reserved47_IRQHandler                 ;  Reserved47 Handler
    DCD   Reserved48_IRQHandler                 ;  Reserved48 Handler
    DCD   CAN0_ORed_IRQHandler                  ;  CAN0_ORed Handler
    DCD   CAN0_Error_IRQHandler                 ;  CAN0_Error Handler
    DCD   CAN0_Wake_Up_IRQHandler                 ;  CAN0_Wake_Up Handler
    DCD   CAN0_ORed_0_15_MB_IRQHandler                 ;  CAN0_ORed_0_15_MB Handler
    DCD   CAN0_ORed_16_31_MB_IRQHandler                 ;  CAN0_ORed_16_31_MB Handler
    DCD   CAN0_ORed_32_47_MB_IRQHandler                 ;  CAN0_ORed_32_47_MB Handler
    DCD   CAN0_ORed_48_63_MB_IRQHandler                 ;  CAN0_ORed_48_63_MB Handler
    DCD   CAN1_ORed_IRQHandler                  ;  CAN1_ORed Handler
    DCD   CAN1_Error_IRQHandler                 ;  CAN1_Error Handler
    DCD   CAN1_Wake_Up_IRQHandler                 ;  CAN1_Wake_Up Handler
    DCD   CAN1_ORed_0_15_MB_IRQHandler                 ;  CAN1_ORed_0_15_MB Handler
    DCD   CAN1_ORed_16_31_MB_IRQHandler                 ;  CAN1_ORed_16_31_MB Handler
    DCD   CAN1_ORed_32_47_MB_IRQHandler                 ;  CAN1_ORed_32_47_MB Handler
    DCD   CAN1_ORed_48_63_MB_IRQHandler                 ;  CAN1_ORed_48_63_MB Handler
    DCD   Reserved49_IRQHandler                 ;  Reserved49 Handler
    DCD   Reserved50_IRQHandler                 ;  Reserved50 Handler
    DCD   Reserved51_IRQHandler                 ;  Reserved51 Handler
    DCD   Reserved52_IRQHandler                 ;  Reserved52 Handler
    DCD   Reserved53_IRQHandler                 ;  Reserved53 Handler
    DCD   Reserved54_IRQHandler                 ;  Reserved54 Handler
    DCD   Reserved55_IRQHandler                 ;  Reserved55 Handler
    DCD   eTMR0_Ch0_Ch1_IRQHandler                 ;  eTMR0_Ch0_Ch1 Handler
    DCD   eTMR0_Ch2_Ch3_IRQHandler                 ;  eTMR0_Ch2_Ch3 Handler
    DCD   eTMR0_Ch4_Ch5_IRQHandler                 ;  eTMR0_Ch4_Ch5 Handler
    DCD   eTMR0_Ch6_Ch7_IRQHandler                 ;  eTMR0_Ch6_Ch7 Handler
    DCD   eTMR0_Fault_IRQHandler                 ;  eTMR0_Fault Handler
    DCD   eTMR0_Ovf_IRQHandler                  ;  eTMR0_Ovf Handler
    DCD   eTMR1_Ch0_Ch1_IRQHandler                 ;  eTMR1_Ch0_Ch1 Handler
    DCD   eTMR1_Ch2_Ch3_IRQHandler                 ;  eTMR1_Ch2_Ch3 Handler
    DCD   eTMR1_Ch4_Ch5_IRQHandler                 ;  eTMR1_Ch4_Ch5 Handler
    DCD   eTMR1_Ch6_Ch7_IRQHandler                 ;  eTMR1_Ch6_Ch7 Handler
    DCD   eTMR1_Fault_IRQHandler                 ;  eTMR1_Fault Handler
    DCD   eTMR1_Ovf_IRQHandler                  ;  eTMR1_Ovf Handler
    DCD   Reserved56_IRQHandler                 ;  Reserved56 Handler
    DCD   Reserved57_IRQHandler                 ;  Reserved57 Handler
    DCD   Reserved58_IRQHandler                 ;  Reserved58 Handler
    DCD   Reserved59_IRQHandler                 ;  Reserved59 Handler
    DCD   Reserved60_IRQHandler                 ;  Reserved60 Handler
    DCD   Reserved61_IRQHandler                 ;  Reserved61 Handler
    DCD   Reserved62_IRQHandler                 ;  Reserved62 Handler
    DCD   Reserved63_IRQHandler                 ;  Reserved63 Handler
    DCD   Reserved64_IRQHandler                 ;  Reserved64 Handler
    DCD   Reserved65_IRQHandler                 ;  Reserved65 Handler
    DCD   Reserved66_IRQHandler                 ;  Reserved66 Handler
    DCD   Reserved67_IRQHandler                 ;  Reserved67 Handler
    DCD   Reserved68_IRQHandler                 ;  Reserved68 Handler
    DCD   Reserved69_IRQHandler                 ;  Reserved69 Handler
    DCD   Reserved70_IRQHandler                 ;  Reserved70 Handler
    DCD   Reserved71_IRQHandler                 ;  Reserved71 Handler
    DCD   Reserved72_IRQHandler                 ;  Reserved72 Handler
    DCD   Reserved73_IRQHandler                 ;  Reserved73 Handler
    DCD   Reserved74_IRQHandler                 ;  Reserved74 Handler
    DCD   Reserved75_IRQHandler                 ;  Reserved75 Handler
    DCD   Reserved76_IRQHandler                 ;  Reserved76 Handler
    DCD   Reserved77_IRQHandler                 ;  Reserved77 Handler
    DCD   Reserved78_IRQHandler                 ;  Reserved78 Handler
    DCD   Reserved79_IRQHandler                 ;  Reserved79 Handler
    DCD   MPWM0_Ch0_Ovf_IRQHandler                 ;  MPWM0_Ch0_Ovf Handler
    DCD   MPWM0_Ch1_Ovf_IRQHandler                 ;  MPWM0_Ch1_Ovf Handler
    DCD   MPWM0_Ch2_Ovf_IRQHandler                 ;  MPWM0_Ch2_Ovf Handler
    DCD   MPWM0_Ch3_Ovf_IRQHandler                 ;  MPWM0_Ch3_Ovf Handler
    DCD   MPWM0_Ch4_Ovf_IRQHandler                 ;  MPWM0_Ch4_Ovf Handler
    DCD   MPWM0_Ch5_Ovf_IRQHandler                 ;  MPWM0_Ch5_Ovf Handler
    DCD   MPWM0_Ch6_Ovf_IRQHandler                 ;  MPWM0_Ch6_Ovf Handler
    DCD   MPWM0_Ch7_Ovf_IRQHandler                 ;  MPWM0_Ch7_Ovf Handler
    DCD   MPWM0_Ch0_IRQHandler                  ;  MPWM0_Ch0 Handler
    DCD   MPWM0_Ch1_IRQHandler                  ;  MPWM0_Ch1 Handler
    DCD   MPWM0_Ch2_IRQHandler                  ;  MPWM0_Ch2 Handler
    DCD   MPWM0_Ch3_IRQHandler                  ;  MPWM0_Ch3 Handler
    DCD   MPWM0_Ch4_IRQHandler                  ;  MPWM0_Ch4 Handler
    DCD   MPWM0_Ch5_IRQHandler                  ;  MPWM0_Ch5 Handler
    DCD   MPWM0_Ch6_IRQHandler                  ;  MPWM0_Ch6 Handler
    DCD   MPWM0_Ch7_IRQHandler                  ;  MPWM0_Ch7 Handler
    DCD   Reserved80_IRQHandler                 ;  Reserved80 Handler
    DCD   Reserved81_IRQHandler                 ;  Reserved81 Handler
    DCD   Reserved82_IRQHandler                 ;  Reserved82 Handler
    DCD   Reserved83_IRQHandler                 ;  Reserved83 Handler
    DCD   Reserved84_IRQHandler                 ;  Reserved84 Handler
    DCD   TRNG_IRQHandler                       ;  TRNG Handler
    DCD   HCU_IRQHandler                        ;  HCU Handler

__Vectors_End

__Vectors_Size  EQU   __Vectors_End - __VECTOR_TABLE


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



    AREA    |.text|, CODE, READONLY
                
Reset_Handler   PROC
    ALIGN   4
    EXPORT  Reset_Handler               [WEAK]
    IMPORT  __main
    IMPORT  SystemInit
    IMPORT  init_data_bss

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
    LDR     R1, =__RAM_START
    LDR     R1, [R1]
    LDR     R2, =__RAM_END
    LDR     R2, [R2]

    SUBS    R2, R1
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
    NOP
#endif
                
    LDR     R0, =__VECTOR_TABLE
    LDR     R1, [R0]
    MSR     MSP, R1
    
    LDR     R0, =SystemInit
    BLX     R0
    LDR     R0, =init_data_bss
    BLX     R0
    
    CPSIE   I               ; Unmask interrupts
    
    LDR     R0, =__main
    BX      R0
    ENDP    


NMI_Handler     PROC
    EXPORT  NMI_Handler             [WEAK]
    B       .
    ENDP

HardFault_Handler\
    PROC
    EXPORT  HardFault_Handler       [WEAK]
    B       .
    ENDP

SVC_Handler     PROC
    EXPORT  SVC_Handler             [WEAK]
    B       .
    ENDP

PendSV_Handler  PROC
    EXPORT  PendSV_Handler          [WEAK]
    B       .
    ENDP

SysTick_Handler PROC
    EXPORT  SysTick_Handler         [WEAK]
    B       .
    ENDP

DefaultISR      PROC
    EXPORT   DefaultISR             [WEAK]
    B       .
    ENDP

Default_Handler PROC
    EXPORT   DMA0_IRQHandler                [WEAK]
    EXPORT   DMA1_IRQHandler                [WEAK]
    EXPORT   DMA2_IRQHandler                [WEAK]
    EXPORT   DMA3_IRQHandler                [WEAK]
    EXPORT   Reserved5_IRQHandler           [WEAK]
    EXPORT   Reserved6_IRQHandler           [WEAK]
    EXPORT   Reserved7_IRQHandler           [WEAK]
    EXPORT   Reserved8_IRQHandler           [WEAK]
    EXPORT   Reserved9_IRQHandler           [WEAK]
    EXPORT   Reserved10_IRQHandler          [WEAK]
    EXPORT   Reserved11_IRQHandler          [WEAK]
    EXPORT   Reserved12_IRQHandler          [WEAK]
    EXPORT   Reserved13_IRQHandler          [WEAK]
    EXPORT   Reserved14_IRQHandler          [WEAK]
    EXPORT   Reserved15_IRQHandler          [WEAK]
    EXPORT   Reserved16_IRQHandler          [WEAK]
    EXPORT   DMA_Error_IRQHandler           [WEAK]
    EXPORT   Reserved17_IRQHandler          [WEAK]
    EXPORT   EFM_IRQHandler                 [WEAK]
    EXPORT   EFM_Error_IRQHandler           [WEAK]
    EXPORT   PCU_IRQHandler                 [WEAK]
    EXPORT   EFM_Ecc_IRQHandler             [WEAK]
    EXPORT   WDG0_IRQHandler                [WEAK]
    EXPORT   Reserved18_IRQHandler          [WEAK]
    EXPORT   I2C0_Master_IRQHandler         [WEAK]
    EXPORT   I2C0_Slave_IRQHandler          [WEAK]
    EXPORT   SPI0_IRQHandler                [WEAK]
    EXPORT   SPI1_IRQHandler                [WEAK]
    EXPORT   SPI2_IRQHandler                [WEAK]
    EXPORT   I2C1_Master_IRQHandler         [WEAK]
    EXPORT   Reserved19_IRQHandler          [WEAK]
    EXPORT   UART0_IRQHandler               [WEAK]
    EXPORT   Reserved20_IRQHandler          [WEAK]
    EXPORT   UART1_IRQHandler               [WEAK]
    EXPORT   Reserved21_IRQHandler          [WEAK]
    EXPORT   UART2_IRQHandler               [WEAK]
    EXPORT   Reserved22_IRQHandler          [WEAK]
    EXPORT   Reserved23_IRQHandler          [WEAK]
    EXPORT   Reserved24_IRQHandler          [WEAK]
    EXPORT   ADC0_IRQHandler                [WEAK]
    EXPORT   Reserved25_IRQHandler          [WEAK]
    EXPORT   ACMP0_IRQHandler               [WEAK]
    EXPORT   Reserved26_IRQHandler          [WEAK]
    EXPORT   Reserved27_IRQHandler          [WEAK]
    EXPORT   EMU0_SB_IRQHandler             [WEAK]
    EXPORT   EMU0_DB_IRQHandler             [WEAK]
    EXPORT   Reserved28_IRQHandler          [WEAK]
    EXPORT   Reserved29_IRQHandler          [WEAK]
    EXPORT   pTMR0_Ch0_IRQHandler           [WEAK]
    EXPORT   pTMR0_Ch1_IRQHandler           [WEAK]
    EXPORT   pTMR0_Ch2_IRQHandler           [WEAK]
    EXPORT   pTMR0_Ch3_IRQHandler           [WEAK]
    EXPORT   Reserved30_IRQHandler          [WEAK]
    EXPORT   Reserved31_IRQHandler          [WEAK]
    EXPORT   Reserved32_IRQHandler          [WEAK]
    EXPORT   Reserved33_IRQHandler          [WEAK]
    EXPORT   Reserved34_IRQHandler          [WEAK]
    EXPORT   SCU_IRQHandler                 [WEAK]
    EXPORT   lpTMR0_IRQHandler              [WEAK]
    EXPORT   GPIOA_IRQHandler               [WEAK]
    EXPORT   GPIOB_IRQHandler               [WEAK]
    EXPORT   GPIOC_IRQHandler               [WEAK]
    EXPORT   GPIOD_IRQHandler               [WEAK]
    EXPORT   GPIOE_IRQHandler               [WEAK]
    EXPORT   Reserved35_IRQHandler          [WEAK]
    EXPORT   Reserved36_IRQHandler          [WEAK]
    EXPORT   Reserved37_IRQHandler          [WEAK]
    EXPORT   Reserved38_IRQHandler          [WEAK]
    EXPORT   Reserved39_IRQHandler          [WEAK]
    EXPORT   Reserved40_IRQHandler          [WEAK]
    EXPORT   Reserved41_IRQHandler          [WEAK]
    EXPORT   Reserved42_IRQHandler          [WEAK]
    EXPORT   Reserved43_IRQHandler          [WEAK]
    EXPORT   Reserved44_IRQHandler          [WEAK]
    EXPORT   Reserved45_IRQHandler          [WEAK]
    EXPORT   Reserved46_IRQHandler          [WEAK]
    EXPORT   Reserved47_IRQHandler          [WEAK]
    EXPORT   Reserved48_IRQHandler          [WEAK]
    EXPORT   CAN0_ORed_IRQHandler           [WEAK]
    EXPORT   CAN0_Error_IRQHandler          [WEAK]
    EXPORT   CAN0_Wake_Up_IRQHandler        [WEAK]
    EXPORT   CAN0_ORed_0_15_MB_IRQHandler   [WEAK]
    EXPORT   CAN0_ORed_16_31_MB_IRQHandler  [WEAK]
    EXPORT   CAN0_ORed_32_47_MB_IRQHandler  [WEAK]
    EXPORT   CAN0_ORed_48_63_MB_IRQHandler  [WEAK]
    EXPORT   CAN1_ORed_IRQHandler           [WEAK]
    EXPORT   CAN1_Error_IRQHandler          [WEAK]
    EXPORT   CAN1_Wake_Up_IRQHandler        [WEAK]
    EXPORT   CAN1_ORed_0_15_MB_IRQHandler   [WEAK]
    EXPORT   CAN1_ORed_16_31_MB_IRQHandler  [WEAK]
    EXPORT   CAN1_ORed_32_47_MB_IRQHandler  [WEAK]
    EXPORT   CAN1_ORed_48_63_MB_IRQHandler  [WEAK]
    EXPORT   Reserved49_IRQHandler          [WEAK]
    EXPORT   Reserved50_IRQHandler          [WEAK]
    EXPORT   Reserved51_IRQHandler          [WEAK]
    EXPORT   Reserved52_IRQHandler          [WEAK]
    EXPORT   Reserved53_IRQHandler          [WEAK]
    EXPORT   Reserved54_IRQHandler          [WEAK]
    EXPORT   Reserved55_IRQHandler          [WEAK]
    EXPORT   eTMR0_Ch0_Ch1_IRQHandler       [WEAK]
    EXPORT   eTMR0_Ch2_Ch3_IRQHandler       [WEAK]
    EXPORT   eTMR0_Ch4_Ch5_IRQHandler       [WEAK]
    EXPORT   eTMR0_Ch6_Ch7_IRQHandler       [WEAK]
    EXPORT   eTMR0_Fault_IRQHandler         [WEAK]
    EXPORT   eTMR0_Ovf_IRQHandler           [WEAK]
    EXPORT   eTMR1_Ch0_Ch1_IRQHandler       [WEAK]
    EXPORT   eTMR1_Ch2_Ch3_IRQHandler       [WEAK]
    EXPORT   eTMR1_Ch4_Ch5_IRQHandler       [WEAK]
    EXPORT   eTMR1_Ch6_Ch7_IRQHandler       [WEAK]
    EXPORT   eTMR1_Fault_IRQHandler         [WEAK]
    EXPORT   eTMR1_Ovf_IRQHandler           [WEAK]
    EXPORT   Reserved56_IRQHandler          [WEAK]
    EXPORT   Reserved57_IRQHandler          [WEAK]
    EXPORT   Reserved58_IRQHandler          [WEAK]
    EXPORT   Reserved59_IRQHandler          [WEAK]
    EXPORT   Reserved60_IRQHandler          [WEAK]
    EXPORT   Reserved61_IRQHandler          [WEAK]
    EXPORT   Reserved62_IRQHandler          [WEAK]
    EXPORT   Reserved63_IRQHandler          [WEAK]
    EXPORT   Reserved64_IRQHandler          [WEAK]
    EXPORT   Reserved65_IRQHandler          [WEAK]
    EXPORT   Reserved66_IRQHandler          [WEAK]
    EXPORT   Reserved67_IRQHandler          [WEAK]
    EXPORT   Reserved68_IRQHandler          [WEAK]
    EXPORT   Reserved69_IRQHandler          [WEAK]
    EXPORT   Reserved70_IRQHandler          [WEAK]
    EXPORT   Reserved71_IRQHandler          [WEAK]
    EXPORT   Reserved72_IRQHandler          [WEAK]
    EXPORT   Reserved73_IRQHandler          [WEAK]
    EXPORT   Reserved74_IRQHandler          [WEAK]
    EXPORT   Reserved75_IRQHandler          [WEAK]
    EXPORT   Reserved76_IRQHandler          [WEAK]
    EXPORT   Reserved77_IRQHandler          [WEAK]
    EXPORT   Reserved78_IRQHandler          [WEAK]
    EXPORT   Reserved79_IRQHandler          [WEAK]
    EXPORT   MPWM0_Ch0_Ovf_IRQHandler       [WEAK]
    EXPORT   MPWM0_Ch1_Ovf_IRQHandler       [WEAK]
    EXPORT   MPWM0_Ch2_Ovf_IRQHandler       [WEAK]
    EXPORT   MPWM0_Ch3_Ovf_IRQHandler       [WEAK]
    EXPORT   MPWM0_Ch4_Ovf_IRQHandler       [WEAK]
    EXPORT   MPWM0_Ch5_Ovf_IRQHandler       [WEAK]
    EXPORT   MPWM0_Ch6_Ovf_IRQHandler       [WEAK]
    EXPORT   MPWM0_Ch7_Ovf_IRQHandler       [WEAK]
    EXPORT   MPWM0_Ch0_IRQHandler           [WEAK]
    EXPORT   MPWM0_Ch1_IRQHandler           [WEAK]
    EXPORT   MPWM0_Ch2_IRQHandler           [WEAK]
    EXPORT   MPWM0_Ch3_IRQHandler           [WEAK]
    EXPORT   MPWM0_Ch4_IRQHandler           [WEAK]
    EXPORT   MPWM0_Ch5_IRQHandler           [WEAK]
    EXPORT   MPWM0_Ch6_IRQHandler           [WEAK]
    EXPORT   MPWM0_Ch7_IRQHandler           [WEAK]
    EXPORT   Reserved80_IRQHandler          [WEAK]
    EXPORT   Reserved81_IRQHandler          [WEAK]
    EXPORT   Reserved82_IRQHandler          [WEAK]
    EXPORT   Reserved83_IRQHandler          [WEAK]
    EXPORT   Reserved84_IRQHandler          [WEAK]
    EXPORT   TRNG_IRQHandler                [WEAK]
    EXPORT   HCU_IRQHandler                 [WEAK]

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
    B       .

    ENDP
    ALIGN 4

    END