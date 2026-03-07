{{ytCopyright "RamInit1.c"}}
#include <stdint.h>
#include <stdbool.h>
#include "device_registers.h"

{{setVar2 'level1' (createArray)~}}
{{#each this.data~}}
{{#if this.children~}}
{{#each (sortBy this.children (createArray 'order' 'name'))~}}
{{push2 @root.level1 this~}}
{{/each~}}
{{/if~}}
{{/each~}}
{{setVar2 'sections' (createObject)~}}
{{#each @root.level1~}}
{{setVar2 'parent' this~}}
{{#if this.children~}}
{{#each this.children~}}
{{#if this.children~}}
{{#each this.children~}}
{{set2 this 'parent' @root.parent~}}
{{set2 @root.sections this.id this~}}
{{/each~}}
{{/if}}
{{/each~}}
{{/if}}
{{/each~}}

#define WORD_ALIGN_MASK (3U)
#define WORD_SHIFT      (2U)

typedef enum
{
    INIT_NULL = 0U,   /*!< No initialization */
    INIT_NORMAL = 1U, /*!< Normal initialization */
    INIT_POR = 2U,    /*!< Power on reset initialization */
} RamInitType;


typedef struct
{
    uint32_t *RamStart;   /*!< Start address of section in RAM */
    uint32_t *RomStart;   /*!< Start address of section in ROM */
    uint32_t *RomEnd;     /*!< End address of section in ROM */
    RamInitType InitType;   /*!< Type of initialization*/
} RamCopyLayoutType;

typedef struct
{
    uint32_t *RamStart;   /*!< Start address of section in RAM */
    uint32_t *RamEnd;     /*!< End address of section in RAM */
    RamInitType InitType;   /*!< Type of initialization*/
} RamZeroLayoutType;

{{setVar2 'copyNum' 0~}}
{{setVar2 'clearNum' 0~}}


{{#each @root.sections}}
{{#if this.copyFrom}}
{{setVar2 'copyNum' (add @root.copyNum 1)~}}
{{#if (or (eq (getProjectInfo 'toolchain') 'CMakeKEIL') (eq (getProjectInfo 'toolchain') 'KEIL'))}}
extern uint32_t Image$${{this.name}}_start$$Base;
extern uint32_t Load$${{this.name}}_start$$Base;
extern uint32_t Load$${{this.name}}_end$$Limit;
{{else}}
extern uint32_t {{this.name}}_ram_start[1U];
extern uint32_t {{this.name}}_rom_start[1U];
extern uint32_t {{this.name}}_rom_end[1U];
{{/if}}
{{/if}}
{{/each}}

{{#each @root.sections}}
{{#if this.clear}}
{{setVar2 'clearNum' (add @root.clearNum 1)~}}
{{#if (or (eq (getProjectInfo 'toolchain') 'CMakeKEIL') (eq (getProjectInfo 'toolchain') 'KEIL'))}}
extern uint32_t Image$${{this.name}}_start$$Base;
extern uint32_t Image$${{this.name}}_end$$Limit;
{{else}}
extern uint32_t {{this.name}}_start[1U];
extern uint32_t {{this.name}}_end[1U];
{{/if}}
{{/if}}
{{/each}}

{{#if @root.copyNum}}
const RamCopyLayoutType CopyLayout[{{@root.copyNum}}]={
{{#each @root.sections}}
{{#if this.copyFrom}}
    {
{{#if (or (eq (getProjectInfo 'toolchain') 'CMakeKEIL') (eq (getProjectInfo 'toolchain') 'KEIL'))}}
        .RamStart=(uint32_t *)&Image$${{this.name}}_start$$Base,
        .RomStart=(uint32_t *)&Load$${{this.name}}_start$$Base,
        .RomEnd=(uint32_t *)&Load$${{this.name}}_end$$Limit,
        .InitType=INIT_{{this.parent.init}},
{{else}}
        .RamStart={{this.name}}_ram_start,
        .RomStart={{this.name}}_rom_start,
        .RomEnd={{this.name}}_rom_end,
        .InitType=INIT_{{this.parent.init}},
{{/if}}
    },
{{/if}}
{{/each}}
};
{{/if}}

{{#if @root.clearNum}}
const RamZeroLayoutType ZeroLayout[{{@root.clearNum}}]={
{{#each @root.sections}}
{{#if this.clear}}
    {
{{#if (or (eq (getProjectInfo 'toolchain') 'CMakeKEIL') (eq (getProjectInfo 'toolchain') 'KEIL'))}}
        .RamStart=(uint32_t *)&Image$${{this.name}}_start$$Base,
        .RamEnd=(uint32_t *)&Image$${{this.name}}_end$$Limit,
        .InitType=INIT_{{this.parent.init}},
{{else}}
        .RamStart={{this.name}}_start,
        .RamEnd={{this.name}}_end,
        .InitType=INIT_{{this.parent.init}},
{{/if}}
    },
{{/if}}
{{/each}}
};
{{/if}}


/**
 * @brief Check MCU is power on reset or not, user can override this function
 * @return 1: POR, 0: Not POR
 */
#define RCU_RSSR_POR_MASK   (0x1U)
static bool IsPOR(void)
{
    bool RetVal = false;
    /* Check if the MCU is power on reset or not */
    if (RCU_RSSR_POR_MASK == (RCU->RSSR & RCU_RSSR_POR_MASK))
    {
        RetVal = true;
    }

    return RetVal;
}


/**
 * @brief RamInit1 for copying initialized data and zeroing uninitialized data
 */
void RamInit1(){
    bool IsPor = IsPOR();
{{#if (or @root.copyNum @root.clearNum)}}
{{#if @root.copyNum}}
    const uint32_t *Rom32;
    const uint8_t *Rom8;
{{/if}}
    uint32_t *Ram32;
    uint8_t *Ram8;
    volatile uint8_t DataPad;
    volatile uint32_t Len = 0U;
    uint32_t Size = 0U;
    uint32_t MajorLoopCnt = 0U;
    uint32_t MinorLoopCnt = 0U;
{{/if}}

{{#if @root.copyNum}}
    /* Copy initialized table */
    Len = {{@root.copyNum}};
    for (MajorLoopCnt = 0U; MajorLoopCnt < Len; ++MajorLoopCnt)
    {
        if(((CopyLayout[MajorLoopCnt].InitType == INIT_POR) && (IsPor != true)) || (CopyLayout[MajorLoopCnt].InitType == INIT_NULL))
        {
            continue;
        }
        Rom32 = (const uint32_t *)CopyLayout[MajorLoopCnt].RomStart;
        Ram32 = (uint32_t *)CopyLayout[MajorLoopCnt].RamStart;
        Size = (uint32_t)CopyLayout[MajorLoopCnt].RomEnd - (uint32_t)CopyLayout[MajorLoopCnt].RomStart;
        /* Make sure the data area to be copied must be aligned with 4. Then, copy 4 bytes at per one read */
        DataPad = Size & WORD_ALIGN_MASK;
        Size = (Size - DataPad) >> WORD_SHIFT;
        for (MinorLoopCnt = 0U; MinorLoopCnt < Size; ++MinorLoopCnt)
        {
            Ram32[MinorLoopCnt] = Rom32[MinorLoopCnt];
        }
        /* For the rest of data, copy 1 bytes at per one read */
        Rom8 = (uint8_t *)(&Rom32[MinorLoopCnt]);
        Ram8 = (uint8_t *)(&Ram32[MinorLoopCnt]);
        for (MinorLoopCnt = 0U; MinorLoopCnt < DataPad; ++MinorLoopCnt)
        {
            Ram8[MinorLoopCnt] = Rom8[MinorLoopCnt];
        }
    }
{{/if}}

{{#if @root.clearNum}}
    /* Clear zero table */
    Len = {{@root.clearNum}};
    for (MajorLoopCnt = 0U; MajorLoopCnt < Len; ++MajorLoopCnt)
    {
        if(((ZeroLayout[MajorLoopCnt].InitType == INIT_POR) && (IsPor != true)) || (ZeroLayout[MajorLoopCnt].InitType == INIT_NULL))
        {
            continue;
        }
        Ram32 = (uint32_t *)ZeroLayout[MajorLoopCnt].RamStart;
        Size = (uint32_t)ZeroLayout[MajorLoopCnt].RamEnd - (uint32_t)ZeroLayout[MajorLoopCnt].RamStart;
        /* Make sure the data area to be zeroed must be aligned with 4. Then, zero 4 bytes at per one read */
        DataPad = Size & WORD_ALIGN_MASK;
        Size = (Size - DataPad) >> WORD_SHIFT;        
        for (MinorLoopCnt = 0U; MinorLoopCnt < Size; ++MinorLoopCnt)
        {
            Ram32[MinorLoopCnt] = 0U;
        }
        /* For the rest of data, zero 1 bytes at per one read */
        Ram8 = (uint8_t *)(&Ram32[MinorLoopCnt]);
        for (MinorLoopCnt = 0U; MinorLoopCnt < DataPad; ++MinorLoopCnt)
        {
            Ram8[MinorLoopCnt] = 0U;
        }
    }
{{/if}}
}


