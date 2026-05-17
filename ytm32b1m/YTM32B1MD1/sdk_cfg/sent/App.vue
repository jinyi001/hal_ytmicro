<template>
  <div>
    <div>
      <el-row>
        <div 
          style="width:100%;"
        >
          <el-form
            ref="masterRef"
            :model="data.channelConfig[activeMasterInst]"
            label-width="400px"
            size="small"
            :rules="ConfigRules"
            hide-required-asterisk
            :disabled="lock"
            style="width:100%;"
          >
            <el-form-item
              label="Read Only"
              prop="readOnly"
            >
              <el-checkbox v-model="data.readonly" />
            </el-form-item>
            <el-form-item
              label="Fast Message DMA Data Buffer"
              prop="fastMsgDmaPtr"
            >
              <el-input v-model="data.fastMsgDmaPtr" />
            </el-form-item>
            <el-form-item
              label="Fast Message DMA Channel"
              prop="fastDmaChannel"
            >
              <el-input v-model="data.fastDmaChannel" />
            </el-form-item>
            <el-form-item
              label="Fast Message DMA FIFO Enable"
              prop="fastDmaFifoEnable"
            >
              <el-checkbox v-model="data.fastDmaFifoEnable" />
            </el-form-item>
            <el-form-item
              label="Fast Message DMA FIFO Size"
              prop="fastDmaFifoSize"
            >
              <el-input v-model="data.fastDmaFifoSize" />
            </el-form-item>
            <el-form-item
              label="Slow Message DMA Buffer"
              prop="slowMsgDmaPtr"
            >
              <el-input v-model="data.slowMsgDmaPtr" />
            </el-form-item>
            <el-form-item
              label="Slow Message DMA Channel"
              prop="slowDmaChannel"
            >
              <el-input v-model="data.slowDmaChannel" />
            </el-form-item>
            <el-form-item
              label="SENT Callback Function"
              prop="callbackFunction"
            >
              <el-input v-model="data.callbackFunction" />
            </el-form-item>
            <el-form-item
              label="SENT Callback Parameter"
              prop="callbackParam"
            >
              <el-input v-model="data.callbackParam" />
            </el-form-item>
            <el-form-item
              label="Number of Channel Config"
              prop="numOfConfigs"
            >
              <el-input v-model="data.numOfConfigs" />
            </el-form-item>
          </el-form>
        </div>
      </el-row>
      <el-row>
        <el-col :span="6">
          <el-table
            :data="data.channelConfig"
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
                    :disabled="lock"
                  />
                  <el-button
                    type="primary"
                    plain
                    :icon="Delete"
                    size="small"
                    @click="deleteItem('master')"
                    :disabled="lock"
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
          <div
            v-if="data.channelConfig[activeMasterInst]"
            style="width:100%;"
          >
            <el-form
              ref="masterRef"
              :model="data.channelConfig[activeMasterInst]"
              label-width="400px"
              size="small"
              :rules="ChannelRules"
              hide-required-asterisk
              :disabled="lock"
              style="width:100%;"
            >
              <el-form-item
                label="Name"
                prop="name"
              >
                <el-input v-model="data.channelConfig[activeMasterInst].name" />
              </el-form-item>
              <el-form-item
                label="SENT Channel ID"
                prop="channelId"
              >
                <el-input v-model="data.channelConfig[activeMasterInst].channelId" />
              </el-form-item>
              <el-form-item
                label="Tick Duration(us)"
                prop="tickDuration"
              >
                <el-input v-model="data.channelConfig[activeMasterInst].tickDuration" />
              </el-form-item>
              <el-form-item
                label="Input Filter"
                prop="inputFilter"
              >
                <el-select v-model="data.channelConfig[activeMasterInst].inputFilter">
                  <el-option
                    label="SENT_INPUT_FILTER_NONE"
                    :value="0"
                  />
                  <el-option
                    label="SENT_INPUT_FILTER_2"
                    :value="1"
                  />
                  <el-option
                    label="SENT_INPUT_FILTER_4"
                    :value="2"
                  />
                  <el-option
                    label="SENT_INPUT_FILTER_8"
                    :value="3"
                  />
                  <el-option
                    label="SENT_INPUT_FILTER_16"
                    :value="4"
                  />
                  <el-option
                    label="SENT_INPUT_FILTER_32"
                    :value="5"
                  />
                  <el-option
                    label="SENT_INPUT_FILTER_64"
                    :value="6"
                  />
                  <el-option
                    label="SENT_INPUT_FILTER_128"
                    :value="7"
                  />
                  <el-option
                    label="SENT_INPUT_FILTER_256"
                    :value="8"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Fast Message Nibbles"
                prop="fastMsgNibbles"
              >
                <el-input v-model="data.channelConfig[activeMasterInst].fastMsgNibbles" />
              </el-form-item>
              <el-form-item
                label="Fast Message DMA Enable"
                prop="fastMsgDmaEnable"
              >
                <el-checkbox v-model="data.channelConfig[activeMasterInst].fastMsgDmaEnable" />
              </el-form-item>
              <el-form-item
                label="Fast Message CRC include Status"
                prop="fastMsgCrcIncludeStatus"
              >
                <el-checkbox v-model="data.channelConfig[activeMasterInst].fastMsgCrcIncludeStatus" />
              </el-form-item>
              <el-form-item
                label="Fast Message CRC Check Disable"
                prop="fastMsgCrcCheckDisable"
              >
                <el-checkbox v-model="data.channelConfig[activeMasterInst].fastMsgCrcCheckDisable" />
              </el-form-item>
              <el-form-item
                label="Fast Message CRC Type"
                prop="fastMsgCrcType"
              >
                <el-select
                  v-model="data.channelConfig[activeMasterInst].fastMsgCrcType"
                  style="width:220px"
                >
                  <el-option
                    v-for="item in ['SENT_CRC_RECOMMENDED', 'SENT_CRC_LEGACY']"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Slow Message DMA Enable"
                prop="slowMsgDmaEnable"
              >
                <el-checkbox v-model="data.channelConfig[activeMasterInst].slowMsgDmaEnable" />
              </el-form-item>
              <el-form-item
                label="Slow Message CRC Type"
                prop="slowMsgCrcType"
              >
                <el-select
                  v-model="data.channelConfig[activeMasterInst].slowMsgCrcType"                  
                  style="width:220px"
                >
                  <el-option
                    v-for="item in ['SENT_CRC_RECOMMENDED', 'SENT_CRC_LEGACY']"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="IDLE Pulse Diagnostics Count"
                prop="idleCount"
              >
                <el-select
                  v-model="data.channelConfig[activeMasterInst].idleCount"
                  style="width:250px"
                >
                  <el-option
                    value="SENT_BUS_IDLE_DISABLED"
                    label="Disable IDLE pulse"
                  />
                  <el-option
                    value="SENT_BUS_IDLE_245_CLK_TICKS"
                    label="IDLE pulse diagnostics with 245 ticks"
                  />
                  <el-option
                    value="SENT_BUS_IDLE_508_CLK_TICKS"
                    label="IDLE pulse diagnostics with 508 ticks"
                  />
                  <el-option
                    value="SENT_BUS_IDLE_1016_CLK_TICKS"
                    label="IDLE pulse diagnostics with 1016 ticks"
                  />
                  <el-option
                    value="SENT_BUS_IDLE_2032_CLK_TICKS"
                    label="IDLE pulse diagnostics with 2032 ticks"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Calibration Pulse Range"
                prop="calibVar"
              >
                <el-select
                  v-model="data.channelConfig[activeMasterInst].calibVar"
                >
                  <el-option
                    value="SENT_CALIB_VAR_20_PERCENT"
                    label="Calibration pulse with 20%"
                  />
                  <el-option
                    value="SENT_CALIB_VAR_25_PERCENT"
                    label="Calibration pulse with 25%"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Pause Pulse Check Select"
                prop="diagPulse"
              >
                <el-select
                  v-model="data.channelConfig[activeMasterInst].diagPulse"
                  style="width:300px"
                >
                  <el-option
                    value="SENT_PULSE_CHECK_BOTH"
                    label="Calibration pulse and pause pulse all check"
                  />
                  <el-option
                    value="SENT_PULSE_CHECK_PAUSE"
                    label="Only pause pulse check"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Pause Pulse Check Enable"
                prop="pausePulse"
              >
                <el-select
                  v-model="data.channelConfig[activeMasterInst].pausePulse"
                >
                  <el-option
                    value="SENT_PAUSE_PULSE_DISABLED"
                    label="Disable SENT pause pulse"
                  />
                  <el-option
                    value="SENT_PAUSE_PULSE_ENABLED"
                    label="Enable SENT pause pulse"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Successive Calibration Check"
                prop="successiveCal"
              >
                <el-select
                  v-model="data.channelConfig[activeMasterInst].successiveCal"
                  style="width:400px"
                >
                  <el-option
                    value="SENT_SUCC_CAL_CHK_LOW_LATENCY"
                    label="Select Low Latency Option as SAE Specification"
                  />
                  <el-option
                    value="SENT_SUCC_CAL_CHK_PREFERRED"
                    label="Select Preferred but High Latency Option as SAE Specification"
                  />
                </el-select>
              </el-form-item>
            </el-form>
          </div>
        </el-col>
      </el-row>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, computed, reactive, watch, toRaw, onBeforeUnmount, nextTick, isProxy } from "vue";
import { Plugin } from "plugin";
import {
  Plus,
  Delete,
  FolderOpened,
} from '@element-plus/icons-vue'
import type { FormInstance, FormRules, FormItemProp } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const sentFeature = ref({"sent_tick_min":3})


/* sent channel configuration */
const defChannelParam = {
  name: 'sent_channel_config0',
  channelId: 0,
  tickDuration: 3,
  inputFilter: 'SENT_INPUT_FILTER_4',
  fastMsgNibbles: 6,
  fastMsgDmaEnable: false,
  fastMsgCrcIncludeStatus: false,
  fastMsgCrcCheckDisable: false,
  fastMsgCrcType: 'SENT_CRC_RECOMMENDED',
  slowMsgDmaEnable: false,
  slowMsgCrcType: 'SENT_CRC_RECOMMENDED',

  idleCount: 'SENT_BUS_IDLE_DISABLED',
  calibVar: 'SENT_CALIB_VAR_25_PERCENT',
  diagPulse: 'SENT_PULSE_CHECK_BOTH',
  pausePulse: 'SENT_PAUSE_PULSE_ENABLED',
  successiveCal: 'SENT_SUCC_CAL_CHK_LOW_LATENCY',
}

/* SENT user configuration */
const defMasterParam = {
  readonly: true,
  fastMsgDmaPtr: 'NULL',
  slowMsgDmaPtr: 'NULL',
  callbackFunction: 'NULL',
  callbackParam: 'NULL',
  fastDmaChannel: 0,
  slowDmaChannel: 0,
  fastDmaFifoEnable: false,
  fastDmaFifoSize: 0,
  numOfConfigs: 1,
  channelConfig: [defChannelParam]
}


const lock=ref(false)

const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = ref(defMasterParam)

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
    const channelItem = JSON.parse(JSON.stringify(defChannelParam))
    channelItem.name = `sent_channel_config${data.value.channelConfig.length}`
    data.value.channelConfig.push(channelItem)
    activeMasterInst.value = data.value.channelConfig.length - 1
    data.value.numOfConfigs = data.value.channelConfig.length
  }

}

function deleteItem(key: string) {
  if (data.value.channelConfig.length >= 0) {
    if (key == 'master') {
      data.value.channelConfig.splice(activeMasterInst.value, 1)
      activeMasterInst.value = data.value.channelConfig.length - 1
      data.value.numOfConfigs = data.value.channelConfig.length
    }
  }
}

function masterRowClick(row) {
  const index = data.value.channelConfig.indexOf(row)
  activeMasterInst.value = index
  nextTick(() => {
    masterRef.value?.validate().catch((e: any) => {
      null
    })
  })
}

function channelCheck0(rule: any, value: any, callback: any){
  const channelCount = Number(data.value.numOfConfigs)
  if(Number.isNaN(channelCount)){
    callback(new Error(`Please input correct format!`))
  }else if (channelCount > sentFeature.value["sent_ch_num"]){
    callback(new Error(`SENT only has ${sentFeature.value["sent_ch_num"]} channels!`))
  }else{
    callback()
  }
}

/* Channel configuration check */
function channelCheck1(rule: any, value: any, callback: any){
  const channelId = Number(data.value.channelConfig[activeMasterInst.value].channelId)
  if(Number.isNaN(channelId)){
    callback(new Error(`Please input correct format!`))
  }else if (channelId >= sentFeature.value["sent_ch_num"]){
    callback(new Error(`SENT only has ${sentFeature.value["sent_ch_num"]} channels!`))
  }else{
    let channel = 0
    for (let i = 0; i < data.value.channelConfig.length; i++)
    {
      if(i != activeMasterInst.value){
        channel = Number(data.value.channelConfig[i].channelId)
        if(channelId == channel)
        {
          callback(new Error(`Duplicate of Channel${channelId}!`))
        }
      }
    }
    callback()
  }
}

function tickCheck(rule: any, value: any, callback: any){
  const tick = Number(value)
  if(Number.isNaN(tick)){
    callback(new Error(`Please input correct format!`))
  }else if (tick < sentFeature.value["sent_tick_min"]){
    callback(new Error(`TICK should >= ${sentFeature.value["sent_tick_min"]}`))
  }else if (tick > sentFeature.value["sent_tick_max"]){
    callback(new Error(`TICK should <= ${sentFeature.value["sent_tick_max"]}`))
  }else{
    callback()
  }
}

function nibbleCheck(rule: any, value: any, callback: any){
  const nibble = Number(value)
  if(Number.isNaN(nibble)){
    callback(new Error(`Please input correct format!`))
  }else if (nibble > sentFeature.value["nibble_max"]){
    callback(new Error(`Data nibble should <= ${sentFeature.value["nibble_max"]}`))
  }else{
    callback()
  }
}

const ConfigRules = reactive<FormRules>({
  numOfConfigs:[
    { validator: channelCheck0, trigger: 'change' }
  ]
})

const ChannelRules = reactive<FormRules>({
  channelId:[
    { validator: channelCheck1, trigger: 'change' }
  ],
  tickDuration:[
    { validator: tickCheck, trigger: 'change' }
  ],
  fastMsgNibbles:[
    { validator: nibbleCheck, trigger: 'change' }
  ]
})



const errorid: string[] = []
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});

function dataCheck() {
  masterRef.value?.validate().catch(()=>{null})
  for (const error of errorid) {
    plugin.deleteLog('error', error)
  }
  const configValidator = new Schema(ConfigRules);
  configValidator.validate(data.value, (errors, fields) => {
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

  const channelValidator = new Schema(ChannelRules);
  for (const i in data.value.channelConfig) {
    channelValidator.validate(data.value.channelConfig[i], (errors, fields) => {
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
      if (initData.pluginData != undefined) {
        data.value = initData.pluginData.data;
        lock.value = initData.pluginData.lock
      }
      /* Get SENT feature */ 
      sentFeature.value = plugin.getMeta("sent_features")
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