<template>
  <div>
    <div class="table">
      <el-auto-resizer>
        <template #default="{ height, width }">
          <el-table-v2
            :columns="columns"
            :data="data"
            :estimated-row-height="30"
            :width="width"
            :header-height="30"
            :height="height"
            :footer-height="40"
            fixed
            class="table"
          >
            <!-- <template #empty>
              <el-empty :image-size="80" />
            </template> -->
            <template #footer>
              <el-alert
                v-if="filterError != ''"
                center
                show-icon
                :title="filterError"
                type="error"
                :closable="false"
              />
            </template>
          </el-table-v2>
        </template>
      </el-auto-resizer>
    </div>
  </div>
</template>
<script setup lang="ts">
import { onMounted, ref, watch, onBeforeUnmount, h } from "vue";
import { Plugin } from "plugin";
import {
  Plus,
} from '@element-plus/icons-vue'
import { ElButton } from 'element-plus'
import type { Column } from '@element-plus/components/table-v2'

const sourceList = ref([
])

const targetList = ref([

])




const columns: Column<any>[] = [
  {
    key: 'triggerSource',
    title: 'Source',
    dataKey: 'triggerSource',
    width: 350,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        size: 'small',
        disabled:lock.value,
        value: rowData.triggerSource,
        'onChange': (event) => rowData.triggerSource = event.target.value,
        style: { width: '100%' }
      },
        sourceList.value.map((i) => {
          return h('option', {
            label: i,
            value: i
          })
        })
      )
    }
  },
  {
    key: 'targetModule',
    title: 'Target',
    dataKey: 'targetModule',
    width: 400,
    align: 'center',
    cellRenderer: ({ rowData }) => {
      return h('select', {
        size: 'small',
        disabled:lock.value,
        value: rowData.targetModule,
        'onChange': (event) => rowData.targetModule = event.target.value,
        style: { width: '100%' }
      },
        targetList.value.map((i) => {
          return h('option', {
            label: i,
            value: i
          })
        })
      )
    }
  },
  {
    width: 100,
    align: 'center',
    fixed: 'right',
    headerCellRenderer: () => {
      return h(ElButton, {
        size: 'small',
        type: "primary",
        plain: true,
        disabled:lock.value,
        icon: Plus,
        'onClick': () => addFilter(),
      })
    },
    cellRenderer: ({ rowData, rowIndex }) => {

      return h(ElButton, {
        size: 'small',
        type: "danger",
        plain: true,
        disabled:lock.value,
        'onClick': () => deleteFilter(rowIndex, rowData),
      }, () => 'Delete')
    }
  },
]

function deleteFilter(index, rowData) {
  data.value.splice(index, 1)
}

function addFilter() {
  data.value.push(JSON.parse(JSON.stringify(defMasterParam)))
}





const defMasterParam = {
  triggerSource: '',
  targetModule: ''
}



const lock=ref(false)
const plugin = new Plugin({
  lockChange:(val)=>{
    lock.value=val;
  }
});
/*Stage and Table height define*/
const data = ref<{ triggerSource: string, targetModule: string }[]>([])
const winW = ref(window.innerWidth)
const winH = ref(window.innerHeight)

function resizeHandler() {
  winW.value = window.innerWidth
  winH.value = window.innerHeight
}

const filterError = ref('')






const errorid: string[] = []
onBeforeUnmount(() => {
  window.removeEventListener("resize", resizeHandler);
});

function dataCheck() {
  for (const error of errorid) {
    plugin.deleteLog('error', error)
  }
  filterError.value = ''
  for (const i in data.value) {
    if (data.value[i].targetModule == '') {
      const id = `Target-${i}`
      plugin.addLog('error', id, 'Please input a target')
      errorid.push(id)
      filterError.value = `${id}:Please input a target`
    }
    if (data.value[i].triggerSource == '') {
      const id = `Trigger-${i}`
      plugin.addLog('error', id, 'Please input a trigger')
      errorid.push(id)
      filterError.value = `${id}:Please input a trigger`
    }
  }

  for (const j in data.value){
    for (const k in data.value){
      if (j != k){
        if ((data.value[j].targetModule == data.value[k].targetModule) && 
             data.value[j].triggerSource == data.value[k].triggerSource){
        const id = `Trigger-${k} Target-${k}`
        plugin.addLog('error', id, 'Duplicate key')
        errorid.push(id)
        filterError.value = `${id}:Duplicate key`
        }
      }
    }
  }
}

onMounted(() => {
  window.addEventListener("resize", resizeHandler);
  plugin
    .getInitValue()
    .then((initData) => {

      sourceList.value = plugin.getMeta('tmu_trigList')
      targetList.value = plugin.getMeta('tmu_targetList')
      if (initData.pluginData != undefined) {
        data.value = initData.pluginData.data;
        lock.value = initData.pluginData.lock
      }
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





</script>
<style>
.el-table .active-row {
  color: #409EFF
}

.table {
  width: v-bind(winW-10+'px');
  height: v-bind(winH - 20+'px');
}

.bg-red {
  background-color: red;
}
</style>