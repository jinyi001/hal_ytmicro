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
                label="Number of independent PWM channels"
                prop="nNumIndependentPwmChannels"
              >
                <el-input-number
                  v-model="item.nNumIndependentPwmChannels"
                  :min="1"
                  :max="8"
                />
              </el-form-item>
              <el-form-item
                label="PWM mode"
                prop="mode"
              >
                <el-select
                  v-model.number="item.mode"
                  style="width:100%"
                >
                  <el-option
                    value="eTMR_CHANNEL_DISABLE"
                    label="Disable"
                  />
                  <el-option
                    value="eTMR_PWM_MODE"
                    label="PWM mode"
                  />
                  <el-option
                    value="eTMR_COMPARE_MODE"
                    label="Compare mode"
                  />
                  <el-option
                    value="eTMR_INPUT_CAPTURE_MODE"
                    label="Input capute mode"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="PWM period in Hz"
                prop="uFrequencyHZ"
              >
                <el-input v-model.number="item.uFrequencyHZ" />
              </el-form-item>

              <el-form-item
                label="Counter initial value from INIT register(INIT=0)"
                prop="counterInitValFromInitReg"
              >
                <el-checkbox
                  v-model.Boolean="item.counterInitValFromInitReg"
                  :disabled="FEATURE_eTMR_HAS_CNT_INIT_SRC==0"
                />
              </el-form-item>

              <el-form-item
                label="Counter initial value from CNT register"
                prop="cntVal"
              >
                <template #label="{ label }">
                  {{ label }}<el-tooltip
                    effect="dark"
                    content="<span>Here configure CNT register value for the first cycle, and the other cycle counter starts from 0<br>
                             该选项配置CNT寄存器值，表示计数器开始后的第一个周期从配的值开始，从第二个周期开始都是从0计数
                             </span>"
                    raw-content
                  >
                    <el-button
                      :icon="QuestionFilled"
                      circle
                      text
                    />
                  </el-tooltip>
                </template>
                <el-input
                  v-model.number="item.cntVal"
                  :disabled="item.counterInitValFromInitReg==true"
                />
              </el-form-item>

              <el-collapse
                v-model="col"
                style="width:100%;margin-bottom: 18px;"
              >
                <el-collapse-item
                  :title="`Independent Channel Configuration ${i}`"
                  v-for="(e, i) in item.nNumIndependentPwmChannels"
                  :key="i"
                  :name="'Independent Channel' + i"
                >
                  <!-- channel ID -->
                  <el-form-item
                    label="Hardware channel ID"
                    :prop="`pwmIndependentChannelConfig[${i}].hwChannelId`"
                    :rules="hwChannelIdRule"
                  >
                    <el-input v-model.number="item.pwmIndependentChannelConfig[i].hwChannelId" />
                  </el-form-item>

                  <!-- initial value of channel output -->
                  <el-form-item
                    label="Channel output initial value"
                    :prop="`pwmIndependentChannelConfig[${i}].channelInitVal`"
                  >
                    <template #label="{ label }">
                      {{ label }}<el-tooltip
                        effect="dark"
                        content="<span>通道的初始状态<br>
                               0: 低电平<br>
                               1: 高电平
                              </span>"
                        raw-content
                      >
                        <el-button
                          :icon="QuestionFilled"
                          circle
                          text
                        />
                      </el-tooltip>
                    </template>
                    <el-input v-model.number="item.pwmIndependentChannelConfig[i].channelInitVal" />
                  </el-form-item>
                  
                  <el-form-item
                    label="Polarity of the channel signal"
                    :prop="`pwmIndependentChannelConfig[${i}].polarity`"
                  >
                    <template #label="{ label }">
                      {{ label }}<el-tooltip
                        effect="dark"
                        content="<span>Configure this item will invert entire signal of selected channel, including initial channel value<br>
                              配置该选项会将所选通道全部翻转，包括pwm之前的初始状态<br>
                              Normal表示不翻转<br>
                              Invert表示翻转
                              </span>"
                        raw-content
                      >
                        <el-button
                          :icon="QuestionFilled"
                          circle
                          text
                        />
                      </el-tooltip>
                    </template>
                    <el-select
                      v-model.number="item.pwmIndependentChannelConfig[i].polarity"
                      style="width:100%"
                    >
                      <el-option
                        value="eTMR_POLARITY_NORMAL"
                        label="Normal"
                      />
                      <el-option
                        value="eTMR_POLARITY_INVERT"
                        label="Invert"
                      />
                    </el-select>
                  </el-form-item>

                  <el-form-item
                    label="Polarity of the channel PWM signal"
                    :prop="`pwmIndependentChannelConfig[${i}].pwmSrcInvert`"
                  >
                    <template #label="{ label }">
                      {{ label }}<el-tooltip
                        effect="dark"
                        content="<span>Configure this item will just invert pwm signal of selected channel<br>
                              配置该选项只会翻转通道的pwm信号，不包括初始值<br>
                              勾选表示翻转
                              </span>"
                        raw-content
                      >
                        <el-button
                          :icon="QuestionFilled"
                          circle
                          text
                        />
                      </el-tooltip>
                    </template>
                    <el-checkbox
                      v-model.number="item.pwmIndependentChannelConfig[i].pwmSrcInvert"
                    />
                  </el-form-item>

                  <el-form-item
                    label="PWM alignment mode"
                    :prop="`pwmIndependentChannelConfig[${i}].align`"
                  >
                    <el-select
                      v-model.number="item.pwmIndependentChannelConfig[i].align"
                      style="width:100%"
                    >
                      <el-option
                        value="eTMR_PWM_RIGHT_EDGE_ALIGN"
                        label="Right"
                      />
                      <el-option
                        value="eTMR_PWM_LEFT_EDGE_ALIGN"
                        label="Left"
                      />
                      <el-option
                        value="eTMR_PWM_CENTER_ALIGN"
                        label="Center"
                      />
                      <el-option
                        value="eTMR_PWM_ASYMMETRICAL"
                        label="Asymmetrical"
                      />
                    </el-select>
                  </el-form-item>

                  <!-- pwm duty cycle unit -->
                  <el-form-item
                    label="PWM type of update"
                    :prop="`pwmIndependentChannelConfig[${i}].typeOfUpdate`"
                  >
                    <el-select
                      v-model.number="item.pwmIndependentChannelConfig[i].typeOfUpdate"
                      style="width:100%"
                    >
                      <el-option
                        value="eTMR_PWM_UPDATE_IN_DUTY_CYCLE"
                        label="eTMR_PWM_UPDATE_IN_DUTY_CYCLE"
                      />
                      <el-option
                        value="eTMR_PWM_UPDATE_IN_TICKS"
                        label="eTMR_PWM_UPDATE_IN_TICKS"
                      />
                    </el-select>
                  </el-form-item>

                  <el-form-item
                    label="PWM duty cycle"
                    v-if="item.pwmIndependentChannelConfig[i].typeOfUpdate=='eTMR_PWM_UPDATE_IN_TICKS'"
                    :prop="`pwmIndependentChannelConfig[${i}].dutyCycle`"
                    :rules="numberRule"
                  >
                    <el-input v-model.number="item.pwmIndependentChannelConfig[i].dutyCycle">
                      <template #prepend>
                        Ticks
                      </template>
                    </el-input>
                  </el-form-item>
                  <el-form-item
                    label="PWM duty cycle"
                    v-else
                    :prop="`pwmIndependentChannelConfig[${i}].dutyCycle`"
                    :rules="hexRule"
                  >
                    <el-input v-model="item.pwmIndependentChannelConfig[i].dutyCycle">
                      <template #prepend>
                        HEX
                      </template>
                    </el-input>
                  </el-form-item>

                  <el-form-item
                    label="Offset for asymmetrical mode"
                    v-if="item.pwmIndependentChannelConfig[i].align=='eTMR_PWM_ASYMMETRICAL'"
                    :prop="`pwmIndependentChannelConfig[${i}].offset`"
                  >
                    <el-input v-model.number="item.pwmIndependentChannelConfig[i].offset" />
                  </el-form-item>

                  <!-- double switch -->
                  <el-form-item
                    v-if="FEATURE_eTMR_HAS_DOUBLE_SWITCH"
                    label="Enable double switch"
                    :prop="`pwmIndependentChannelConfig[${i}].enableDoubleSwitch`"
                  >
                    <template #label="{ label }">
                      {{ label }}<el-tooltip
                        effect="dark"
                        content="<span>如果使能double switch模式，那么通道对（0-1，2-3，4-5，6-7）的两个通道都输出两个通道pwm异或之后的信号<br>
                                     如果使能double switch模式和互补模式，那么会在异或之后将通道对的奇数通道翻转，与偶数通道形成互补状态
                              </span>"
                        raw-content
                      >
                        <el-button
                          :icon="QuestionFilled"
                          circle
                          text
                        />
                      </el-tooltip>
                    </template>
                    <el-checkbox v-model.number="item.pwmIndependentChannelConfig[i].enableDoubleSwitch" />
                  </el-form-item>

                  <!-- complementary mode -->
                  <el-form-item
                    label="Enable complementary mode"
                    :prop="`pwmIndependentChannelConfig[${i}].enableSecondChannelOutput`"
                  >
                    <template #label="{ label }">
                      {{ label }}<el-tooltip
                        effect="dark"
                        content="<span>使能互补模式
                              </span>"
                        raw-content
                      >
                        <el-button
                          :icon="QuestionFilled"
                          circle
                          text
                        />
                      </el-tooltip>
                    </template>
                    <el-checkbox v-model.number="item.pwmIndependentChannelConfig[i].enableSecondChannelOutput" />
                  </el-form-item>

                  <!-- polarity of complementary odd channel signal -->
                  <el-form-item
                    label="Polarity of the complementary odd channel signal"
                    v-if="item.pwmIndependentChannelConfig[i].enableSecondChannelOutput==true"
                    :prop="`pwmIndependentChannelConfig[${i}].secondChannelPolarity`"
                  >
                    <template #label="{ label }">
                      {{ label }}<el-tooltip
                        effect="dark"
                        content="<span>互补模式下的通道对的奇数通道是否再次翻转，也就是说是否将奇数通道翻转成与偶数通道相同的信号<br>
                              如果Normal，那么奇数通道不翻转，通道对互补<br>
                              如果Invert，那么奇数通道翻转，通道对一致
                              </span>"
                        raw-content
                      >
                        <el-button
                          :icon="QuestionFilled"
                          circle
                          text
                        />
                      </el-tooltip>
                    </template>
                    <el-select
                      v-model.number="item.pwmIndependentChannelConfig[i].secondChannelPolarity"
                      style="width:100%"
                    >
                      <el-option
                        value="eTMR_POLARITY_NORMAL"
                        label="Normal"
                      />
                      <el-option
                        value="eTMR_POLARITY_INVERT"
                        label="Invert"
                      />
                    </el-select>
                  </el-form-item>

                  <!-- dead time for odd channel -->
                  <el-form-item
                    v-if="FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL && item.pwmIndependentChannelConfig[i].enableSecondChannelOutput==true"
                    label="Dead time for odd channel"
                    :prop="`pwmIndependentChannelConfig[${i}].oddDeadTime`"
                  >
                    <el-input v-model.number="item.pwmIndependentChannelConfig[i].oddDeadTime" />
                  </el-form-item>

                  <!-- dead time for even channel -->
                  <el-form-item
                    v-if="FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL && item.pwmIndependentChannelConfig[i].enableSecondChannelOutput==true"
                    label="Dead time for even channel"
                    :prop="`pwmIndependentChannelConfig[${i}].evenDeadTime`"
                  >
                    <el-input v-model.number="item.pwmIndependentChannelConfig[i].evenDeadTime" />
                  </el-form-item>

                  <el-form-item
                    v-if="FEATURE_eTMR_HAS_CHANNEL_VALUE_MATCH_INTERRUPT_IN_PWM_MODE"
                    label="Enable interrupt"
                    :prop="`pwmIndependentChannelConfig[${i}].interruptEnable`"
                  >
                    <el-checkbox v-model.number="item.pwmIndependentChannelConfig[i].interruptEnable" />
                  </el-form-item>
                </el-collapse-item>

                <el-collapse-item
                  v-if="FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL==false"
                  title="Deadtime Configuration"
                  name="2"
                >
                  <el-form-item
                    label="Deadtime prescaler"
                  >
                    <el-input v-model="item.deadtimeConfig.prs" />
                  </el-form-item>
                  <el-form-item
                    label="Even channel deadtime value"
                  >
                    <el-input v-model="item.deadtimeConfig.evenDeadTime" />
                  </el-form-item>
                  <el-form-item
                    label="Odd channel deadtime value"
                  >
                    <el-input v-model="item.deadtimeConfig.oddDeadTime" />
                  </el-form-item>
                </el-collapse-item>
                <el-collapse-item
                  title="Fault Channel Configuration"
                  name="3"
                >
                  <el-form-item
                    label="Enable PWM fault interrupt"
                    prop="faultConfig.pwmFaultInterrupt"
                  >
                    <el-checkbox v-model.number="item.faultConfig.pwmFaultInterrupt" />
                  </el-form-item>
                  <el-form-item
                    label="Fault filter sample counter"
                    prop="faultConfig.faultFilterSampleCounter"
                  >
                    <el-input v-model.number="item.faultConfig.faultFilterSampleCounter" />
                  </el-form-item>
                  <el-form-item
                    label="Fault filter sample period"
                    prop="faultConfig.faultFilterSamplePeriod"
                  >
                    <el-input v-model.number="item.faultConfig.faultFilterSamplePeriod" />
                  </el-form-item>
                  <el-form-item
                    label="Fault input strentch"
                    prop="faultConfig.faultInputStrentch"
                  >
                    <el-input v-model.number="item.faultConfig.faultInputStrentch" />
                  </el-form-item>
                  <el-form-item
                    label="PWM Recovery opportunity"
                    prop="faultConfig.pwmRecoveryOpportunity"
                  >
                    <el-select
                      v-model.number="item.faultConfig.pwmRecoveryOpportunity"
                      style="width:100%"
                    >
                      <el-option
                        label="Disable"
                        value="eTMR_FAULT_PWM_RECOVERY_DISABLED"
                      />
                      <el-option
                        label="Matches with MID"
                        value="eTMR_FAULT_PWM_RECOVERY_WITH_MID"
                      />
                      <el-option
                        label="Matches with MOD"
                        value="eTMR_FAULT_PWM_RECOVERY_WITH_MOD"
                      />
                      <el-option
                        label="Immediately"
                        value="eTMR_FAULT_PWM_RECOVERY_IMMEDIATELY"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="PWM Recovery automatic mode"
                    prop="faultConfig.pwmAutoRecoveryMode"
                  >
                    <el-select
                      v-model.number="item.faultConfig.pwmAutoRecoveryMode"
                      style="width:100%"
                    >
                      <el-option
                        label="Recovery needs clearing fault flag first"
                        value="eTMR_MANUAL_CLEAR_FAULT_FLAG_THEN_AUTO_RECOVERY"
                      />
                      <el-option
                        label="Recovery regardless of clearing fault flag"
                        value="eTMR_AUTO_RECOVERY_WITHOUT_CLEAR_FAULT_FLAG"
                      />
                      <el-option
                        label="Recovery and clearing fault flag"
                        value="eTMR_AUTO_RECOVERY_WITH_CLEAR_FAULT_FLAG"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label="Fault mode"
                    prop="faultConfig.faultMode"
                  >
                    <el-select
                      v-model.number="item.faultConfig.faultMode"
                      style="width:100%"
                    >
                      <el-option
                        label="Clock is still existing"
                        value="eTMR_FAULT_WITH_CLK"
                      />
                      <el-option
                        label="Clock is lost"
                        value="eTMR_FAULT_WITHOUT_CLK"
                      />
                    </el-select>
                  </el-form-item>
                  <el-form-item
                    label-width="0"
                    prop="faultConfig.etmrFaultChannelParam"
                  >
                    <div class="table">
                      <el-auto-resizer>
                        <template #default="{ height, width }">
                          <el-table-v2
                            :columns="columns"
                            :data="item.faultConfig.etmrFaultChannelParam"
                            :estimated-row-height="30"
                            :width="width"
                            :header-height="30"
                            :height="height"
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
                        </template>
                      </el-auto-resizer>
                    </div>
                  </el-form-item>
                  <el-form-item
                    label-width="0"
                    prop="faultConfig.safeState"
                  >
                    <div class="table">
                      <el-auto-resizer>
                        <template #default="{ height, width }">
                          <el-table-v2
                            :columns="columns2"
                            :data="item.faultConfig.safeState"
                            :estimated-row-height="30"
                            :width="width"
                            :header-height="30"
                            :height="height"
                            :footer-height="40"
                            fixed
                          >
                            <!-- <template #empty>
              <el-empty :image-size="80" />
            </template> -->
                            <template #footer>
                              <el-alert
                                v-if="filterError1 != ''"
                                center
                                show-icon
                                :title="filterError1"
                                type="error"
                                :closable="false"
                              />
                            </template>
                          </el-table-v2>
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
import { onMounted, ref, h, reactive, watch, toRef, onBeforeUnmount, nextTick, inject} from "vue";
import {
  Plus,
  Delete,
  FolderOpened,
  QuestionFilled
} from '@element-plus/icons-vue'
import { ElButton } from 'element-plus'
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'
import Schema from 'async-validator';
import type { Column } from '@element-plus/components/table-v2'
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<Record<number, FormInstance>>({})
const col = ref(['col1'])
const FEATURE_eTMR_HAS_DOUBLE_SWITCH=ref(false)
const FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL=ref(false)
const FEATURE_eTMR_HAS_CHANNEL_VALUE_MATCH_INTERRUPT_IN_PWM_MODE=ref(false)
const FEATURE_eTMR_HAS_CNT_INIT_SRC=ref(false)

const chDef = {
  hwChannelId: 0,
  polarity: 'eTMR_POLARITY_NORMAL',
  pwmSrcInvert:false,
  align: 'eTMR_PWM_RIGHT_EDGE_ALIGN',
  typeOfUpdate:'eTMR_PWM_UPDATE_IN_DUTY_CYCLE',
  channelInitVal: 0,
  dutyCycle: 0,
  offset: 0,
  enableSecondChannelOutput: false,
  secondChannelPolarity:'eTMR_POLARITY_NORMAL',
  evenDeadTime: 0,
  oddDeadTime: 0,
  enableDoubleSwitch:false,
  interruptEnable:false,
}
const defMasterParam = {

  name: 'ETMR_PWM_Config',
  readonly: false,
  nNumIndependentPwmChannels: 1,
  mode: 'eTMR_PWM_MODE',
  uFrequencyHZ: 0,
  counterInitValFromInitReg:false,
  counterInitSrc: 'COUNTER_VAL_FROM_CNT',
  pwmIndependentChannelConfig: [chDef, chDef, chDef, chDef, chDef, chDef, chDef, chDef],
  deadtimeConfig: {
    prs: 1,
    evenDeadTime: 0,
    oddDeadTime: 0,
  },
  faultConfig: {
    pwmFaultInterrupt: false,
    faultFilterSampleCounter: 0,
    faultFilterSamplePeriod: 0,
    faultInputStrentch: 0,
    pwmRecoveryOpportunity: 'eTMR_FAULT_PWM_RECOVERY_DISABLED',
    pwmAutoRecoveryMode: 'eTMR_MANUAL_CLEAR_FAULT_FLAG_THEN_AUTO_RECOVERY',
    faultMode: 'eTMR_FAULT_WITH_CLK',
    etmrFaultChannelParam: [
      {
        faultChannelEnabled: false,
        faultInputPolarity: 'eTMR_FAULT_SIGNAL_HIGH'
      },
      {
        faultChannelEnabled: false,
        faultInputPolarity: 'eTMR_FAULT_SIGNAL_HIGH'
      },
      {
        faultChannelEnabled: false,
        faultInputPolarity: 'eTMR_FAULT_SIGNAL_HIGH'
      },
      {
        faultChannelEnabled: false,
        faultInputPolarity: 'eTMR_FAULT_SIGNAL_HIGH'
      }
    ],
    safeState: [
      {
        val:'eTMR_LOW_STATE',
      },
      {
        val:'eTMR_LOW_STATE',
      },
      {
        val:'eTMR_LOW_STATE',
      },
      {
        val:'eTMR_LOW_STATE',
      }
    ]
  }
}
const columns2: Column<any>[] = [
  {
  key: 'column-n-1',
    width: 100,
    title: 'Row No.',
    cellRenderer: ({ rowIndex }) => `${rowIndex + 1}`,
    align: 'center',
  },
  {
    key: 'state',
    title: 'Fault state',
    dataKey: 'val',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        size: 'small',
        disabled: lock.value,
        value: rowData.val,
        'onChange': (event) => rowData.val = event.target.value,
        style: { width: '100%' }
      },
        [h('option', {
          label: 'eTMR_LOW_STATE',
          value: 'eTMR_LOW_STATE'
        }),
        h('option', {
          label: 'eTMR_HIGH_STATE',
          value: 'eTMR_HIGH_STATE'
        }),
        h('option', {
          label: 'eTMR_TRI_STATE',
          value: 'eTMR_TRI_STATE'
        }),
        ])
    }
  },
]
const columns: Column<any>[] = [
  {
  key: 'column-n-1',
    width: 100,
    title: 'Row No.',
    cellRenderer: ({ rowIndex }) => `${rowIndex + 1}`,
    align: 'center',
  },
  {
    key: 'faultChannelEnabled',
    title: 'Fault channel enable',
    dataKey: 'faultChannelEnabled',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('input', {
        type: 'checkbox',
        disabled: lock.value,
        checked: rowData.faultChannelEnabled,
        'onInput': (event) => {
          rowData.faultChannelEnabled = event.target.checked
        }
      })
    }
  },
  {
    key: 'faultInputPolarity',
    title: 'Fault input polsignal',
    dataKey: 'faultInputPolarity',
    width: 200,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        size: 'small',
        disabled: lock.value,
        value: rowData.faultInputPolarity,
        'onChange': (event) => rowData.faultInputPolarity = event.target.value,
        style: { width: '100%' }
      },
        [h('option', {
          label: 'Low',
          value: 'eTMR_FAULT_SIGNAL_LOW'
        }),
        h('option', {
          label: 'High',
          value: 'eTMR_FAULT_SIGNAL_HIGH'
        })
        ])
    }
  },
]
const filterError = ref('')
const filterError1 = ref('')
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
  // syncMethod: {
  //     type: 'object',
  //     required: true,
  //     fields: {
  //         regSyncFreq: regSyncFreqRule
  //     }
  // },
  // outputTrigConfig: {
  //     type: 'object',
  //     required: true,
  //     fields: {
  //         pwmOutputChannel: pwmOutputChannelRule,
  //         outputTrigWidth: outputTrigWidthRule,
  //         outputTrigFreq: outputTrigFreqRule,
  //     }

  // }
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


  const cnt = {}
  for (let i = 0; i < val.nNumIndependentPwmChannels; i++) {
    if (cnt[val.pwmIndependentChannelConfig[i].hwChannelId] == undefined) {
      cnt[val.pwmIndependentChannelConfig[i].hwChannelId] = 1
    } else {
      cnt[val.pwmIndependentChannelConfig[i].hwChannelId]++
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
    { type: 'number', required: true, min: 0, max: 7, message: 'Hardware channel ID from 0-7', trigger: 'change' },
    { validator: hwChannelId0RuleCheck },
  ]


const emits = defineEmits(['change'])

onMounted(() => {
  FEATURE_eTMR_HAS_DOUBLE_SWITCH.value=plugin.getMeta('FEATURE_eTMR_HAS_DOUBLE_SWITCH')
  FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL.value=plugin.getMeta('FEATURE_eTMR_DEADTIME_CONFIG_EACH_CHANNEL')
  FEATURE_eTMR_HAS_CHANNEL_VALUE_MATCH_INTERRUPT_IN_PWM_MODE.value=plugin.getMeta('FEATURE_eTMR_HAS_CHANNEL_VALUE_MATCH_INTERRUPT_IN_PWM_MODE')
  FEATURE_eTMR_HAS_CNT_INIT_SRC.value=plugin.getMeta('FEATURE_eTMR_HAS_CNT_INIT_SRC')
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