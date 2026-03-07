<template>
  <div>
    <div style="margin:5px">
      <el-switch
        v-model="enable"
        active-text="Enable"
        inactive-text="Disable"
        @change="emits('change')"
        :disabled="lock"
      />
    </div>
    <div v-if="enable">
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
                </span><span v-if="scope.row.name">{{ scope.row.name }}</span><span v-else>{{
                  scope.$index
                }}</span>
              </template>
            </el-table-column>
          </el-table>
        </el-col>
        <el-col :span="18">
          <div
            v-for="(item, index) in data"
            :key="index"
          >
            <el-form
              :ref="el => { masterRef[index] = el }"
              :model="item"
              label-width="300px"
              :disabled="lock"
              size="small"
              v-show="index == activeMasterInst"
              :rules="MasterRules"
              hide-required-asterisk
            >
              <el-form-item
                label="Name"
                prop="name"
              >
                <el-input v-model="item.name" />
              </el-form-item>
              <el-form-item
                label="Read-only"
                prop="readonly"
              >
                <el-checkbox v-model="item.readonly" />
              </el-form-item>
              
              <el-form-item
                label="Number of channels"
                prop="channelNum"
              >
                <el-input-number
                  v-model="item.channelNum"
                  :min="1"
                  :max="mpwm_channel_num"
                />
              </el-form-item>
              
              <!-- channel configuration -->
              <el-collapse
                v-model="col"
                style="width:100%;margin-bottom: 18px;"
              >
                <el-collapse-item
                  :title="`Channel Configuration ${i}`"
                  v-for="(e, i) in item.channelNum"
                  :key="i"
                  :name="'Channel' + i"
                >
                  <!-- hardware channel ID -->
                  <el-form-item
                    label="Hardware channel ID"
                    :prop="`pulseCounterChannelConfig[${i}].hwChannelId`"
                    :rules="hwChannelIdRule"
                  >
                    <el-input v-model.number="item.pulseCounterChannelConfig[i].hwChannelId" />
                  </el-form-item>

                  <!-- clock source prescaler -->
                  <el-form-item
                    label="Clock source prescaler"
                    :prop="`pulseCounterChannelConfig[${i}].clkPrs`"
                  >
                    <el-select
                      v-model.number="item.pulseCounterChannelConfig[i].clkPrs"
                      style="width:100%"
                    >
                      <el-option
                        label="1"
                        value="MPWM_CLK_DIV_BY_1"
                      />

                      <el-option
                        label="2"
                        value="MPWM_CLK_DIV_BY_2"
                      />

                      <el-option
                        label="4"
                        value="MPWM_CLK_DIV_BY_4"
                      />

                      <el-option
                        label="8"
                        value="MPWM_CLK_DIV_BY_8"
                      />

                      <el-option
                        label="16"
                        value="MPWM_CLK_DIV_BY_16"
                      />

                      <el-option
                        label="32"
                        value="MPWM_CLK_DIV_BY_32"
                      />

                      <el-option
                        label="64"
                        value="MPWM_CLK_DIV_BY_64"
                      />

                      <el-option
                        label="128"
                        value="MPWM_CLK_DIV_BY_128"
                      />
                    </el-select>
                  </el-form-item>

                  <!-- capture edge -->
                  <el-form-item
                    label="Capture edge selection"
                  >
                    <el-select
                      v-model.number="item.pulseCounterChannelConfig[i].edge"
                      style="width:100%"
                    >
                      <el-option
                        value="MPWM_PULSE_DISABLE"
                        label="No edge, disable capture"
                      />
                      <el-option
                        value="MPWM_PULSE_RISING_EDGE"
                        label="Rising edge"
                      />
                      <el-option
                        value="MPWM_PULSE_FALLING_EDGE"
                        label="Falling edge"
                      />
                      <el-option
                        value="MPWM_PULSE_BOTH_EDGES"
                        label="Both risign and falling edges"
                      />
                    </el-select>
                  </el-form-item>

                  <!-- final value of counter -->
                  <el-form-item
                    label="Final value of pulse counts"
                    :prop="`pulseCounterChannelConfig[${i}].maxPulseNum`"
                  >
                    <el-input v-model.number="item.pulseCounterChannelConfig[i].maxPulseNum" />
                  </el-form-item>

                  <!-- compare value of counter -->
                  <el-form-item
                    label="Compare value of pulse counts"
                    :prop="`pulseCounterChannelConfig[${i}].cmpPulseNum`"
                  >
                    <el-input v-model.number="item.pulseCounterChannelConfig[i].cmpPulseNum" />
                  </el-form-item>


                  <!-- reload point(source) -->
                  <el-form-item
                    label="Reload point"
                    :prop="`pulseCounterChannelConfig[${i}].reloadSrc`"
                  >
                    <el-select
                      v-model.number="item.pulseCounterChannelConfig[i].reloadSrc"
                      style="width:100%"
                    >
                      <el-option
                        label="Reload when count is equal to PERIOD"
                        value="MPWM_RELOAD_ON_PERIOD"
                      />
                      <el-option
                        label="Reload when hardware trigger arrives"
                        value="MPWM_RELOAD_ON_HW_TRIG"
                      />
                      <el-option
                        label="Reload when software trigger arrives"
                        value="MPWM_RELOAD_ON_SW_TRIG"
                      />
                      <el-option
                        label="Reload machanism is disabled, it reloads immediately"
                        value="MPWM_RELOAD_DISABLE"
                      />
                    </el-select>
                  </el-form-item>

                  <!-- enable dma -->
                  <el-form-item
                    label="DMA enable"
                    :prop="`pulseCounterChannelConfig[${i}].dmaEn`"
                  >
                    <el-checkbox v-model.number="item.pulseCounterChannelConfig[i].dmaEn" />
                  </el-form-item>

                  <!-- Count mode -->
                  <el-form-item
                    label="Count mode"
                    :prop="`pulseCounterChannelConfig[${i}].countMode`"
                  >
                    <el-select
                      v-model.number="item.pulseCounterChannelConfig[i].countMode"
                      style="width:100%"
                    >
                      <el-option
                        label="Continuous mode"
                        value="MPWM_CONTINUOUS_MODE"
                      />
                      <el-option
                        label="One-shot mode"
                        value="MPWM_ONESHOT_MODE"
                      />
                    </el-select>
                  </el-form-item>

                  <!-- debug mode -->
                  <el-form-item
                    label="Debug mode"
                    :prop="`pulseCounterChannelConfig[${i}].debugMode`"
                  >
                    <el-checkbox v-model.number="item.pulseCounterChannelConfig[i].debugMode" />
                  </el-form-item>

                  

                  <!-- channel interrupt enable -->
                  <el-form-item
                    label="Channel interrupt enable"
                    :prop="`pulseCounterChannelConfig[${i}].intEn`"
                  >
                    <el-checkbox v-model.number="item.pulseCounterChannelConfig[i].intEn" />
                  </el-form-item>

                  <!-- Timer overflow interrupt enable -->
                  <el-form-item
                    label="Overflow interrupt enable"
                    :prop="`pulseCounterChannelConfig[${i}].timerOverflowIntEn`"
                  >
                    <el-checkbox v-model.number="item.pulseCounterChannelConfig[i].timerOverflowIntEn" />
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
import { onMounted, ref, h, reactive, watch, toRef, onBeforeUnmount, inject, onBeforeMount } from "vue";
import {
  Plus,
  Delete,
  FolderOpened
} from '@element-plus/icons-vue'
import { ElButton } from 'element-plus'
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'

const activeMasterInst = ref(0);
const masterRef = ref<Record<number, FormInstance>>({})
const col = ref(['1'])
const plugin = inject("plugin") as any;
const chDef = {
  hwChannelId: 0,
  clkPrs: 'MPWM_CLK_DIV_BY_1',
  edge: 'MPWM_PULSE_DISABLE',
  maxPulseNum: 0xFFFF,
  cmpPulseNum: 0,
  dmaEn: false,
  debugMode: false,
  reloadSrc: 'MPWM_RELOAD_ON_PERIOD',
  countMode: 'MPWM_CONTINUOUS_MODE',
  intEn: false,
  timerOverflowIntEn: false,
}

const defMasterParam = {

  name: 'MPWM_PC_Config',
  readonly: false,
  channelNum: 1,
  pulseCounterChannelConfig: [chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef]
}

const props = defineProps<{
  data: any;
  lock: boolean
}>();
const data = toRef(props.data, 'data')
const lock = toRef(props, 'lock')
const enable = toRef(props.data, 'enable')
const winW = ref(window.innerWidth)
function resizeHandler() {
  winW.value = window.innerWidth
}

function addItem(key: string) {

  if (key == 'master') {
    const newItem=JSON.parse(JSON.stringify(defMasterParam))
    const default_name=newItem.name
    newItem.name=`${default_name}${data.value.length}`
    data.value.push(newItem)
    activeMasterInst.value = data.value.length - 1

  }

}

function deleteItem(key: string) {
  if (data.value.length >= 0) {
    if (key == 'master') {
      data.value.splice(activeMasterInst.value, 1)
      delete masterRef.value[activeMasterInst.value]
      activeMasterInst.value = data.value.length - 1
      
    }
  }
}

function masterRowClick(row) {
  const index = data.value.indexOf(row)
  activeMasterInst.value = index
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

let dataCheckIndex=0


function hwChannelId0RuleCheck(rule: any, value: any, callback: any, source: any) {
  const err = false;
  const val = data.value[dataCheckIndex]

  if (value >= mpwm_channel_num.value)
  {
    callback(new Error('Hardware channel number is wrong'))
  }

  const cnt = {}
  for (let i = 0; i < val.channelNum; i++) {
    if (cnt[val.pulseCounterChannelConfig[i].hwChannelId] == undefined) {
      cnt[val.pulseCounterChannelConfig[i].hwChannelId] = 1
    } else {
      cnt[val.pulseCounterChannelConfig[i].hwChannelId]++
    }
  }

  for (const key of Object.keys(cnt)) {
    if (cnt[key] > 1) {
      callback(new Error(`Duplicate hardware channel ${key}`))
    }
  }
  if (!err) {
    callback()
  }


}


const hwChannelIdRule: FormItemRule[] | FormItemRule = 
[
  { validator: hwChannelId0RuleCheck },
]



const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  nNumOutputChannels: [
    { type: 'number', required: true, min: 0, max: 8, message: 'Please input a channel num 0-8', trigger: 'change' },
  ],
})



const errorid: string[] = []
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});






async function dataCheck() {
  for (const error of errorid) {
    plugin.deleteLog('error', error)
  }

  for (const i in Object.keys(masterRef.value)) {
    dataCheckIndex=Number(i)
    try{
      await masterRef.value[i]?.validate()
    }catch(e){
      const errs =e as Record<string,any[]>
      for (const val of Object.values(errs)) {
          for (const j in val) {
            const id = `OC-${val[j].field}-${i}-${j}`
            plugin.addLog('error', id, val[j].message)
            errorid.push(id)
          }
        }
    }
    dataCheckIndex=activeMasterInst.value
  }
}

const emits = defineEmits(['change'])


const mpwm_channel_num = ref(0)

onBeforeMount(() => {
  mpwm_channel_num.value = plugin.getMeta('mpwm_channel_num')
})
onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  dataCheck();
  watch(data, () => {
    dataCheck();
    emits('change')
  }, { deep: true })
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

.table {
  width: v-bind(winW * 0.75 - 60 + 'px');
  height: 160px;
  border-radius: 4px;
  border: 1px solid var(--el-border-color);
}
</style>