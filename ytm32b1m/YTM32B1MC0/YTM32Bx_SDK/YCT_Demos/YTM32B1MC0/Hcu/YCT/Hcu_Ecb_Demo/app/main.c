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
#include "printf.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
const uint32_t key[4] = { 0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c };
const uint32_t plainText[16] = { 0x6bc1bee2, 0x2e409f96, 0xe93d7e11, 0x7393172a, 0xae2d8a57, 0x1e03ac9c,
                                 0x9eb76fac, 0x45af8e51, 0x30c81c46, 0xa35ce411, 0xe5fbc119, 0x1a0a52ef,
                                 0xf69f2445, 0xdf4f9b17, 0xad2b417b, 0xe66c3710 };
const uint32_t cipherText[16] = { 0xC48D5784, 0xF6BB1688, 0xA1F6AC18, 0xEEDAB413, 0x39CBB8F3, 0x7FAB8C25,
                                  0x3B1C1E50, 0x8C53E9D0, 0xDF273015, 0x3603F913, 0xD4298D27, 0xBB91F6FA,
                                  0xF95B31D0, 0x9053DFF1, 0x214A42EF, 0xF22A080D };
uint32_t result[16];
aes_algorithm_config_t encryptCfg = {
    .dataOutputPtr = result,
    .dataInputPtr = plainText,
    .msgLen = 64,
    .swKeyPtr = key,
    .hwKeySelected = false,
    .keySize = KEY_SIZE_128_BITS,
};
/* USER CODE END PV */

/* Private function declare --------------------------------------------------*/
/* USER CODE BEGIN PFDC */
/* USER CODE END PFDC */
static void Board_Init(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
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
    PRINTF("Hello ECB demo!\r\n");

    PRINTF("ECB Encrypt using 128bit key test. \r\n");
    status |= HCU_DRV_EncryptECB(&encryptCfg);
    /* Check result if equal cipherText */
    if (0 != memcmp((const uint8_t *)cipherText, (const uint8_t *)result, sizeof(cipherText)))
    {
        PRINTF("ECB Encrypt Fail! \r\n");
    }
    else
    {
        PRINTF("ECB Encrypt Pass! \r\n");
    }
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
    }
    return status;
    /* USER CODE END 3 */
}

static void Board_Init(void)
{
    CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT, g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    if(STATUS_SUCCESS != CLOCK_SYS_UpdateConfiguration(CLOCK_MANAGER_ACTIVE_INDEX,CLOCK_MANAGER_POLICY_AGREEMENT))
    {
        /* USER CODE BEGIN ERROR_HANDLER 1 */
        SystemSoftwareReset();
        /* USER CODE END ERROR_HANDLER 1 */
    }
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    UTILITY_PRINT_Init();
    FLASH_DRV_Init(0, &flash_config0, &flash_config0_State);
    HCU_DRV_Init(&hcu_config0, &hcu_config0_State);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
