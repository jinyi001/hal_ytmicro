<template>
  <div>
    <div v-if="initDone">
      <el-tabs
        type="border-card"
        class="demo-tabs"
      >
        <el-tab-pane label="Common">
          <CommonVUE
            :data="data.flashConfig"
            :lock="lock"
            @change="dataChange"
          />
        </el-tab-pane>
        <el-tab-pane label="Cluster-Group">
          <ClusterVUE
            :data="data.clusterConfig"
            :lock="lock"
            :sector="data.flashConfig.sectorList"
            @change="dataChange"
          />
        </el-tab-pane>
        <el-tab-pane label="Block">
          <BlockVUE
            :lock="lock"
            :data="data.blockConfig"
            :cluster="data.clusterConfig"
            @change="dataChange"
          />
        </el-tab-pane>
      </el-tabs>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, reactive, watch, onBeforeUnmount, nextTick, provide } from "vue";
import { Plugin } from "plugin";
import Schema from 'async-validator';
import CommonVUE from './Common.vue'
import ClusterVUE from './Cluster.vue'
import BlockVUE from './Block.vue'

Schema.warning = function () { null };
const initDone = ref(false)
function dataChange() {
  plugin.dataChange(data);
}

const lock=ref(false)

const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
provide('plugin', plugin)
/*Stage and Table height define*/
const data = reactive(
  {
    flashConfig: {
      name:'FEEGenConfig',
      readonly:true,
      acEraseFunPtr:'NULL',
      acWriteFunPtr:'NULL',
      jobEndNotificationFunPtr:'Fee_JobEndNotification',
      jobErrorNotificationFunPtr:'Fee_JobErrorNotification',
      eDefaultMode:'MEMIF_MODE_SLOW',
      maxReadFastMode:512,
      maxReadNormalMode:256,
      maxWriteFastMode:128,
      maxWriteNormalMode:8,
      sectorList:[]
    },
    clusterConfig:[],
    blockConfig:[]
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
        Object.assign(data, initData.pluginData.data);
        lock.value = initData.pluginData.lock
      }
      initDone.value = true
      /*always push a data when init done*/
      plugin.dataChange(data);
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