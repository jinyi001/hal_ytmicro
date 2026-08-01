/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * @file ee_low_level.c
 * @version 1.4.1
 *
 * @brief EEPROM Emulation low-level Flash access implementation.
 */

#include "ee_emulation.h"
#include "ee_hardware.h"
#include "interrupt_manager.h"

/*!
 * @brief RAM-resident helper that launches one EFM command and services the
 *        optional callback while Flash is busy.
 */
START_FUNCTION_DECLARATION_RAMSECTION
status_t Eed_FlashLaunchCommand(ee_callback_t p_call_back, uint8_t command) __attribute__((noinline))
END_FUNCTION_DECLARATION_RAMSECTION

/*******************************************************************************
 * Flash Access
 ******************************************************************************/

/*!
 * @brief Erase one Flash sector and verify that it is blank.
 */
status_t Eed_FlashErase(uint32_t dest)
{
    INT_SYS_DisableIRQGlobal();
    EFM_ENABLE_WE_COMMAND();
    /* Load the destination sector address into the EFM command buffer. */
    *(uint32_t *)dest = dest;
    EFM_DISABLE_WE_COMMAND();
    INT_SYS_EnableIRQGlobal();

    Eed_FlashLaunchCommand(p_gEECallBack, FEATURE_EFM_ERASE_SECTOR_CMD_CODE);
    return (Eed_FlashEraseVerify(dest, EE_SECTOR_SIZE >> 0x2U));
}

/*!
 * @brief Verify that a Flash region contains only erased long words.
 */
status_t Eed_FlashEraseVerify(uint32_t dest, uint16_t number)
{
    status_t ret_val = STATUS_SUCCESS;
    uint32_t * flash_dest = (uint32_t *) dest;
    for (int i = 0; i < number; i++)
    {
        if (0xFFFFFFFFU != flash_dest[i])
        {
            ret_val = STATUS_EdPROM_PROG_VERIFY_ERROR;
            break;
        }
    }
    return ret_val;
}

/*!
 * @brief Program one Flash region and verify the written data.
 */
/*lint -e{931} */
status_t Eed_FlashProgram(uint32_t dest, uint8_t size, uint32_t source)
{
    status_t ret_val = STATUS_SUCCESS;
    uint32_t val_dest;
    uint32_t val_src;
    uint8_t temp;
    uint8_t i;

    while (size > 0U)
    {
        temp = FEATURE_EFM_WRITE_UNIT_SIZE - (size & (FEATURE_EFM_WRITE_UNIT_SIZE - 0x1U));

        INT_SYS_DisableIRQGlobal();
        EFM_ENABLE_WE_COMMAND();
        for (i = 0U; i < temp; i += 4U)
        {
            *(uint32_t *) (dest + i) = READ32(source + i);
        }
        EFM_DISABLE_WE_COMMAND();
        INT_SYS_EnableIRQGlobal();

        Eed_FlashLaunchCommand(p_gEECallBack, FEATURE_EFM_PROGRAM_CMD_CODE);

        /* Read back each programmed write unit before advancing the source and destination pointers. */
        for (i = 0U; i < temp; i += 4U)
        {
            val_dest = READ32(dest + i);
            val_src = READ32(source + i);
            if (val_dest != val_src)
            {
                ret_val = STATUS_EdPROM_PROG_VERIFY_ERROR;
            }
        }
        size -= temp;
        source += temp;
        dest += temp;
    }
    return (ret_val);
}

/*!
 * @brief Launch one EFM command and poll until the controller returns to
 *        idle.
 */
START_FUNCTION_DEFINITION_RAMSECTION
status_t Eed_FlashLaunchCommand(const ee_callback_t p_call_back, uint8_t command)
{
    status_t ret = STATUS_SUCCESS;

    /* Clear stale command-status flags before issuing the next EFM command. */
    EFM->STS = FEATURE_EFM_CMD_STS_CLEAR_MASK;
#if (EE_SYS_GLOBAL_IRQ_DISABLE)
    DISABLE_INTERRUPTS();
#endif
    EFM_UNLOCK_CMD_REGISTER(EFM);
    EFM->CMD = command;
    while (0U == (EFM->STS & EFM_STS_IDLE_MASK))
    {
        /* Poll until the controller becomes idle, servicing the optional callback if enabled. */
        if (EE_NULL_CALLBACK != p_call_back)
        {
            /* The callback must remain callable from RAM while Flash is busy. */
            DISABLE_CHECK_RAMSECTION_FUNCTION_CALL
            (p_call_back)();
            ENABLE_CHECK_RAMSECTION_FUNCTION_CALL
        }
    }
#if (EE_SYS_GLOBAL_IRQ_DISABLE)
    ENABLE_INTERRUPTS();
#endif
    if ( (FEATURE_EFM_CMD_ERROR_MASK & EFM->STS) != 0U)
    {
        ret = STATUS_ERROR;
    }
    return ret;
}
END_FUNCTION_DEFINITION_RAMSECTION
