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
                </span><span v-if="scope.row.name">{{ scope.row.name }}</span><span v-else>{{ scope.$index }}</span>
              </template>
            </el-table-column>
          </el-table>
        </el-col>
        <el-col :span="18">
          <div v-if="data[activeMasterInst]">
            <el-form
              ref="masterRef"
              :disabled="lock"
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
                label="Load mode"
                prop="loadValueMode"
              >
                <el-select
                  v-model="data[activeMasterInst].loadValueMode"
                  style="width:100%"
                >
                  <el-option
                    value="PTU_LOAD_VAL_IMMEDIATELY"
                    label="Loaded iimmediately"
                  />
                  <el-option
                    value="PTU_LOAD_VAL_AT_MODULO_COUNTER"
                    label="Loaded when counter hits the modulo"
                  />
                  <el-option
                    value="PTU_LOAD_VAL_AT_NEXT_TRIGGER"
                    label="Loaded when detecting an input trigger"
                  />
                  <el-option
                    value="PTU_LOAD_VAL_AT_MODULO_COUNTER_OR_NEXT_TRIGGER"
                    label="Loaded when counter hits the modulo or detecting an input trigger"
                  />
                </el-select>
              </el-form-item>
              <!-- <el-form-item
                label="Sequence Error Interrupt"
                prop="seqErrIntEnable"
              >
                <el-checkbox v-model="data[activeMasterInst].seqErrIntEnable" />
              </el-form-item> -->
              <el-form-item
                label="Prescaler divider"
                prop="clkPreDiv"
              >
                <el-select
                  v-model="data[activeMasterInst].clkPreDiv"
                  style="width:100%"
                >
                  <el-option
                    value="PTU_CLK_PREDIV_BY_1"
                    label="/1"
                  />
                  <el-option
                    value="PTU_CLK_PREDIV_BY_2"
                    label="/2"
                  />
                  <el-option
                    value="PTU_CLK_PREDIV_BY_4"
                    label="/4"
                  />
                  <el-option
                    value="PTU_CLK_PREDIV_BY_8"
                    label="/8"
                  />
                  <el-option
                    value="PTU_CLK_PREDIV_BY_16"
                    label="/16"
                  />
                  <el-option
                    value="PTU_CLK_PREDIV_BY_32"
                    label="/32"
                  />
                  <el-option
                    value="PTU_CLK_PREDIV_BY_64"
                    label="/64"
                  />
                  <el-option
                    value="PTU_CLK_PREDIV_BY_128"
                    label="/128"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Multiplication factor"
                prop="clkPreMultFactor"
              >
                <el-select
                  v-model="data[activeMasterInst].clkPreMultFactor"
                  style="width:100%"
                >
                  <el-option
                    value="PTU_CLK_PREMULT_FACT_AS_1"
                    label="*1"
                  />
                  <el-option
                    value="PTU_CLK_PREMULT_FACT_AS_10"
                    label="*10"
                  />
                  <el-option
                    value="PTU_CLK_PREMULT_FACT_AS_20"
                    label="*20"
                  />
                  <el-option
                    value="PTU_CLK_PREMULT_FACT_AS_40"
                    label="*40"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Trigger input source"
                prop="triggerInput"
              >
                <el-select
                  v-model="data[activeMasterInst].triggerInput"
                  style="width:100%"
                >
                  <el-option
                    value="PTU_EXTERNAL_TRIGGER"
                    label="External"
                  />
                  <el-option
                    value="PTU_SOFTWARE_TRIGGER"
                    label="Software"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Continuous mode"
                prop="continuousModeEnable"
              >
                <el-checkbox v-model="data[activeMasterInst].continuousModeEnable" />
              </el-form-item>
              <el-form-item
                label="DMA for timer"
                prop="dmaEnable"
              >
                <el-checkbox v-model="data[activeMasterInst].dmaEnable" />
              </el-form-item>
              <el-form-item
                label="Interrupt for timer"
                prop="intEnable"
              >
                <el-checkbox v-model="data[activeMasterInst].intEnable" />
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
import { fa } from "element-plus/es/locale";
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()


const defMasterParam = {
  name: 'ptu_config0',
  readonly: true,
  loadValueMode: 'PTU_LOAD_VAL_IMMEDIATELY',
  seqErrIntEnable: false,
  clkPreDiv: 'PTU_CLK_PREDIV_BY_1',
  clkPreMultFactor: 'PTU_CLK_PREMULT_FACT_AS_1',
  triggerInput: 'PTU_SOFTWARE_TRIGGER',
  continuousModeEnable: false,
  dmaEnable: false,
  intEnable: false,
  instanceBackToBackEnable: false,
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

const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
})



const errorid: string[] = []
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});

function dataCheck() {
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