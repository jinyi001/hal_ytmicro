{{ytCopyright "sent_config.h"}}



#ifndef __SENT_CONFIG_H__
#define __SENT_CONFIG_H__


#include "sent_driver.h"

{{#unless (or (eq this.data.fastMsgDmaPtr "") (eq this.data.fastMsgDmaPtr "NULL"))}}
extern sent_raw_msg_t {{this.data.fastMsgDmaPtr}};
{{/unless}}

{{#unless (or (eq this.data.slowMsgDmaPtr "") (eq this.data.slowMsgDmaPtr "NULL"))}}
extern sent_raw_msg_t {{this.data.slowMsgDmaPtr}};
{{/unless}}

{{#unless (or (eq this.data.callbackFunction "") (eq this.data.callbackFunction "NULL"))}}
void {{this.data.callbackFunction}}(uint32_t instance, uint32_t channel, sent_callback_type_t type, void * param);
{{/unless}}

extern const sent_channel_config_t sentChannelConfig[{{this.data.numOfConfigs}}];

extern {{#isTruthy this.data.readonly}}const {{/isTruthy}}sent_driver_user_config_t sentUserConfig;

extern sent_state_t sentState;

#endif

