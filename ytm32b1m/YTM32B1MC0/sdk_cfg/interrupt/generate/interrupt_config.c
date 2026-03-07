{{ytCopyright "interrupt_config.c"}}

#include <stddef.h>
#include "interrupt_config.h"

{{#each (filter this.data true property="enable")}}
{{#isTruthy this.newCallback}}
extern void {{this.newCallback}}(void);
{{/isTruthy}}
{{/each}}

void INT_SYS_ConfigInit(void)
{
    INT_SYS_DisableIRQGlobal();
{{#each (filter this.data true property="enable")}}
    /* {{this.name}}({{this.num}}) {{this.callback}}*/
    INT_SYS_SetPriority({{this.name}},{{this.priority}});
    INT_SYS_EnableIRQ({{this.name}});
{{#if this.newCallback}}
    INT_SYS_InstallHandler({{this.name}},{{this.newCallback}},NULL);
{{/if}}
{{/each}}
    INT_SYS_EnableIRQGlobal();
}

