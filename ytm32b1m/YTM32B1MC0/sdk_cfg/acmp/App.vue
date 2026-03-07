<template>
  <div>
    <div>
      <el-row>
        <el-col :span="6">
          <el-table
            :data="data"
            style="width: 80%"
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
              label-width="250px"
              size="small"
              :disabled="lock"
              :rules="rules"
              hide-required-asterisk
            >
              <el-form-item
                label="Name"
                :prop="`${activeMasterInst}.name`"
              >
                <el-input v-model="data[activeMasterInst].name" />
              </el-form-item>
              <el-form-item
                label="Read-only"
                :prop="`${activeMasterInst}.readonly`"
              >
                <el-checkbox v-model="data[activeMasterInst].readonly" />
              </el-form-item>

              <el-collapse
                v-model="col1"
                style="width:100%;margin-bottom: 18px;"
              >
                <el-collapse-item
                  title="Comparator configuration"
                  name="1"
                >
                  <el-form-item
                    v-show="FEATURE_ACMP_HAS_CLK_SRC_SEL"
                    label="ACMP clock source"
                    :prop="`${activeMasterInst}.comparator.acmpClkSrc`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].comparator.acmpClkSrc"
                      style="width:100%"
                    >
                      <el-option
                        label="IPC clock"
                        value="ACMP_CLK_SRC_IPC"
                      />
                      <el-option
                        label="SIRC"
                        value="ACMP_CLK_SRC_SIRC"
                      />
                      <el-option
                        v-show="FEATURE_ACMP_SUPPORT_SXOSC_CLK"
                        label="SXOSC"
                        value="ACMP_CLK_SRC_SXOSC"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Sample mode"
                    :prop="`${activeMasterInst}.comparator.sampleMode`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].comparator.sampleMode"
                      style="width:100%"
                    >
                      <el-option
                        value="ACMP_COMMON_MODE"
                        label="Common mode"
                      />
                      <el-option
                        value="ACMP_TRIGGER_MODE"
                        label="Trigger mode"
                      />
                      <el-option
                        value="ACMP_WINDOW_MODE"
                        label="Window mode"
                      />
                      <el-option
                        v-show="FEATURE_ACMP_HAS_CONTINUOUS_MODE_IN_CTRL_MODE"
                        value="ACMP_CONTINUOUS_MODE"
                        label="Continuous mode"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Edge selection"
                    :prop="`${activeMasterInst}.comparator.edgeSelection`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].comparator.edgeSelection"
                      style="width:100%"
                    >
                      <el-option
                        value="ACMP_NO_EDGE"
                        label="No edge detection"
                      />
                      <el-option
                        value="ACMP_RISING_EDGE"
                        label="Rising edge detection"
                      />
                      <el-option
                        value="ACMP_FALLING_EDGE"
                        label="Falling edge detection"
                      />
                      <el-option
                        value="ACMP_BOTH_EDGES"
                        label="Dual edges detection"
                      />
                      <el-option
                        value="ACMP_HIGH_LEVEL"
                        label="High level detection"
                        :disabled="FEATURE_ACMP_SUPPORT_LEVEL_DETECTION==false"
                      />
                      <el-option
                        value="ACMP_LOW_LEVEL"
                        label="Low level detection"
                        :disabled="FEATURE_ACMP_SUPPORT_LEVEL_DETECTION==false"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    v-if="FEATURE_ACMP_HAS_OFFSET_CONTROL"
                    label="Disable hysteresis"
                    :prop="`${activeMasterInst}.comparator.hardBlockOffset`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].comparator.hardBlockOffset" />
                  </el-form-item>
                  <el-form-item
                    v-show="data[activeMasterInst].comparator.hardBlockOffset==false"
                    label="Hysteresis control"
                    :prop="`${activeMasterInst}.comparator.hysteresisLevel`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].comparator.hysteresisLevel"
                      style="width:100%"
                    >
                      <el-option
                        value="ACMP_HYS_LEVEL_0"
                        label="Comparator has no hysteresis internally"
                      />
                      <el-option
                        value="ACMP_HYS_LEVEL_1"
                        label="Comparator has 20mV hysteresis internally"
                      />
                      <el-option
                        value="ACMP_HYS_LEVEL_2"
                        label="Comparator has 40mV hysteresis internally"
                      />
                      <!-- <el-option value="ACMP_LEVEL_HYS_3" label="ACMP output to pin directly"></el-option> -->
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Output filtered or directly"
                    :prop="`${activeMasterInst}.comparator.outputSelect`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].comparator.outputSelect"
                      style="width:100%"
                    >
                      <el-option
                        value="ACMP_OUTPUT_FILTERED"
                        label="ACMP output filtered to pin"
                      />
                      <el-option
                        value="ACMP_OUTPUT_DIRECTLY"
                        label="ACMP output to pin directly"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Output polarity"
                    :prop="`${activeMasterInst}.comparator.outputPolarity`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].comparator.outputPolarity"
                      style="width:100%"
                    >
                      <el-option
                        value="ACMP_OUT_POL_NOT_INVERT"
                        label="Normal"
                      />
                      <el-option
                        value="ACMP_OUT_POL_INVERT"
                        label="Invert"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Power mode"
                    :prop="`${activeMasterInst}.comparator.powerMode`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].comparator.powerMode"
                      style="width:100%"
                    >
                      <el-option
                        value="ACMP_LOW_POWER"
                        label="Low power"
                      />
                      <el-option
                        value="ACMP_HIGH_SPEED"
                        label="High speed"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Filter enable"
                    :prop="`${activeMasterInst}.comparator.filterEnable`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].comparator.filterEnable" />
                  </el-form-item>
                  <el-form-item
                    v-show="data[activeMasterInst].comparator.filterEnable"
                    label="Filter clock source"
                    :prop="`${activeMasterInst}.comparator.filterClkSrc`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].comparator.filterClkSrc"
                      style="width:100%"
                    >
                      <el-option
                        label="Function clock"
                        value="ACMP_FILTER_SEL_FUNC_CLK"
                      />
                      <el-option
                        label="Bus clock"
                        value="ACMP_FILTER_SEL_BUS_CLK"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    v-show="data[activeMasterInst].comparator.filterEnable"
                    label="Filter sample period"
                    :prop="`${activeMasterInst}.comparator.filterSamplePeriod`"
                  >
                    <el-input v-model.number="data[activeMasterInst].comparator.filterSamplePeriod" />
                  </el-form-item>
                  <el-form-item
                    v-show="data[activeMasterInst].comparator.filterEnable"
                    label="Filter sample count"
                    :prop="`${activeMasterInst}.comparator.filterSampleCount`"
                  >
                    <el-input v-model.number="data[activeMasterInst].comparator.filterSampleCount" />
                  </el-form-item>
                  <el-form-item
                    v-if="FEATURE_ACMP_HAS_AUTODIS"
                    label="Disable hard block automatically"
                    :prop="`${activeMasterInst}.comparator.autoDisableHardBlock`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].comparator.autoDisableHardBlock" />
                  </el-form-item>
                  <el-form-item
                    label="Comparator interrupt enable"
                    :prop="`${activeMasterInst}.comparator.interruptEnable`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].comparator.interruptEnable" />
                  </el-form-item>
                  <el-form-item
                    label="DMA transfer enable"
                    :prop="`${activeMasterInst}.comparator.dmaTriggerEnable`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].comparator.dmaTriggerEnable" />
                  </el-form-item>
                </el-collapse-item>
                <el-collapse-item
                  title="Mux configuration"
                  name="2"
                >
                  <el-form-item
                    label="Positive port input signal source"
                    :prop="`${activeMasterInst}.mux.positiveInputSrc`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].mux.positiveInputSrc"
                      style="width:100%"
                    >
                      <el-option
                        label="Input source from DAC"
                        value="ACMP_INPUT_SRC_DAC"
                      />
                      <el-option
                        label="Input source from channel"
                        value="ACMP_INPUT_SRC_MUX"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Negative port input signal source"
                    :prop="`${activeMasterInst}.mux.negativeInputSrc`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].mux.negativeInputSrc"
                      style="width:100%"
                    >
                      <el-option
                        label="Input source from DAC"
                        value="ACMP_INPUT_SRC_DAC"
                      />
                      <el-option
                        label="Input source from channel"
                        value="ACMP_INPUT_SRC_MUX"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Positive port input channel"
                    :prop="`${activeMasterInst}.mux.positiveInputChnSel`"
                  >
                    <el-input v-model.number="data[activeMasterInst].mux.positiveInputChnSel" />
                  </el-form-item>
                  <el-form-item
                    label="Negative port input channel"
                    :prop="`${activeMasterInst}.mux.negativeInputChnSel`"
                  >
                    <el-input v-model.number="data[activeMasterInst].mux.negativeInputChnSel" />
                  </el-form-item>
                </el-collapse-item>
                <el-collapse-item
                  title="DAC configuration"
                  name="3"
                >
                  <el-form-item
                    label="DAC enable"
                    :prop="`${activeMasterInst}.dac.enable`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].dac.enable" />
                  </el-form-item>
                  <el-form-item
                    v-if="FEATURE_ACMP_HAS_DAC_VOLTAGE_REF_SRC"
                    label="DAC voltage reference"
                    :prop="`${activeMasterInst}.dac.voltageReferenceSource`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].dac.voltageReferenceSource"
                      style="width:100%"
                    >
                      <el-option
                        value="ACMP_EXT_REF"
                        label="VREFH/VREFL as supply reference source for DAC"
                      />
                      <el-option
                        value="ACMP_INT_REF"
                        label="Internal supply reference source(VDDA) for DAC"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="DAC digital value"
                    :prop="`${activeMasterInst}.dac.voltage`"
                  >
                    <el-input v-model.number="data[activeMasterInst].dac.voltage" />
                  </el-form-item>
                  <el-form-item
                    v-show="FEATURE_ACMP_HAS_DAC_OUTPUT"
                    label="DAC output enable"
                    :prop="`${activeMasterInst}.dac.outputEnable`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].dac.outputEnable" />
                  </el-form-item>
                </el-collapse-item>
                <el-collapse-item
                  title="Continuous mode configuration"
                  name="4"
                >
                  <el-form-item
                    label="Continuous mode enable"
                    :prop="`${activeMasterInst}.continuous.continuousEnable`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].continuous.continuousEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Continuous mode"
                    :prop="`${activeMasterInst}.continuous.continuousMode`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].continuous.continuousMode"
                      style="width:100%"
                    >
                      <el-option
                        value="ACMP_CONTINUOUS_LOOP_MODE"
                        label="Loop mode"
                      />
                      <el-option
                        value="ACMP_CONTINUOUS_ONE_SHOT_MODE"
                        label="One-shot mode"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    v-show="FEATURE_ACMP_HAS_TRIG_MODE_GATE"
                    label="One-shot trigger mode enable"
                    :prop="`${activeMasterInst}.continuous.oneshotTriggerEnable`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].continuous.oneshotTriggerEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Fixed port"
                    :prop="`${activeMasterInst}.continuous.fixedPort`"
                  >
                    <el-select
                      v-model="data[activeMasterInst].continuous.fixedPort"
                      style="width:100%"
                    >
                      <el-option
                        value="ACMP_FIXED_NEG_PORT"
                        label="Negative port"
                      />
                      <el-option
                        value="ACMP_FIXED_POS_PORT"
                        label="Positive port"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Continuous mode interrupt enable"
                    :prop="`${activeMasterInst}.continuous.continuousInterruptEnable`"
                  >
                    <el-checkbox v-model="data[activeMasterInst].continuous.continuousInterruptEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Sample period"
                    :prop="`${activeMasterInst}.continuous.samplePeriod`"
                  >
                    <el-input v-model.number="data[activeMasterInst].continuous.samplePeriod" />
                  </el-form-item>
                  <el-form-item
                    label="Sample position"
                    :prop="`${activeMasterInst}.continuous.samplePosition`"
                  >
                    <el-input v-model.number="data[activeMasterInst].continuous.samplePosition" />
                  </el-form-item>
                  <el-form-item
                    label="ACMP channel config"
                    :prop="`${activeMasterInst}.continuous.channelConfig`"
                  >
                    <div class="table">
                      <el-auto-resizer>
                        <template #default="{ height, width }">
                          <el-table-v2
                            :columns="columns"
                            :data="data[activeMasterInst].continuous.channelConfig"
                            :estimated-row-height="30"
                            :width="width"
                            :header-height="30"
                            :height="height"
                            fixed
                          />
                        </template>
                      </el-auto-resizer>
                    </div>
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
import { onMounted, ref, h, reactive, watch, onBeforeUnmount, nextTick } from "vue";
import { Plugin } from "plugin";
import {
  Plus,
  Delete,
  FolderOpened
} from '@element-plus/icons-vue'
import {valid} from "./valid.js"
import type { FormInstance, FormRules, FormItemRule, Column } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const col1 = ref([])

const columns: Column<any>[] = [
  {
    key: 'column-n-1',
    width: 100,
    title: 'Row No.',
    cellRenderer: ({ rowIndex }) => h('span', rowIndex),
    align: 'center',
  },
  {
    key: 'enable',
    title: 'Enable',
    dataKey: 'enable',
    width: 100,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        type: 'checkbox',
        disabled: lock.value,
        checked: rowData.enable,
        'onInput': (event) => {
          rowData.enable = event.target.checked
        }
      })
    }
  },
  {
    key: 'expectation',
    title: 'Expectation',
    dataKey: 'expectation',
    width: 150,
    align: 'center',
    cellRenderer: ({ rowData }) => {
            return h('select', {
                size: 'small',
                disabled:lock.value,
                value: rowData.expectation,
                'onChange': (event) => rowData.expectation = event.target.value,
                style: { width: '100%' }
            },
                [h('option', {
                    label: 'Pos less than neg',
                    value: 'ACMP_EXPECT_POS_LESS_THAN_NEG'
                }),
                h('option', {
                    label: 'Pos greater than neg',
                    value: 'ACMP_EXPECT_POS_GREAT_THAN_NEG'
                })
                ])
        }
  },
]

const defMasterParam: any = {
  name: 'ACMP_Config0',
  readonly: true,
  comparator: {
    acmpClkSrc: 'ACMP_CLK_SRC_IPC',
    sampleMode: 'ACMP_COMMON_MODE',
    edgeSelection: 'ACMP_NO_EDGE',
    hardBlockOffset: false,
    hysteresisLevel: 'ACMP_HYS_LEVEL_0',
    outputSelect: 'ACMP_OUTPUT_FILTERED',
    outputPolarity: 'ACMP_OUT_POL_NOT_INVERT',
    powerMode: 'ACMP_HIGH_SPEED',
    filterEnable: false,
    filterClkSrc: 'ACMP_FILTER_SEL_FUNC_CLK',
    filterSamplePeriod: 48,
    filterSampleCount: 24,
    autoDisableHardBlock: false,
    interruptEnable: false,
    dmaTriggerEnable: false,
  },
  mux: {
    positiveInputSrc: 'ACMP_INPUT_SRC_DAC',
    negativeInputSrc: 'ACMP_INPUT_SRC_DAC',
    positiveInputChnSel: 0,
    negativeInputChnSel: 0
  },
  dac: {
    voltageReferenceSource: 'ACMP_EXT_REF',
    enable: false,
    outputEnable: false,
    voltage: 0,
  },
  continuous: {
    continuousEnable: false,
    continuousMode: 'ACMP_CONTINUOUS_LOOP_MODE',
    oneshotTriggerEnable: false,
    fixedPort: 'ACMP_FIXED_NEG_PORT',
    samplePeriod: 48,
    samplePosition: 24,
    continuousInterruptEnable: false,
    channelConfig: [
      {
        enable: false,
        expectation: 'ACMP_EXPECT_POS_LESS_THAN_NEG'
      },
      {
        enable: false,
        expectation: 'ACMP_EXPECT_POS_LESS_THAN_NEG'
      },
      {
        enable: false,
        expectation: 'ACMP_EXPECT_POS_LESS_THAN_NEG'
      },
      {
        enable: false,
        expectation: 'ACMP_EXPECT_POS_LESS_THAN_NEG'
      },
      {
        enable: false,
        expectation: 'ACMP_EXPECT_POS_LESS_THAN_NEG'
      },
      {
        enable: false,
        expectation: 'ACMP_EXPECT_POS_LESS_THAN_NEG'
      },
      {
        enable: false,
        expectation: 'ACMP_EXPECT_POS_LESS_THAN_NEG'
      },
      {
        enable: false,
        expectation: 'ACMP_EXPECT_POS_LESS_THAN_NEG'
      },
    ]
  }
}

const lock = ref(false)

const plugin = new Plugin({
  lockChange: (val) => {
    lock.value = val;
  }
});
/*Stage and Table height define*/
const data = ref<any[]>([defMasterParam])
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
    
    // const lastName=data.value[data.value.length-1].name
    /* create new  master name*/
    // const name = lastName.replace(/(\d+)$/, function (fullMatch, n) {
    //   return String(Number(n) + 1);
    // });
    const newItem=JSON.parse(JSON.stringify(defMasterParam))
    newItem.name = `ACMP_Config${data.value.length}`
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
  masterRef.value?.clearValidate()
  nextTick(() => {

    masterRef.value?.validate().catch((e: any) => {
      null
    })
  })
}

const formErrors = ref<Record<string, string>>({})
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});




const rules=ref<FormRules>({})
watch(formErrors, (val: Record<string, string>) => {
    rules.value = {}
    const keys = Object.keys(val)
    for (const key of keys) {
        // const skey=key.split('.')
        // if(Number(skey[0])==activeMasterInst.value){
        //   rules.value[skey.slice(1).join('.')] = {
        //     validator: () => false,
        //     message: val[key]
        // }
        rules.value[key] = {
            validator: () => false,
            message: val[key]
        }
      }


       
    
    masterRef.value?.clearValidate()
    void nextTick(() => {
        void masterRef.value?.validate().catch(() => { null })
    })
})




function dataCheck() {
  for (const error of Object.keys(formErrors.value)) {
    plugin.deleteLog('error', error)
  }
  valid(data.value).then((val:any)=>{
    formErrors.value=val
    for(const key of Object.keys(formErrors.value)){
      plugin.addLog('error', key, formErrors.value[key])
    }
  }).catch((val)=>{
      plugin.addLog('error', 'system', val.message)
  })
  
}





const FEATURE_ACMP_HAS_AUTODIS = ref(false)
const FEATURE_ACMP_HAS_CLK_SRC_SEL = ref(false)
const FEATURE_ACMP_HAS_DAC_VOLTAGE_REF_SRC=ref(false)
const FEATURE_ACMP_HAS_DAC_OUTPUT=ref(false)
const FEATURE_ACMP_HAS_TRIG_MODE_GATE=ref(false)
const FEATURE_ACMP_SUPPORT_LEVEL_DETECTION = ref(true)
const FEATURE_ACMP_SUPPORT_SXOSC_CLK = ref(false)
const FEATURE_ACMP_HAS_OFFSET_CONTROL = ref(false)
const FEATURE_ACMP_HAS_CONTINUOUS_MODE_IN_CTRL_MODE = ref(false)
const FEATURE_ACMP_HAS_FILTER_BYPASS=ref(false)
onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {
      FEATURE_ACMP_HAS_AUTODIS.value = plugin.getMeta('FEATURE_ACMP_HAS_AUTODIS')
      FEATURE_ACMP_HAS_CLK_SRC_SEL.value=plugin.getMeta('FEATURE_ACMP_HAS_CLK_SRC_SEL')
      FEATURE_ACMP_HAS_DAC_VOLTAGE_REF_SRC.value=plugin.getMeta('FEATURE_ACMP_HAS_DAC_VOLTAGE_REF_SRC')
      FEATURE_ACMP_HAS_DAC_OUTPUT.value=plugin.getMeta('FEATURE_ACMP_HAS_DAC_OUTPUT')
      FEATURE_ACMP_HAS_TRIG_MODE_GATE.value=plugin.getMeta('FEATURE_ACMP_HAS_TRIG_MODE_GATE')
      FEATURE_ACMP_SUPPORT_LEVEL_DETECTION.value=plugin.getMeta('FEATURE_ACMP_SUPPORT_LEVEL_DETECTION')
      FEATURE_ACMP_SUPPORT_SXOSC_CLK.value=plugin.getMeta('FEATURE_ACMP_SUPPORT_SXOSC_CLK')
      FEATURE_ACMP_HAS_OFFSET_CONTROL.value=plugin.getMeta('FEATURE_ACMP_HAS_OFFSET_CONTROL')
      FEATURE_ACMP_HAS_CONTINUOUS_MODE_IN_CTRL_MODE.value=plugin.getMeta('FEATURE_ACMP_HAS_CONTINUOUS_MODE_IN_CTRL_MODE')
      FEATURE_ACMP_HAS_FILTER_BYPASS.value=plugin.getMeta('FEATURE_ACMP_HAS_FILTER_BYPASS')

      if (initData.pluginData != undefined) {
        data.value = initData.pluginData.data;
        lock.value = initData.pluginData.lock
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

.table {
  width: v-bind(winW*0.75-60+'px');
  height: 320px;
  border-radius: 4px;
  border: 1px solid var(--el-border-color);
}
</style>