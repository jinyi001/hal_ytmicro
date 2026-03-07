<template>
  <div>
    <div v-if="initDone">
      <el-tabs
        type="border-card"
        class="demo-tabs"
      >
        <el-tab-pane label="Common">
          <CommonVUE
            :data="data.common"
            @change="dataChange"
            :lock="lock"
          />
        </el-tab-pane>
        <el-tab-pane label="MC">
          <MCVUE
            :data="data.mc"
            @change="dataChange"
            :lock="lock"
          />
        </el-tab-pane>
        <el-tab-pane label="PWM">
          <PWMVUE
            :data="data.pwm"
            @change="dataChange"
            :lock="lock"
          />
        </el-tab-pane>
        <el-tab-pane label="IC">
          <ICVUE
            :data="data.ic"
            @change="dataChange"
            :lock="lock"
          />
        </el-tab-pane>
        <el-tab-pane label="OC">
          <OCVUE
            :data="data.oc"
            @change="dataChange"
            :lock="lock"
          />
        </el-tab-pane>
        <el-tab-pane label="QD">
          <QDVUE
            :data="data.qd"
            @change="dataChange"
            :lock="lock"
          />
        </el-tab-pane>
      </el-tabs>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, provide, watch, onBeforeUnmount, nextTick } from "vue";
import { Plugin } from "plugin";
import Schema from 'async-validator';
import CommonVUE from './Common.vue'
import PWMVUE from './PWM.vue'
import ICVUE from './IC.vue'
import OCVUE from './OC.vue'
import QDVUE from './QD.vue'
import MCVUE from './MC.vue'
Schema.warning = function () { null };
const initDone = ref(false)
function dataChange() {
  plugin.dataChange(data.value);
}

const lock=ref(false)

const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
provide('plugin', plugin)
/*Stage and Table height define*/
const data = ref(
  {
    common: [],
    pwm: {
      enable: false,
      data: []
    },
    ic: {
      enable: false,
      data: []
    },
    oc: {
      enable: false,
      data: []
    },
    qd: {
      enable: false,
      data: []
    },
    mc: {
      enable: false,
      data: []
    }
  })

function resizeHandler() {
  null
}

onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});



onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {
      if (initData.pluginData != undefined) {
        data.value = Object.assign(data.value, initData.pluginData.data);
        lock.value = initData.pluginData.lock
      }
      initDone.value = true
      /*always push a data when init done*/
      plugin.dataChange(data.value);
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