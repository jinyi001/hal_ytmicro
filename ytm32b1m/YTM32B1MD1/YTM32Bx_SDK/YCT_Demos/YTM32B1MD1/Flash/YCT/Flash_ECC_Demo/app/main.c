/* USER CODE BEGIN Header */
/* you can remove the copyright */

/*
 *  Copyright 2020-2023 Yuntu Microelectronics Co., Ltd.
 *  All rights reserved.
 * 
 *  SPDX-License-Identifier: BSD-3-Clause
 * 
 * @file main.c
 * @brief 
 * 
 */

/* USER CODE END Header */
#include "sdk_project_config.h"
/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define FLASH_INST (0U)

#define TEST_DATA_START_ADDR (0x18000U)
#define TEST_DATA_END_ADDR   (0x188FFU)
#define TEST_ECC_INJECT_ADDR (0x18400U)
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint32_t g_sw2_pressed = 1;
volatile uint32_t g_sw3_pressed = 1;
volatile bool selfTestDone = false;
volatile bool EccDemoError = true;
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void GPIOB_IRQHandler(void)
{
    if (PINS_DRV_GetPortIntFlag(GPIOB) & 0x4)
    {
        PINS_DRV_ClearPinIntFlagCmd(GPIOB, 2);
        g_sw2_pressed = 1;
    }
}

void GPIOC_IRQHandler(void)
{
    if (PINS_DRV_GetPortIntFlag(GPIOC) & 0x2)
    {
        PINS_DRV_ClearPinIntFlagCmd(GPIOC, 1);
        g_sw3_pressed = 1;
    }
}

void EFM_RecoveryErrorHandler(uint32_t error_addr)
{
    uint32_t data[FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE / 4] = { 0 };
    uint32_t error_sector_addr = error_addr & ~(FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE - 1);

    /* Note: Debug output inserted into interrupt routine for demo clarity. Might introduce delay. */
    PRINTF("Flash ECC Recovery Error Handler.\r\n");
    PRINTF("Ecc Error Address: 0x%x\r\n", error_addr);

    /* Clear ECC error flag, EFM->ECCERR_ADDR will be cleared at the same time. */
    EFM->STS = EFM_STS_RECOVERR_MASK;

    /* Decide whether to erase and reprogram based on the specific application scenario.
       Note: It would be powered-off during the reprogramming process. */
    if (error_addr > TEST_DATA_START_ADDR && error_addr < TEST_DATA_END_ADDR)
    {
        for (uint32_t i = 0; i < FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE / 4; i++)
        {
            data[i] = *(volatile uint32_t *)(error_sector_addr + i * 4);
        }

        /* The sector containing the 1-bit ECC error address can be erased and reprogrammed to fix the error. */
        FLASH_DRV_EraseSector(FLASH_INST, error_sector_addr, FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE);
        FLASH_DRV_Program(FLASH_INST, error_sector_addr, FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE, (uint32_t *)&data);
        PRINTF("Erased and reprogrammed the sector containing the ECC error address.\r\n");
    }
    else
    {
        // Do nothing for the location of the program
    }
}

void EFM_UnrecoveryErrorHandler(void)
{
    uint32_t error_addr = EFM->ERR_ADDR;
    PRINTF("Flash ECC Un-recovery Error Handler.\r\n");
    PRINTF("Ecc Error Address(EFM->ERR_ADDR): 0x%x\r\n", error_addr);

    /* Clear ECC error flag, EFM->ERR_ADDR will be cleared at the same time. */
    EFM->STS = EFM_STS_UNRECOVERR_MASK;

    /* The sector containing the ECC error address MUST be erased to fix the error. */
    if (error_addr > TEST_DATA_START_ADDR && error_addr < TEST_DATA_END_ADDR)
    {
        FLASH_DRV_EraseSector(FLASH_INST,
                              error_addr & ~(FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE - 1),
                              FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE);
        PRINTF("Erased the sector containing the ECC error address.\r\n");
    }
    else
    {
        // Do nothing for the location of the program
    }
}

void EFM_ECC_IRQHandler(void)
{
    volatile uint32_t read_data = 0;
    (void)read_data;
    if (EFM->STS & EFM_STS_UNRECOVERR_MASK)
    {
        EFM_UnrecoveryErrorHandler();
    }
    if (EFM->STS & EFM_STS_RECOVERR_MASK)
    {
        /* Due to ERR_ADDR register only show the unrecovry error, traverse to find the error address. */
        EFM->STS = EFM_STS_RECOVERR_MASK;
        for (uint32_t addr = TEST_DATA_START_ADDR; addr < TEST_DATA_END_ADDR; addr += 4)
        {
            read_data = *(volatile uint32_t *)(addr);
            if (EFM->STS & EFM_STS_RECOVERR_MASK)
            {
                EFM_RecoveryErrorHandler(addr);
            }
        }
    }
}

void Flash_ECC_Inject(uint32_t addr)
{
    uint32_t pre_timing2;
    pre_timing2 = EFM->TIMING2;
    FLASH_DRV_EraseSector(FLASH_INST,
                          addr & ~(FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE - 1),
                          FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE);
    FLASH_DRV_Program(FLASH_INST, addr, FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE, (const void *)0x20000000);
    /* It's only for test by modify timing. MUST NOT use in production environment. */
    EFM->TIMING2 = 0x20;
    FLASH_DRV_EraseSector(FLASH_INST,
                          addr & ~(FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE - 1),
                          FEATURE_EFM_MAIN_ARRAY_SECTOR_SIZE);
    EFM->TIMING2 = pre_timing2;
}

uint32_t FLASH_ECC_Scan(uint32_t start_addr, uint32_t end_addr)
{
    volatile uint32_t read_data = 0;
    for (uint32_t addr = start_addr; addr < end_addr; addr += 4)
    {
        read_data = *(volatile uint32_t *)(addr);
    }
    /* Only for self test */
    EccDemoError = false;
    selfTestDone = true;
    return read_data;
}

/* USER CODE END 0 */


/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */
    status_t status = STATUS_SUCCESS;

    /* USER CODE END 1 */ 
    Board_Init();
    /* USER CODE BEGIN 2 */
    PRINTF("Build %s %s\n", __DATE__, __TIME__);
    PRINTF("Flash ECC demo!\r\n");

    /* YTM32B1MD1 Flash ECC Unrecovery Error would not cause the HardFault!
    So, enable the EFM ECC Unrecovery Error interrupt. */
    INT_SYS_InstallHandler(EFM_Ecc_IRQn, EFM_ECC_IRQHandler, NULL);
    FLASH_DRV_EnableSingleBitFaultInterrupt(FLASH_INST);
    FLASH_DRV_EnableDoubleBitFaultInterrupt(FLASH_INST);
    NVIC_EnableIRQ(EFM_Ecc_IRQn);

    NVIC_EnableIRQ(GPIOB_IRQn);
    NVIC_EnableIRQ(GPIOC_IRQn);

    PRINTF("This Demo will inject ecc error and scan it at first time.\n");

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        if (status != STATUS_SUCCESS)
        {
            break;
        }
        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */
        if (g_sw2_pressed)
        {
            g_sw2_pressed = 0;
            PRINTF("=> SW2 pressed!\r\n");
            PRINTF("Inject the ECC Error @ 0x%x!\r\n", TEST_ECC_INJECT_ADDR);
            Flash_ECC_Inject(TEST_ECC_INJECT_ADDR);
        }

        if (g_sw3_pressed)
        {
            g_sw3_pressed = 0;
            PRINTF("=> SW3 pressed!\r\n");
            PRINTF("Scan the ECC Error!\r\n");
            FLASH_ECC_Scan(TEST_DATA_START_ADDR, TEST_DATA_END_ADDR);
        }

        /* Only for self test */
        if(selfTestDone)
        {
            selfTestDone = false;
            if(EccDemoError)
            {
                PRINTF("=> Flash Ecc Demo Fail!\r\n");
            }else{
                PRINTF("=> Flash Ecc Demo Pass!\r\n");
            }
            PRINTF("=> Flash Ecc Demo End!\r\n");
        }
    }
    return status;
    /* USER CODE END 3 */
}

static void Board_Init(void)
{
    CLOCK_SYS_Init(g_clockManConfigsArr,CLOCK_MANAGER_CONFIG_CNT,g_clockManCallbacksArr,CLOCK_MANAGER_CALLBACK_CNT);
    if(STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0,g_pin_mux_InitConfigArr0);
    UTILITY_PRINT_Init();
    FLASH_DRV_Init(0,&flash_config0,&flash_config0_State);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
