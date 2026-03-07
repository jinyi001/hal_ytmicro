<template>
  <div>
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
              :disabled="lock"
              ref="masterRef"
              :model="data[activeMasterInst]"
              label-width="150px"
              size="small"
              :rules="MasterRules"
              hide-required-asterisk
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
                label="Baudrate"
                prop="baudrate"
              >
                <el-input v-model.number="data[activeMasterInst].baudrate" />
              </el-form-item>
              <el-form-item
                label="Parity type"
                prop="parityType"
              >
                <el-select
                  v-model="data[activeMasterInst].parityType"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in ['UART_PARITY_DISABLED', 'UART_PARITY_EVEN', 'UART_PARITY_ODD']"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Stop bits count"
                prop="stopBitsCount"
              >
                <el-select
                  v-model="data[activeMasterInst].stopBitsCount"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in ['UART_ONE_STOP_BIT', 'UART_TWO_STOP_BIT']"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Word length"
                prop="wordLength"
              >
                <el-select
                  v-model="data[activeMasterInst].wordLength"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in ['UART_7_BITS_PER_CHAR', 'UART_8_BITS_PER_CHAR','UART_9_BITS_PER_CHAR','UART_10_BITS_PER_CHAR']"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Transfer type"
                prop="transferType"
                v-show="DmaSupport"
              >
                <el-select
                  v-model="data[activeMasterInst].transferType"
                  style="width:100%"
                >
                  <el-option
                    label="UART_USING_DMA"
                    value="UART_USING_DMA"
                  />
                  <el-option
                    label="UART_USING_INTERRUPTS"
                    value="UART_USING_INTERRUPTS"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="TX DMA channel"
                prop="txDMAChannel"
                v-show="DmaSupport"
              >
                <el-input
                  v-model.number="data[activeMasterInst].txDMAChannel"
                  :disabled="data[activeMasterInst].transferType=='UART_USING_INTERRUPTS'"
                />
              </el-form-item>
              <el-form-item
                label="RX DMA channel"
                prop="rxDMAChannel"
                v-show="DmaSupport"
              >
                <el-input
                  v-model.number="data[activeMasterInst].rxDMAChannel"
                  :disabled="data[activeMasterInst].transferType=='UART_USING_INTERRUPTS'"
                />
              </el-form-item>
              <el-form-item
                label="IDLE Error Int Enable"
                prop="idleErrorIntEnable"
              >
                <el-checkbox v-model="data[activeMasterInst].idleErrorIntEnable" />
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
  FolderOpened
} from '@element-plus/icons-vue'
import type { FormInstance, FormRules, FormItemProp } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const DmaSupport = ref(true)


const defMasterParam = {
  name: 'uart_config0',
  readonly: true,
  baudrate: 115200,
  parityType: 'UART_PARITY_DISABLED',
  stopBitsCount: 'UART_ONE_STOP_BIT',
  wordLength: 'UART_8_BITS_PER_CHAR',
  transferType: 'UART_USING_INTERRUPTS',
  txDMAChannel: 0,
  rxDMAChannel: 0,
  idleErrorIntEnable: false,
}



const lock=ref(false)
const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = ref([defMasterParam])

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
    if (cnt[data.value[i].name] == undefined) {
      cnt[data.value[i].name] = 1
    } else {
      cnt[data.value[i].name]++
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




function masterDmaCheck(rule: any, value: any, callback: any, source: any) {
  const val=isProxy(source)?source:data.value[activeMasterInst.value]
  if ((val.txDMAChannel == val.rxDMAChannel) && (val.transferType == 'UART_USING_DMA')) {
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
    { type: 'number', max: 10000000, message: 'baudrate from 0-10Mbps', trigger: 'change' },
    { type: 'number', required: true, message: 'Please input baudrate', trigger: 'change' },

  ],
  txDMAChannel: [
    { validator: masterDmaCheck, trigger: 'change' }
  ],
  rxDMAChannel: [
    { validator: masterDmaCheck, trigger: 'change' }
  ],
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

onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {
      if (initData.pluginData != undefined) {
        data.value = initData.pluginData.data;
        lock.value = initData.pluginData.lock
      }
      if(plugin.getMeta('dma_num') == 0){
        DmaSupport.value = false
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
</style>