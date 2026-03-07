<template>
  <div>
    <el-divider content-position="left">
      SPI Master Config
    </el-divider>
    <div>
      <el-row>
        <el-col :span="6">
          <el-table
            :data="data.master"
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
                    :disabled="lock"
                    type="primary"
                    plain
                    :icon="Plus"
                    size="small"
                    @click="addItem('master')"
                  />
                  <el-button
                    :disabled="lock"
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
          <div v-if="data.master[activeMasterInst]">
            <el-form
              ref="masterRef"
              :model="data.master[activeMasterInst]"
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
                <el-input v-model="data.master[activeMasterInst].name" />
              </el-form-item>
              <el-form-item
                label="Read-only"
                prop="readonly"
              >
                <el-checkbox v-model="data.master[activeMasterInst].readonly" />
              </el-form-item>
              <el-form-item
                label="Baudrate"
                prop="baudrate"
              >
                <el-input v-model.number="data.master[activeMasterInst].baudrate" />
              </el-form-item>
              <el-form-item
                label="PCS"
                prop="pcs"
              >
                <el-select
                  v-model="data.master[activeMasterInst].pcs"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in pcsList"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="PCS polarity"
                prop="polarity"
              >
                <el-select
                  v-model="data.master[activeMasterInst].polarity"
                  style="width:100%"
                >
                  <el-option
                    label="SPI_ACTIVE_LOW"
                    value="SPI_ACTIVE_LOW"
                  />
                  <el-option
                    label="SPI_ACTIVE_HIGH"
                    value="SPI_ACTIVE_HIGH"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="PCS continuous"
                prop="isPcsContinuous"
              >
                <el-checkbox v-model="data.master[activeMasterInst].isPcsContinuous" />
              </el-form-item>
              <el-form-item
                label="Bits/frame"
                prop="bitcount"
              >
                <el-input v-model.number="data.master[activeMasterInst].bitcount" />
              </el-form-item>
              <el-form-item
                label="Phase"
                prop="clkPhase"
              >
                <el-select
                  v-model="data.master[activeMasterInst].clkPhase"
                  style="width:100%"
                >
                  <el-option
                    label="SPI_CLOCK_PHASE_1ST_EDGE"
                    value="SPI_CLOCK_PHASE_1ST_EDGE"
                  />
                  <el-option
                    label="SPI_CLOCK_PHASE_2ND_EDGE"
                    value="SPI_CLOCK_PHASE_2ND_EDGE"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="CLK polarity"
                prop="clkPolarity"
              >
                <el-select
                  v-model="data.master[activeMasterInst].clkPolarity"
                  style="width:100%"
                >
                  <el-option
                    label="SPI_SCK_ACTIVE_HIGH"
                    value="SPI_SCK_ACTIVE_HIGH"
                  />
                  <el-option
                    label="SPI_SCK_ACTIVE_LOW"
                    value="SPI_SCK_ACTIVE_LOW"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Bit width"
                prop="width"
              >
                <el-select
                  v-model="data.master[activeMasterInst].width"
                  style="width:100%"
                >
                  <el-option
                    label="SPI_SINGLE_BIT"
                    value="SPI_SINGLE_BIT_XFER"
                  />
                  <el-option
                    label="SPI_TWO_BIT"
                    value="SPI_TWO_BIT_XFER"
                  />
                  <el-option
                    label="SPI_FOUR_BIT"
                    value="SPI_FOUR_BIT_XFER"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="LSB"
                prop="lsbFirst"
              >
                <el-checkbox v-model="data.master[activeMasterInst].lsbFirst" />
              </el-form-item>
              <el-form-item
                label="Transfer type"
                prop="transferType"
              >
                <el-select
                  v-model="data.master[activeMasterInst].transferType"
                  style="width:100%"
                >
                  <el-option
                    label="SPI_USING_INTERRUPTS"
                    value="SPI_USING_INTERRUPTS"
                  />
                  <el-option
                    label="SPI_USING_DMA"
                    value="SPI_USING_DMA"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="TX DMA channel"
                prop="txDMAChannel"
              >
                <el-input
                  v-model.number="data.master[activeMasterInst].txDMAChannel"
                  :disabled="data.master[activeMasterInst].transferType=='SPI_USING_INTERRUPTS'"
                />
              </el-form-item>
              <el-form-item
                label="RX DMA channel"
                prop="rxDMAChannel"
              >
                <el-input
                  v-model.number="data.master[activeMasterInst].rxDMAChannel"
                  :disabled="data.master[activeMasterInst].transferType=='SPI_USING_INTERRUPTS'"
                />
              </el-form-item>
              <el-form-item
                label="Callback"
                prop="callback"
              >
                <el-input v-model="data.master[activeMasterInst].callback" />
              </el-form-item>
              <el-form-item
                label="Calllback parameter"
                prop="callbackParam"
              >
                <el-input v-model="data.master[activeMasterInst].callbackParam" />
              </el-form-item>
            </el-form>
          </div>
        </el-col>
      </el-row>
    </div>
    <el-divider content-position="left">
      SPI Slave Config
    </el-divider>
    <div>
      <el-row>
        <el-col :span="6">
          <el-table
            :data="data.slave"
            style="width: 100%"
            border
            
            :row-class-name="tableSlaveRowClassName"
            @row-click="slaveRowClick"
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
                    :disabled="lock"
                    :icon="Plus"
                    size="small"
                    @click="addItem('slave')"
                  />
                  <el-button
                    type="primary"
                    plain
                    :disabled="lock"
                    :icon="Delete"
                    size="small"
                    @click="deleteItem('slave')"
                  />
                </el-button-group>
              </template>
              <template #default="scope">
                <span
                  v-if="scope.$index==activeSlaveInst"
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
          <div v-if="data.slave[activeSlaveInst]">
            <el-form
              :disabled="lock"
              ref="slaveRef"
              :model="data.slave[activeSlaveInst]"
              size="small"
              label-width="150px"
              :rules="slaveRules"
              hide-required-asterisk
            >
              <el-form-item
                label="Name"
                prop="name"
              >
                <el-input v-model="data.slave[activeSlaveInst].name" />
              </el-form-item>
              <el-form-item
                label="Read-only"
                prop="readonly"
              >
                <el-checkbox v-model="data.slave[activeSlaveInst].readonly" />
              </el-form-item>
              <el-form-item
                label="PCS"
                prop="pcs"
              >
                <el-select
                  v-model="data.slave[activeSlaveInst].pcs"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in pcsList"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="PCS polarity"
                prop="polarity"
              >
                <el-select
                  v-model="data.slave[activeSlaveInst].polarity"
                  style="width:100%"
                >
                  <el-option
                    label="SPI_ACTIVE_LOW"
                    value="SPI_ACTIVE_LOW"
                  />
                  <el-option
                    label="SPI_ACTIVE_HIGH"
                    value="SPI_ACTIVE_HIGH"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Bits/frame"
                prop="bitcount"
              >
                <el-input v-model.number="data.slave[activeSlaveInst].bitcount" />
              </el-form-item>
              <el-form-item
                label="Phase"
                prop="clkPhase"
              >
                <el-select
                  v-model="data.slave[activeSlaveInst].clkPhase"
                  style="width:100%"
                >
                  <el-option
                    label="SPI_CLOCK_PHASE_1ST_EDGE"
                    value="SPI_CLOCK_PHASE_1ST_EDGE"
                  />
                  <el-option
                    label="SPI_CLOCK_PHASE_2ND_EDGE"
                    value="SPI_CLOCK_PHASE_2ND_EDGE"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="CLK polarity"
                prop="clkPolarity"
              >
                <el-select
                  v-model="data.slave[activeSlaveInst].clkPolarity"
                  style="width:100%"
                >
                  <el-option
                    label="SPI_SCK_ACTIVE_HIGH"
                    value="SPI_SCK_ACTIVE_HIGH"
                  />
                  <el-option
                    label="SPI_SCK_ACTIVE_LOW"
                    value="SPI_SCK_ACTIVE_LOW"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Bit width"
                prop="width"
              >
                <el-select
                  v-model="data.slave[activeSlaveInst].width"
                  style="width:100%"
                >
                  <el-option
                    label="SPI_SINGLE_BIT"
                    value="SPI_SINGLE_BIT_XFER"
                  />
                  <el-option
                    label="SPI_TWO_BIT"
                    value="SPI_TWO_BIT_XFER"
                  />
                  <el-option
                    label="SPI_FOUR_BIT"
                    value="SPI_FOUR_BIT_XFER"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="LSB"
                prop="lsbFirst"
              >
                <el-checkbox v-model="data.slave[activeSlaveInst].lsbFirst" />
              </el-form-item>
              <el-form-item
                label="Transfer type"
                prop="transferType"
              >
                <el-select
                  v-model="data.slave[activeSlaveInst].transferType"
                  style="width:100%"
                >
                  <el-option
                    label="SPI_USING_INTERRUPTS"
                    value="SPI_USING_INTERRUPTS"
                  />
                  <el-option
                    label="SPI_USING_DMA"
                    value="SPI_USING_DMA"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="TX DMA channel"
                prop="txDMAChannel"
              >
                <el-input
                  v-model.number="data.slave[activeSlaveInst].txDMAChannel"
                  :disabled="data.slave[activeSlaveInst].transferType=='SPI_USING_INTERRUPTS'"
                />
              </el-form-item>
              <el-form-item
                label="RX DMA channel"
                prop="rxDMAChannel"
              >
                <el-input
                  v-model.number="data.slave[activeSlaveInst].rxDMAChannel"
                  :disabled="data.slave[activeSlaveInst].transferType=='SPI_USING_INTERRUPTS'"
                />
              </el-form-item>
              <el-form-item
                label="Callback"
                prop="callback"
              >
                <el-input v-model="data.slave[activeSlaveInst].callback" />
              </el-form-item>
              <el-form-item
                label="Calllback parameter"
                prop="callbackParam"
              >
                <el-input v-model="data.slave[activeSlaveInst].callbackParam" />
              </el-form-item>
            </el-form>
          </div>
        </el-col>
      </el-row>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, computed, reactive, watch, toRaw, onBeforeUnmount, nextTick, isReactive } from "vue";
import { Plugin } from "plugin";
import {
  Plus,
  Delete,
  FolderOpened
} from '@element-plus/icons-vue'
import type { FormInstance, FormRules, FormItemProp } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const activeSlaveInst = ref(0);
const masterRef = ref<FormInstance>()
const slaveRef = ref<FormInstance>()
const pcsList = ref([])
// const spiMaxBaud = ref(0)

const defMasterParam = {
  name: 'spi_MasterConfig0',
  readonly: true,
  baudrate: 100000,
  pcs: 'SPI_PCS0',
  polarity: 'SPI_ACTIVE_LOW',
  isPcsContinuous: false,
  bitcount: 8,
  clkPhase: 'SPI_CLOCK_PHASE_1ST_EDGE',
  clkPolarity: 'SPI_SCK_ACTIVE_HIGH',
  lsbFirst: false,
  txDMAChannel: 0,
  rxDMAChannel: 0,
  transferType: 'SPI_USING_INTERRUPTS',
  callback: 'NULL',
  callbackParam: 'NULL',
  width:'SPI_SINGLE_BIT_XFER'
}

const defSlaveParam = {
  name: 'spi_SlaveConfig0',
  readonly: true,
  pcs: 'SPI_PCS0',
  polarity: 'SPI_ACTIVE_LOW',
  bitcount: 8,
  clkPhase: 'SPI_CLOCK_PHASE_1ST_EDGE',
  clkPolarity: 'SPI_SCK_ACTIVE_HIGH',
  lsbFirst: false,
  txDMAChannel: 0,
  rxDMAChannel: 0,
  transferType: 'SPI_USING_INTERRUPTS',
  callback: 'NULL',
  callbackParam: 'NULL',
  width:'SPI_SINGLE_BIT_XFER'
}

const lock=ref(false)
const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = reactive({
  master: [JSON.parse(JSON.stringify(defMasterParam))],
  slave: [JSON.parse(JSON.stringify(defSlaveParam))]
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

watch(activeSlaveInst, () => {
  nextTick(() => {
    slaveRef.value?.validate().catch((e: any) => {
      null
    })
  })
})
function addItem(key: string) {

  if (key == 'master') {
    data[key].push(JSON.parse(JSON.stringify(defMasterParam)))
    activeMasterInst.value = data[key].length - 1

  } else {
    data[key].push(JSON.parse(JSON.stringify(defSlaveParam)))
    activeSlaveInst.value = data[key].length - 1

  }


}

function deleteItem(key: string) {
  if (data[key].length >= 0) {
    if (key == 'master') {
      data[key].splice(activeMasterInst.value, 1)
      activeMasterInst.value = data[key].length - 1
    } else {
      data[key].splice(activeSlaveInst.value, 1)
      activeSlaveInst.value = data[key].length - 1
    }
  }
}

function masterRowClick(row) {
  const index = data.master.indexOf(row)
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
  for (let i = 0; i < data.master.length; i++) {
    if (cnt[data.master[i].name] == undefined) {
      cnt[data.master[i].name] = 1
    } else {
      cnt[data.master[i].name]++
    }
  }

  for (const key of Object.keys(cnt)) {
    if (cnt[key] > 1) {
      callback(new Error(`Duplicate SPI${key} name`))
    }
  }

  if (!err) {
    callback()
  }
}



function masterDmaCheck(rule: any, value: any, callback: any, source: any) {
  const val=isReactive(source)?source:data.master[activeMasterInst.value]
  if ((val.txDMAChannel == val.rxDMAChannel) && (val.transferType == 'SPI_USING_DMA')) {
    callback(new Error(`Duplicate DMA channle`))
  } else {
    callback()
  }
}

const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  baudrate: [
    // { type: 'number', min:0, max:spiMaxBaud.value, message: `baudrate from 0-${spiMaxBaud.value}bps`, trigger: 'change' },
    { type: 'number', required: true, message: 'Please input baudrate', trigger: 'change' },

  ],
  bitcount: [
    { type: 'number', min: 1, max: 4095, message: 'bitcount from 1-4095', trigger: 'change' },
    { type: 'number', required: true, message: 'Please input bitcount', trigger: 'change' },

  ],
  pcs: [
    { required: true, message: 'Please choose pcs', trigger: 'change' },
  ],
  width: [
    { required: true, message: 'Please choose bits width', trigger: 'change' },
  ],
  txDMAChannel: [
    { validator: masterDmaCheck, trigger: 'change' }
  ],
  rxDMAChannel: [
    { validator: masterDmaCheck, trigger: 'change' }
  ],
  callback: [
    { required: true, message: 'Please input callback', trigger: 'change',pattern:/^[-_a-zA-Z0-9]+$/gi},
  ],
  callbackParam: [
    { required: true, message: 'Please input callback parameter', trigger: 'change', pattern:/^[-_a-zA-Z0-9]+$/gi},
  ],
})

function slaveRowClick(row) {
  const index = data.slave.indexOf(row)
  activeSlaveInst.value = index
}

function slavewNameCheck(rule: any, value: any, callback: any) {
  let err = false;
  const cnt = {}
  for (let i = 0; i < data.slave.length; i++) {
    if (cnt[data.slave[i].name] == undefined) {
      cnt[data.slave[i].name] = 1
    } else {
      cnt[data.slave[i].name]++
    }
  }

  for (const key of Object.keys(cnt)) {
    if (cnt[key] > 1) {
      err = true
      callback(new Error(`Duplicate SPI${key} name`))
    }
  }

  if (!err) {
    callback()
  }
}



function slaveDmaCheck(rule: any, value: any, callback: any, source: any) {
  const val=isReactive(source)?source:data.slave[activeSlaveInst.value]
  if ((val.txDMAChannel == val.rxDMAChannel) && (val.transferType == 'SPI_USING_DMA')) {
    callback(new Error(`Duplicate DMA channle`))
  } else {
    callback()
  }
}

const slaveRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: slavewNameCheck, trigger: 'change' }
  ],
  bitcount: [
    { type: 'number', required: true, message: 'Please input bitcount', trigger: 'change' },
    { type: 'number', min: 8, max: 32, message: 'bitcount from 8-32', trigger: 'change' }
  ],
  width: [
    { required: true, message: 'Please choose bits width', trigger: 'change' },
  ],
  txDMAChannel: [
    { validator: slaveDmaCheck, trigger: 'change' }
  ],
  rxDMAChannel: [
    { validator: slaveDmaCheck, trigger: 'change' }
  ],
  callback: [
    { required: true, message: 'Please input callback', trigger: 'change',pattern:/^[-_a-zA-Z0-9]+$/gi},
  ],
  callbackParam: [
    { required: true, message: 'Please input callback parameter', trigger: 'change', pattern:/^[-_a-zA-Z0-9]+$/gi},
  ],
})

const errorid: string[] = []
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});

function dataCheck() {
  masterRef.value?.validate().catch(()=>{null})
  slaveRef.value?.validate().catch(()=>{null})
  for (const error of errorid) {
    plugin.deleteLog('error', error)
  }
  const masterValidator = new Schema(MasterRules);
  for (const i in data.master) {
    masterValidator.validate(data.master[i], (errors, fields) => {
      if (errors) {
        // validation failed, errors is an array of all errors
        // fields is an object keyed by field name with an array of
        // errors per field
        for (const j in errors) {
          const id = `${errors[j].field}-Master${i}-${j}`
          plugin.addLog('error', id, errors[j].message)
          errorid.push(id)
        }
      }

    });
  }
  const slaveValidator = new Schema(slaveRules);
  for (const i in data.slave) {
    slaveValidator.validate(data.slave[i], (errors, fields) => {
      if (errors) {
        // validation failed, errors is an array of all errors
        // fields is an object keyed by field name with an array of
        // errors per field
        for (const j in errors) {
          const id = `${errors[j].field}-Salve${i}-${j}`
          plugin.addLog('error', id, errors[j].message)
          errorid.push(id)
        }
      }
      // validation passed
    });
  }
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
      // spiMaxBaud.value = plugin.getMeta("spi_max_baudrate")
      /* Get SPI PCS List */
      pcsList.value = plugin.getMeta("spi_pcs_list")
      nextTick(() => {
        masterRef.value?.validate().catch((e: any) => {
          null
        })
        slaveRef.value?.validate().catch((e: any) => {
          null
        })
      })
      dataCheck();
      /*always push a data when init done*/
      plugin.dataChange(toRaw(data));
      watch(data, (raw) => {
        dataCheck();
        plugin.dataChange(toRaw(raw));
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

const tableSlaveRowClassName = ({
  row,
  rowIndex,
}: {
  row: any
  rowIndex: number
}) => {
  if (rowIndex === activeSlaveInst.value) {
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