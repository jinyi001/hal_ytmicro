<template>
  <div class="main">
    Direction:
    <el-select
      v-model="dir"
      style="margin-left: 1em"
      @change="changeHandler"
    >
      <el-option
        v-for="dir in props.dirList"
        :key="dir"
        :label="dir.label"
        :value="dir.value"
      />
    </el-select>
  </div>
</template>
<script lang="ts" setup>
import { ref } from "vue";

const emits = defineEmits(["change"]);
const props = defineProps(["dirList"]);
const dir = ref(props.dirList[0].value);
emits("change", props.dirList[0].value);

function changeHandler(val: any) {
  emits("change", val);
}
</script>
<style scoped>
.main {
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen,
    Ubuntu, Cantarell, "Open Sans", "Helvetica Neue", sans-serif;
}
</style>