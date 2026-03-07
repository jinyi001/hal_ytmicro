{{ytCopyright "power_config.h"}}



#ifndef __POWER_CONFIG_H__
#define __POWER_CONFIG_H__

#include "power_manager.h"

#define POWER_MANAGER_CONFIG_CNT                ({{this.data.user.length}}U)
#define POWER_MANAGER_CALLBACK_CNT              ({{this.data.callback.length}}U)

extern power_manager_user_config_t* powerConfigsArr[{{this.data.user.length}}];
{{#if this.data.callback.length}}
extern power_manager_callback_user_config_t* powerStaticCallbacksConfigsArr[{{this.data.callback.length}}];


#define POWER_MANAGER_CONFIG_MACRO (&powerConfigsArr,POWER_MANAGER_CONFIG_CNT,&powerStaticCallbacksConfigsArr,POWER_MANAGER_CALLBACK_CNT)
{{/if}}
#endif


