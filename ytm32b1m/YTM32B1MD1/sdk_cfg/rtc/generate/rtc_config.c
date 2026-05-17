{{ytCopyright "rtc_config.c"}}

#include <stddef.h>
#include "rtc_config.h"


{{#each this.data}}
/*{{this.name}}*/

{{#isTruthy this.rtcAlarmConfig.rtcAlarmCallback}}extern void {{{this.rtcAlarmConfig.rtcAlarmCallback}}}(void *callbackParam);{{/isTruthy}}
{{#isTruthy this.rtcAlarmConfig.callbackParams}}extern void* {{{this.rtcAlarmConfig.callbackParams}}};{{/isTruthy}}

{{#isTruthy this.rtcSecondsConfig.rtcSecondsCallback}}extern void {{{this.rtcSecondsConfig.rtcSecondsCallback}}}(void *callbackParam);{{/isTruthy}}
{{#isTruthy this.rtcSecondsConfig.callbackParams}}extern void* {{{this.rtcSecondsConfig.callbackParams}}};{{/isTruthy}}

{{#isTruthy this.rtcOverflowConfig.rtcOverflowCallback}}extern void {{{this.rtcOverflowConfig.rtcOverflowCallback}}}(void *callbackParam);{{/isTruthy}}
{{#isTruthy this.rtcOverflowConfig.callbackParams}}extern void* {{{this.rtcOverflowConfig.callbackParams}}};{{/isTruthy}}


static rtc_alarm_config_t {{this.name}}Alarm={
    .alarmTime={
        .year={{this.rtcAlarmConfig.alarmTime.year}},
        .month={{this.rtcAlarmConfig.alarmTime.month}},
        .day={{this.rtcAlarmConfig.alarmTime.day}},
        .hour={{this.rtcAlarmConfig.alarmTime.hour}},
        .minutes={{this.rtcAlarmConfig.alarmTime.minutes}},
        .seconds={{this.rtcAlarmConfig.alarmTime.seconds}}
    },
    .repetitionInterval={{this.rtcAlarmConfig.repetitionInterval}},
    .numberOfRepeats={{this.rtcAlarmConfig.numberOfRepeats}},
    .repeatForever={{this.rtcAlarmConfig.repeatForever}},
    .alarmIntEnable={{this.rtcAlarmConfig.alarmIntEnable}},
    .rtcAlarmCallback={{{this.rtcAlarmConfig.rtcAlarmCallback}}},
    .callbackParams={{{this.rtcAlarmConfig.callbackParams}}},
};

static rtc_overflow_config_t {{this.name}}Overflow={
    .overflowIntEnable={{this.rtcOverflowConfig.overflowIntEnable}},
    .rtcOverflowCallback={{{this.rtcOverflowConfig.rtcOverflowCallback}}},
    .callbackParams={{{this.rtcOverflowConfig.callbackParams}}},
};

static rtc_seconds_config_t {{this.name}}Seconds={
    .secondsIntConfig={{this.rtcSecondsConfig.secondsIntConfig}},
    .secondsIntEnable={{this.rtcSecondsConfig.secondsIntEnable}},
    .rtcSecondsCallback={{this.rtcSecondsConfig.rtcSecondsCallback}},
    .callbackParams={{{this.rtcSecondsConfig.callbackParams}}},
};


{{#isTruthy this.readonly}}const {{/isTruthy}}rtc_init_config_t {{this.name}} = {
    .compensationInterval={{this.compensationInterval}},
    .compensation={{this.compensation}},
    .clockSource={{this.clockSource}},
    .clockOutConfig={{this.clockOutConfig}},
    .debugEnable={{this.debugEnable}},
    .rtcAlarmConfig=&{{this.name}}Alarm,
    .rtcOverflowConfig=&{{this.name}}Overflow,
    .rtcSecondsConfig=&{{this.name}}Seconds,
};
{{/each}}

