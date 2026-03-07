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
                  v-if="scope.$index==activeMasterInst"
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
                <el-input v-model="data[activeMasterInst].name" />
              </el-form-item>
              <el-form-item
                label="FlexCAN Hardware Channel"
                prop="inst"
              >
                <el-select
                  v-model.number="data[activeMasterInst].inst"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in canNum"
                    :value="item-1"
                    :key="item"
                    :label="`FLEXCAN_${item-1}`"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Read-only"
                prop="readonly"
              >
                <el-checkbox v-model="data[activeMasterInst].readonly" />
              </el-form-item>
              <el-form-item
                label="Operation Mode"
                prop="flexcanMode"
              >
                <el-select
                  v-model="data[activeMasterInst].flexcanMode"
                  style="width:100%"
                >
                  <el-option
                    value="FLEXCAN_NORMAL_MODE"
                    label="FLEXCAN_NORMAL_MODE"
                  />
                  <el-option
                    value="FLEXCAN_LISTEN_ONLY_MODE"
                    label="FLEXCAN_LISTEN_ONLY_MODE"
                  />
                  <el-option
                    value="FLEXCAN_LOOPBACK_MODE"
                    label="FLEXCAN_LOOPBACK_MODE"
                  />
                  <el-option
                    value="FLEXCAN_FREEZE_MODE"
                    label="FLEXCAN_FREEZE_MODE"
                  />
                  <el-option
                    value="FLEXCAN_DISABLE_MODE"
                    label="FLEXCAN_DISABLE_MODE"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Enable FD"
                prop="fd_enable"
              >
                <el-checkbox
                  v-model="data[activeMasterInst].fd_enable"
                />
              </el-form-item>
              <el-form-item
                label="RAM Block Message Buffer Payload max size(Bytes)"
                prop="payload"
              >
                <el-select
                  v-model="data[activeMasterInst].payload"
                  style="width:100%"
                >
                  <el-option
                    value="FLEXCAN_PAYLOAD_SIZE_8"
                    label="8"
                  />
                  <el-option
                    value="FLEXCAN_PAYLOAD_SIZE_16"
                    label="16"
                  />
                  <el-option
                    value="FLEXCAN_PAYLOAD_SIZE_32"
                    label="32"
                  />
                  <el-option
                    value="FLEXCAN_PAYLOAD_SIZE_64"
                    label="64"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="MBs Number"
                prop="max_num_mb"
              >
                <el-input v-model.number="data[activeMasterInst].max_num_mb" />
              </el-form-item>
              <el-form-item
                label="Use Rx FIFO"
                prop="is_rx_fifo_needed"
              >
                <el-checkbox v-model="data[activeMasterInst].is_rx_fifo_needed" />
              </el-form-item>
              <el-form-item
                label="Rx FIFO Transfer Type"
                prop="transfer_type"
              >
                <el-select
                  v-model="data[activeMasterInst].transfer_type"
                  style="width:100%"
                  :disabled="!data[activeMasterInst].is_rx_fifo_needed"
                >
                  <el-option
                    value="FLEXCAN_RXFIFO_USING_INTERRUPTS"
                    label="Using interrupts"
                  />
                  <el-option
                    value="FLEXCAN_RXFIFO_USING_DMA"
                    label="Using DMA"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Rx FIFO filters number"
                prop="num_id_filters"
              >
                <el-select
                  v-model="data[activeMasterInst].num_id_filters"
                  style="width:100%"
                  :disabled="!data[activeMasterInst].is_rx_fifo_needed"
                >
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_8"
                    label="8 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_16"
                    label="16 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_24"
                    label="24 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_32"
                    label="32 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_40"
                    label="40 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_48"
                    label="48 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_56"
                    label="56 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_64"
                    label="64 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_72"
                    label="72 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_88"
                    label="88 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_96"
                    label="96 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_104"
                    label="104 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_112"
                    label="112 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_120"
                    label="120 Rx FIFO Filters"
                  />
                  <el-option
                    value="FLEXCAN_RX_FIFO_ID_FILTERS_128"
                    label="128 Rx FIFO Filters"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Rx FIFO DMA Channel"
                prop="rxFifoDMAChannel"
              >
                <el-input
                  v-model.number="data[activeMasterInst].rxFifoDMAChannel"
                  :disabled="data[activeMasterInst].transfer_type=='FLEXCAN_RXFIFO_USING_INTERRUPTS'"
                />
              </el-form-item>
              <el-form-item
                label="PE clock source"
                prop="pe_clock"
              >
                <el-select
                  v-model="data[activeMasterInst].pe_clock"
                  style="width:100%"
                >
                  <el-option
                    value="FLEXCAN_CLK_SOURCE_OSC"
                    label="Oscillator clock"
                  />
                  <el-option
                    value="FLEXCAN_CLK_SOURCE_PERIPH"
                    label="Peripheral clock"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="PE clock frequency (Hz)"
                prop="freq"
              >
                <el-input
                  v-model.number="freq"
                  disabled
                />
              </el-form-item>
              <el-form-item
                label="Bitrate to time segments"
                prop="bte" 
                v-if="0"
              >
                <el-checkbox v-model="data[activeMasterInst].bte" />
              </el-form-item>


              <el-collapse
                v-model="col2"
                style="width:100%;margin-bottom: 18px;"
              >
                <el-collapse-item
                  title="FlexCAN Phase"
                  name="1"
                >
                  <div>
                    <el-form-item
                      label="Propagation segment"
                      prop="bitrate.propSeg"
                      :rules="bitratePropSegRule"
                    >
                      <el-input v-model.number="data[activeMasterInst].bitrate.propSeg" />
                    </el-form-item>
                    <el-form-item
                      label="Phase segment 1"
                      prop="bitrate.phaseSeg1"
                      :rules="bitratePhaseSeg1"
                    >
                      <el-input v-model.number="data[activeMasterInst].bitrate.phaseSeg1" />
                    </el-form-item>
                    <el-form-item
                      label="Phase segment 2"
                      prop="bitrate.phaseSeg2"
                      :rules="bitratePhaseSeg2"
                    >
                      <el-input v-model.number="data[activeMasterInst].bitrate.phaseSeg2" />
                    </el-form-item>
                    <el-form-item
                      label="Prescaler division factor"
                      prop="bitrate.preDivider"
                      :rules="bitratePreDivider"
                    >
                      <el-input v-model.number="data[activeMasterInst].bitrate.preDivider" />
                    </el-form-item>
                    <el-form-item
                      label="Resync jump width"
                      prop="bitrate.rJumpwidth"
                      :rules="bitraterJumpwidth"
                    >
                      <el-input v-model.number="data[activeMasterInst].bitrate.rJumpwidth" />
                    </el-form-item>
                    <el-form-item label="Bitrate (Kbps)">
                      <span><span>{{ bitrateCal(freq,data[activeMasterInst].bte,data[activeMasterInst].bitrate) }}</span></span>
                    </el-form-item>
                    <el-form-item label="Sampling point (%)">
                      <span>{{ samplePoint(data[activeMasterInst].bte,data[activeMasterInst].bitrate) }}</span>
                    </el-form-item>
                  </div>
                </el-collapse-item>
                <el-collapse-item
                  title="FlexCAN CBT Phase"
                  name="2"
                >
                  <el-form-item
                    label="Propagation segment"
                    prop="bitrate_cbt.propSeg"
                    :rules="bitrateCBTPropSegRule"
                  >
                    <el-input
                      v-model.number="data[activeMasterInst].bitrate_cbt.propSeg"
                      :disabled="!data[activeMasterInst].fd_enable"
                    />
                  </el-form-item>
                  <el-form-item
                    label="Phase segment 1"
                    prop="bitrate_cbt.phaseSeg1"
                    :rules="bitrateCBTPhaseSeg1"
                  >
                    <el-input
                      v-model.number="data[activeMasterInst].bitrate_cbt.phaseSeg1"
                      :disabled="!data[activeMasterInst].fd_enable"
                    />
                  </el-form-item>
                  <el-form-item
                    label="Phase segment 2"
                    prop="bitrate_cbt.phaseSeg2"
                    :rules="bitrateCBTPhaseSeg2"
                  >
                    <el-input
                      v-model.number="data[activeMasterInst].bitrate_cbt.phaseSeg2"
                      :disabled="!data[activeMasterInst].fd_enable"
                    />
                  </el-form-item>
                  <el-form-item
                    label="Prescaler division factor"
                    prop="bitrate_cbt.preDivider"
                    :rules="bitrateCBTPreDivider"
                  >
                    <el-input
                      v-model.number="data[activeMasterInst].bitrate_cbt.preDivider"
                      :disabled="!data[activeMasterInst].fd_enable"
                    />
                  </el-form-item>
                  <el-form-item
                    label="Resync jump width"
                    prop="bitrate_cbt.rJumpwidth"
                    :rules="bitrateCBTrJumpwidth"
                  >
                    <el-input
                      v-model.number="data[activeMasterInst].bitrate_cbt.rJumpwidth"
                      :disabled="!data[activeMasterInst].fd_enable"
                    />
                  </el-form-item>
                  <el-form-item label="Bitrate (Kbps)">
                    <span>{{ bitrateCBTCal(freq,data[activeMasterInst].bte,data[activeMasterInst].bitrate_cbt) }}</span>
                  </el-form-item>
                  <el-form-item label="Sampling point (%)">
                    <span>{{ samplePointCBT(data[activeMasterInst].bte,data[activeMasterInst].bitrate_cbt) }}</span>
                  </el-form-item>
                </el-collapse-item>
              </el-collapse>



              <el-form-item
                label="The last work of Rx FIFO DMA"
                prop="rxFifoDMALastWord"
                label-width="300px"
                v-show="FEATURE_CAN_HAS_ENHANCE_FIFO"
              >
                <el-input
                  v-model.number="data[activeMasterInst].rxFifoDMALastWord"
                  :disabled="!data[activeMasterInst].is_enhance_rx_fifo_needed"
                />
              </el-form-item>
              <el-form-item
                label="The number of enhanced ID filter elements"
                prop="num_enhance_rx_fifo_filters"
                label-width="300px"
                v-show="FEATURE_CAN_HAS_ENHANCE_FIFO"
              >
                <el-input
                  v-model.number="data[activeMasterInst].num_enhance_rx_fifo_filters"
                  :disabled="!data[activeMasterInst].is_enhance_rx_fifo_needed"
                />
              </el-form-item>
              <el-form-item
                label="The number of enhanced extended  ID filter elements"
                prop="num_enhance_rx_fifo_extid_filters"
                label-width="300px"
                v-show="FEATURE_CAN_HAS_ENHANCE_FIFO"
              >
                <el-input
                  v-model.number="data[activeMasterInst].num_enhance_rx_fifo_extid_filters"
                  :disabled="!data[activeMasterInst].is_enhance_rx_fifo_needed"
                />
              </el-form-item>
              <el-form-item
                label="The number of enhanced Rx FIFO watermarks/The number of Message Buffers Transferred by DMA"
                label-width="300px"
                prop="num_enhance_rx_fifo_min_messages"
                v-show="FEATURE_CAN_HAS_ENHANCE_FIFO"
              >
                <div style="display:flex;align-items:center;height:40px;width:100%">
                  <el-input
                    v-model.number="data[activeMasterInst].num_enhance_rx_fifo_min_messages"
                    :disabled="!data[activeMasterInst].is_enhance_rx_fifo_needed"
                  />
                </div>
              </el-form-item>
              <el-form-item
                label="Enable/Disable the Enhanced Rx FIFO feature"
                prop="is_enhance_rx_fifo_needed"
                label-width="300px"
                v-show="FEATURE_CAN_HAS_ENHANCE_FIFO"
              >
                <el-checkbox v-model="data[activeMasterInst].is_enhance_rx_fifo_needed" />
              </el-form-item>
            </el-form>
          </div>
        </el-col>
      </el-row>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, computed, reactive, watch, onBeforeUnmount, nextTick, isProxy } from "vue";
import { Plugin } from "plugin";
import {
  Plus,
  Delete,
  FolderOpened
} from '@element-plus/icons-vue'
import type { FormInstance, FormRules } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const col2 = ref(['1', '2'])
const oscFreq = ref(0)
const periFreq = ref(0)
const show=ref(false)
const FEATURE_CAN_HAS_ENHANCE_FIFO=ref(false)
const freq = computed(() => {
  if (data.value[activeMasterInst.value].pe_clock == 'FLEXCAN_CLK_SOURCE_OSC') {
    return oscFreq.value
  } else {
    return periFreq.value
  }
})

const defMasterParam = {
  name: 'flexcanInitConfig0',
  readonly: true,
  inst: 0,
  max_num_mb: 32,
  num_id_filters: 'FLEXCAN_RX_FIFO_ID_FILTERS_8',
  flexcanMode: 'FLEXCAN_NORMAL_MODE',
  payload: 'FLEXCAN_PAYLOAD_SIZE_8',
  fd_enable: false,
  pe_clock: 'FLEXCAN_CLK_SOURCE_OSC',
  transfer_type: 'FLEXCAN_RXFIFO_USING_INTERRUPTS',
  rxFifoDMAChannel: 0,
  rxFifoDMALastWord: 0,
  is_enhance_rx_fifo_needed: false,
  num_enhance_rx_fifo_filters: 0,
  num_enhance_rx_fifo_extid_filters: 0,
  num_enhance_rx_fifo_min_messages: 0,
  is_rx_fifo_needed: false,
  bte: false,
  bitrate: {
    propSeg: 5,    /*!< Propagation segment*/
    phaseSeg1: 8,  /*!< Phase segment 1*/
    phaseSeg2: 2,  /*!< Phase segment 2*/
    preDivider: 3, /*!< Clock prescaler division factor*/
    rJumpwidth: 2, /*!< Resync jump width*/
  },
  bitrate_cbt: {
    propSeg: 6,    /*!< Propagation segment*/
    phaseSeg1: 3,  /*!< Phase segment 1*/
    phaseSeg2: 2,  /*!< Phase segment 2*/
    preDivider: 1, /*!< Clock prescaler division factor*/
    rJumpwidth: 2, /*!< Resync jump width*/
  }
}

const canNum = ref(0)

const canBitRateRules = ref()

const lock=ref(false)

const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = ref([defMasterParam])


function bitrateCal(fcanclk: number, bte: boolean, data: any) {
  const tq = fcanclk / (data.preDivider * 1000)
  let num = 0
  if (bte) {
    num = data.phaseSeg1 + data.phaseSeg2 + 3
  } else {
    num = data.phaseSeg1 + data.phaseSeg2 + 1 + data.propSeg
  }
  const freq = parseFloat(Number(tq / num).toFixed(3))
  return freq
}

function samplePoint(bte: boolean, data: any) {
  let num = 0
  if (bte) {
    num = data.phaseSeg1 + data.phaseSeg2 + 3
  } else {
    num = data.phaseSeg1 + data.phaseSeg2 + 1 + data.propSeg
  }
  const percent = parseFloat(Number((num - data.phaseSeg2) * 100 / num).toFixed(3))
  return percent
}


function bitrateCBTCal(fcanclk: number, bte: boolean, data: any) {
  const tq = fcanclk / (data.preDivider * 1000)
  let num = 0
  if (bte) {
    num = data.phaseSeg1 + data.phaseSeg2 + 3
  } else {
    num = data.phaseSeg1 + data.phaseSeg2 + 1 + data.propSeg
  }
  const freq = parseFloat(Number(tq / num).toFixed(3))
  return freq
}

function samplePointCBT(bte: boolean, data: any) {
  let num = 0
  if (bte) {
    num = data.phaseSeg1 + data.phaseSeg2 + 3
  } else {
    num = data.phaseSeg1 + data.phaseSeg2  + 1 + data.propSeg
  }
  const percent = parseFloat(Number((num - data.phaseSeg2) * 100 / num).toFixed(3))
  return percent
}

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
function payloadCheck(rule: any, value: any, callback: any, source: any) {
  const val=isProxy(source)?source:data.value[activeMasterInst.value]

  if (value != 'FLEXCAN_PAYLOAD_SIZE_8' && !val.fd_enable) {
    callback(new Error(`Need enable CAN-FD`))
  } else {
    callback()
  }
}

function mbCheck(rule: any, value: any, callback: any, source: any) {
  const val=isProxy(source)?source:data.value[activeMasterInst.value]
  const mbLimitation = {
    'FLEXCAN_PAYLOAD_SIZE_8': 32,
    'FLEXCAN_PAYLOAD_SIZE_16': 21,
    'FLEXCAN_PAYLOAD_SIZE_32': 12,
    'FLEXCAN_PAYLOAD_SIZE_64': 7
  }
  const mbNum = plugin.getMeta('can_mb')
  const genMB = mbNum[val.inst]
  const lim = (genMB / 32) * mbLimitation[val.payload]

  if (value > lim) {
    callback(new Error(`Max MB should <=${lim}`))
  } else {
    callback()
  }



}

const bitrateCBTPropSegRule={ type: 'number', min:0, max: 31, message: 'Please input PROPSEG from 0-31', required: true, trigger: 'change', };
const bitrateCBTPhaseSeg1={ type: 'number', min: 1, max: 8, message: 'Please input PSEG1 from 1-8', required: true };
const bitrateCBTPhaseSeg2={ type: 'number', min: 1, max: 8, message: 'Please input PSEG2 from 1-8', required: true };
const bitrateCBTPreDivider={ type: 'number', min: 1, max: 1024, message: 'Please input PRESDIV from 1-1024', required: true };
const bitrateCBTrJumpwidth={ type: 'number', min: 1, max: 8, message: 'Please input resync jump width from 1-8', required: true };

const bitratePropSegRule={ type: 'number', min:1, max: 8, message: 'Please input PROPSEG from 1-8', required: true, trigger: 'change', };
const bitratePhaseSeg1={ type: 'number', min: 1, max: 8, message: 'Please input PSEG1 from 1-8', required: true };
const bitratePhaseSeg2={ type: 'number', min: 1, max: 8, message: 'Please input PSEG2 from 1-8', required: true };
const bitratePreDivider={ type: 'number', min: 1, max: 256, message: 'Please input PRESDIV from 1-256', required: true };
const bitraterJumpwidth={ type: 'number', min: 1, max: 4, message: 'Please input resync jump width from 1-4', required: true };



/* deep rule has some different */
const MasterRules = {
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  inst: [
    { required: true, type: 'number', message: 'Please choose a hardware instance', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  fd_enable: [{validator: rulesReInit, trigger: 'change'}],
  payload: [
    { required: true, message: 'Please choose payload size', trigger: 'change' },
    { validator: payloadCheck, trigger: 'change' }
  ],
  max_num_mb: [
    { required: true, type: 'number', message: 'Please input MB sizes', trigger: 'change' },
    { validator: mbCheck, trigger: 'change' }
  ],
  bitrate: {
    type: 'object',
    required: true,
    trigger: 'change',
    fields: {
      propSeg: bitratePropSegRule,
      phaseSeg1: bitratePhaseSeg1,
      phaseSeg2: bitratePhaseSeg2,
      preDivider: bitratePreDivider,
      rJumpwidth: bitraterJumpwidth,
    }
  },
  bitrate_cbt: {
    type: 'object',
    required: true,
    trigger: 'change',
    fields: {
      propSeg: bitrateCBTPropSegRule,
      phaseSeg1: bitrateCBTPhaseSeg1,
      phaseSeg2: bitrateCBTPhaseSeg2,
      preDivider: bitrateCBTPreDivider,
      rJumpwidth: bitrateCBTrJumpwidth,
    }
  },
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

function ruleChange(value: any, bounder: number, label: string){
    value.max = bounder
    if(value == bitrateCBTPropSegRule){
        value.message = `Please input ${label} from 0-${bounder}`
    }else{
        value.message = `Please input ${label} from 1-${bounder}`
    }
}

function rulesReInit(rule: any, value: any, callback: any) {
    /* 0: FlexCAN, 1: CANFD, 2: CBT */
    if(value){
        ruleChange(bitratePropSegRule, canBitRateRules.value.propSeg[1], "PROPSEG")
        ruleChange(bitratePhaseSeg1, canBitRateRules.value.phaseSeg1[1], "PSEG1")
        ruleChange(bitratePhaseSeg2, canBitRateRules.value.phaseSeg2[1], "PSEG2")
        ruleChange(bitratePreDivider, canBitRateRules.value.preDivider[1], "PRESDIV")
        ruleChange(bitraterJumpwidth, canBitRateRules.value.rJumpwidth[1], "resync jump width")
    }else{
        ruleChange(bitratePropSegRule, canBitRateRules.value.propSeg[0], "PROPSEG")
        ruleChange(bitratePhaseSeg1, canBitRateRules.value.phaseSeg1[0], "PSEG1")
        ruleChange(bitratePhaseSeg2, canBitRateRules.value.phaseSeg2[0], "PSEG2")
        ruleChange(bitratePreDivider, canBitRateRules.value.preDivider[0], "PRESDIV")
        ruleChange(bitraterJumpwidth, canBitRateRules.value.rJumpwidth[0], "resync jump width")
    }
    ruleChange(bitrateCBTPropSegRule, canBitRateRules.value.propSeg[2], "PROPSEG")
    ruleChange(bitrateCBTPhaseSeg1, canBitRateRules.value.phaseSeg1[2], "PSEG1")
    ruleChange(bitrateCBTPhaseSeg2, canBitRateRules.value.phaseSeg2[2], "PSEG2")
    ruleChange(bitrateCBTPreDivider, canBitRateRules.value.preDivider[2], "PRESDIV")
    ruleChange(bitrateCBTrJumpwidth, canBitRateRules.value.rJumpwidth[2], "resync jump width")
}


onMounted(() => {
  window.addEventListener("resize", resizeHandler);
 
  plugin
    .getInitValue()
    .then((initData) => {
      show.value=true

      /* init from meta */
      canNum.value = plugin.getMeta('can_num')
      FEATURE_CAN_HAS_ENHANCE_FIFO.value = plugin.getMeta('FEATURE_CAN_HAS_ENHANCE_FIFO')
      canBitRateRules.value = plugin.getMeta("can_bitrate_rules")

      if (initData.pluginData != undefined) {
        data.value = initData.pluginData.data;
        lock.value = initData.pluginData.lock
      }
      oscFreq.value = initData.globalData.clock.data.clockArray[initData.globalData.clock.data.activeInst].data.fxosc.freq.raw
      periFreq.value = initData.globalData.clock.data.clockArray[initData.globalData.clock.data.activeInst].data.fastClock.freq.raw
      plugin.watch('clock', (newData) => {
        oscFreq.value = newData.data.clockArray[newData.data.activeInst].data.fxosc.freq.raw
        periFreq.value = newData.data.clockArray[newData.data.activeInst].data.fastClock.freq.raw
      })
      
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