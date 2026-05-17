<template>
  <div>
    <div>
      <el-row>
        <el-col :span="6">
          <el-table
            :data="data"
            style="width: 100%"
            border
            :row-class-name="tableRowClassName"
            @row-click="masterRowClick"
          >
            <el-table-column
              align="right"
              :resizable="false"
            >
              <template #header>
                <el-button-group>
                  <el-button
                    type="primary"
                    plain
                    :icon="Plus"
                    size="small"
                    :disabled="lock"
                    @click="addItem('master')"
                  />
                  <el-button
                    type="primary"
                    plain
                    :icon="Delete"
                    size="small"
                    :disabled="lock"
                    @click="deleteItem('master')"
                  />
                </el-button-group>
              </template>
              <template #default="scope">
                <span
                  v-if="scope.$index == activeMasterInst"
                  style="margin-right:5px"
                >
                  <el-icon>
                    <FolderOpened />
                  </el-icon>
                </span><span v-if="scope.row.name">{{ scope.row.name }}</span><span v-else>{{ scope.$index }}</span>
              </template>
            </el-table-column>
          </el-table>
        </el-col>
        <el-col :span="18">
          <div v-if="data[activeMasterInst]">
            <el-form
              ref="masterRef"
              :disabled="lock"
              :model="data[activeMasterInst]"
              label-width="150px"
              size="small"
              :rules="MasterRules"
              hide-required-asterisk
            >
              <el-form-item
                label="Name"
                prop="name"
              >
                <el-input v-model="data[activeMasterInst].name" />
              </el-form-item>
              <el-form-item
                label="Read-only"
                prop="readonly"
              >
                <el-checkbox v-model="data[activeMasterInst].readonly" />
              </el-form-item>
              <el-form-item
                label="Compensation interval"
                prop="compensationInterval"
              >
                <el-input v-model.number="data[activeMasterInst].compensationInterval" />
              </el-form-item>
              <el-form-item
                label="Compensation value"
                prop="compensation"
              >
                <el-input v-model.number="data[activeMasterInst].compensation" />
              </el-form-item>
              <el-form-item
                label="RTC clock source"
                prop="clockSource"
              >
                <el-select
                  v-model="data[activeMasterInst].clockSource"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in rtcClkSrc"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="RTC clock output"
                prop="clockOutConfig"
              >
                <el-select
                  v-model="data[activeMasterInst].clockOutConfig"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in ['RTC_CLKOUT_DISABLED', 'RTC_CLKOUT_SRC_SEC', 'RTC_CLKOUT_SRC_CLKSEL']"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Debug enable"
                prop="debugEnable"
              >
                <el-checkbox v-model="data[activeMasterInst].debugEnable" />
              </el-form-item>

              <el-collapse
                v-model="col1"
                style="width:100%;margin-bottom: 18px;"
              >
                <el-collapse-item
                  title="Alarm configuration"
                  name="1"
                >
                  <el-form-item
                    label="Alarm Time"
                    prop="rtcAlarmConfig.data1"
                    required
                    :rules="data1Rule"
                  >
                    <el-col :span="11">
                      <el-date-picker
                        v-model="data[activeMasterInst].rtcAlarmConfig.data1"
                        type="date"
                        placeholder="Pick a date"
                        style="width: 100%"
                        @change="dateChange"
                      />
                    </el-col>
                    <el-col
                      :span="2"
                      style="text-align: center;"
                    >
                      <span>---</span>
                    </el-col>
                    <el-col :span="11">
                      <el-time-picker
                        v-model="data[activeMasterInst].rtcAlarmConfig.data2"
                        placeholder="Pick a time"
                        style="width: 100%"
                        @change="timeChange"
                      />
                    </el-col>
                  </el-form-item>
                  <el-form-item
                    label="Interval of repetition in sec"
                    prop="rtcAlarmConfig.repetitionInterval"
                    :rules="repetitionIntervalRule"
                  >
                    <el-input v-model.number="data[activeMasterInst].rtcAlarmConfig.repetitionInterval" />
                  </el-form-item>
                  <el-form-item
                    label="Number of alarm repeats"
                    prop="rtcAlarmConfig.numberOfRepeats"
                    :rules="numberOfRepeatsRule"
                  >
                    <el-input v-model.number="data[activeMasterInst].rtcAlarmConfig.numberOfRepeats" />
                  </el-form-item>
                  <el-form-item
                    label="Repeat forever"
                    prop="rtcAlarmConfig.repeatForever"
                  >
                    <el-checkbox v-model="data[activeMasterInst].rtcAlarmConfig.repeatForever" />
                  </el-form-item>
                  <el-form-item
                    label="Enable alarm interrupt"
                    prop="rtcAlarmConfig.alarmIntEnable"
                  >
                    <el-checkbox v-model="data[activeMasterInst].rtcAlarmConfig.alarmIntEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Callback"
                    prop="rtcAlarmConfig.rtcAlarmCallback"
                    :rules="CallbackRule"
                  >
                    <el-input v-model="data[activeMasterInst].rtcAlarmConfig.rtcAlarmCallback" />
                  </el-form-item>
                  <el-form-item
                    label="Callback parameter"
                    prop="rtcAlarmConfig.callbackParams"
                    :rules="callbackParamsRule"
                  >
                    <el-input v-model="data[activeMasterInst].rtcAlarmConfig.callbackParams" />
                  </el-form-item>
                </el-collapse-item>
                <el-collapse-item
                  title="Overflow configuration"
                  name="2"
                >
                  <el-form-item
                    label="Enable overflow interrupt"
                    prop="rtcOverflowConfig.overflowIntEnable"
                  >
                    <el-checkbox v-model="data[activeMasterInst].rtcOverflowConfig.overflowIntEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Callback"
                    prop="rtcOverflowConfig.rtcOverflowCallback"
                    :rules="CallbackRule"
                  >
                    <el-input v-model="data[activeMasterInst].rtcOverflowConfig.rtcOverflowCallback" />
                  </el-form-item>
                  <el-form-item
                    label="Callback parameter"
                    prop="rtcOverflowConfig.callbackParams"
                    :rules="callbackParamsRule"
                  >
                    <el-input v-model="data[activeMasterInst].rtcOverflowConfig.callbackParams" />
                  </el-form-item>
                </el-collapse-item>
                <el-collapse-item
                  title="Second configuration"
                  name="3"
                >
                  <el-form-item
                    label="Second freq"
                    prop="rtcSecondsConfig.secondsIntConfig"
                  >
                    <el-select
                      v-model="data[activeMasterInst].rtcSecondsConfig.secondsIntConfig"
                      style="width:100%"
                    >
                      <el-option
                        v-for="item in ['RTC_INT_1HZ', 'RTC_INT_2HZ', 'RTC_INT_4HZ', 'RTC_INT_8HZ', 'RTC_INT_16HZ', 'RTC_INT_32HZ', 'RTC_INT_64HZ', 'RTC_INT_128HZ']"
                        :key="item"
                        :value="item"
                        :label="item"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Enable second interrupt"
                    prop="rtcSecondsConfig.secondsIntEnable"
                  >
                    <el-checkbox v-model="data[activeMasterInst].rtcSecondsConfig.secondsIntEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Callback"
                    prop="rtcSecondsConfig.rtcSecondsCallback"
                    :rules="CallbackRule"
                  >
                    <el-input v-model="data[activeMasterInst].rtcSecondsConfig.rtcSecondsCallback" />
                  </el-form-item>
                  <el-form-item
                    label="Callback parameter"
                    prop="rtcSecondsConfig.callbackParams"
                    :rules="callbackParamsRule"
                  >
                    <el-input v-model="data[activeMasterInst].rtcSecondsConfig.callbackParams" />
                  </el-form-item>
                </el-collapse-item>
              </el-collapse>
            </el-form>
          </div>
        </el-col>
      </el-row>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, computed, reactive, watch, toRaw, onBeforeUnmount, nextTick } from "vue";
import { Plugin } from "plugin";
import {
  Plus,
  Delete,
  FolderOpened
} from '@element-plus/icons-vue'
import type { FormInstance, FormRules, FormItemProp, FormItemRule } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const col1 = ref(['1', '2', '3'])
const rtcClkSrc = ref([])
const defMasterParam = {
  name: 'rtc_config0',
  readonly: true,
  compensationInterval: 0,
  compensation: 0,
  clockSource: '',
  clockOutConfig: 'RTC_CLKOUT_DISABLED',
  debugEnable: false,
  rtcAlarmConfig: {
    data1: '',
    data2: '',
    alarmTime:{
      year:0,
      month:0,
      day:0,
      hour:0,
      minutes:0,
      seconds:0,
    },
    repetitionInterval: 0,
    numberOfRepeats: 0,
    repeatForever: false,
    alarmIntEnable: false,
    rtcAlarmCallback: 'NULL',
    callbackParams: 'NULL'
  },
  rtcOverflowConfig: {
    overflowIntEnable: false,
    rtcOverflowCallback: 'NULL',
    callbackParams: 'NULL'
  },
  rtcSecondsConfig: {
    secondsIntConfig: 'RTC_INT_1HZ',
    secondsIntEnable: false,
    rtcSecondsCallback: 'NULL',
    callbackParams: 'NULL',
  }
}

function dateChange(){
  const d=data.value[activeMasterInst.value].rtcAlarmConfig.data1 as unknown as Date
  if(d){
    data.value[activeMasterInst.value].rtcAlarmConfig.alarmTime.year=d.getFullYear()
    data.value[activeMasterInst.value].rtcAlarmConfig.alarmTime.month=d.getMonth()+1
    data.value[activeMasterInst.value].rtcAlarmConfig.alarmTime.day=d.getDate()
  }
} 

function timeChange(){
  const d=data.value[activeMasterInst.value].rtcAlarmConfig.data2 as unknown as Date
  if(d){
    data.value[activeMasterInst.value].rtcAlarmConfig.alarmTime.hour=d.getHours()
    data.value[activeMasterInst.value].rtcAlarmConfig.alarmTime.minutes=d.getMinutes()
    data.value[activeMasterInst.value].rtcAlarmConfig.alarmTime.seconds=d.getSeconds()
  }
}

const lock=ref(false)
const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = ref([defMasterParam])

function resizeHandler() {
  null
}

watch(activeMasterInst, () => {
  nextTick(() => {
    masterRef.value?.validate().catch((e: any) => {
      null
    })
  })
})

function addItem(key: string) {

  if (key == 'master') {
    data.value.push(JSON.parse(JSON.stringify(defMasterParam)))
    activeMasterInst.value = data.value.length - 1

  }

}

function deleteItem(key: string) {
  if (data.value.length >= 0) {
    if (key == 'master') {
      data.value.splice(activeMasterInst.value, 1)
      activeMasterInst.value = data.value.length - 1
    }
  }
}

function masterRowClick(row) {
  const index = data.value.indexOf(row)
  activeMasterInst.value = index
  nextTick(() => {
    masterRef.value?.validate().catch((e: any) => {
      null
    })
  })
}
function masterNameCheck(rule: any, value: any, callback: any) {
  const err = false;
  const cnt = {}

  for (let i = 0; i < data.value.length; i++) {
    if (cnt[data.value[i][rule.field]] == undefined) {
      cnt[data.value[i][rule.field]] = 1
    } else {
      cnt[data.value[i][rule.field]]++
    }
  }

  for (const key of Object.keys(cnt)) {
    if (cnt[key] > 1) {
      callback(new Error(`Duplicate ${key}`))
    }
  }
  if (!err) {
    callback()
  }
}

const repetitionIntervalRule:FormItemRule[] =[
    { type: 'number', required: true, message: 'Please input repetition interval', min: 0, trigger: 'change' },
];

const numberOfRepeatsRule:FormItemRule[]= [
  { type: 'number', required: true, message: 'Please input number of alarm repetition ', min: 0, trigger: 'change' },
];

const CallbackRule:FormItemRule[]=[
    { required: true, message: 'Please input callback function name', trigger: 'change',pattern:/^[-_a-zA-Z0-9]+$/gi},
];
const callbackParamsRule:FormItemRule[]= [
    { required: true, message: 'Please input callback parameters', trigger: 'change',pattern:/^[-_a-zA-Z0-9]+$/gi},
];
const data1Rule=[
    { type: 'date', required: true, message: 'Please choose a date', trigger: 'change' },
]

const data2Rule=[
    { type: 'date', required: true, message: 'Please choose a time', trigger: 'change' },
];


const MasterRules = {
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  compensationInterval: [
    { type: 'number', required: true, message: 'Please input  compensation interval', min: 0, max: 255, trigger: 'change' },
  ],
  compensation: [
    { type: 'number', required: true, message: 'Please input  compensation value', min: -128, max: 128, trigger: 'change' },
  ],
  clockSource: [
    { required: true, message: 'Please select RTC clock source', trigger: 'change' },
  ],
  rtcAlarmConfig: {
    type: 'object',
    required: true,
    fields: {
      repetitionInterval: repetitionIntervalRule,
      numberOfRepeats: numberOfRepeatsRule,
      rtcAlarmCallback: CallbackRule,
      callbackParams: callbackParamsRule,
      data1: data1Rule,
      data2: data2Rule
    },
  },
  rtcSecondsConfig: {
    type: 'object',
    required: true,
    fields: {
      rtcSecondsCallback: CallbackRule,
      callbackParams: callbackParamsRule,
    },
  },
  rtcOverflowConfig: {
    type: 'object',
    required: true,
    fields: {
      rtcOverflowCallback: CallbackRule,
      callbackParams: callbackParamsRule,
    },
  },
}



const errorid: string[] = []
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});

function dataCheck() {
  for (const error of errorid) {
    plugin.deleteLog('error', error)
  }
  const masterValidator = new Schema(MasterRules);
  for (const i in data.value) {
    masterValidator.validate(data.value[i], (errors, fields) => {
      if (errors) {
        // validation failed, errors is an array of all errors
        // fields is an object keyed by field name with an array of
        // errors per field
        for (const j in errors) {
          const id = `${errors[j].field}-${i}-${j}`
          plugin.addLog('error', id, errors[j].message)
          errorid.push(id)
        }
      }

    });
  }
}

onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {
      rtcClkSrc.value = plugin.getMeta("rtc_clock_src")
      if (initData.pluginData != undefined) {
        data.value = initData.pluginData.data;
        lock.value = initData.pluginData.lock
      }else{
        data.value[activeMasterInst.value].clockSource = rtcClkSrc.value[0]
      }
      nextTick(() => {
        masterRef.value?.validate().catch((e: any) => {
          null
        })
      })
      dataCheck();
      /*always push a data when init done*/
      plugin.dataChange(data.value);
      watch(data, (raw) => {
        dataCheck();
        plugin.dataChange(data.value);
      }, {
        deep: true
      })

    })
    .catch((e: any) => {
      console.error(e);
    });


});



const tableRowClassName = ({
  row,
  rowIndex,
}: {
  row: any
  rowIndex: number
}) => {
  if (rowIndex === activeMasterInst.value) {
    return 'active-row'
  }
  return ''
}

</script>
<style>
.el-table .active-row {
  color: #409EFF
}
</style>