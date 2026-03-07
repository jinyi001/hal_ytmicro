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
                label="Read-only"
                prop="readonly"
              >
                <el-checkbox v-model="data[activeMasterInst].readonly" />
              </el-form-item>

              <el-form-item
                label="CRC width"
                prop="crcWidth"
              >
                <el-select
                  v-model="data[activeMasterInst].crcWidth"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in crcWidthList"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Read transpose"
                prop="readTranspose"
              >
                <el-select
                  v-model="data[activeMasterInst].readTranspose"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in ['CRC_TRANSPOSE_NONE', 'CRC_TRANSPOSE_BITS']"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Write transpose"
                prop="writeTranspose"
              >
                <el-select
                  v-model="data[activeMasterInst].writeTranspose"
                  style="width:100%"
                >
                  <el-option
                    v-for="item in ['CRC_TRANSPOSE_NONE', 'CRC_TRANSPOSE_BITS']"
                    :key="item"
                    :value="item"
                    :label="item"
                  />
                </el-select>
              </el-form-item>
              <el-form-item
                label="Complement checksum"
                prop="complementChecksum"
              >
                <el-checkbox v-model="data[activeMasterInst].complementChecksum" />
              </el-form-item>
              <el-form-item
                label="CRC seed"
                prop="seed"
              >
                <el-input v-model="data[activeMasterInst].seed" />
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
  FolderOpened,
} from '@element-plus/icons-vue'
import type { FormInstance, FormRules, FormItemProp } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };

const activeMasterInst = ref(0);
const masterRef = ref<FormInstance>()
const crcWidthList = ref([])

const defMasterParam = {
  name: 'crc_config0',
  readonly: true,
  crcWidth: 'CRC_BITS_16',
  readTranspose: 'CRC_TRANSPOSE_NONE',
  writeTranspose: 'CRC_TRANSPOSE_NONE',
  complementChecksum: false,
  seed: '0x8d95'
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


function seedCheck(rule: any, value: any, callback: any, source: any) {
  const val=isProxy(source)?source:data.value[activeMasterInst.value]
  let maxSize=15
  if(val.crcWidth=='CRC_BITS_16'){
    maxSize=0xffff
  }else if(val.crcWidth=='CRC_BITS_32'){
    maxSize=0xffffffff
  }
  else if(val.crcWidth=='CRC_BITS_4'){
    maxSize=0xf
  }
  else if(val.crcWidth=='CRC_BITS_8'){
    maxSize=0xff
  }
  const seed=Number(value)
  if(seed>maxSize||seed<0){
    callback(`Seed value from 0-${maxSize}`)
  }
  callback()
}




const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
    { validator: masterNameCheck, trigger: 'change' }
  ],
  seed: [
    { required: true, message: 'Please input seed', trigger: 'change' },
    { pattern: /^0x[0-9a-fA-F]+$/, message: 'Please input hex format', trigger: 'change' },
    { validator: seedCheck, trigger: 'change'}
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
      /* Init crc width from meta */ 
      crcWidthList.value = plugin.getMeta("crc_width")
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