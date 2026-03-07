<template>
  <div>
    <div>
      <el-form
        ref="masterRef"
        :model="data"
        label-width="200px"
        size="small"
        :rules="MasterRules"
        hide-required-asterisk
        :disabled="lock"
      >
        <el-form-item
          label="Name"
          prop="name"
        >
          <el-input v-model="data.name" />
        </el-form-item>
        <el-form-item
          label="Read-only"
          prop="readonly"
        >
          <el-checkbox v-model="data.readonly" />
        </el-form-item>
        <el-form-item
          label="Enable run in debug mode"
          prop="enableRunInDebug"
        >
          <el-checkbox v-model="data.enableRunInDebug" />
        </el-form-item>
        
        <el-form-item
          v-show="FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE"
          label="Select function clock"
          prop="data.useFunctionClockSource"
        >
          <el-checkbox v-model="data.useFunctionClockSource" />
        </el-form-item>

        <el-row>
          <el-col :span="6">
            <el-table
              :data="data.channel"
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
                      @click="addItem('master')"
                    />
                    <el-button
                      type="primary"
                      plain
                      :icon="Delete"
                      size="small"
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
                  </span><span>Channel-{{ scope.$index }}</span>
                </template>
              </el-table-column>
            </el-table>
          </el-col>
          <el-col :span="18">
            <div v-if="data.channel[activeMasterInst]">
              <el-form-item
                label="Channel"
                :prop="`channel[${activeMasterInst}].channel`"
                :rules="chanelCheck"
              >
                <el-input v-model.number="data.channel[activeMasterInst].channel" />
              </el-form-item>
              <el-form-item label="Period unit">
                <el-select
                  v-model="data.channel[activeMasterInst].periodUnits"
                  style="width: 100%;"
                >
                  <el-option
                    value="pTMR_PERIOD_UNITS_COUNTS"
                    label="UNIT_Counter"
                  />
                  <el-option
                    value="pTMR_PERIOD_UNITS_MICROSECONDS"
                    label="UNIT_Microseconds"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Period value"
                :prop="`channel[${activeMasterInst}].period`"
                :rules="compareValueCheck"
              >
                <el-input v-model.number="data.channel[activeMasterInst].period" />
              </el-form-item>
              <el-form-item label="Enable channel chain">
                <el-checkbox v-model="data.channel[activeMasterInst].chainChannel" />
              </el-form-item>
              <el-form-item label="Enable interrupt">
                <el-checkbox v-model="data.channel[activeMasterInst].isInterruptEnabled" />
              </el-form-item>
            </div>
          </el-col>
        </el-row>
      </el-form>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, reactive, watch, onBeforeUnmount, nextTick, isProxy } from "vue";
import { Plugin } from "plugin";
import {
  Plus,
  Delete,
  FolderOpened
} from '@element-plus/icons-vue'
import { FormInstance, FormRules, FormItemRule, ElNotification } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };



const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const clockFreq=ref(0)

const defChannelParam = {
  channel: 0,
  periodUnits: 'pTMR_PERIOD_UNITS_COUNTS',
  period: 0,
  chainChannel: false,
  isInterruptEnabled: false,
}



const lock=ref(false)
const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = reactive(
  {
    name: 'PTMR_Config',
    readonly: true,
    enableRunInDebug: false,
    useFunctionClockSource: false,
    channel: [defChannelParam]
  })

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



function masterRowClick(row) {
  const index = data.channel.indexOf(row)
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

  for (let i = 0; i < data.channel.length; i++) {
    if (cnt[data.channel[i].channel] == undefined) {
      cnt[data.channel[i].channel] = 1
    } else {
      cnt[data.channel[i].channel]++
    }
  }

  for (const key of Object.keys(cnt)) {
    if (cnt[key] > 1 && value == key) {
      callback(new Error(`Duplicate ${key}`))
    }
  }
  callback()
}

function compareValueValid(rule: any, value: any, callback: any, source:any) {
  const val=isProxy(source)?source:data.channel[activeMasterInst.value]
  if(Number.isNaN(Number(value))){
    callback('Please input correct format')
  }
  if(val.periodUnits=='pTMR_PERIOD_UNITS_COUNTS'){
    if(value<0||value>0xffffffff){
      callback('Compare value from 0-0xFFFF_FFFF')
    }
  }else{
    if(value<0||value>Math.round(0xffffffff/clockFreq.value)){
      callback(`Compare value from 0-${Math.round(0xffffffff/clockFreq.value)}`)
    }
  }
  callback()
}


const chanelCheck: FormItemRule[] = [
  { type: 'number', min: 0, max: 4, message: 'Channel number from 0-3', },
  { type: 'number', required: true, message: 'Please input a channel' },
  { validator: masterNameCheck }
]
const compareValueCheck: FormItemRule[] = [
  { type: 'number', validator:compareValueValid },
  { type: 'number', required: true, message: 'Please input compare value' },
]
const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
  ],
  channel:
  {
    type: 'array', max: 4, message: 'Max channel number is 4', trigger: 'change', fields: {
      0: {
        type: 'object', fields: {
          channel: chanelCheck,
          period: compareValueCheck,
        },
      },
      1: {
        type: 'object', fields: {
          channel: chanelCheck,
          period: compareValueCheck,
        },
      },
      2: {
        type: 'object', fields: {
          channel: chanelCheck,
          period: compareValueCheck,
        },
      },
      3: {
        type: 'object', fields: {
          channel: chanelCheck,
          period: compareValueCheck,
        },
      },
    },
  },
})

function addItem(key: string) {

  if (key == 'master') {
    if(data.channel.length==4){
      ElNotification({
        title: 'Warning',
        message: 'Max channel number is 4',
        type: 'warning',
      })
    }else{
      data.channel.push(JSON.parse(JSON.stringify(defChannelParam)))
    activeMasterInst.value = data.channel.length - 1
    }
   
  }

}

function deleteItem(key: string) {
  if (data.channel.length >= 0) {
    if (key == 'master') {
      data.channel.splice(activeMasterInst.value, 1)
      activeMasterInst.value = data.channel.length - 1
    }
  }
}

const errorid: string[] = []
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});

function dataCheck() {
  masterRef.value?.validate().catch(()=>{null})
  for (const error of errorid) {
    plugin.deleteLog('error', error)
  }
  let warn=false
  for(const val of data.channel){
    if(val.period==0){
      warn=true
      break
    }
  }
  if (warn) {
    plugin.addLog('warn', 'PeriodValue', 'The period value is zero,the hardware trigger will remain asserted until the PTMR is disable')
  } else {
    plugin.deleteLog('warn', 'PeriodValue')
  }


  const masterValidator = new Schema(MasterRules);
  masterValidator.validate(data, (errors, fields) => {
    if (errors) {
      // validation failed, errors is an array of all errors
      // fields is an object keyed by field name with an array of
      // errors per field
      for (const j in errors) {
        const id = `${errors[j].field}-${j}`
        plugin.addLog('error', id, errors[j].message)
        errorid.push(id)
      }
    }
  });

}

const FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE = ref(false)
onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {

      FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE.value = plugin.getMeta('FEATURE_pTMR_HAS_IPC_CLOCK_SOURCE')

      if (initData.pluginData != undefined) {
        Object.assign(data, initData.pluginData.data);
        lock.value = initData.pluginData.lock
      }
      const activeClock=initData.globalData.clock.data.activeInst
      clockFreq.value=initData.globalData.clock.data.clockArray[activeClock].data.slowClock.freq.raw/1000000
      plugin.watch('clock', (newData) => {
        const activeClock=newData.data.activeInst
        clockFreq.value=newData.data.clockArray[activeClock].data.slowClock.freq.raw/1000000
      })

      nextTick(() => {
        masterRef.value?.validate().catch((e: any) => {
          null
        })
      })

      dataCheck();
      /*always push a data when init done*/
      plugin.dataChange(data);
      watch(data, (raw) => {
        dataCheck();
        plugin.dataChange(data);
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