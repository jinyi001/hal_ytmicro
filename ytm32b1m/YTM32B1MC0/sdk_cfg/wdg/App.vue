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
              :model="data[activeMasterInst]"
              label-width="150px"
              size="small"
              :disabled="lock"
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
                label="Clock source"
                prop="clockSource"
              >
                <el-select
                  v-model="data[activeMasterInst].clockSource"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in wdgClockSource"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>

              <el-form-item
                label="Deep sleep mode"
                prop="opMode.deepsleep"
              >
                <el-checkbox v-model="data[activeMasterInst].opMode.deepsleep" />
              </el-form-item>
              <el-form-item
                label="Debug mode"
                prop="opMode.debug"
              >
                <el-checkbox v-model="data[activeMasterInst].opMode.debug" />
              </el-form-item>
              <el-form-item
                label="Further update"
                prop="updateEnable"
              >
                <el-checkbox v-model="data[activeMasterInst].updateEnable" />
              </el-form-item>
              <el-form-item
                label="Enable window mode"
                prop="winEnable"
              >
                <el-checkbox v-model="data[activeMasterInst].winEnable" />
              </el-form-item>
              <el-form-item
                label="Window value"
                prop="windowValue"
                v-if="data[activeMasterInst].winEnable"
              >
                <el-input
                  v-model.number="data[activeMasterInst].windowValue"
                />
              </el-form-item>
              <el-form-item
                label="Enable interrupt"
                prop="intEnable"
              >
                <el-checkbox v-model="data[activeMasterInst].intEnable" />
              </el-form-item>


              <el-form-item
                label="Timeout value"
                prop="timeoutValue"
              >
                <el-input v-model.number="data[activeMasterInst].timeoutValue" />
              </el-form-item>
              <el-form-item
                label="APB error reset"
                prop="apbErrorResetEnable"
              >
                <el-checkbox v-model="data[activeMasterInst].apbErrorResetEnable" />
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
const masterRef = ref<FormInstance>()
const wdgClockSource = ref([])


const defMasterParam = {
  name: 'wdg_config0',
  readonly: true,
  clockSource: 'WDG_SIRC_CLOCK',
  opMode: {
    deepsleep: false,
    debug: false,
  },
  updateEnable: false,
  intEnable: false,
  winEnable: false,
  windowValue: 0,
  timeoutValue: 0,
  apbErrorResetEnable: 0
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
  windowValue: [
    { type: 'number', required: true, message: 'Please input window value', trigger: 'change' },
    { type: 'number', min: 0, max: 0xffffffff, message: 'Window value from 0-0xffff_ffff', trigger: 'change' }
  ],
  timeoutValue: [
    { type: 'number', required: true, message: 'Please input timeout value', trigger: 'change' },
    { type: 'number', min: 0, max: 0xffffffff, message: 'timeout value from 0-0xffff_ffff', trigger: 'change' }
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
      /* Init wdg clock source */
      wdgClockSource.value = plugin.getMeta("wdg_clk")
      if (initData.pluginData != undefined) {
        data.value = initData.pluginData.data;
        lock.value = initData.pluginData.lock
      }else{
        data.value[activeMasterInst.value].clockSource = wdgClockSource.value[0]
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