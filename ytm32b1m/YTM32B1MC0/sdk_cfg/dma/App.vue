<template>
  <div>
    <div>
      <el-row>
        <el-divider content-position="left">
          General Configuration
        </el-divider>
        <el-form
          :model="data.userConfig"
          label-width="300px"
          size="small"
          hide-required-asterisk
          :disabled="lock"
        >
          <el-form-item
            label="Halt on error"
            prop="haltOnError"
          >
            <el-checkbox v-model="data.userConfig.haltOnError" />
          </el-form-item>
        </el-form>
        <el-divider content-position="left">
          Channels Configuration
        </el-divider>
        <el-col :span="6">
          <el-table
            :data="data.chs"
            style="width: 90%"
            border
            :row-class-name="tableRowClassName"
            @row-click="chsRowClick"
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
                    @click="addItem('chs')"
                    :disabled="lock"
                  />
                  <el-button
                    type="primary"
                    plain
                    :icon="Delete"
                    size="small"
                    @click="deleteItem('chs')"
                    :disabled="lock"
                  />
                </el-button-group>
              </template>
              <template #default="scope">
                <span
                  v-if="scope.$index == activeChsInst"
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
          <div v-if="data.chs[activeChsInst]">
            <el-form
              ref="chsRef"
              :model="data.chs[activeChsInst]"
              label-width="350px"
              size="small"
              :rules="chsRules"
              hide-required-asterisk
              :disabled="lock"
            >
              <el-form-item
                label="Name"
                prop="name"
              >
                <el-input v-model="data.chs[activeChsInst].name" />
              </el-form-item>
              <el-form-item
                label="Read-only"
                prop="readonly"
              >
                <el-checkbox v-model="data.chs[activeChsInst].readonly" />
              </el-form-item>

              <el-form-item
                label="Channel"
                prop="virtChnConfig"
              >
                <el-select
                  v-model.number="data.chs[activeChsInst].virtChnConfig"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in chnList"
                    :key="item"
                    :value="item-1"
                    :label="`DMA_CH${item-1}`"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Source"
                prop="source"
              >
                <el-select
                  v-model="data.chs[activeChsInst].source"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in sourceList"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Callback"
                prop="callback"
              >
                <el-input v-model="data.chs[activeChsInst].callback" />
              </el-form-item>
              <el-form-item
                label="Callback param"
                prop="callbackParam"
              >
                <el-input v-model="data.chs[activeChsInst].callbackParam" />
              </el-form-item>
            </el-form>
          </div>
        </el-col>
      </el-row>
    </div>
    <el-divider content-position="left">
      Transfer Configuration
    </el-divider>
    <div style="margin: 5px">
      <el-switch
        v-model="data.transEnable"
        active-text="Enable"
        inactive-text="Disable"
        @change="emits('change')"
        :disabled="lock"
      />
    </div>
    <div v-if="data.transEnable">
      <el-row>
        <el-col :span="6">
          <el-table
            :data="data.trans"
            style="width: 90%"
            border
            :row-class-name="tableRowClassTransName"
            @row-click="transRowClick"
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
                    @click="addItem('trans')"
                    :disabled="lock"
                  />
                  <el-button
                    type="primary"
                    plain
                    :icon="Delete"
                    size="small"
                    @click="deleteItem('trans')"
                    :disabled="lock"
                  />
                </el-button-group>
              </template>
              <template #default="scope">
                <span
                  v-if="scope.$index == activeTransInst"
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
          <div v-if="data.trans[activeTransInst]">
            <el-form
              ref="transRef"
              :model="data.trans[activeTransInst]"
              label-width="350px"
              size="small"
              :rules="transRules"
              hide-required-asterisk
              :disabled="lock"
            >
              <el-form-item
                label="Name"
                prop="name"
              >
                <el-input v-model="data.trans[activeTransInst].name" />
              </el-form-item>
              <el-form-item
                label="Source address"
                prop="srcAddr"
              >
                <el-input v-model="data.trans[activeTransInst].srcAddr" />
              </el-form-item>

              <el-form-item
                label="Destination address"
                prop="destAddr"
              >
                <el-input v-model="data.trans[activeTransInst].destAddr" />
              </el-form-item>

              <el-form-item
                label="Source offset"
                prop="srcOffset"
              >
                <el-input v-model="data.trans[activeTransInst].srcOffset" />
              </el-form-item>

              <el-form-item
                label="Destination offset"
                prop="destOffset"
              >
                <el-input v-model="data.trans[activeTransInst].destOffset" />
              </el-form-item>

              <el-form-item
                label="Source transfer size"
                prop="srcTransferSize"
              >
                <el-select
                  v-model.number="data.trans[activeTransInst].srcTransferSize"
                  style="width:100%"
                >
                  <el-option
                    label="1 Byte"
                    value="DMA_TRANSFER_SIZE_1B"
                  />
                  <el-option
                    label="2 Bytes"
                    value="DMA_TRANSFER_SIZE_2B"
                  />
                  <el-option
                    label="4 Bytes"
                    value="DMA_TRANSFER_SIZE_4B"
                  />
                  <el-option
                    label="8 Bytes"
                    value="DMA_TRANSFER_SIZE_8B"
                  />
                  <el-option
                    label="16 Bytes"
                    value="DMA_TRANSFER_SIZE_16B"
                  />
                  <el-option
                    label="32 Bytes"
                    value="DMA_TRANSFER_SIZE_32B"
                  />
                  <el-option
                    label="64 Bytes"
                    value="DMA_TRANSFER_SIZE_64B"
                  />
                </el-select>
              </el-form-item>

              <el-form-item
                label="Destination transfer size"
                prop="destTransferSize"
              >
                <el-select
                  v-model.number="data.trans[activeTransInst].destTransferSize"
                  style="width:100%"
                >
                  <el-option
                    label="1 Byte"
                    value="DMA_TRANSFER_SIZE_1B"
                  />
                  <el-option
                    label="2 Bytes"
                    value="DMA_TRANSFER_SIZE_2B"
                  />
                  <el-option
                    label="4 Bytes"
                    value="DMA_TRANSFER_SIZE_4B"
                  />
                  <el-option
                    label="8 Bytes"
                    value="DMA_TRANSFER_SIZE_8B"
                  />
                  <el-option
                    label="16 Bytes"
                    value="DMA_TRANSFER_SIZE_16B"
                  />
                  <el-option
                    label="32 Bytes"
                    value="DMA_TRANSFER_SIZE_32B"
                  />
                  <el-option
                    label="64 Bytes"
                    value="DMA_TRANSFER_SIZE_64B"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Source modulo"
                prop="srcModulo"
              >
                <el-select
                  v-model="data.trans[activeTransInst].srcModulo"
                  style="width:100%"
                >
                  <el-option
                    label="OFF"
                    value="DMA_MODULO_OFF"
                  />
                  <el-option
                    label="2 Bytes"
                    value="DMA_MODULO_2B"
                  />
                  <el-option
                    label="4 Bytes"
                    value="DMA_MODULO_4B"
                  />
                  <el-option
                    label="8 Bytes"
                    value="DMA_MODULO_8B"
                  />
                  <el-option
                    label="16 Bytes"
                    value="DMA_MODULO_16B"
                  />
                  <el-option
                    label="32 Bytes"
                    value="DMA_MODULO_32B"
                  />
                  <el-option
                    label="64 Bytes"
                    value="DMA_MODULO_64B"
                  />
                  <el-option
                    label="128 Bytes"
                    value="DMA_MODULO_128B"
                  />
                  <el-option
                    label="256 Bytes"
                    value="DMA_MODULO_256B"
                  />
                  <el-option
                    label="512 Bytes"
                    value="DMA_MODULO_512B"
                  />
                  <el-option
                    label="1 MBytes"
                    value="DMA_MODULO_1MB"
                  />
                  <el-option
                    label="2 MBytes"
                    value="DMA_MODULO_2MB"
                  />
                  <el-option
                    label="4 MBytes"
                    value="DMA_MODULO_4MB"
                  />
                  <el-option
                    label="8 MBytes"
                    value="DMA_MODULO_8MB"
                  />
                  <el-option
                    label="16 MBytes"
                    value="DMA_MODULO_16MB"
                  />
                  <el-option
                    label="32 MBytes"
                    value="DMA_MODULO_32MB"
                  />
                  <el-option
                    label="64 MBytes"
                    value="DMA_MODULO_64MB"
                  />
                  <el-option
                    label="128 MBytes"
                    value="DMA_MODULO_128MB"
                  />
                  <el-option
                    label="256 MBytes"
                    value="DMA_MODULO_256MB"
                  />
                  <el-option
                    label="512 MBytes"
                    value="DMA_MODULO_512MB"
                  />
                  <el-option
                    label="1 GBytes"
                    value="DMA_MODULO_1GB"
                  />
                  <el-option
                    label="2 GBytes"
                    value="DMA_MODULO_2GB"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Destination modulo"
                prop="destModulo"
              >
                <el-select
                  v-model="data.trans[activeTransInst].destModulo"
                  style="width:100%"
                >
                  <el-option
                    label="OFF"
                    value="DMA_MODULO_OFF"
                  />
                  <el-option
                    label="2 Bytes"
                    value="DMA_MODULO_2B"
                  />
                  <el-option
                    label="4 Bytes"
                    value="DMA_MODULO_4B"
                  />
                  <el-option
                    label="8 Bytes"
                    value="DMA_MODULO_8B"
                  />
                  <el-option
                    label="16 Bytes"
                    value="DMA_MODULO_16B"
                  />
                  <el-option
                    label="32 Bytes"
                    value="DMA_MODULO_32B"
                  />
                  <el-option
                    label="64 Bytes"
                    value="DMA_MODULO_64B"
                  />
                  <el-option
                    label="128 Bytes"
                    value="DMA_MODULO_128B"
                  />
                  <el-option
                    label="256 Bytes"
                    value="DMA_MODULO_256B"
                  />
                  <el-option
                    label="512 Bytes"
                    value="DMA_MODULO_512B"
                  />
                  <el-option
                    label="1 MBytes"
                    value="DMA_MODULO_1MB"
                  />
                  <el-option
                    label="2 MBytes"
                    value="DMA_MODULO_2MB"
                  />
                  <el-option
                    label="4 MBytes"
                    value="DMA_MODULO_4MB"
                  />
                  <el-option
                    label="8 MBytes"
                    value="DMA_MODULO_8MB"
                  />
                  <el-option
                    label="16 MBytes"
                    value="DMA_MODULO_16MB"
                  />
                  <el-option
                    label="32 MBytes"
                    value="DMA_MODULO_32MB"
                  />
                  <el-option
                    label="64 MBytes"
                    value="DMA_MODULO_64MB"
                  />
                  <el-option
                    label="128 MBytes"
                    value="DMA_MODULO_128MB"
                  />
                  <el-option
                    label="256 MBytes"
                    value="DMA_MODULO_256MB"
                  />
                  <el-option
                    label="512 MBytes"
                    value="DMA_MODULO_512MB"
                  />
                  <el-option
                    label="1 GBytes"
                    value="DMA_MODULO_1GB"
                  />
                  <el-option
                    label="2 GBytes"
                    value="DMA_MODULO_2GB"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Transfer loop byte counts"
                prop="transferLoopByteCount"
              >
                <el-input v-model="data.trans[activeTransInst].transferLoopByteCount" />
              </el-form-item>
              <el-form-item
                label="Last source address adjustment"
                prop="srcLastAddrAdjust"
              >
                <el-input v-model="data.trans[activeTransInst].srcLastAddrAdjust" />
              </el-form-item>
              <el-form-item
                label="Last destination address adjustment"
                prop="destLastAddrAdjust"
              >
                <el-input v-model="data.trans[activeTransInst].destLastAddrAdjust" />
              </el-form-item>
              <el-form-item
                label="Interrupt enable"
                prop="interruptEnable"
              >
                <el-checkbox v-model="data.trans[activeTransInst].interruptEnable" />
              </el-form-item>
              <el-collapse
                v-model="col"
                style="width:100%;margin-bottom: 20px;"
              >
                <el-collapse-item
                  title="Loop transfer configuration"
                  name="1"
                >
                  <el-form-item
                    label="Number of trigger loop iteration"
                    prop="triggerLoopIterationCount"
                  >
                    <el-input v-model="data.trans[activeTransInst].loopTransferConfig.triggerLoopIterationCount" />
                  </el-form-item>
                  <el-form-item
                    label="Enable source address loop offset"
                    prop="srcOffsetEnable"
                  >
                    <el-checkbox v-model="data.trans[activeTransInst].loopTransferConfig.srcOffsetEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Enable destination address loop offset"
                    prop="dstOffsetEnable"
                  >
                    <el-checkbox v-model="data.trans[activeTransInst].loopTransferConfig.dstOffsetEnable" />
                  </el-form-item>
                  <el-form-item
                    label="Transfer loop offset value"
                    prop="triggerLoopOffset"
                  >
                    <el-input v-model="data.trans[activeTransInst].loopTransferConfig.triggerLoopOffset" />
                  </el-form-item>
                  <el-form-item
                    label="Enable channel linking when loop complete"
                    prop="transferLoopChnLinkEnable"
                  >
                    <el-checkbox v-model="data.trans[activeTransInst].loopTransferConfig.transferLoopChnLinkEnable" />
                  </el-form-item>
                  <el-form-item
                    label="The number of the next channel to be started when transfer loop complete"
                    prop="transferLoopChnLinkNumber"
                  >
                    <el-input v-model="data.trans[activeTransInst].loopTransferConfig.transferLoopChnLinkNumber" />
                  </el-form-item>

                  <el-form-item
                    label="Enable ch-to-ch linking on trigger loop complete"
                    prop="triggerLoopChnLinkEnable"
                  >
                    <el-checkbox v-model="data.trans[activeTransInst].loopTransferConfig.triggerLoopChnLinkEnable" />
                  </el-form-item>
                  <el-form-item
                    label="The number of the next channel to be started when trigger loop complete"
                    prop="triggerLoopChnLinkNumber"
                  >
                    <el-input v-model="data.trans[activeTransInst].loopTransferConfig.triggerLoopChnLinkNumber" />
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
import { type FormInstance, type FormRules, type FormItemProp, tagEmits, ElNotification } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };

const activeChsInst = ref(0);
const activeTransInst = ref(0);
const chsRef = ref<FormInstance>()
const transRef = ref<FormInstance>()
const col=ref([])
const sourceList = ref([
])
const chnList =ref(0)
const linDmaErrata = ref(false)
const defChsParam = {
  name: 'dma_config0',
  readonly: true,
  virtChnConfig: 0,
  source: 'DMA_REQ_DISABLED',
  callback: 'NULL',
  callbackParam: 'NULL',
}

const defTransParam = {
  name: 'dma_transfer_config0',
  srcAddr: 0,
  destAddr: 0,
  srcOffset: 0,
  destOffset: 0,
  srcTransferSize: "DMA_TRANSFER_SIZE_1B",
  destTransferSize: "DMA_TRANSFER_SIZE_1B",
  srcModulo: "DMA_MODULO_OFF",
  destModulo: 'DMA_MODULO_OFF',
  loopTransferConfig: {
    triggerLoopIterationCount: 1,
    srcOffsetEnable: false,
    dstOffsetEnable: false,
    triggerLoopOffset: 0,
    transferLoopChnLinkEnable: false,
    transferLoopChnLinkNumber: 0,
    triggerLoopChnLinkEnable: false,
    triggerLoopChnLinkNumber: 0,
  },
  transferLoopByteCount: 0,
  srcLastAddrAdjust: 0,
  destLastAddrAdjust: 0,
  interruptEnable: false,
}

function gotoField(field: string) {
  const fs = field.split('-')
  // console.log(fs)
  activeChsInst.value = Number(fs[1])
  nextTick(() => {
    chsRef.value?.scrollToField(fs[0])
  })
}

const lock=ref(false)

const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = reactive({
  userConfig: {
    haltOnError: false,
    errataDma: false,
  },
  chs: [defChsParam],
  transEnable: false,
  trans: [defTransParam]
}
)

function resizeHandler() {
  null
}

watch(activeChsInst, () => {
  nextTick(() => {
    chsRef.value?.validate().catch((e: any) => {
      null
    })
  })
})

watch(activeTransInst, () => {
  nextTick(() => {
    transRef.value?.validate().catch((e: any) => {
      null
    })
  })
})

function addItem(key: string) {

  if (key == 'chs') {
    const chsItem=JSON.parse(JSON.stringify(defChsParam))
    chsItem.name=`dma_config${data[key].length}`
    data.chs.push(chsItem)
    activeChsInst.value = data[key].length - 1
  }
  else if (key == 'trans') {
    const transItem=JSON.parse(JSON.stringify(defTransParam))
    transItem.name=`dma_transfer_config${data[key].length}`
    data.trans.push(transItem)
    activeTransInst.value = data[key].length - 1
  }
}

function deleteItem(key: string) {
  if (data[key].length >= 0) {
    if (key == 'chs') {
      data[key].splice(activeChsInst.value, 1)
      activeChsInst.value = data[key].length - 1
    }
    else if (key == 'trans')
    {
      data[key].splice(activeTransInst.value, 1)
      activeTransInst.value = data[key].length - 1
    }
  }
}

function chsRowClick(row) {
  const index = data.chs.indexOf(row)
  activeChsInst.value = index
  nextTick(() => {
    chsRef.value?.validate().catch((e: any) => {
      null
    })
  })
}
function chsNameCheck(rule: any, value: any, callback: any) {
  const err = false;
  const cnt = {}

  for (let i = 0; i < data.chs.length; i++) {
    if (cnt[data.chs[i][rule.field]] == undefined) {
      cnt[data.chs[i][rule.field]] = 1
    } else {
      cnt[data.chs[i][rule.field]]++
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

function transRowClick(row) {
  const index = data.trans.indexOf(row)
  activeTransInst.value = index
  nextTick(() => {
    transRef.value?.validate().catch((e: any) => {
      null
    })
  })
}
function transNameCheck(rule: any, value: any, callback: any) {
  const err = false;
  const cnt = {}

  for (let i = 0; i < data.trans.length; i++) {
    if (cnt[data.trans[i][rule.field]] == undefined) {
      cnt[data.trans[i][rule.field]] = 1
    } else {
      cnt[data.trans[i][rule.field]]++
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

function channelSourceCheck(rule: any, value: any, callback: any) {
  let linUseDma = false
  /* Only Aglaia and Phoebe2 need this errata */
  if(linDmaErrata.value){
    /* MAX channel used as channel link */
    for (const i in data.chs){
        if(data.chs[i].source.includes("LINFlexD")){
            linUseDma = true
            data.userConfig.errataDma = true
            plugin.addLog('warn', 'Channel-Source', `Channel ${chnList.value - 1} used for Channel Link to help LINFlexD DMA transfer!`)
        }
    }
    if(linUseDma){
        for (const i in data.chs){
            if(data.chs[i].virtChnConfig == (chnList.value - 1)){
                callback(new Error(`Channel ${chnList.value - 1} can't be used.`))
            }
        }
    }else{
        data.userConfig.errataDma = false
        plugin.deleteLog('warn', 'Channel-Source')
        callback()
    }
  }else{
    callback()
  }
}

const chsRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: chsNameCheck, trigger: 'change' }
  ],
  virtChnConfig: [
    { type: 'number', required: true, message: 'Please choose channel', trigger: 'change' },
    { validator: chsNameCheck, trigger: 'change' }
  ],
  source: [
    { required: true, message: 'Please choose source', trigger: 'change' },
    { validator: channelSourceCheck, trigger: 'change' }
  ],
  callback: [
    { required: true, message: 'Please input callback', trigger: 'change',pattern:/^[-_a-zA-Z0-9]+$/gi},
  ],
  callbackParam: [
    { required: true, message: 'Please input callback parameter', trigger: 'change', pattern:/^[-_a-zA-Z0-9]+$/gi},
  ],
})

const transRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: transNameCheck, trigger: 'change' }
  ]
})

const errorid: string[] = []
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});

function dataCheck() {
  chsRef.value?.validate().catch(()=>{null})
  transRef.value?.validate().catch(()=>{null})
  for (const error of errorid) {
    plugin.deleteLog('error', error)
  }
  const chsValidator = new Schema(chsRules);
  for (const i in data.chs) {
    chsValidator.validate(data.chs[i], (errors, fields) => {
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

  const transValidator = new Schema(transRules);
  for (const i in data.chs) {
    transValidator.validate(data.chs[i], (errors, fields) => {
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

const emits = defineEmits(['change'])

onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {
      chnList.value=plugin.getMeta('dma_num')
      sourceList.value=plugin.getMeta('dma_list')
      linDmaErrata.value = plugin.getMeta('LINFlexD_DMA_Errata')

      if (initData.pluginData != undefined) {
        if (initData.pluginData.data.userConfig) {
          data.userConfig = initData.pluginData.data.userConfig;
        }
        if (initData.pluginData.data.chs) {
          data.chs = initData.pluginData.data.chs;
        }
        if (initData.pluginData.data.trans) {
          data.trans = initData.pluginData.data.trans;
        }
        if (initData.pluginData.data.transEnable) {
          data.transEnable = initData.pluginData.data.transEnable;
        }
        lock.value = initData.pluginData.lock
      }
      nextTick(() => {
        chsRef.value?.validate().catch((e: any) => {
          null
        })
        transRef.value?.validate().catch((e: any) => {
          null
        })
      })
      dataCheck();
      /*always push a data when init done*/
      plugin.dataChange(data);
      watch(data, (raw) => {
        dataCheck();
        plugin.dataChange(data);
      }, { deep: true })

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
  if (rowIndex === activeChsInst.value) {
    return 'active-row'
  }
  return ''
}

const tableRowClassTransName = ({
  row,
  rowIndex,
}: {
  row: any
  rowIndex: number
}) => {
  if (rowIndex === activeTransInst.value) {
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