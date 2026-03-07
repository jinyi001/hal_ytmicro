<template>
  <div v-if="show">
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
          <div v-if="data[activeMasterInst]">
            <el-form
              ref="masterRef"
              :model="data[activeMasterInst]"
              label-width="180px"
              size="small"
              :rules="MasterRules"
              hide-required-asterisk
              :disabled="lock"
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
                label="ADC instance"
                prop="inst"
              >
                <el-select
                  v-model.number="data[activeMasterInst].inst"
                  style="width:100%"
                >
                  <el-option
                    v-for="i in adcNum"
                    :value="i-1"
                    :key="i"
                    :label="`ADC${i-1}`"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Clock divider"
                prop="clockDivider"
              >
                <el-input v-model.number="data[activeMasterInst].clockDivider" />
              </el-form-item>
              <el-form-item label="Func_clk_div">
                {{ Func_clk_div }}MHz (Change in Clock Configuration)
              </el-form-item>
              <el-form-item
                label="Start time"
                prop="startTime"
              >
                <template #label="{ label }">
                  {{ label }}<el-tooltip
                    effect="dark"
                    content="ADC stabilization time(min 2us) count, Start time needs be set more than 2us*Func_clk_div"
                  >
                    <el-button
                      :icon="QuestionFilled"
                      circle
                      text
                    />
                  </el-tooltip>
                </template>
                <el-input v-model.number="data[activeMasterInst].startTime">
                  <template #append>
                    Ticks
                  </template>
                </el-input>
              </el-form-item>
              <el-form-item
                label="Sample time"
                prop="sampleTime"
              >
                <template #label="{ label }">
                  {{ label }}<el-tooltip
                    effect="dark"
                    content="ADC sample time for all channels"
                  >
                    <el-button
                      :icon="QuestionFilled"
                      circle
                      text
                    />
                  </el-tooltip>
                </template>
                <el-input v-model.number="data[activeMasterInst].sampleTime">
                  <template #append>
                    Ticks
                  </template>
                </el-input>
              </el-form-item>
              <el-form-item
                label="Enable overrun mode"
                prop="overrunMode"
              >
                <template #label="{ label }">
                  {{ label }}<el-tooltip
                    effect="dark"
                    content="ADC overrun mode configuration, set the performance when ADC FIFO overrun"
                  >
                    <el-button
                      :icon="QuestionFilled"
                      circle
                      text
                    />
                  </el-tooltip>
                </template>
                <el-checkbox v-model="data[activeMasterInst].overrunMode" />
              </el-form-item>
              <el-form-item
                label="Enable autooff"
                prop="autoOffEnable"
              >
                <template #label="{ label }">
                  {{ label }}<el-tooltip
                    effect="dark"
                    content="ADC auto-off model,set the power management feature when ADC idle"
                  >
                    <el-button
                      :icon="QuestionFilled"
                      circle
                      text
                    />
                  </el-tooltip>
                </template>
                <el-checkbox
                  v-model="data[activeMasterInst].autoOffEnable"
                  @change="autoOffChange"
                />
              </el-form-item>
              <el-form-item
                label="Wait FIFO read"
                prop="waitEnable"
              >
                <template #label="{ label }">
                  {{ label }}<el-tooltip
                    effect="dark"
                    content=" ADC wait model, a new conversation can start only when FIFO is not full"
                  >
                    <el-button
                      :icon="QuestionFilled"
                      circle
                      text
                    />
                  </el-tooltip>
                </template>
                <el-checkbox v-model="data[activeMasterInst].waitEnable" />
              </el-form-item>
              <el-form-item
                label="Trigger source"
                prop="triggerSource"
                v-show="false"
              >
                <el-input v-model.number="data[activeMasterInst].triggerSource" />
              </el-form-item>
              <el-form-item
                label="Trigger type"
                prop="trigger"
              >
                <el-select
                  v-model="data[activeMasterInst].trigger"
                  style="width:100%"
                >
                  <el-option
                    value="ADC_TRIGGER_SOFTWARE"
                    label="Software trigger"
                  />
                  <el-option
                    value="ADC_TRIGGER_HARDWARE"
                    label="Hardware trigger"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Alignment"
                prop="align"
              >
                <el-select
                  v-model="data[activeMasterInst].align"
                  style="width:100%"
                >
                  <el-option
                    value="ADC_ALIGN_LEFT"
                    label="Left alignment"
                  />
                  <el-option
                    value="ADC_ALIGN_RIGHT"
                    label="Right alignment"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Resolution"
                prop="resolution"
              >
                <el-select
                  v-model="data[activeMasterInst].resolution"
                  style="width:100%"
                  @change="data[activeMasterInst].compareConfig.resolution=data[activeMasterInst].resolution"
                >
                  <el-option
                    value="ADC_RESOLUTION_12BIT"
                    label="12-bit resolution mode"
                  />
                  <el-option
                    value="ADC_RESOLUTION_10BIT"
                    label="10-bit resolution mode"
                  />
                  <el-option
                    value="ADC_RESOLUTION_8BIT"
                    label="8-bit resolution mode"
                  />
                  <el-option
                    value="ADC_RESOLUTION_6BIT"
                    label="6-bit resolution mode"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Enable ADC DMA"
                prop="dmaEnable"
              >
                <el-checkbox
                  v-model="data[activeMasterInst].dmaEnable"
                />
              </el-form-item>
              <el-form-item
                label="DMA water mark"
                prop="dmaWaterMark"
              >
                <el-input
                  v-model.number="data[activeMasterInst].dmaWaterMark"
                  :disabled="!data[activeMasterInst].dmaEnable"
                />
              </el-form-item>

              <el-collapse
                v-model="col1"
                style="width:100%;margin-bottom: 18px;"
              >
                <el-collapse-item
                  title="Sequence configuration"
                  name="1"
                >
                  <el-form-item
                    label="Alarm Time"
                    prop="sequenceConfig.sequenceMode"
                    required
                  >
                    <el-select
                      v-model.number="data[activeMasterInst].sequenceConfig.sequenceMode"
                      style="width:100%"
                    >
                      <el-option
                        value="ADC_CONV_STEP"
                        label="One channel convert per trigger"
                      />
                      <el-option
                        value="ADC_CONV_LOOP"
                        label="One loop convert per trigger"
                      />
                      <el-option
                        value="ADC_CONV_CONTINUOUS"
                        label="Continuous convert per trigger"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Sequence end interrupt"
                    prop="sequenceConfig.sequenceIntEnable"
                  >
                    <el-checkbox v-model="data[activeMasterInst].sequenceConfig.sequenceIntEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Conversion end interrupt"
                    prop="sequenceConfig.convIntEnable"
                  >
                    <el-checkbox v-model="data[activeMasterInst].sequenceConfig.convIntEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Ready interrupt"
                    prop="sequenceConfig.readyIntEnable"
                  >
                    <el-checkbox v-model="data[activeMasterInst].sequenceConfig.readyIntEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Overrun interrupt"
                    prop="sequenceConfig.ovrunIntEnable"
                  >
                    <el-checkbox v-model="data[activeMasterInst].sequenceConfig.ovrunIntEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Sample interrupt"
                    prop="sequenceConfig.sampIntEnable"
                  >
                    <el-checkbox v-model="data[activeMasterInst].sequenceConfig.sampIntEnable" />
                  </el-form-item>
                  <el-form-item
                    label="ADC channels"
                    prop="sequenceConfig.channels"
                  >
                    <el-table-v2
                      :columns="columns"
                      :data="data[activeMasterInst].sequenceConfig.channels"
                      :estimated-row-height="30"
                      :width="winW * 0.75 - 60"
                      :header-height="30"
                      :height="320"
                      :footer-height="40"
                      fixed
                    >
                      <!-- <template #empty>
              <el-empty :image-size="80" />
            </template> -->
                      <template #footer>
                        <el-alert
                          v-if="filterError != ''"
                          center
                          show-icon
                          :title="filterError"
                          type="error"
                          :closable="false"
                        />
                      </template>
                    </el-table-v2>
                  </el-form-item>
                </el-collapse-item>
                <el-form-item
                  style="margin-top: 10px;"
                  label="Number of compare channel"
                  prop="adcCompareCnt"
                  v-show="wdgCntShow"
                >
                  <el-input-number
                    v-model="data[activeMasterInst].adcCompareCnt"
                    :min="0"
                    :max="wdgInstance"
                  />
                </el-form-item>
                <el-collapse-item
                  :title="`Compare configuration #${i}`"
                  v-for="i in data[activeMasterInst].adcCompareCnt"
                  :key="i"
                  :name="`2${i}`"
                >
                  <el-form-item
                    label="Enable compare feature"
                    :prop="`compareConfig.${i - 1}.compareEnable`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].compareConfig[i - 1].compareEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Enable all channel compare"
                    :prop="`compareConfig.${i - 1}.compareAllChannelEnable`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].compareConfig[i - 1].compareAllChannelEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Compare channel select"
                    :prop="`compareConfig.${i - 1}.compChalSel`"
                  >
                    <el-select
                      v-model.number="data[activeMasterInst].compareConfig[i - 1].compChalSel"
                      style="width: 100%;"
                      :disabled="!(data[activeMasterInst].compareConfig[i - 1].compareEnable && !data[activeMasterInst].compareConfig[i - 1].compareAllChannelEnable)"
                    >
                      <el-option
                        v-for="val in data[activeMasterInst].sequenceConfig.channels"
                        :value="val.channel"
                        :key="val.channel"
                        :label="val.channel"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Watch dog instance"
                    :prop="`compareConfig.${i - 1}.comWdchIndex`"
                    v-show="wdgCntShow"
                  >
                    <el-input-number
                      v-model="data[activeMasterInst].compareConfig[i - 1].comWdchIndex"
                      :min="0"
                      :max="wdgInstance-1"
                    />
                  </el-form-item>
                  <el-form-item
                    label="Compare high"
                    :prop="`compareConfig.${i - 1}.compHigh`"
                    :rules="compHighRule"
                  >
                    <el-input v-model.number="data[activeMasterInst].compareConfig[i - 1].compHigh" />
                  </el-form-item>
                  <el-form-item
                    label="Compare low"
                    :prop="`compareConfig.${i - 1}.compLow`"
                    :rules="compLowRule"
                  >
                    <el-input v-model.number="data[activeMasterInst].compareConfig[i - 1].compLow" />
                  </el-form-item>
                  <el-form-item
                    label="Compare interrupt"
                    :prop="`compareConfig.${i - 1}.compIntEnable`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].compareConfig[i - 1].compIntEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Watchdog effective mode"
                    :prop="`compareConfig.${i - 1}.effectiveMode`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].compareConfig[i - 1].effectiveMode"
                      style="width:100%"
                    >
                      <el-option
                        value="ADC_AWG_EFFECTIVE_OUTSIDE"
                        label="Effective outside"
                      />
                      <el-option
                        value="ADC_AWG_EFFECTIVE_INSIDE"
                        label="Effective inside"
                      />
                    </el-select>
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
import { onMounted, ref, h, reactive, watch, onBeforeUnmount, nextTick, computed, isProxy, isReactive} from "vue";
import { Plugin } from "plugin";
import {
  Plus,
  Delete,
  FolderOpened,
  QuestionFilled
} from '@element-plus/icons-vue'
import { FormInstance, FormRules, FormItemRule, ElNotification } from 'element-plus'
import Schema from 'async-validator';
import { ElButton } from 'element-plus'
import type { Column } from '@element-plus/components/table-v2'
Schema.warning = function () { null };

const show=ref(false)
const Func_clk = ref([0,0,0,0])

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const col1 = ref(['1', '2'])
const wdgInstance = ref(1)
const wdgCntShow = ref(false)

const Func_clk_div = computed(()=>{
  return Func_clk.value[data.value[activeMasterInst.value].inst]/(data.value[activeMasterInst.value].clockDivider+1)
})

const defComConfig = {
    resolution: 'ADC_RESOLUTION_12BIT',
    compareEnable: false,
    compareAllChannelEnable: false,
    compHigh: 4095,
    compLow: 0,
    comWdchIndex:0,
    compIntEnable: false,
    effectiveMode:'ADC_AWG_EFFECTIVE_OUTSIDE'
  }

const defMasterParam: any = {
  name: 'adc_config0',
  readonly: true,
  inst: 0,
  clockDivider: 0,
  startTime: 0,
  sampleTime: 40,
  overrunMode: false,
  autoOffEnable: false,
  waitEnable: false,
  triggerSource: 0,
  trigger: 'ADC_TRIGGER_SOFTWARE',
  align: 'ADC_ALIGN_RIGHT',
  resolution: 'ADC_RESOLUTION_12BIT',
  dmaWaterMark: 0,
  dmaEnable: false,
  sequenceConfig: {
    channels: [],
    sequenceMode: 'ADC_CONV_STEP',
    sequenceIntEnable: false,
    convIntEnable: false,
    readyIntEnable: false,
    sampIntEnable: false,
    ovrunIntEnable: false,
  },
  adcCompareCnt: 1,
  compareConfig: [JSON.parse(JSON.stringify(defComConfig)), JSON.parse(JSON.stringify(defComConfig)), JSON.parse(JSON.stringify(defComConfig)), JSON.parse(JSON.stringify(defComConfig))],
}
const filterError = ref('')
function deleteFilter(index, rowData) {
  data.value[activeMasterInst.value].sequenceConfig.channels.splice(index, 1)
}

function addFilter() {
  if(data.value[activeMasterInst.value].sequenceConfig.channels.length==8){
    ElNotification.warning("Max sequence size is 8")
  }else{
    data.value[activeMasterInst.value].sequenceConfig.channels.push(JSON.parse(JSON.stringify({
      channel: ''
    })))
  }
}

function getADCChannel(): { label: string, value: string }[] {
  const chs: any[] = []

  for (const i of adcChs.value[data.value[activeMasterInst.value].inst]) {
    chs.push({
      value: `ADC_INPUTCHAN_EXT${i}`,
      label: `ADC_INPUTCHAN_EXT${i}`,
    })
  }
  if (adcExt.value.length > 0) {
    for(const v of adcExt.value[data.value[activeMasterInst.value].inst]){
      chs.push({
        value: `ADC_INPUTCHAN_EXT${v.ch}`,
        label: `ADC_INPUTCHAN_${v.label}`,
      })
    }
  }
  return chs
}
const columns: Column<any>[] = [
  {
    key: 'column-n-1',
    width: 100,
    title: 'Row No.',
    cellRenderer: ({ rowIndex }) => `${rowIndex + 1}`,
    align: 'center',
  },
  {
    key: 'channel',
    title: 'ADC Channel',
    dataKey: 'channel',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        size: 'small',
        value: rowData.channel,
        'onChange': (event) => rowData.channel = event.target.value,
        style: { width: '100%' }
      }, getADCChannel().map((i) => {
        return h('option', {
          label: i.label,
          value: i.value
        })
      }))
    }
  },
  {
    width: 100,
    align: 'center',
    fixed: 'right',
    headerCellRenderer: () => {
      return h(ElButton, {
        size: 'small',
        type: "primary",
        plain: true,
        icon: Plus,
        'onClick': () => addFilter(),
      })
    },
    cellRenderer: ({ rowData, rowIndex }) => {

      return h(ElButton, {
        size: 'small',
        type: "danger",
        plain: true,
        'onClick': () => deleteFilter(rowIndex, rowData),
      }, () => 'Delete')
    }
  },
]

const lock=ref(false)

const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = ref<any[]>([])
const winW = ref(window.innerWidth)
function resizeHandler() {
  winW.value = window.innerWidth
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
    // data.value.push(JSON.parse(JSON.stringify(defMasterParam)))
    // activeMasterInst.value = data.value.length - 1
    const newItem = JSON.parse(JSON.stringify(defMasterParam))
    newItem.name = `adc_config${data.value.length}`
    data.value.push(newItem)
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

function funcClkCheck(rule: any, value: any, callback: any) {
    if(Func_clk_div.value < adcFuncClk.value[0]){
        callback(new Error(`Function Clock should >= ${adcFuncClk.value[0]}MHz`))
    }else if(Func_clk_div.value > adcFuncClk.value[1]){
        callback(new Error(`Function Clock should <= ${adcFuncClk.value[1]}MHz`))
    }else {
        callback()
    }
}

const clockDividerRule: FormItemRule[] = [
  { type: 'number', message: 'Please input clock divider', trigger: 'change' },
  { type: 'number', min: 0, max: 15, message: 'Clock divider from 0-15', trigger: 'change' },
  { validator: funcClkCheck, trigger: 'change' }
]


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

function startTimeCheck(rule: any, value: any, callback: any) {
  if (value < 2 * Func_clk_div.value) {
    callback(new Error('Start time should >=2us*Func_clk_div'))
  } else if (value > 255) {
    callback(new Error('Start time should <256'))
  }
  else {
    callback()
  }
}

function dmaWaterMarkCheck(rule: any, value: any, callback: any, source: any) {
  const val=isProxy(source)?source:data.value[activeMasterInst.value]
  if (val.dmaEnable) {
    if ((value < 0) || (value > (val.sequenceConfig.channels.length - 1))) {
      callback(new Error('Water mark from 0 to ADC channels-1'))
    }
  } else {
    callback()
  }
}

function channelCheck(rule: any, value: { channel: string }[], callback: any) {
  filterError.value = ''
  const chs = getADCChannel().map((i) => {
    return i.value
  })
  if (value.length > 8) {
    filterError.value = 'Sequence max channels is 8'
    callback(new Error('filterError.value'))
  }
  const cA:string[]=[]
  for (const i in value) {
    if (value[i].channel == '') {
      filterError.value = `Please choose chanel ${i}`
      callback(new Error(filterError.value))
    } else {
      cA.push(value[i].channel)
      /* confirm instance match */
      if(Number(i) == activeMasterInst.value)
      {
        if (chs.indexOf(value[i].channel) == -1) {
            filterError.value = `ADC${data.value[activeMasterInst.value].inst} can't use the channel-${i}`
            callback(new Error(filterError.value))
        }
      }
    }
  }
  const t=[...new Set(cA)]
  if(t.length!=cA.length){
    plugin.addLog('warn', 'seq-ch','Duplicate channels are selected')
  }else{
    plugin.deleteLog('warn', 'seq-ch')
  }
  callback()
}

function compHighCheck(rule: any, value: any, callback: any, source: any) {
  const val=isProxy(source)?source:data.value[activeMasterInst.value].compareConfig
  let maxValue = 0
  switch (val.resolution) {
    case 'ADC_RESOLUTION_12BIT':
      maxValue = 4096;
      break;
    case 'ADC_RESOLUTION_10BIT':
      maxValue = 1024;
      break;
    case 'ADC_RESOLUTION_8BIT':
      maxValue = 256;
      break;
    case 'ADC_RESOLUTION_6BIT':
      maxValue = 64;
      break;
  }
  if(val.compareEnable){
    if (value >= maxValue) {
    callback(new Error(`high value should less than ${maxValue}`))
  }
  if (value < val.compLow) {
    callback(new Error('high value should > low'))
  }
  }
  
  callback()
}
const compHighRule: FormItemRule[] =
  [{ type: 'number', required: true, message: 'Please input compare high value', trigger: 'change' },
  { type: 'number', validator: compHighCheck, }
]


function compLowCheck(rule: any, value: any, callback: any, source: any) {
  const val=isProxy(source)?source:data.value[activeMasterInst.value].compareConfig

  if(val.compareEnable){
    if (value >= val.compHigh) { 
      callback(new Error('low value should < high'))
    }
    if (value < 0) {
      callback(new Error('low value should > 0'))
    }
  }
  callback()
}
const compLowRule: FormItemRule[] =
  [{ type: 'number', required: true, message: 'Please input compare low value', trigger: 'change' },
  { type: 'number', validator: compLowCheck, }
]



const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  inst: { type: 'number', required: true, message: 'Please choose a instance', trigger: 'change' },
  clockDivider: clockDividerRule,
  startTime: [
    { type: 'number', required: true, message: 'Please input start time', trigger: 'change' },
    { validator: startTimeCheck, trigger: 'change' }
  ],
  sampleTime: [
    { type: 'number', required: true, message: 'Please input sample time', trigger: 'change' },
    { type: 'number', min: 0, max: 255, message: 'Sample time from 0-255', trigger: 'change' },
  ],
  triggerSource: [
    { type: 'number', required: true, message: 'Please input trigger source', trigger: 'change' },
    { type: 'number', min: 0, max: 116, message: 'Trigger source from 0-116', trigger: 'change' },
  ],
  dmaWaterMark: { validator: dmaWaterMarkCheck, trigger: 'change' },
  sequenceConfig: {
    type: 'object',
    trigger: 'change',
    fields: {
      channels: { validator: channelCheck },
    }
  },
  // compareConfig: {
  //   type: 'object',
  //   trigger: 'change',
  //   fields: {
  //     compHigh: compHighRule,
  //     compLow: compLowRule,
  //   }
  // }
})


function autoOffChange(val){
  // const id=`autoOff-${activeMasterInst.value}`
  // if(!val){
  //   plugin.deleteLog('warn', id)
  // }else{
  //   plugin.addLog('warn', id, 'A startup-time(≥2us) is automatically inserted between the trigger event which starts the sampling and the conversion time of the ADC.')
  // }
 
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
const adcNum=ref(0)
const adcExt=ref<{label:string,ch:number}[]>([])
const adcChs= ref<number[][]>([])
const adcFuncClk = ref([])


onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {
      const pn = initData.projectInfo.partNumber
      adcNum.value=plugin.getMeta('adc_num')
      adcChs.value=plugin.getMeta('adc_chList')[pn]
      adcExt.value=plugin.getMeta('adc_ext')
      wdgInstance.value=plugin.getMeta('adc_wdg_inst')
      adcFuncClk.value=plugin.getMeta('adc_func_clk')
      if(wdgInstance.value>1){
        wdgCntShow.value=true
      }
      const perClkList=plugin.getMeta('perClkList')

      function updateClock(activeClock,clockData){
        const freqs=[0]
        for(const i of perClkList){
          freqs.push(clockData.data.clockArray[activeClock].data[i].freq.raw)
        }
       
        for(let index=0;index<adcNum.value;index++){
          let clockIndex=-1
          for(const i in initData.globalData.clock.data.clockArray[activeClock].data.periClocks){
            if(initData.globalData.clock.data.clockArray[activeClock].data.periClocks[i].name==`ADC${index}_CLK`){
              clockIndex=Number(i)
              break  
            }
          }
          Func_clk.value[index]=freqs[clockData.data.clockArray[activeClock].data.periClocks[clockIndex].periSrc]/1000000/(clockData.data.clockArray[activeClock].data.periClocks[clockIndex].div+1)
        }
      }
      updateClock(initData.globalData.clock.data.activeInst,initData.globalData.clock)
      defMasterParam.startTime=Func_clk.value[defMasterParam.inst]/(defMasterParam.clockDivider+1)*2
      
      plugin.watch('clock', (newData) => {
        if(data.value[activeMasterInst.value]){
          const activeClock=newData.data.activeInst
          updateClock(activeClock,newData)
          nextTick(()=>{
            dataCheck()
          })
          
        }
      })
      if (initData.pluginData != undefined) {
        data.value = initData.pluginData.data;
        lock.value = initData.pluginData.lock
      }
      else{
          addItem('master')
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
      show.value=true

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
<style scoped>
.el-table .active-row {
  color: #409EFF
}
</style>