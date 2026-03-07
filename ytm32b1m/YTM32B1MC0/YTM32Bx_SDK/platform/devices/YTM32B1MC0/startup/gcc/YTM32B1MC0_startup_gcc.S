/* ---------------------------------------------------------------------------------------
 *  @file:    YTM32B1MC0_startup_gcc.S
 *  @purpose: GNU Compiler Collection Startup File
 *            YTM32B1MC0
 *  @version: 1.3
 *  @date:    2024-06-25
 * ---------------------------------------------------------------------------------------*/
/*****************************************************************************/
/* Version: GNU Compiler Collection                                          */
/*****************************************************************************/
    .syntax unified
    .arch armv8-m.main

    .section .isr_vector, "a"
    .align 2
    .globl __isr_vector
__isr_vector:
    .long   __StackTop                                        /* Top of Stack */
    .long   Reset_Handler                                     /* Reset Handler */
    .long   NMI_Handler                                                  /* NMI Handler*/
    .long   HardFault_Handler                                            /* HardFault Handler*/
    .long   MemManage_Handler                                            /* MemManage Handler*/
    .long   BusFault_Handler                                             /* BusFault Handler*/
    .long   UsageFault_Handler                                           /* UsageFault Handler*/
    .long   0                                                 /* Reserved*/
    .long   0                                                 /* Reserved*/
    .long   0                                                 /* Reserved*/
    .long   0                                                 /* Reserved*/
    .long   SVC_Handler                                                  /* SVC Handler*/
    .long   DebugMon_Handler                                             /* DebugMon Handler*/
    .long   0                                                 /* Reserved*/
    .long   PendSV_Handler                                               /* PendSV Handler*/
    .long   SysTick_Handler                                              /* SysTick Handler*/

/* External Interrupts*/
    .long   DMA0_IRQHandler                                             /* DMA0 Handler*/
    .long   DMA1_IRQHandler                                             /* DMA1 Handler*/
    .long   DMA2_IRQHandler                                             /* DMA2 Handler*/
    .long   DMA3_IRQHandler                                             /* DMA3 Handler*/
    .long   Reserved5_IRQHandler                                        /* Reserved5 Handler*/
    .long   Reserved6_IRQHandler                                        /* Reserved6 Handler*/
    .long   Reserved7_IRQHandler                                        /* Reserved7 Handler*/
    .long   Reserved8_IRQHandler                                        /* Reserved8 Handler*/
    .long   Reserved9_IRQHandler                                        /* Reserved9 Handler*/
    .long   Reserved10_IRQHandler                                       /* Reserved10 Handler*/
    .long   Reserved11_IRQHandler                                       /* Reserved11 Handler*/
    .long   Reserved12_IRQHandler                                       /* Reserved12 Handler*/
    .long   Reserved13_IRQHandler                                       /* Reserved13 Handler*/
    .long   Reserved14_IRQHandler                                       /* Reserved14 Handler*/
    .long   Reserved15_IRQHandler                                       /* Reserved15 Handler*/
    .long   Reserved16_IRQHandler                                       /* Reserved16 Handler*/
    .long   DMA_Error_IRQHandler                                        /* DMA_Error Handler*/
    .long   Reserved17_IRQHandler                                       /* Reserved17 Handler*/
    .long   EFM_IRQHandler                                              /* EFM Handler*/
    .long   EFM_Error_IRQHandler                                        /* EFM_Error Handler*/
    .long   PCU_IRQHandler                                              /* PCU Handler*/
    .long   EFM_Ecc_IRQHandler                                          /* EFM_Ecc Handler*/
    .long   WDG0_IRQHandler                                             /* WDG0 Handler*/
    .long   Reserved18_IRQHandler                                       /* Reserved18 Handler*/
    .long   I2C0_Master_IRQHandler                                      /* I2C0_Master Handler*/
    .long   I2C0_Slave_IRQHandler                                       /* I2C0_Slave Handler*/
    .long   SPI0_IRQHandler                                             /* SPI0 Handler*/
    .long   SPI1_IRQHandler                                             /* SPI1 Handler*/
    .long   SPI2_IRQHandler                                             /* SPI2 Handler*/
    .long   I2C1_Master_IRQHandler                                      /* I2C1_Master Handler*/
    .long   Reserved19_IRQHandler                                       /* Reserved19 Handler*/
    .long   UART0_IRQHandler                                            /* UART0 Handler*/
    .long   Reserved20_IRQHandler                                       /* Reserved20 Handler*/
    .long   UART1_IRQHandler                                            /* UART1 Handler*/
    .long   Reserved21_IRQHandler                                       /* Reserved21 Handler*/
    .long   UART2_IRQHandler                                            /* UART2 Handler*/
    .long   Reserved22_IRQHandler                                       /* Reserved22 Handler*/
    .long   Reserved23_IRQHandler                                       /* Reserved23 Handler*/
    .long   Reserved24_IRQHandler                                       /* Reserved24 Handler*/
    .long   ADC0_IRQHandler                                             /* ADC0 Handler*/
    .long   Reserved25_IRQHandler                                       /* Reserved25 Handler*/
    .long   ACMP0_IRQHandler                                            /* ACMP0 Handler*/
    .long   Reserved26_IRQHandler                                       /* Reserved26 Handler*/
    .long   Reserved27_IRQHandler                                       /* Reserved27 Handler*/
    .long   EMU0_SB_IRQHandler                                          /* EMU0_SB Handler*/
    .long   EMU0_DB_IRQHandler                                          /* EMU0_DB Handler*/
    .long   Reserved28_IRQHandler                                       /* Reserved28 Handler*/
    .long   Reserved29_IRQHandler                                       /* Reserved29 Handler*/
    .long   pTMR0_Ch0_IRQHandler                                        /* pTMR0_Ch0 Handler*/
    .long   pTMR0_Ch1_IRQHandler                                        /* pTMR0_Ch1 Handler*/
    .long   pTMR0_Ch2_IRQHandler                                        /* pTMR0_Ch2 Handler*/
    .long   pTMR0_Ch3_IRQHandler                                        /* pTMR0_Ch3 Handler*/
    .long   Reserved30_IRQHandler                                       /* Reserved30 Handler*/
    .long   Reserved31_IRQHandler                                       /* Reserved31 Handler*/
    .long   Reserved32_IRQHandler                                       /* Reserved32 Handler*/
    .long   Reserved33_IRQHandler                                       /* Reserved33 Handler*/
    .long   Reserved34_IRQHandler                                       /* Reserved34 Handler*/
    .long   SCU_IRQHandler                                              /* SCU Handler*/
    .long   lpTMR0_IRQHandler                                           /* lpTMR0 Handler*/
    .long   GPIOA_IRQHandler                                            /* GPIOA Handler*/
    .long   GPIOB_IRQHandler                                            /* GPIOB Handler*/
    .long   GPIOC_IRQHandler                                            /* GPIOC Handler*/
    .long   GPIOD_IRQHandler                                            /* GPIOD Handler*/
    .long   GPIOE_IRQHandler                                            /* GPIOE Handler*/
    .long   Reserved35_IRQHandler                                       /* Reserved35 Handler*/
    .long   Reserved36_IRQHandler                                       /* Reserved36 Handler*/
    .long   Reserved37_IRQHandler                                       /* Reserved37 Handler*/
    .long   Reserved38_IRQHandler                                       /* Reserved38 Handler*/
    .long   Reserved39_IRQHandler                                       /* Reserved39 Handler*/
    .long   Reserved40_IRQHandler                                       /* Reserved40 Handler*/
    .long   Reserved41_IRQHandler                                       /* Reserved41 Handler*/
    .long   Reserved42_IRQHandler                                       /* Reserved42 Handler*/
    .long   Reserved43_IRQHandler                                       /* Reserved43 Handler*/
    .long   Reserved44_IRQHandler                                       /* Reserved44 Handler*/
    .long   Reserved45_IRQHandler                                       /* Reserved45 Handler*/
    .long   Reserved46_IRQHandler                                       /* Reserved46 Handler*/
    .long   Reserved47_IRQHandler                                       /* Reserved47 Handler*/
    .long   Reserved48_IRQHandler                                       /* Reserved48 Handler*/
    .long   CAN0_ORed_IRQHandler                                        /* CAN0_ORed Handler*/
    .long   CAN0_Error_IRQHandler                                       /* CAN0_Error Handler*/
    .long   CAN0_Wake_Up_IRQHandler                                     /* CAN0_Wake_Up Handler*/
    .long   CAN0_ORed_0_15_MB_IRQHandler                                /* CAN0_ORed_0_15_MB Handler*/
    .long   CAN0_ORed_16_31_MB_IRQHandler                               /* CAN0_ORed_16_31_MB Handler*/
    .long   CAN0_ORed_32_47_MB_IRQHandler                               /* CAN0_ORed_32_47_MB Handler*/
    .long   CAN0_ORed_48_63_MB_IRQHandler                               /* CAN0_ORed_48_63_MB Handler*/
    .long   CAN1_ORed_IRQHandler                                        /* CAN1_ORed Handler*/
    .long   CAN1_Error_IRQHandler                                       /* CAN1_Error Handler*/
    .long   CAN1_Wake_Up_IRQHandler                                     /* CAN1_Wake_Up Handler*/
    .long   CAN1_ORed_0_15_MB_IRQHandler                                /* CAN1_ORed_0_15_MB Handler*/
    .long   CAN1_ORed_16_31_MB_IRQHandler                               /* CAN1_ORed_16_31_MB Handler*/
    .long   CAN1_ORed_32_47_MB_IRQHandler                               /* CAN1_ORed_32_47_MB Handler*/
    .long   CAN1_ORed_48_63_MB_IRQHandler                               /* CAN1_ORed_48_63_MB Handler*/
    .long   Reserved49_IRQHandler                                       /* Reserved49 Handler*/
    .long   Reserved50_IRQHandler                                       /* Reserved50 Handler*/
    .long   Reserved51_IRQHandler                                       /* Reserved51 Handler*/
    .long   Reserved52_IRQHandler                                       /* Reserved52 Handler*/
    .long   Reserved53_IRQHandler                                       /* Reserved53 Handler*/
    .long   Reserved54_IRQHandler                                       /* Reserved54 Handler*/
    .long   Reserved55_IRQHandler                                       /* Reserved55 Handler*/
    .long   eTMR0_Ch0_Ch1_IRQHandler                                    /* eTMR0_Ch0_Ch1 Handler*/
    .long   eTMR0_Ch2_Ch3_IRQHandler                                    /* eTMR0_Ch2_Ch3 Handler*/
    .long   eTMR0_Ch4_Ch5_IRQHandler                                    /* eTMR0_Ch4_Ch5 Handler*/
    .long   eTMR0_Ch6_Ch7_IRQHandler                                    /* eTMR0_Ch6_Ch7 Handler*/
    .long   eTMR0_Fault_IRQHandler                                      /* eTMR0_Fault Handler*/
    .long   eTMR0_Ovf_IRQHandler                                        /* eTMR0_Ovf Handler*/
    .long   eTMR1_Ch0_Ch1_IRQHandler                                    /* eTMR1_Ch0_Ch1 Handler*/
    .long   eTMR1_Ch2_Ch3_IRQHandler                                    /* eTMR1_Ch2_Ch3 Handler*/
    .long   eTMR1_Ch4_Ch5_IRQHandler                                    /* eTMR1_Ch4_Ch5 Handler*/
    .long   eTMR1_Ch6_Ch7_IRQHandler                                    /* eTMR1_Ch6_Ch7 Handler*/
    .long   eTMR1_Fault_IRQHandler                                      /* eTMR1_Fault Handler*/
    .long   eTMR1_Ovf_IRQHandler                                        /* eTMR1_Ovf Handler*/
    .long   Reserved56_IRQHandler                                       /* Reserved56 Handler*/
    .long   Reserved57_IRQHandler                                       /* Reserved57 Handler*/
    .long   Reserved58_IRQHandler                                       /* Reserved58 Handler*/
    .long   Reserved59_IRQHandler                                       /* Reserved59 Handler*/
    .long   Reserved60_IRQHandler                                       /* Reserved60 Handler*/
    .long   Reserved61_IRQHandler                                       /* Reserved61 Handler*/
    .long   Reserved62_IRQHandler                                       /* Reserved62 Handler*/
    .long   Reserved63_IRQHandler                                       /* Reserved63 Handler*/
    .long   Reserved64_IRQHandler                                       /* Reserved64 Handler*/
    .long   Reserved65_IRQHandler                                       /* Reserved65 Handler*/
    .long   Reserved66_IRQHandler                                       /* Reserved66 Handler*/
    .long   Reserved67_IRQHandler                                       /* Reserved67 Handler*/
    .long   Reserved68_IRQHandler                                       /* Reserved68 Handler*/
    .long   Reserved69_IRQHandler                                       /* Reserved69 Handler*/
    .long   Reserved70_IRQHandler                                       /* Reserved70 Handler*/
    .long   Reserved71_IRQHandler                                       /* Reserved71 Handler*/
    .long   Reserved72_IRQHandler                                       /* Reserved72 Handler*/
    .long   Reserved73_IRQHandler                                       /* Reserved73 Handler*/
    .long   Reserved74_IRQHandler                                       /* Reserved74 Handler*/
    .long   Reserved75_IRQHandler                                       /* Reserved75 Handler*/
    .long   Reserved76_IRQHandler                                       /* Reserved76 Handler*/
    .long   Reserved77_IRQHandler                                       /* Reserved77 Handler*/
    .long   Reserved78_IRQHandler                                       /* Reserved78 Handler*/
    .long   Reserved79_IRQHandler                                       /* Reserved79 Handler*/
    .long   MPWM0_Ch0_Ovf_IRQHandler                                    /* MPWM0_Ch0_Ovf Handler*/
    .long   MPWM0_Ch1_Ovf_IRQHandler                                    /* MPWM0_Ch1_Ovf Handler*/
    .long   MPWM0_Ch2_Ovf_IRQHandler                                    /* MPWM0_Ch2_Ovf Handler*/
    .long   MPWM0_Ch3_Ovf_IRQHandler                                    /* MPWM0_Ch3_Ovf Handler*/
    .long   MPWM0_Ch4_Ovf_IRQHandler                                    /* MPWM0_Ch4_Ovf Handler*/
    .long   MPWM0_Ch5_Ovf_IRQHandler                                    /* MPWM0_Ch5_Ovf Handler*/
    .long   MPWM0_Ch6_Ovf_IRQHandler                                    /* MPWM0_Ch6_Ovf Handler*/
    .long   MPWM0_Ch7_Ovf_IRQHandler                                    /* MPWM0_Ch7_Ovf Handler*/
    .long   MPWM0_Ch0_IRQHandler                                        /* MPWM0_Ch0 Handler*/
    .long   MPWM0_Ch1_IRQHandler                                        /* MPWM0_Ch1 Handler*/
    .long   MPWM0_Ch2_IRQHandler                                        /* MPWM0_Ch2 Handler*/
    .long   MPWM0_Ch3_IRQHandler                                        /* MPWM0_Ch3 Handler*/
    .long   MPWM0_Ch4_IRQHandler                                        /* MPWM0_Ch4 Handler*/
    .long   MPWM0_Ch5_IRQHandler                                        /* MPWM0_Ch5 Handler*/
    .long   MPWM0_Ch6_IRQHandler                                        /* MPWM0_Ch6 Handler*/
    .long   MPWM0_Ch7_IRQHandler                                        /* MPWM0_Ch7 Handler*/
    .long   Reserved80_IRQHandler                                       /* Reserved80 Handler*/
    .long   Reserved81_IRQHandler                                       /* Reserved81 Handler*/
    .long   Reserved82_IRQHandler                                       /* Reserved82 Handler*/
    .long   Reserved83_IRQHandler                                       /* Reserved83 Handler*/
    .long   Reserved84_IRQHandler                                       /* Reserved84 Handler*/
    .long   TRNG_IRQHandler                                             /* TRNG Handler*/
    .long   HCU_IRQHandler                                              /* HCU Handler*/

    .size    __isr_vector, . - __isr_vector

    .text
    .thumb

/* Reset Handler */

    .thumb_func
    .align 2
    .globl   Reset_Handler
    .weak    Reset_Handler
    .type    Reset_Handler, %function
Reset_Handler:
    cpsid   i               /* Mask interrupts */

    /* Init the rest of the registers */
    ldr     r1,=0
    ldr     r2,=0
    ldr     r3,=0
    ldr     r4,=0
    ldr     r5,=0
    ldr     r6,=0
    ldr     r7,=0
    mov     r8,r7
    mov     r9,r7
    mov     r10,r7
    mov     r11,r7
    mov     r12,r7

#ifndef START_NO_ECC_INIT

    /* Init ECC RAM */
    ldr     r1, =__RAM_START
    ldr     r2, =__RAM_END

    subs    r2, r1
    subs    r2, #1
    ble     SRAM_LOOP_END

    movs    r0, #0
    movs    r3, #4
SRAM_LOOP:
    str     r0, [r1]
    add     r1, r1, r3
    subs    r2, #4
    bge     SRAM_LOOP
SRAM_LOOP_END:
#endif
    
    /* Initialize the stack pointer */
    ldr     r0,=__StackTop
    mov     r13,r0

#ifndef __NO_SYSTEM_INIT
    /* Call the system init routine */
    ldr     r0,=SystemInit
    blx     r0
#endif

    /* Init .data and .bss sections */
    ldr     r0,=init_data_bss
    blx     r0
    cpsie   i               /* Unmask interrupts */
    bl      main
JumpToSelf:
    b       JumpToSelf

    .pool
    .size Reset_Handler, . - Reset_Handler

    .align  1
    .thumb_func
    .weak DefaultISR
    .type DefaultISR, %function
DefaultISR:
    b       DefaultISR
    .size DefaultISR, . - DefaultISR

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
    .macro def_irq_handler  handler_name
    .weak \handler_name
    .set  \handler_name, DefaultISR
    .endm

    /* Exception Handlers */
    def_irq_handler    NMI_Handler
    def_irq_handler    HardFault_Handler
    def_irq_handler    MemManage_Handler
    def_irq_handler    BusFault_Handler
    def_irq_handler    UsageFault_Handler
    def_irq_handler    Reserved0_Handler
    def_irq_handler    Reserved1_Handler
    def_irq_handler    Reserved2_Handler
    def_irq_handler    Reserved3_Handler
    def_irq_handler    SVC_Handler
    def_irq_handler    DebugMon_Handler
    def_irq_handler    Reserved4_Handler
    def_irq_handler    PendSV_Handler
    def_irq_handler    SysTick_Handler

    /* Device Exception Handlers */

    def_irq_handler    DMA0_IRQHandler
    def_irq_handler    DMA1_IRQHandler
    def_irq_handler    DMA2_IRQHandler
    def_irq_handler    DMA3_IRQHandler
    def_irq_handler    Reserved5_IRQHandler
    def_irq_handler    Reserved6_IRQHandler
    def_irq_handler    Reserved7_IRQHandler
    def_irq_handler    Reserved8_IRQHandler
    def_irq_handler    Reserved9_IRQHandler
    def_irq_handler    Reserved10_IRQHandler
    def_irq_handler    Reserved11_IRQHandler
    def_irq_handler    Reserved12_IRQHandler
    def_irq_handler    Reserved13_IRQHandler
    def_irq_handler    Reserved14_IRQHandler
    def_irq_handler    Reserved15_IRQHandler
    def_irq_handler    Reserved16_IRQHandler
    def_irq_handler    DMA_Error_IRQHandler
    def_irq_handler    Reserved17_IRQHandler
    def_irq_handler    EFM_IRQHandler
    def_irq_handler    EFM_Error_IRQHandler
    def_irq_handler    PCU_IRQHandler
    def_irq_handler    EFM_Ecc_IRQHandler
    def_irq_handler    WDG0_IRQHandler
    def_irq_handler    Reserved18_IRQHandler
    def_irq_handler    I2C0_Master_IRQHandler
    def_irq_handler    I2C0_Slave_IRQHandler
    def_irq_handler    SPI0_IRQHandler
    def_irq_handler    SPI1_IRQHandler
    def_irq_handler    SPI2_IRQHandler
    def_irq_handler    I2C1_Master_IRQHandler
    def_irq_handler    Reserved19_IRQHandler
    def_irq_handler    UART0_IRQHandler
    def_irq_handler    Reserved20_IRQHandler
    def_irq_handler    UART1_IRQHandler
    def_irq_handler    Reserved21_IRQHandler
    def_irq_handler    UART2_IRQHandler
    def_irq_handler    Reserved22_IRQHandler
    def_irq_handler    Reserved23_IRQHandler
    def_irq_handler    Reserved24_IRQHandler
    def_irq_handler    ADC0_IRQHandler
    def_irq_handler    Reserved25_IRQHandler
    def_irq_handler    ACMP0_IRQHandler
    def_irq_handler    Reserved26_IRQHandler
    def_irq_handler    Reserved27_IRQHandler
    def_irq_handler    EMU0_SB_IRQHandler
    def_irq_handler    EMU0_DB_IRQHandler
    def_irq_handler    Reserved28_IRQHandler
    def_irq_handler    Reserved29_IRQHandler
    def_irq_handler    pTMR0_Ch0_IRQHandler
    def_irq_handler    pTMR0_Ch1_IRQHandler
    def_irq_handler    pTMR0_Ch2_IRQHandler
    def_irq_handler    pTMR0_Ch3_IRQHandler
    def_irq_handler    Reserved30_IRQHandler
    def_irq_handler    Reserved31_IRQHandler
    def_irq_handler    Reserved32_IRQHandler
    def_irq_handler    Reserved33_IRQHandler
    def_irq_handler    Reserved34_IRQHandler
    def_irq_handler    SCU_IRQHandler
    def_irq_handler    lpTMR0_IRQHandler
    def_irq_handler    GPIOA_IRQHandler
    def_irq_handler    GPIOB_IRQHandler
    def_irq_handler    GPIOC_IRQHandler
    def_irq_handler    GPIOD_IRQHandler
    def_irq_handler    GPIOE_IRQHandler
    def_irq_handler    Reserved35_IRQHandler
    def_irq_handler    Reserved36_IRQHandler
    def_irq_handler    Reserved37_IRQHandler
    def_irq_handler    Reserved38_IRQHandler
    def_irq_handler    Reserved39_IRQHandler
    def_irq_handler    Reserved40_IRQHandler
    def_irq_handler    Reserved41_IRQHandler
    def_irq_handler    Reserved42_IRQHandler
    def_irq_handler    Reserved43_IRQHandler
    def_irq_handler    Reserved44_IRQHandler
    def_irq_handler    Reserved45_IRQHandler
    def_irq_handler    Reserved46_IRQHandler
    def_irq_handler    Reserved47_IRQHandler
    def_irq_handler    Reserved48_IRQHandler
    def_irq_handler    CAN0_ORed_IRQHandler
    def_irq_handler    CAN0_Error_IRQHandler
    def_irq_handler    CAN0_Wake_Up_IRQHandler
    def_irq_handler    CAN0_ORed_0_15_MB_IRQHandler
    def_irq_handler    CAN0_ORed_16_31_MB_IRQHandler
    def_irq_handler    CAN0_ORed_32_47_MB_IRQHandler
    def_irq_handler    CAN0_ORed_48_63_MB_IRQHandler
    def_irq_handler    CAN1_ORed_IRQHandler
    def_irq_handler    CAN1_Error_IRQHandler
    def_irq_handler    CAN1_Wake_Up_IRQHandler
    def_irq_handler    CAN1_ORed_0_15_MB_IRQHandler
    def_irq_handler    CAN1_ORed_16_31_MB_IRQHandler
    def_irq_handler    CAN1_ORed_32_47_MB_IRQHandler
    def_irq_handler    CAN1_ORed_48_63_MB_IRQHandler
    def_irq_handler    Reserved49_IRQHandler
    def_irq_handler    Reserved50_IRQHandler
    def_irq_handler    Reserved51_IRQHandler
    def_irq_handler    Reserved52_IRQHandler
    def_irq_handler    Reserved53_IRQHandler
    def_irq_handler    Reserved54_IRQHandler
    def_irq_handler    Reserved55_IRQHandler
    def_irq_handler    eTMR0_Ch0_Ch1_IRQHandler
    def_irq_handler    eTMR0_Ch2_Ch3_IRQHandler
    def_irq_handler    eTMR0_Ch4_Ch5_IRQHandler
    def_irq_handler    eTMR0_Ch6_Ch7_IRQHandler
    def_irq_handler    eTMR0_Fault_IRQHandler
    def_irq_handler    eTMR0_Ovf_IRQHandler
    def_irq_handler    eTMR1_Ch0_Ch1_IRQHandler
    def_irq_handler    eTMR1_Ch2_Ch3_IRQHandler
    def_irq_handler    eTMR1_Ch4_Ch5_IRQHandler
    def_irq_handler    eTMR1_Ch6_Ch7_IRQHandler
    def_irq_handler    eTMR1_Fault_IRQHandler
    def_irq_handler    eTMR1_Ovf_IRQHandler
    def_irq_handler    Reserved56_IRQHandler
    def_irq_handler    Reserved57_IRQHandler
    def_irq_handler    Reserved58_IRQHandler
    def_irq_handler    Reserved59_IRQHandler
    def_irq_handler    Reserved60_IRQHandler
    def_irq_handler    Reserved61_IRQHandler
    def_irq_handler    Reserved62_IRQHandler
    def_irq_handler    Reserved63_IRQHandler
    def_irq_handler    Reserved64_IRQHandler
    def_irq_handler    Reserved65_IRQHandler
    def_irq_handler    Reserved66_IRQHandler
    def_irq_handler    Reserved67_IRQHandler
    def_irq_handler    Reserved68_IRQHandler
    def_irq_handler    Reserved69_IRQHandler
    def_irq_handler    Reserved70_IRQHandler
    def_irq_handler    Reserved71_IRQHandler
    def_irq_handler    Reserved72_IRQHandler
    def_irq_handler    Reserved73_IRQHandler
    def_irq_handler    Reserved74_IRQHandler
    def_irq_handler    Reserved75_IRQHandler
    def_irq_handler    Reserved76_IRQHandler
    def_irq_handler    Reserved77_IRQHandler
    def_irq_handler    Reserved78_IRQHandler
    def_irq_handler    Reserved79_IRQHandler
    def_irq_handler    MPWM0_Ch0_Ovf_IRQHandler
    def_irq_handler    MPWM0_Ch1_Ovf_IRQHandler
    def_irq_handler    MPWM0_Ch2_Ovf_IRQHandler
    def_irq_handler    MPWM0_Ch3_Ovf_IRQHandler
    def_irq_handler    MPWM0_Ch4_Ovf_IRQHandler
    def_irq_handler    MPWM0_Ch5_Ovf_IRQHandler
    def_irq_handler    MPWM0_Ch6_Ovf_IRQHandler
    def_irq_handler    MPWM0_Ch7_Ovf_IRQHandler
    def_irq_handler    MPWM0_Ch0_IRQHandler
    def_irq_handler    MPWM0_Ch1_IRQHandler
    def_irq_handler    MPWM0_Ch2_IRQHandler
    def_irq_handler    MPWM0_Ch3_IRQHandler
    def_irq_handler    MPWM0_Ch4_IRQHandler
    def_irq_handler    MPWM0_Ch5_IRQHandler
    def_irq_handler    MPWM0_Ch6_IRQHandler
    def_irq_handler    MPWM0_Ch7_IRQHandler
    def_irq_handler    Reserved80_IRQHandler
    def_irq_handler    Reserved81_IRQHandler
    def_irq_handler    Reserved82_IRQHandler
    def_irq_handler    Reserved83_IRQHandler
    def_irq_handler    Reserved84_IRQHandler
    def_irq_handler    TRNG_IRQHandler
    def_irq_handler    HCU_IRQHandler

    .end