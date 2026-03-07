<template>
  <div>
    <div>
      <el-row>
        <el-col :span="6">
          <div class="table-container">
            <el-table
              :data="data"
              style="width: 100%"
              border
              :row-class-name="tableRowClassName"
              @row-click="masterRowClick"
              height="100%"
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
                  </span><span>{{
                    'Block-' + scope.row.BlockNumber
                  }}</span>
                </template>
              </el-table-column>
            </el-table>
          </div>
        </el-col>
        <el-col :span="18">
          <div
            class="config-container"
            v-if="data[activeMasterInst]"
          >
            <el-form
              :disabled="lock"
              ref="masterRef"
              :model="data[activeMasterInst]"
              label-width="250px"
              size="small"
              :rules="MasterRules"
              hide-required-asterisk
            >
              <el-form-item
                label="Block name"
                prop="BlockName"
              >
                <el-input v-model="data[activeMasterInst].BlockName" />
              </el-form-item>
              <el-form-item
                label="Block number"
                prop="BlockNumber"
              >
                <template #label="{ label }">
                  {{ label }}<el-tooltip
                    effect="dark"
                    content="Fee block number"
                  >
                    <el-button
                      :icon="QuestionFilled"
                      circle
                      text
                    />
                  </el-tooltip>
                </template>
                <el-input v-model.number="data[activeMasterInst].BlockNumber" />
              </el-form-item>
              <el-form-item
                label="Block size"
                prop="BlockSize"
              >
                <template #label="{ label }">
                  {{ label }}<el-tooltip
                    effect="dark"
                    content="Size of Fee block in bytes"
                  >
                    <el-button
                      :icon="QuestionFilled"
                      circle
                      text
                    />
                  </el-tooltip>
                </template>
                <el-input v-model.number="data[activeMasterInst].BlockSize" />
              </el-form-item>
              <el-form-item
                label="Cluster group"
                prop="ClrGrp"
              >
                <el-select
                  v-model="data[activeMasterInst].ClrGrp"
                  style="width: 100%;"
                >
                  <el-option
                    v-for="(item, index) in cluster"
                    :key="index"
                    :value="Number(index)"
                    :label="'Cluster-' + index"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Immediate data"
                prop="ImmediateData"
              >
                <template #label="{ label }">
                  {{ label }}<el-tooltip
                    effect="dark"
                    content="true if immediate data block"
                  >
                    <el-button
                      :icon="QuestionFilled"
                      circle
                      text
                    />
                  </el-tooltip>
                </template>
                <el-checkbox v-model="data[activeMasterInst].ImmediateData" />
              </el-form-item>
              <el-form-item
                label="Block assignment"
                prop="BlockAssignment"
              >
                <el-select
                  v-model="data[activeMasterInst].BlockAssignment"
                  style="width: 100%;"
                >
                  <el-option
                    value="FEE_PROJECT_RESERVED"
                    label="Reserved"
                  />
                  <el-option
                    value="FEE_PROJECT_SHARED"
                    label="Shared"
                  />
                  <el-option
                    value="FEE_PROJECT_APPLICATION"
                    label="Application"
                  />
                  <el-option
                    value="FEE_PROJECT_BOOTLOADER"
                    label="Bootloader"
                  />
                </el-select>
              </el-form-item>
            </el-form>
          </div>
        </el-col>
      </el-row>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, h, reactive, watch, toRef, onBeforeUnmount, nextTick } from "vue";
import { Plugin } from "plugin";
import {
  Plus,
  Delete,
  FolderOpened,
  QuestionFilled
} from '@element-plus/icons-vue'
import { ElButton } from 'element-plus'
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const defMasterParam = {
  BlockName: 'FEE_BLOCK_1',
  BlockNumber: 1,
  BlockSize: 32,
  ClrGrp: '',
  ImmediateData: false,
  BlockAssignment: 'FEE_PROJECT_RESERVED'
}





const plugin = new Plugin();
const props = defineProps<{
  data: any[],
  cluster: any[],
  lock: boolean
}>();
const lock = toRef(props, 'lock')
const data = toRef(props, 'data')
const cluster = toRef(props, 'cluster')
const winW = ref(window.innerWidth)
const contentHeight = ref(window.innerHeight - 100) // 新增高度ref

function resizeHandler() {
  winW.value = window.innerWidth
  contentHeight.value = window.innerHeight - 100 // 更新高度
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
    if(data.value.length != 0){
      const lastNum = data.value[data.value.length-1].BlockNumber
      defMasterParam.BlockName = 'FEE_BLOCK_' + (lastNum+1)
      defMasterParam.BlockNumber = lastNum + 1
      defMasterParam.BlockSize = data.value[data.value.length-1].BlockSize
      defMasterParam.ClrGrp = data.value[data.value.length-1].ClrGrp
    }
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

function blockNumCheck(rule: any, value: any, callback: any) {
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

function clrCheck(rule: any, value: any, callback: any) {
  if (value < cluster.value.length) {
    callback()
  } else {
    callback('Please choose cluster group ref')
  }
}

function blockTotalSizeCheck(rule: any, value: any, callback: any) {
  if (cluster.value.length > 0) {
    /* cluster group info size: 32B */
    const totalBlockSize = new Array(cluster.value.length).fill(32);
    const maxBlockSize = new Array(cluster.value.length).fill(32);

    for (let i = 0; i < cluster.value.length; i++) {
      cluster.value[i].ReservedSize = 0
    }

    for (let i = 0; i < data.value.length; i++){
      const blockSizeAlign = Math.ceil(data.value[i].BlockSize / 8) * 8
      if ((data.value[i].ClrGrp !== undefined) && (data.value[i].ClrGrp !== "")) {
        /* Block header info size: 32B */
        totalBlockSize[data.value[i].ClrGrp] += blockSizeAlign + 32;
        maxBlockSize[data.value[i].ClrGrp] = Math.max(maxBlockSize[data.value[i].ClrGrp], blockSizeAlign + 32)

        /*update Reserved size */
        if (data.value[i].ImmediateData == true) {
          cluster.value[data.value[i].ClrGrp].ReservedSize += blockSizeAlign + 32
        }
      }
    }

    totalBlockSize.forEach((size, index) => {
      if (cluster.value[index].clusterList.length > 0) {
        if (size > (cluster.value[index].clusterList[0].Length - cluster.value[index].ReservedSize - maxBlockSize[index] - 32)) {
          if (value == index){
            callback(new Error(`Block total size exceeds cluster size in cluster group: ${index}, suggest not to exceed half!`))
          }
        }
      }
    })
  }
}


const MasterRules = reactive<FormRules>({
  BlockName: [
    { required: true, message: 'Please input BlockName', trigger: 'change' },
    { validator: blockNumCheck, trigger: 'change' }
  ],
  BlockNumber: [
    { type: 'number', required: true, message: 'Please input BlockNumber', trigger: 'change' },
    { type: 'number', min: 1, max: 65535, message: 'value from 0-65535', trigger: 'change' },
    { validator: blockNumCheck, trigger: 'change' }
  ],
  BlockSize: [
    { type: 'number', required: true, message: 'Please input BlockSize', trigger: 'change' },
    { type: 'number', min: 1, message: 'value greater than 0', trigger: 'change' },
  ],
  ClrGrp: [
    { type: 'number', required: true, message: 'Please choose cluster group', trigger: 'change' },
    { validator: clrCheck, trigger: 'change' },
    { validator: blockTotalSizeCheck, trigger: 'change' }
  ]
})



let logList: any = []
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});

function dataCheck() {
  plugin.deleteLogs('error', logList);
  logList = [];
  const masterValidator = new Schema(MasterRules);
  for (const i in data.value) {
    masterValidator.validate(data.value[i], (errors, fields) => {
      if (errors) {
        // validation failed, errors is an array of all errors
        // fields is an object keyed by field name with an array of
        // errors per field
        for (const j in errors) {
          const id = `Block-${errors[j].field}-${i}-${j}`
          logList.push({
            id: id,
            val: errors[j].message
          });
        }
      }
    });
  }
  plugin.addLogs('error', logList);
}

const emits = defineEmits(['change'])
let dataCheck_handle: any = undefined;

onMounted(() => {
  window.addEventListener("resize", resizeHandler);

  nextTick(() => {
    masterRef.value?.validate().catch((e: any) => {
      null
    })
  })
  dataCheck();
  watch(data, () => {
    const curMaster = data.value[activeMasterInst.value]
    data.value.sort((a, b) => a.BlockNumber - b.BlockNumber)
    activeMasterInst.value = data.value.indexOf(curMaster)
    
    if (dataCheck_handle != undefined) {
      clearTimeout(dataCheck_handle);
    }
    dataCheck_handle = setTimeout(()=>{
      dataCheck();
      emits('change')
    }, 200)
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
  width: v-bind(winW*0.75-60+'px');
  height: 300px;
  border-radius: 4px;
  border: 1px solid var(--el-border-color);
}

.config-container {
  height: v-bind(contentHeight + 'px');
  overflow-y: auto;
  padding-right: 16px;
}

.table-container {
  height: v-bind(contentHeight + 'px');
  overflow-y: auto;
}
</style>