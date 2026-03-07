<template>
  <div>
    <div>
      <el-form
        ref="masterRef"
        :model="data"
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
          <el-input
            v-model="data.name"
            disabled
          />
        </el-form-item>
        <el-form-item
          label="Instance"
          prop="inst"
        >
          <el-input
            v-model.number="data.inst"
            disabled
          />
        </el-form-item>
        <el-form-item
          label="Entropy delay"
          prop="entropyDelay"
          :rules="entropyDelayRule"
        >
          <el-tooltip
            content="Defines the length of each entropy sample taken"
            :show-after="1000"
          >
            <el-input v-model="data.entropyDelay" />
          </el-tooltip>
        </el-form-item>
      </el-form>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, reactive, watch, onBeforeUnmount, nextTick } from "vue";
import { Plugin } from "plugin";
import {
  QuestionFilled
} from '@element-plus/icons-vue'
import type { FormInstance, FormRules, FormItemRule } from 'element-plus'
import Schema from 'async-validator';
Schema.warning = function () { null };



const masterRef = ref<FormInstance>()

  function masterSlaveAddrCheck(rule: any, value: string, callback: any) {
  const val = Number(value)
  if (Number.isNaN(val)) {
    callback(new Error(`Please input correct format,0xaa or 54`))
    return
  }
  if ((val > 65536)||(val<0)) {
    callback(new Error(`entroy delay from 0-65535`))
  } else {
    callback()
  }
}


const entropyDelayRule:FormItemRule[]=[
    { required: true, type: 'string',  message: 'Please input entroy delay', trigger: 'change' },
    { validator: masterSlaveAddrCheck, trigger: 'change' }
]

const lock=ref(false)
const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = reactive(
  {
    name: 'TRNG_Config',
    inst: 0,
    entropyDelay: '0x0A00',
  })

function resizeHandler() {
  null
}


const MasterRules = reactive<FormRules>({
  name: [
    { required: true, message: 'Please input name', trigger: 'change' },
  ],
  entropyDelay: entropyDelayRule
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
  masterValidator.validate(data, (errors, fields) => {
    if (errors) {
      // validation failed, errors is an array of all errors
      // fields is an object keyed by field name with an array of
      // errors per field
      for (const j in errors) {
        const id = `${errors[j].field}-${j}`
        plugin.addLog('error', id, errors[j].message)
        errorid.push(id)
      }
    }
  });

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
      })
      dataCheck();
      /*always push a data when init done*/
      plugin.dataChange(data);
      watch(data, (raw) => {
        dataCheck();
        plugin.dataChange(data);
      }, {
        deep: true
      })

    })
    .catch((e: any) => {
      console.error(e);
    });


});




</script>
<style>
.el-table .active-row {
  color: #409EFF
}
</style>