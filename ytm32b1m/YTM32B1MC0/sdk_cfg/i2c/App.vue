<template>
  <div>
    <el-divider content-position="left">
      I2C Master Config
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
          <div v-if="data.master[activeMasterInst]">
            <el-form
              ref="masterRef"
              hide-required-asterisk
              :model="data.master[activeMasterInst]"
              label-width="200px"
              size="small"
              :rules="MasterRules"
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
                label="Slave address,7-bit or 10-bit"
                prop="slaveAddress"
              >
                <el-input v-model="data.master[activeMasterInst].slaveAddress" />
              </el-form-item>
              <el-form-item
                label="10-bit slave address"
                prop="is10bitAddr"
              >
                <el-checkbox
                  v-model="data.master[activeMasterInst].is10bitAddr"
                />
              </el-form-item>
              <el-form-item
                label="I2C Operating mode"
                prop="operatingMode"
              >
                <el-select
                  v-model="data.master[activeMasterInst].operatingMode"
                  style="width:100%"
                >
                  <el-option
                    label="Fast-mode (Fm), bidirectional data transfers up to 400 kbit/s"
                    value="I2C_FAST_MODE"
                  />
                  <el-option
                    label="Standard-mode (Sm), bidirectional data transfers up to 100 kbit/s"
                    value="I2C_STANDARD_MODE"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Baudrate"
                prop="baudRate"
              >
                <el-input v-model.number="data.master[activeMasterInst].baudRate" />
              </el-form-item>
              <el-form-item
                label="Type of LPI2C transfer"
                prop="transferType"
              >
                <el-select
                  v-model="data.master[activeMasterInst].transferType"
                  style="width:100%"
                >
                  <el-option
                    label="I2C_USING_INTERRUPTS"
                    value="I2C_USING_INTERRUPTS"
                  />
                  <el-option
                    label="I2C_USING_DMA"
                    value="I2C_USING_DMA"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="DMA channel"
                prop="dmaChannel"
              >
                <el-input
                  v-model.number="data.master[activeMasterInst].dmaChannel"
                  :disabled="data.master[activeMasterInst].transferType=='I2C_USING_INTERRUPTS'"
                />
              </el-form-item>
              <el-form-item
                label="Master callback function"
                prop="masterCallback"
              >
                <el-input v-model="data.master[activeMasterInst].masterCallback" />
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
      I2C Slave Config
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
                    :icon="Plus"
                    size="small"
                    @click="addItem('slave')"
                    :disabled="lock"
                  />
                  <el-button
                    type="primary"
                    plain
                    :icon="Delete"
                    size="small"
                    @click="deleteItem('slave')"
                    :disabled="lock"
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
              ref="slaveRef"
              :disabled="lock"
              hide-required-asterisk
              :model="data.slave[activeSlaveInst]"
              size="small"
              label-width="200px"
              :rules="slaveRules"
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
                label="Slave address,7-bit or 10-bit"
                prop="slaveAddress"
              >
                <el-input v-model="data.slave[activeSlaveInst].slaveAddress" />
              </el-form-item>
              <el-form-item
                label="10-bit slave address"
                prop="is10bitAddr"
              >
                <el-checkbox
                  v-model="data.slave[activeSlaveInst].is10bitAddr"
                />
              </el-form-item>
              <el-form-item
                label="I2C Operating mode"
                prop="operatingMode"
              >
                <el-select
                  v-model="data.slave[activeSlaveInst].operatingMode"
                  style="width:100%"
                >
                  <el-option
                    label="Fast-mode (Fm), bidirectional data transfers up to 400 kbit/s"
                    value="I2C_FAST_MODE"
                  />
                  <el-option
                    label="Standard-mode (Sm), bidirectional data transfers up to 100 kbit/s"
                    value="I2C_STANDARD_MODE"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Enable slave listening"
                prop="slaveListening"
              >
                <el-checkbox v-model="data.slave[activeSlaveInst].slaveListening" />
              </el-form-item>
              <el-form-item
                label="Type of LPI2C transfer"
                prop="transferType"
              >
                <el-select
                  v-model="data.slave[activeSlaveInst].transferType"
                  style="width:100%"
                >
                  <el-option
                    label="I2C_USING_INTERRUPTS"
                    value="I2C_USING_INTERRUPTS"
                  />
                  <el-option
                    label="I2C_USING_DMA"
                    value="I2C_USING_DMA"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="DMA channel"
                prop="dmaChannel"
              >
                <el-input
                  v-model.number="data.slave[activeSlaveInst].dmaChannel"
                  :disabled="data.slave[activeSlaveInst].transferType=='I2C_USING_INTERRUPTS'"
                />
              </el-form-item>
              <el-form-item
                label="Slave callback function"
                prop="slaveCallback"
              >
                <el-input v-model="data.slave[activeSlaveInst].slaveCallback" />
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
import { onMounted, ref, computed, reactive, watch, toRaw, onBeforeUnmount, nextTick } from "vue";
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

const defMasterParam = {
  name: 'I2C_MasterConfig0',
  readonly: true,
  slaveAddress: '0x45',
  is10bitAddr: false,
  operatingMode: 'I2C_FAST_MODE',
  baudRate: 400000,
  baudRateHS: 0,
  masterCode: 0,
  transferType: 'I2C_USING_INTERRUPTS',
  dmaChannel: 0,
  masterCallback: 'NULL',
  callbackParam: 'NULL',
}

const defSlaveParam = {
  name: 'I2C_SlaveConfig0',
  readonly: true,
  slaveAddress: '0x45',
  is10bitAddr: false,
  operatingMode: 'I2C_STANDARD_MODE',
  slaveListening: true,
  transferType: 'I2C_USING_INTERRUPTS',
  dmaChannel: 0,
  slaveCallback: 'NULL',
  callbackParam: 'NULL',
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
      callback(new Error(`Duplicate ${key} name`))
    }
  }

  if (!err) {
    callback()
  }
}
function masterSlaveAddrCheck(rule: any, value: string, callback: any) {
  const is10bitAddr = data.master[activeMasterInst.value].is10bitAddr
  const addr = Number(value)
  if (Number.isNaN(addr)) {
    callback(new Error(`Please input correct format,0x35 or 54`))
    return
  }
  if (is10bitAddr && (addr > 2047)) {
    callback(new Error(`Address should less than 2048`))
  } else if (!is10bitAddr && (addr > 127)) {
    callback(new Error(`Address should less than 128`))
  } else {
    callback()
  }
}

function masterBaudRateCheck(rule: any, value: number, callback: any) {
  const fast = data.master[activeMasterInst.value].operatingMode
  if (fast == 'I2C_FAST_MODE') {
    if (value > 400000) {
      callback(new Error(`Max baudrate is 400Kbit/s`))
      return
    }
  }
  if (fast == 'I2C_STANDARD_MODE') {
    if (value > 100000) {
      callback(new Error(`Max baudrate is 100Kbit/s`))
      return
    }
  }
  callback()
}
function masterDmaCheck(rule: any, value: number, callback: any) {
  const dmaList:any[]=[]
  for(const i in data.slave){
    if(data.slave[i].transferType=='I2C_USING_DMA'){
      dmaList.push(data.slave[i].dmaChannel)
    }
  }
  for(const i in data.master){
    if(data.master[i].transferType=='I2C_USING_DMA'){
      if(dmaList.indexOf(data.master[i].dmaChannel)==-1){
        dmaList.push(data.master[i].dmaChannel)
      }else{
        callback('Duplicate DMA channel')
      }
    }
  }
  callback()
}

function slaveDmaCheck(rule: any, value: number, callback: any) {
  const dmaList:any[]=[]
  for(const i in data.master){
    if(data.master[i].transferType=='I2C_USING_DMA'){
      dmaList.push(data.master[i].dmaChannel)
    }
    
  }
  for(const i in data.slave){
    if(data.slave[i].transferType=='I2C_USING_DMA'){
      if(dmaList.indexOf(data.slave[i].dmaChannel)==-1){
        dmaList.push(data.slave[i].dmaChannel)
      }else{
        callback('Duplicate DMA channel')
      }
    }
  }
  callback()
}

const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  baudRate: [
    { type: 'number', required: true, min: 0, message: 'Please input baudrate', trigger: 'change' },
    { validator: masterBaudRateCheck, trigger: 'change' }
  ],
  slaveAddress: [
    { required: true, type: 'string', min: 1, max: 5, message: 'Slave address format error', trigger: 'change' },
    { validator: masterSlaveAddrCheck, trigger: 'change' }
  ],
  dmaChannel: [
    { required: true, type: 'number', min: 0, max: 32, message: 'Plese choose a DMA channel', trigger: 'change' },
    // { validator: masterDmaCheck, trigger: 'change' }
  ],
  masterCallback: [
    { required: true, trigger: 'change', pattern:/^[-_a-zA-Z0-9]+$/gi},
  ],
  callbackParam: [
    { required: true, trigger: 'change', pattern:/^[-_a-zA-Z0-9]+$/gi},
  ]
})

function slaveRowClick(row) {
  const index = data.slave.indexOf(row)
  activeSlaveInst.value = index
}

function slaveNameCheck(rule: any, value: any, callback: any) {
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
      callback(new Error(`Duplicate ${key} name`))
    }
  }

  if (!err) {
    callback()
  }
}
function slaveSlaveAddrCheck(rule: any, value: string, callback: any) {
  const is10bitAddr = data.slave[activeSlaveInst.value].is10bitAddr
  const addr = Number(value)
  if (Number.isNaN(addr)) {
    callback(new Error(`Please input correct format,0x35 or 54`))
    return
  }
  if (is10bitAddr && (addr > 2047)) {
    callback(new Error(`Address should less than 2048`))
  } else if (!is10bitAddr && (addr > 127)) {
    callback(new Error(`Address should less than 128`))
  } else {
    callback()
  }
}



const slaveRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: slaveNameCheck, trigger: 'change' }
  ],
  slaveAddress: [
    { required: true, type: 'string', min: 1, max: 5, message: 'Slave address format error', trigger: 'change' },
    { validator: slaveSlaveAddrCheck, trigger: 'change' }
  ],
  dmaChannel: [
    { required: true, type: 'number', min: 0, max: 32, message: 'Plese choose a DMA channel', trigger: 'change' },
    // { validator: slaveDmaCheck, trigger: 'change' }
  ],
  slaveCallback: [
    { required: true, trigger: 'change', pattern:/^[-_a-zA-Z0-9]+$/gi},
  ],
  callbackParam: [
    { required: true, trigger: 'change', pattern:/^[-_a-zA-Z0-9]+$/gi},
  ]
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