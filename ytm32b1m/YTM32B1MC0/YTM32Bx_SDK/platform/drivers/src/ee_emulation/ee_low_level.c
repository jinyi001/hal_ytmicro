/*
 * Copyright 2020-2022 Yuntu Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ee_emulation.h"
#include "ee_hardware.h"
#include "interrupt_manager.h"

START_FUNCTION_DECLARATION_RAMSECTION
status_t Eed_FlashLaunchCommand(ee_callback_t p_call_back, uint8_t command) __attribute__((noinline))
END_FUNCTION_DECLARATION_RAMSECTION
/******************************************************************************
Function Name    : Eed_FlashErase
Description      : This function is to erase a flash sector synchronously.
Arguments        :
                   - dest: address of sector.
Return Value     :
                   - STATUS_SUCCESS: erase sector successfully.
                   - not STATUS_SUCCESS: erase sector unsuccessfully.
*******************************************************************************/
status_t Eed_FlashErase(uint32_t dest)
{
    INT_SYS_DisableIRQGlobal();
    EFM_ENABLE_WE_COMMAND();
    /* load erase block address registers */
    *(uint32_t *)dest = dest;
    EFM_DISABLE_WE_COMMAND();
    INT_SYS_EnableIRQGlobal();

    /* launch flash command */
    Eed_FlashLaunchCommand(p_gEECallBack, FEATURE_EFM_ERASE_SECTOR_CMD_CODE);
    /* need to verify section here */
    return (Eed_FlashEraseVerify(dest, EE_SECTOR_SIZE >> 0x2U));
}

/******************************************************************************
Function Name    : Eed_FlashEraseVerify
Description      : This function is to erase verify for given number of long word.
Arguments        :
                   - dest: address of sector.
                   - number: number of long word to be verified.
Return Value     :
                   - STATUS_SUCCESS: section is fully blanked.
                   - not STATUS_SUCCESS: section is not blanked.
*******************************************************************************/
status_t Eed_FlashEraseVerify(uint32_t dest, uint16_t number)
{
    status_t ret_val = STATUS_SUCCESS;
    uint32_t * flash_dest = (uint32_t *) dest;
    for(int i = 0; i < number; i++)
    {
        if (0xFFFFFFFFU != flash_dest[i])
        {
            ret_val = STATUS_EdPROM_PROG_VERIFY_ERROR;
            break;
        }
    }
    return ret_val;
}

/******************************************************************************
Function Name    : Eed_FlashProgram
Description      : This function is to program data to flash.
Arguments        :
                   - dest: address of sector.
                   - size: size to be programmed.
                   - source: source data to be programmed.
Return Value     :
                   - STATUS_SUCCESS: program data successfully.
                   - STATUS_EdPROM_PROG_VERIFY_ERROR: program data unsuccessfully.
*******************************************************************************/
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

        /* verify again by normal read*/
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

/******************************************************************************
Function Name    : Eed_SynFlashLaunchCommand
Description      : This function is to launch flash command.
Arguments        : None.
Return Value     : None.
*******************************************************************************/
START_FUNCTION_DEFINITION_RAMSECTION
status_t Eed_FlashLaunchCommand(const ee_callback_t p_call_back, uint8_t command)
{
    status_t ret = STATUS_SUCCESS;    /* Return code variable */
    /* Clear access error flag in flash status register. Write 1 to clear */
    EFM->STS = FEATURE_EFM_CMD_ERROR_MASK;
#if (EE_SYS_GLOBAL_IRQ_DISABLE)
    DISABLE_INTERRUPTS();
#endif 
    EFM_UNLOCK_CMD_REGISTER(EFM);
    /* Write command register to launch command */
    EFM->CMD = command;
    while (0U == (EFM->STS & EFM_STS_IDLE_MASK))
    {
        /* Wait till IDLE bit is set
         * Serve callback function as often as possible
         */
        if (EE_NULL_CALLBACK != p_call_back)
        {
            /* Temporarily disable compiler's check for ROM access call from within a ram function.
             * The use of a function pointer type makes this check irrelevant.
             * Nevertheless, it is imperative that the user-provided callback be defined in RAMSECTION */
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
