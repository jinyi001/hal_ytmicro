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
              :disabled="lock"
              ref="masterRef"
              :model="item"
              label-width="300px"
              size="small"
              :rules="MasterRules"
              hide-required-asterisk
              v-show="index == activeMasterInst"
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
                    :prop="`pwmChannelConfig[${i}].hwChannelId`"
                    :rules="hwChannelIdRule"
                  >
                    <el-input v-model.number="item.pwmChannelConfig[i].hwChannelId" />
                  </el-form-item>

                  <!-- clock source prescaler -->
                  <el-form-item
                    label="Clock source prescaler"
                    :prop="`pwmChannelConfig[${i}].clkPrs`"
                  >
                    <el-select
                      v-model.number="item.pwmChannelConfig[i].clkPrs"
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

                  <!-- frequency in Hz -->
                  <el-form-item
                    label="Frequency in Hz"
                    :prop="`pwmChannelConfig[${i}].frequencyHz`"
                  >
                    <el-input v-model.number="item.pwmChannelConfig[i].frequencyHz" />
                  </el-form-item>

                  <!-- pwm duty cycle unit -->
                  <el-form-item
                    label="PWM update unit"
                    :prop="`pwmChannelConfig[${i}].unit`"
                  >
                    <el-select
                      v-model.number="item.pwmChannelConfig[i].unit"
                      style="width:100%"
                    >
                      <el-option
                        value="MPWM_UPDATE_IN_DUTY_CYCLE"
                        label="In hex"
                      />
                      <el-option
                        value="MPWM_UPDATE_IN_TICKS"
                        label="In ticks"
                      />
                    </el-select>
                  </el-form-item>

                  <!-- duty cycle percent -->
                  <el-form-item
                    label="PWM duty cycle"
                    v-if="item.pwmChannelConfig[i].unit=='MPWM_UPDATE_IN_TICKS'"
                    :prop="`pwmChannelConfig[${i}].dutyCyclePercent`"
                    :rules="numberRule"
                  >
                    <el-input v-model.number="item.pwmChannelConfig[i].dutyCyclePercent">
                      <template #prepend>
                        Ticks
                      </template>
                    </el-input>
                  </el-form-item>
                  <el-form-item
                    label="PWM duty cycle"
                    v-else
                    :prop="`pwmChannelConfig[${i}].dutyCyclePercent`"
                    :rules="hexRule"
                  >
                    <el-input v-model="item.pwmChannelConfig[i].dutyCyclePercent">
                      <template #prepend>
                        HEX
                      </template>
                    </el-input>
                  </el-form-item>

                  <!-- align mode -->
                  <el-form-item
                    label="PWM align mode"
                    :prop="`pwmChannelConfig[${i}].alignMode`"
                  >
                    <el-select
                      v-model.number="item.pwmChannelConfig[i].alignMode"
                      style="width:100%"
                    >
                      <el-option
                        value="MPWM_PWM_RIGHT_ALIGN"
                        label="Right align"
                      />
                      <el-option
                        value="MPWM_PWM_LEFT_ALIGN"
                        label="Left align"
                      />
                      <el-option
                        value="MPWM_PWM_CENTER_H_ALIGN"
                        v-show="FEATURE_MPWM_HAS_CENTER_ALIGN_MODE==true"
                        label="Center align (high level)"
                      />
                      <el-option
                        value="MPWM_PWM_CENTER_HCOMPL_ALIGN"
                        v-show="FEATURE_MPWM_HAS_CENTER_ALIGN_MODE==true"
                        label="Center align (high level) complementary mode"
                      />
                      <el-option
                        value="MPWM_PWM_CENTER_L_ALIGN"
                        v-show="FEATURE_MPWM_HAS_CENTER_ALIGN_MODE==true"
                        label="Center align (low level)"
                      />
                      <el-option
                        value="MPWM_PWM_CENTER_LCOMPL_ALIGN"
                        v-show="FEATURE_MPWM_HAS_CENTER_ALIGN_MODE==true"
                        label="Center align (low level) complementary mode"
                      />
                    </el-select>
                  </el-form-item>

                  <!-- channel output initial value -->
                  <el-form-item
                    label="PWM output initial value"
                    :prop="`pwmChannelConfig[${i}].outputInit`"
                  >
                    <el-select
                      v-model.number="item.pwmChannelConfig[i].outputInit"
                      style="width:100%"
                    >
                      <el-option
                        value="MPWM_OUTPUT_INIT_0"
                        label="Output low level"
                      />
                      <el-option
                        value="MPWM_OUTPUT_INIT_1"
                        label="Output high level"
                      />
                    </el-select>
                  </el-form-item>

                  <!-- reload point(source) -->
                  <el-form-item
                    label="Reload point"
                    :prop="`pwmChannelConfig[${i}].reloadSrc`"
                  >
                    <el-select
                      v-model.number="item.pwmChannelConfig[i].reloadSrc"
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

                  <!-- debug mode -->
                  <el-form-item
                    label="Debug mode"
                    :prop="`pwmChannelConfig[${i}].debugMode`"
                  >
                    <el-checkbox v-model.number="item.pwmChannelConfig[i].debugMode" />
                  </el-form-item>

                  <!-- output trigger enable when matching compare value -->
                  <el-form-item
                    label="Trigger output enable when matching compare value"
                    :prop="`pwmChannelConfig[${i}].outputTrigConfig.cmpTrigOutputEn`"
                  >
                    <el-checkbox v-model.number="item.pwmChannelConfig[i].outputTrigConfig.cmpTrigOutputEn" />
                  </el-form-item>

                  <!-- output trigger enable when matching period value -->
                  <el-form-item
                    label="Trigger output enable when matching period value"
                    :prop="`pwmChannelConfig[${i}].outputTrigConfig.perTrigOutputEn`"
                  >
                    <el-checkbox v-model.number="item.pwmChannelConfig[i].outputTrigConfig.perTrigOutputEn" />
                  </el-form-item>

                  <!-- channel interrupt enable -->
                  <el-form-item
                    label="Channel interrupt enable"
                    :prop="`pwmChannelConfig[${i}].intEn`"
                  >
                    <el-checkbox v-model.number="item.pwmChannelConfig[i].intEn" />
                  </el-form-item>

                  <!-- Timer overflow interrupt enable -->
                  <el-form-item
                    label="Overflow interrupt enable"
                    :prop="`pwmChannelConfig[${i}].timerOverflowIntEn`"
                  >
                    <el-checkbox v-model.number="item.pwmChannelConfig[i].timerOverflowIntEn" />
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
import { onMounted, ref, h, reactive, watch, toRef, onBeforeUnmount, nextTick, inject, onBeforeMount} from "vue";
import {
  Plus,
  Delete,
  FolderOpened
} from '@element-plus/icons-vue'
import { ElButton } from 'element-plus'
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'
import Schema from 'async-validator';
import type { Column } from '@element-plus/components/table-v2'
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<Record<number, FormInstance>>({})
const col = ref(['col1'])
const mpwm_channel_num=ref(0);
const FEATURE_MPWM_HAS_CENTER_ALIGN_MODE = ref(false);

const chDef = {
  hwChannelId: 0,
  clkPrs: 'MPWM_CLK_DIV_BY_1',
  frequencyHz: 10000,
  unit: 'MPWM_UPDATE_IN_DUTY_CYCLE',
  dutyCyclePercent: 0,
  alignMode: 'MPWM_PWM_RIGHT_ALIGN',
  outputInit: 'MPWM_OUTPUT_INIT_0',
  debugMode: false,
  reloadSrc: 'MPWM_RELOAD_ON_PERIOD',
  outputTrigConfig: {
    cmpTrigOutputEn: false,
    perTrigOutputEn: false,
  },
  intEn: false,
  timerOverflowIntEn: false,
}
const defMasterParam = {

  name: 'MPWM_PWM_Config',
  readonly: false,
  channelNum: 1,
  pwmChannelConfig: [chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef],
  
}


const plugin = inject("plugin") as any;
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

watch(activeMasterInst, () => {
  nextTick(() => {
    dataCheck()
  })
})

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




const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
})

const numberRule: FormItemRule[] | FormItemRule =
  [
    { type: 'number', required: true, trigger: 'change' , pattern: /^[0-9]*$/, message: 'Please input dec value' },
  ]
  const hexRule: FormItemRule[] | FormItemRule =
  [
    { type: 'string', required: true, trigger: 'change', pattern: /^0[x|X][0-9a-fA-F]{1,8}$/, message: 'Please input hex value' },
  ]

const errorid: string[] = []
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});
let dataCheckIndex = 0

async function dataCheck() {
  for (const error of errorid) {
    plugin.deleteLog('error', error)
  }

  for (const i in Object.keys(masterRef.value)) {
    dataCheckIndex = Number(i)
    try {
      await masterRef.value[i]?.validate()
    } catch (e) {
      const errs = e as Record<string, any[]>
      for (const val of Object.values(errs)) {
        for (const j in val) {
          const id = `PWM-${val[j].field}-${i}-${j}`
          plugin.addLog('error', id, val[j].message)
          errorid.push(id)
        }
      }
    }
    dataCheckIndex = activeMasterInst.value
  }
}





function hwChannelId0RuleCheck(rule: any, value: any, callback: any, source: any) {
  const err = false;
  const val = data.value[dataCheckIndex]

  if (value >= mpwm_channel_num.value)
  {
    callback(new Error('Hardware channel number is wrong'))
  }

  const cnt = {}
  for (let i = 0; i < val.channelNum; i++) {
    if (cnt[val.pwmChannelConfig[i].hwChannelId] == undefined) {
      cnt[val.pwmChannelConfig[i].hwChannelId] = 1
    } else {
      cnt[val.pwmChannelConfig[i].hwChannelId]++
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


const emits = defineEmits(['change'])

onBeforeMount(() => {
  mpwm_channel_num.value=plugin.getMeta('mpwm_channel_num')
})
onMounted(() => {
  
  let feature = false;
  try {
      feature = plugin.getMeta && plugin.getMeta('FEATURE_MPWM_HAS_CENTER_ALIGN_MODE');
  } catch (e) {
      feature = false;
  }
  FEATURE_MPWM_HAS_CENTER_ALIGN_MODE.value = !!feature;

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
  height: 200px;
  border-radius: 4px;
  border: 1px solid var(--el-border-color);
}
</style>