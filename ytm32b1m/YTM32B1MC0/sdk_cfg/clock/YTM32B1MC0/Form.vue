<template>
  <div class="form">
    <el-form
      ref="ruleFormRef"
      :model="form"
      size="small"
      :label-width="props.width==undefined?200:props.width"
      hide-required-asterisk
      :rules="props.rules"
      label-position="left"
    >
      <el-form-item
        v-for="item in Array.isArray(props.config)?props.config:props.config.value"
        :key="item.prop"
        :label="item.label"
        :prop="item.prop"
      >
        <el-input
          v-if="item.type == 'input'"
          v-model="form[item.prop]"
          :disabled="item.disable"
        />
        <el-input
          v-if="item.type == 'number'"
          v-model.number="form[item.prop]"
          :disabled="item.disable"
        >
          <template #append>
            {{ item.unit }}
          </template>
        </el-input>
        <el-switch
          v-if="item.type == 'switch'"
          v-model="form[item.prop]"
          :disabled="item.disable"
        />
        <el-select
          v-if="item.type == 'select'"
          v-model="form[item.prop]"
          :disabled="item.disable"
        >
          <el-option
            v-for="opt in item.opts"
            :key="opt.value"
            :label="opt.label"
            :value="opt.value"
          />
        </el-select>
      </el-form-item>
    </el-form>
  </div>
</template>
<script lang="ts" setup>
import { reactive, watch, toRaw, ref } from "vue";
import type { FormInstance } from 'element-plus'

const ruleFormRef = ref<FormInstance>()
const props = defineProps(["data", "config", "rules", "width"]);
const form = reactive(props.data);


const emits = defineEmits(["change"]);
watch(form, (raw) => {
  if (ruleFormRef.value != undefined) {

    ruleFormRef.value.validate((valid, fields) => {
      if (valid) {
        emits("change", toRaw(raw));
      }

    })
  }

}, { deep: true });
</script>
<style scoped>
.form {
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen,
    Ubuntu, Cantarell, "Open Sans", "Helvetica Neue", sans-serif;
}
</style>