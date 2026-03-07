{{ytCopyright "RamInit0.S"}}
{{setVar2 'level1' (createArray)~}}
{{#each this.data~}}
{{#if this.children~}}
{{#each (sortBy this.children (createArray 'order' 'name'))~}}
{{#if this.isFlash}}
{{else}}
{{push2 @root.level1 this~}}
{{/if}}
{{/each~}}
{{/if~}}
{{/each}}

   PRESERVE8
   THUMB
   AREA    |.text|, CODE, READONLY
RamInit0
   EXPORT  RamInit0
_GET_POR
    LDR     R1, ={{getGlobalVar 'meta.RCU_RSSR_ADDR'}}
    LDR     R4, [R1]
    LDR     R3, ={{getGlobalVar 'meta.RCU_RSSR_POR_MASK'}}
/* R4 store POR result */
    ANDS    R4, R3
{{#if (compare (occurrences (getProjectInfo 'partNumber') 'YTM32B1H') '>' 0)}}
#define ITCMCR  (0xE000EF90)
#define DTCMCR  (0xE000EF94)

/* Enable ITCM and DTCM with ECC support */
_TCM_Enable
    LDR     R11, =ITCMCR
    LDR     R0, [R11]
    ORR     R0, R0, #0x7
    STR     R0, [R11]
    LDR     R11, =DTCMCR
    LDR     R0, [R11]
    ORR     R0, R0, #0x7
    STR     R0, [R11]
{{/if}}   
{{#each @root.level1}}
{{#if (not (eq this.init 'NULL'))}} 
{{#if (eq this.init 'POR')}}
    CMP     R4, #0
    BEQ     _{{this.name}}_end
{{/if}}
_{{this.name}}_start
{{#if (eq (modulo (Number this.size) 8) 0)}}
/* 8bytes align */
    LDR     R1, ={{this.startAddress}}
    LDR     R2, =({{this.startAddress}}+{{this.size}})
    
    SUBS    R2, R2, R1
    SUBS    R2, #1
    BLE     _{{this.name}}_end

    LDR     R0, =0x5A5A5A5A
    LDR     R3, =0x5A5A5A5A

_{{this.name}}_loop
    STM     R1!, {R0,R3}
    SUBS    R2, #8
    BGE      _{{this.name}}_loop

_{{this.name}}_end

{{else}}
{{#if (eq (modulo (Number this.size) 4) 0)}}
/* 4bytes align */
    LDR     R1, ={{this.startAddress}}
    LDR     R2, =({{this.startAddress}}+{{this.size}})
    
    SUBS    R2, R2, R1
    SUBS    R2, #1
    BLE     _{{this.name}}_end
    LDR     R0, =0x5A5A5A5A

_{{this.name}}_loop
    STR     R0, [R1], #4
    SUBS    R2, #4
    BGE      _{{this.name}}_loop

_{{this.name}}_end

{{else}}
{{#if (eq (modulo (Number this.size) 2) 0)}}
/* 2bytes align */
    LDR     R1, ={{this.startAddress}}
    LDR     R2, =({{this.startAddress}}+{{this.size}})
    
    SUBS    R2, R2, R1
    SUBS    R2, #1
    BLE     _{{this.name}}_end

    LDR     R0, =0x5A5A

_{{this.name}}_loop
    STRH    R0, [R1], #2
    SUBS    R2, #2
    BGE      _{{this.name}}_loop

_{{this.name}}_end
{{else}}
/* 1bytes align */
    LDR     R1, ={{this.startAddress}}
    LDR     R2, =({{this.startAddress}}+{{this.size}})
    
    SUBS    R2, R2, R1
    SUBS    R2, #1
    BLE     _{{this.name}}_end

    LDR     R0, =0x5A

_{{this.name}}_loop
    STRB    R0, [R1], #1
    SUBS    R2, #1
    BGE      _{{this.name}}_loop

_{{this.name}}_end
{{/if}}
{{/if}}
{{/if}}
{{/if}}
{{/each}}
   BX LR
   ALIGN 4
   END

