<template>
  <div>
    <div>
      <el-form
        ref="masterRef"
        :model="data"
        label-width="150px"
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
          label="Clock divider value"
          prop="clockPrescaler"
          required
        >
          <el-input v-model.number="data.clockPrescaler" />
        </el-form-item>
        <el-form-item
          label="Stop in debug mode"
          prop="stopInDebugMode"
        >
          <el-checkbox v-model="data.stopInDebugMode" />
        </el-form-item>
        <el-form-item
          label="Start counter value"
          prop="startValue"
          required
        >
          <el-input v-model.number="data.startValue" />
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
              <el-form-item
                label="Compare value"
                :prop="`channel[${activeMasterInst}].compareValue`"
                :rules="compareValueCheck"
              >
                <el-input v-model.number="data.channel[activeMasterInst].compareValue">
                  <template #append>
                    Ticks
                  </template>
                </el-input>
              </el-form-item>
              <el-form-item
                v-show="chIntEnShow"
                label="Interrupt Enable"
                :prop="`channel[${activeMasterInst}].intEn`"
              >
              <el-checkbox v-model="data.channel[activeMasterInst].intEn" />
              </el-form-item>
            </div>
          </el-col>
        </el-row>
      </el-form>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, reactive, watch, onBeforeUnmount, nextTick } from "vue";
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
const chIntEnShow = ref(false)


const defChannelParam = {
  channel: 0,
  compareValue: 0,
  intEn: false,
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
    name: 'TMR_Config',
    readonly: true,
    clockPrescaler: 0,
    stopInDebugMode: false,
    startValue: 0,
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
    if (cnt[key] > 1&&value==key) {
      callback(new Error(`Duplicate ${key}`))
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
  { type: 'number', min: 0, max: 0xffffffff, message: 'Compare value from 0-0xffffffff', },
  { type: 'number', required: true, message: 'Please input compare value' },
]
const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
  ],
  clockPrescaler: [
    { type: 'number', min: 0, max: 255, required: true, message: 'Please input clockPrescaler from 0-255', trigger: 'change' },
  ],
  startValue: [
    { type: 'number', min: 0, max: 0xffffffff, required: true, message: 'Please input startValue', trigger: 'change' },
  ],
  channel:
  {
    type: 'array', max: 4, message: 'Max channel number is 4', trigger: 'change', fields: {
      0: {
        type: 'object', fields: {
          channel: chanelCheck,
          compareValue: compareValueCheck,
        },
      },
      1: {
        type: 'object', fields: {
          channel: chanelCheck,
          compareValue: compareValueCheck,
        },
      },
      2: {
        type: 'object', fields: {
          channel: chanelCheck,
          compareValue: compareValueCheck,
        },
      },
      3: {
        type: 'object', fields: {
          channel: chanelCheck,
          compareValue: compareValueCheck,
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

  let warn=false
  for(const val of data.channel){
    if(val.compareValue==0){
      warn=true
      break
    }
  }
  if (warn) {
    plugin.addLog('warn', 'CompareValue', 'The compare value is zero,the hardware trigger will remain asserted until the PTMR is disable')
  } else {
    plugin.deleteLog('warn', 'CompareValue')
  }

  for (const error of errorid) {
    plugin.deleteLog('error', error)
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

onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {
      if (initData.pluginData != undefined) {
        Object.assign(data, initData.pluginData.data);
        lock.value = initData.pluginData.lock
      }
      /* Show channel interrupt or not */
      chIntEnShow.value = plugin.getMeta("tmr_has_channel_int")
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