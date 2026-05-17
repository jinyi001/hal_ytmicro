import { ref, h, Ref } from "vue";
import Form from "./Form.vue";
import Konva from "konva";
import { ElMessageBox, ElNotification } from "element-plus";
import { Plugin } from "plugin";
const defaultScaleRatio = 1.1
const plugin = new Plugin();
const freqFontSize = 18
const plusIcon =
    '<svg t="1661419115069" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="2335" width="200" height="200"><path d="M1005.312 914.752l-198.528-198.464A448 448 0 1 0 0 448a448 448 0 0 0 716.288 358.784l198.4 198.4a64 64 0 1 0 90.624-90.432zM384 384H256v128h128v128h128V512h128V384H512V256H384v128z m64 384A320 320 0 1 1 448 128a320 320 0 0 1 0 640z" fill="#409EFF" p-id="2336"></path></svg>';
const minIcon =
    '<svg t="1661420224594" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="3238" width="200" height="200"><path d="M1005.312 914.752l-198.528-198.464A448 448 0 1 0 0 448a448 448 0 0 0 716.288 358.784l198.4 198.4a64 64 0 1 0 90.624-90.432zM448 767.936A320 320 0 1 1 448 128a320 320 0 0 1 0 640zM256 384v128h384V384H256z" fill="#409EFF" p-id="3239"></path></svg>';
const fullScreenIcon =
    '<svg t="1661420254009" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="4177" width="200" height="200"><path d="M213.333333 213.333333h213.333334V128H170.666667a42.666667 42.666667 0 0 0-42.666667 42.666667v256h85.333333V213.333333zM170.666667 896h256v-85.333333H213.333333v-213.333334H128v256a42.666667 42.666667 0 0 0 42.666667 42.666667z m725.333333-42.666667v-256h-85.333333v213.333334h-213.333334v85.333333h256a42.666667 42.666667 0 0 0 42.666667-42.666667zM597.333333 213.333333h213.333334v213.333334h85.333333V170.666667a42.666667 42.666667 0 0 0-42.666667-42.666667h-256v85.333333z" fill="#409EFF" p-id="4178"></path></svg>';


const dark=document.documentElement.className.indexOf("dark")>-1;
export function getVarColor(p:string){
    if(p=='black'){
        if(dark){
            return '#ffffff'
        }else{
            return '#000000'
        }
    }else if(p=='white'){
        if(dark){
            return '#000000'
        }else{
            return '#ffffff'
        }
    }else
        return getComputedStyle(document.documentElement).getPropertyValue(p).trim()
}

interface SwitchItem {
  rect: Rect;
  points: number[];
  label: string;
  freq: () => Freq;
  getEnable: () => boolean;
}


enum FreqUnit {
  MHz = 'MHz',
  KHz = 'KHz',
  Hz = 'Hz'
}

interface Freq {
  value: number,
  unit: FreqUnit,
  raw: number
}

interface fircData {
  enable: boolean,
  dsEnable: boolean,
  freq: Freq,
}


interface fxoscData {
  enable: boolean,
  dsEnable: boolean,
  freq: Freq,
  gain: number,
  mode: number
}
interface sircData {
  enable: boolean,
  dsEnable: boolean,
  stbEnable: boolean,
  pdEnable: boolean,
  freq: Freq
}
interface sxoscData {
  enable: boolean,
  dsEnable: boolean,
  pdEnable: boolean,
  stbEnable: boolean,
  freq: Freq,
  gain: number,
  mode: number
}

interface coreClockData {
  div: number,
  ref: number,
  flash: number,
  freq: Freq,
}

interface fastClockData {
  div: number,
  freq: Freq,
}
interface slowClockData {
  div: number,
  freq: Freq,
}
interface pllData {
  enable: boolean,
  refdivV: number,
  fbdivV: number,
  ref: number,
  freq: Freq,
  dsEnable: boolean,
}
interface cmuData {
  enable: boolean,
  reset: boolean,
  ref: number,
  compareHigh: number,
  compareLow: number,
}

interface clkOutData {
  enable: boolean,
  div: number,
  ref: number,
}




function roundFun(value: number, n = 3) {
  return Math.round(value * Math.pow(10, n)) / Math.pow(10, n);
}
function freqToString(val: Freq): string {
  return roundFun(val.value) + val.unit
}
function freqToNumber(val: Freq): number {
  if (val.unit == FreqUnit.MHz) {
    return val.value * 1000000
  } else if (val.unit == FreqUnit.KHz) {
    return val.value * 1000
  } else {
    return val.value
  }
}

function setFreqRaw(val: Freq) {
  val.raw = freqToNumber(val)
}

function numberToFreq(val: number): Freq {
  if (val >= 1000000) {
    return {
      value: val / 1000000,
      unit: FreqUnit.MHz,
      raw: val,
    }
  } else if (val >= 1000) {
    return {
      value: val / 1000,
      unit: FreqUnit.KHz,
      raw: val,
    }
  } else {
    return {
      value: val,
      unit: FreqUnit.Hz,
      raw: val,
    }
  }
}


class Rect {
  parent: Konva.Group;
  rect: Konva.Rect;
  group: Konva.Group;
  text: Konva.Text;
  height = 50;
  width = 100;
  x: number;
  y: number;
  label: string;
  color: string;
  featureChange: (() => void)[] = [];
  constructor(
    parent: Konva.Group,
    x: number,
    y: number,
    label: string,
    color: string,
    width = 100,
  ) {
    this.label = label;
    this.color = color;
    this.x = x;
    this.y = y;
    this.width = width;
    this.parent = parent;
    this.group = new Konva.Group({
      x: x,
      y: y,
    });
    this.text = new Konva.Text({
      text: label,
      fontSize: 20,
      padding: (this.height - 20) / 2,
      fontFamily: "Calibri",
      fill: getVarColor('--el-text-color-primary'),
      x: 0,
      y: 0,
      width: this.width,
      align: "center",
    });
    this.rect = new Konva.Rect({
      x: 0,
      y: 0,
      height: this.height,
      width: this.width,
      stroke: this.color,
      strokeWidth: 2,
      fill: "",
      opacity: 0.3,
    });
    this.rect.on("mouseenter", (evt) => {
      const stage = evt.target.getStage();
      if (stage != null) {
        stage.container().style.cursor = "pointer";
        this.rect.setAttrs({ 'strokeWidth': 4, opacity: 0.6 })
      }
    });
    this.rect.on("mouseleave", (evt) => {
      const stage = evt.target.getStage();
      if (stage != null) {
        stage.container().style.cursor = "default";
        this.rect.setAttrs({ 'strokeWidth': 2, opacity: 0.3 })
      }
    });
    this.group.add(this.text);
    this.group.add(this.rect);
    this.parent.add(this.group);
  }
  getLeftPos() {
    return {
      x: this.x,
      y: this.y + this.height / 2,
    };
  }
  getRightPos() {
    return {
      x: this.x + this.width,
      y: this.y + this.height / 2,
    };
  }
  addFeatureChange(func: () => void) {
    const index = this.featureChange.indexOf(func)
    if (index == -1) {
      this.featureChange.push(func)
    }
  }
  removeFeatureChange(func: () => void) {
    const index = this.featureChange.indexOf(func)
    if (index > -1) {
      this.featureChange.splice(index, 1)
    }
  }
}

class FIRC extends Rect {
  color = getVarColor('--el-color-success');
  freqText: Konva.Text;
  data: fircData;
  cmu: CMU;
  constructor(
    parent: Konva.Group,
    x: number,
    y: number,
    cmu: CMU,
    data: fircData,
  ) {
    super(parent, x, y, "FIRC", getVarColor('--el-color-success'));
    this.data = data;
    this.cmu = cmu;
    this.freqText = new Konva.Text({
      text: freqToString(this.data.freq),
      fontSize: freqFontSize,
      padding: 4,
      fontFamily: "Calibri",
      fill: getVarColor('--el-text-color-primary'),
      x: this.width,
      y: 0,
    });
    this.group.add(this.freqText);

    this.rect.on("click", (evt) => {
      this.openEdit();
    });

  }
  openEdit() {
    ElMessageBox({
      title: `${this.label} Configuration`,
      message: h(Form, {
        data: {
          freq: this.data.freq.value,
          enable: this.data.enable,
          dsEnable: this.data.dsEnable,
        },
        config: [
          {
            type: "number",
            label: `Frequency`,
            prop: "freq",
            unit: this.data.freq.unit,
            disable: true,
          },
          {
            type: "switch",
            label: `Enable`,
            prop: "enable",
            disable: false,
          },
          {
            type: "switch",
            label: `DeepSleep Enable`,
            prop: "dsEnable",
            disable: false,
          },
        ],
        rules: {},
        onChange: (val: any) => {
          // direction = val;
          this.data.enable = val.enable;
          this.data.dsEnable = val.dsEnable;

        },
      }),
      showCancelButton: false,
      showConfirmButton: false,
    }).catch(() => {
      null;
    });
  }
  getFreq() {
    return JSON.parse(JSON.stringify(this.data.freq));
  }
  getEnable() {
    return this.data.enable
  }
  errorCheck() {
    if (!this.data.enable) {
      this.rect.setAttr("fill", "black");
      this.rect.setAttr("stroke", getVarColor('--el-color-info-dark-2'));
    } else {
      this.rect.setAttr("fill", "");
      this.rect.setAttr("stroke", this.color);
    }
    for (const i in this.featureChange) {
      this.featureChange[i]()
    }
    this.cmu.errorCheck()
  }
}

class FXOSC extends Rect {
  color = getVarColor('--el-color-primary');
  freqText: Konva.Text;
  data: fxoscData;
  cmu: CMU
  constructor(
    parent: Konva.Group,
    x: number,
    y: number,
    cmu: CMU,
    data: fxoscData
  ) {
    super(parent, x, y, "FXOSC", getVarColor('--el-color-primary'));
    this.data = data;
    this.cmu = cmu;
    setFreqRaw(this.data.freq);
    this.group.add(
      new Konva.Line({
        points: [0, 5, -30, 5, -30, 15, -45, 15, -15, 15],
        stroke: getVarColor('--el-text-color-primary'),
        strokeWidth: 2,
        lineJoin: "round",
      })
    );
    this.group.add(
      new Konva.Line({
        points: [0, 45, -30, 45, -30, 35, -45, 35, -15, 35],
        stroke: getVarColor('--el-text-color-primary'),
        strokeWidth: 2,
        lineJoin: "round",
      })
    );
    this.group.add(
      new Konva.Rect({
        x: -45,
        y: 20,
        height: 10,
        width: 30,
        fill: getVarColor('--el-text-color-primary'),
      })
    );
    this.freqText = new Konva.Text({
      text: freqToString(this.data.freq),
      fontSize: freqFontSize,
      padding: 4,
      fontFamily: "Calibri",
      fill: this.color,
      x: this.width,
      y: 0,
    });
    this.group.add(this.freqText);

    this.rect.on("click", (evt) => {
      this.openEdit();
    });
    
  }
  openEdit() {
    const configs = ref([
      {
        type: "number",
        label: `Frequency`,
        prop: "freq",
        unit: this.data.freq.unit,
        disable: false,
      },
      {
        type: "switch",
        label: `Enable`,
        prop: "enable",
        disable: false,
      },
      {
        type: "select",
        label: `Mode`,
        prop: "mode",
        disable: !this.data.enable,
        opts: [
          {
            value: 0,
            label: "Crystal Mode",
          },
          {
            value: 1,
            label: "Bypass Mode",
          },
        ],
      },
      {
        type: "number",
        label: `Gain`,
        prop: "gain",
        disable: !this.data.enable,
        unit: ''
      },
      {
        type: "switch",
        label: `DeepSleep Enable`,
        prop: "dsEnable",
        disable: !this.data.enable,
      },
    ]);
    ElMessageBox({
      title: `${this.label} Configuration`,
      message: h(Form, {
        data: {
          freq: this.data.freq.value,
          enable: this.data.enable,
          mode: this.data.mode,
          gain: this.data.gain,
          dsEnable: this.data.dsEnable,
        },
        config: configs,
        rules: {
          freq: [
            {
              type: "number",
              min: 4,
              max: 40,
              message: `Only support 4-40${this.data.freq.unit} crystal`,
              trigger: "change",
            },
            {
              required: true,
              message: "Please input a number",
              trigger: "change",
            },
          ],
          gain:[
            {
              type: "number",
              min: 0,
              max: 15,
              message: `Gain from 0-15`,
              trigger: "change",
            },
            {
              type: "number",
              required: true,
              message: "Please input a number",
              trigger: "change",
            },
          ]
        },
        onChange: (val: any) => {
          // direction = val;
          this.data.enable = val.enable;
          this.data.mode = val.mode;
          this.data.freq.value = val.freq;
          setFreqRaw(this.data.freq)
          this.data.gain = val.gain;
          this.data.dsEnable = val.dsEnable
          this.freqText.text(freqToString(this.data.freq));
          configs.value[2].disable = !this.data.enable;
          configs.value[3].disable = !this.data.enable;
          configs.value[4].disable = !this.data.enable;
          this.errorCheck()
        },
      }),
      showCancelButton: false,
      showConfirmButton: false,
    }).catch(() => {
      null;
    });
  }
  getFreq() {
    return JSON.parse(JSON.stringify(this.data.freq));
  }
  getEnable() {
    return this.data.enable
  }
  errorCheck() {
    if (!this.data.enable) {
      this.rect.setAttr("fill", "black");
      this.rect.setAttr("stroke", getVarColor('--el-color-info-dark-2'));
    } else {
      this.rect.setAttr("fill", "");
      this.rect.setAttr("stroke", this.color);
    }
    for (const i in this.featureChange) {
      this.featureChange[i]()
    }

    this.cmu.errorCheck()
  }
}

class TRNG  extends Rect{
  freq: Freq = {
    value: 12,
    unit: FreqUnit.MHz,
    raw: 96000000
  };
  color = "#67C23A";
  constructor(parent: Konva.Group,
    x: number,
    y: number,) {
      super(parent, x, y, "TRNG", "#b88230");
  }
  getFreq() {
    return JSON.parse(JSON.stringify(this.freq));
  }
  getEnable() {
    return true
  }
}
class SIRC extends Rect {
  color = "#fab6b6";
  freqText: Konva.Text;
  data: sircData
  constructor(parent: Konva.Group, x: number, y: number, data: sircData) {
    super(parent, x, y, "SIRC", "#fab6b6");
    this.data = data;
    this.freqText = new Konva.Text({
      text: freqToString(this.data.freq),
      fontSize: freqFontSize,
      padding: 4,
      fontFamily: "Calibri",
      fill: getVarColor('--el-text-color-primary'),
      x: this.width,
      y: 0,
    });
    this.group.add(this.freqText);
    this.rect.on("click", (evt) => {
      this.openEdit();
    });
    this.errorCheck()
  }
  openEdit() {
    ElMessageBox({
      title: `${this.label} Configuration`,
      message: h(Form, {
        data: {
          freq: this.data.freq.value,
          enable: this.data.enable,
          dsEnable: this.data.dsEnable,
          stbEnable: this.data.stbEnable,
          pdEnable: this.data.pdEnable,
        },
        config: [
          {
            type: "number",
            label: `Frequency`,
            prop: "freq",
            unit: this.data.freq.unit,
            disable: true,
          },
          {
            type: "switch",
            label: `Enable`,
            prop: "enable",
            disable: true,
          },
          {
            type: "switch",
            label: `Standby Enable`,
            prop: "stbEnable",
          },
          {
            type: "switch",
            label: `DeepSleep Enable`,
            prop: "dsEnable",
          },
          {
            type: "switch",
            label: `PowerDown  Enable`,
            prop: "pdEnable",
          },
        ],
        rules: {},
        onChange: (val: any) => {
          // direction = val;
          this.data.stbEnable = val.stbEnable;
          this.data.dsEnable = val.dsEnable;
          this.data.pdEnable = val.pdEnable;
          this.errorCheck()
        },
      }),
      showCancelButton: false,
      showConfirmButton: false,
    }).catch(() => {
      null;
    });
  }
  getFreq() {
    return JSON.parse(JSON.stringify(this.data.freq));
  }
  getEnable() {
    return this.data.enable
  }
  errorCheck() {
    if (!this.data.enable) {
      this.rect.setAttr("fill", "black");
      this.rect.setAttr("stroke", " #73767a");
    } else {
      this.rect.setAttr("fill", "");
      this.rect.setAttr("stroke", this.color);
    }
  }
}


class CoreClock extends Rect {
  color = getVarColor('--el-text-color-primary');
  freqText: Konva.Text;
  sw: Switcher;
  min = 0;
  max = 120000000;
  err = false;
  data: coreClockData;
  constructor(
    parent: Konva.Group,
    x: number,
    y: number,
    sw: Switcher,
    data: coreClockData
  ) {
    super(parent, x, y, "Core/PlatClock DIV", getVarColor('--el-text-color-primary'), 200);
    this.data = data;
    this.sw = sw;
    this.data.freq = this.sw.labels[this.sw.pos].freq();
    this.group.add(
      new Konva.Arrow({
        x: 0,
        y: 0,
        points: [-170, 65, -100, 65, -100, 25, 0, 25],
        pointerLength: 5,
        pointerWidth: 5,
        fill: "#909399",
        stroke: "#909399",
        strokeWidth: 3,
      })
    );
    this.freqText = new Konva.Text({
      text: freqToString(this.data.freq),
      fontSize: freqFontSize,
      padding: 4,
      fontFamily: "Calibri",
      fill: getVarColor('--el-text-color-primary'),
      x: this.width,
      y: 0,
    });
    this.group.add(this.freqText);
    this.rect.on("click", (evt) => {
      this.openEdit();
    });
    for (const i in this.sw.labels) {
      this.sw.labels[i].rect.addFeatureChange(this.errorCheck.bind(this))
    }
    this.sw.addFeatureChange(this.errorCheck.bind(this))
    this.errorCheck();
  }
  openEdit() {
    this.data.freq = this.sw.labels[this.sw.pos].freq();
    const divOpts: { value: number, label: string }[] = [];
    for (let i = 0; i < 16; i++) {
      divOpts.push({
        value: i,
        label: `/${i + 1}`,
      });
    }
    const configs = ref([
      {
        type: "input",
        label: `Input Ref Frequency`,
        prop: "inputFreq",
        disable: true,
      },
      {
        type: "select",
        label: `Divider`,
        prop: "div",
        opts: divOpts,
      },
    ]);
    ElMessageBox({
      title: `${this.label} Configuration`,
      message: h(Form, {
        width: 200,
        data: {
          inputFreq: this.sw.labels[this.sw.pos].label + ':' + freqToString(this.data.freq),
          div: this.data.div,
        },
        config: configs,
        rules: {},
        onChange: (val: any) => {
          // direction = val;
          this.data.div = val.div;
          this.errorCheck()
        },
      }),
      showCancelButton: false,
      showConfirmButton: false,
    }).catch(() => {
      null;
    });
  }
  errorCheck() {
    this.data.freq.unit = this.sw.labels[this.sw.pos].freq().unit
    this.data.freq.value = this.sw.labels[this.sw.pos].freq().value / (this.data.div + 1);
    setFreqRaw(this.data.freq)
    this.freqText.text(freqToString(this.data.freq));
    if (freqToNumber(this.data.freq) > this.max || freqToNumber(this.data.freq) < this.min) {
      this.err = true;

      this.rect.setAttr("fill", "red");
      plugin.addLog('error', `CoreClock`,
        `${this.label} clock output frequency range ${freqToString(numberToFreq(this.min))}-${freqToString(numberToFreq(this.max))}`
      );
    } else {
      this.err = false;
      plugin.deleteLog('error', 'CoreClock')
      this.rect.setAttr("fill", "");
    }
    /* Change RWS by core clock freq */
    if(freqToNumber(this.data.freq) <= 40000000){
      this.data.flash = 0
    }else if (freqToNumber(this.data.freq) <= 80000000){
      this.data.flash = 1
    }else if (freqToNumber(this.data.freq) <= 120000000){
      this.data.flash = 2
    }else{
      this.data.flash = 3
    }
    this.data.flash += 1
    for (const i in this.featureChange) {
      this.featureChange[i]()
    }
  }
  getFreq() {
    return JSON.parse(JSON.stringify(this.data.freq));
  }
  getEnable() {
    return true;
  }
}

class FastBus extends Rect {
  color = getVarColor('--el-text-color-primary');
  freqText: Konva.Text;
  coreClock: CoreClock;
  min = 0;
  max = 120000000;
  err = false;
  data: fastClockData
  constructor(
    parent: Konva.Group,
    x: number,
    y: number,
    coreClock: CoreClock,
    data: fastClockData
  ) {
    super(parent, x, y, "FastBus Clock DIV", getVarColor('--el-text-color-primary'), 200);
    this.coreClock = coreClock;
    this.data = data
    this.data.freq = this.coreClock.getFreq();
    this.group.add(
      new Konva.Arrow({
        x: 0,
        y: 0,
        points: [200, -75, 250, -75, 250, -25, -50, -25, -50, 25, 0, 25],
        pointerLength: 5,
        pointerWidth: 5,
        fill: "#909399",
        stroke: "#909399",
        strokeWidth: 3,
      })
    );
    this.freqText = new Konva.Text({
      text: freqToString(this.data.freq),
      fontSize: freqFontSize,
      padding: 4,
      fontFamily: "Calibri",
      fill: getVarColor('--el-text-color-primary'),
      x: this.width,
      y: 0,
    });
    this.group.add(this.freqText);
    this.rect.on("click", (evt) => {
      this.openEdit();
    });
    this.coreClock.addFeatureChange(this.errorCheck.bind(this))
    this.errorCheck();
  }
  openEdit() {
    const divOpts: { value: number, label: string }[] = [];
    for (let i = 0; i < 16; i++) {
      divOpts.push({
        value: i,
        label: `/${i + 1}`,
      });
    }
    const configs = ref([
      {
        type: "input",
        label: `Input Ref Frequency`,
        prop: "inputFreq",
        disable: true,
      },
      {
        type: "select",
        label: `Divider`,
        prop: "div",
        opts: divOpts,
      },
    ]);
    ElMessageBox({
      title: `${this.label} Configuration`,
      message: h(Form, {
        width: 200,
        data: {
          inputFreq: 'CoreClock:' + freqToString(this.coreClock.getFreq()),
          div: this.data.div,
        },
        config: configs,
        rules: {},
        onChange: (val: any) => {
          // direction = val;
          this.data.div = val.div;
          this.errorCheck()
        },
      }),
      showCancelButton: false,
      showConfirmButton: false,
    }).catch(() => {
      null;
    });
  }
  errorCheck() {
    this.data.freq.unit = this.coreClock.getFreq().unit
    this.data.freq.value = this.coreClock.getFreq().value / (this.data.div + 1);
    setFreqRaw(this.data.freq)
    this.freqText.text(freqToString(this.data.freq));
    if (freqToNumber(this.data.freq) > this.max || freqToNumber(this.data.freq) < this.min) {
      this.err = true;

      this.rect.setAttr("fill", "red");
      plugin.addLog('error', 'FastBus',
        `${this.label} clock output frequency range ${freqToString(numberToFreq(this.min))}-${freqToString(numberToFreq(this.max))}`
      );

    } else {
      this.err = false;
      plugin.deleteLog('error', 'FastBus');
      this.rect.setAttr("fill", "");
    }
    for (const i in this.featureChange) {
      this.featureChange[i]()
    }
  }
  getFreq() {
    return JSON.parse(JSON.stringify(this.data.freq));
  }
  getEnable() {
    return true
  }
}

class SlowBus extends Rect {
  color = getVarColor('--el-text-color-primary');
  freqText: Konva.Text;
  fastClock: FastBus;
  min = 0;
  max = 40000000;
  err = false;
  data: slowClockData
  cmu: CMU
  constructor(
    parent: Konva.Group,
    x: number,
    y: number,
    fastClock: FastBus,
    cmu:CMU,
    data: slowClockData
  ) {
    super(parent, x, y, "SlowBus Clock DIV", getVarColor('--el-text-color-primary'), 200);
    this.fastClock = fastClock;
    this.data = data;
    this.data.freq = this.fastClock.getFreq();
    this.cmu = cmu
    this.group.add(
      new Konva.Arrow({
        x: 0,
        y: 0,
        points: [200, -75, 250, -75, 250, -25, -50, -25, -50, 25, 0, 25],
        pointerLength: 5,
        pointerWidth: 5,
        fill: "#909399",
        stroke: "#909399",
        strokeWidth: 3,
      })
    );
    this.freqText = new Konva.Text({
      text: freqToString(this.data.freq),
      fontSize: freqFontSize,
      padding: 4,
      fontFamily: "Calibri",
      fill: getVarColor('--el-text-color-primary'),
      x: this.width,
      y: 0,
    });
    this.group.add(this.freqText);
    this.rect.on("click", (evt) => {
      this.openEdit();
    });
    this.fastClock.addFeatureChange(this.errorCheck.bind(this))
    // this.errorCheck();
  }
  openEdit() {
    const divOpts: { value: number, label: string }[] = [];
    for (let i = 0; i < 16; i++) {
      divOpts.push({
        value: i,
        label: `/${i + 1}`,
      });
    }
    const configs = ref([
      {
        type: "input",
        label: `Input Ref Frequency`,
        prop: "inputFreq",
        disable: true,
      },
      {
        type: "select",
        label: `Divider`,
        prop: "div",
        opts: divOpts,
      },
    ]);
    ElMessageBox({
      title: `${this.label} Configuration`,
      message: h(Form, {
        width: 200,
        data: {
          inputFreq: 'FastClock:' + freqToString(this.fastClock.getFreq()),
          div: this.data.div,
        },
        config: configs,
        rules: {},
        onChange: (val: any) => {
          // direction = val;
          this.data.div = val.div;
          this.errorCheck()
        },
      }),
      showCancelButton: false,
      showConfirmButton: false,
    }).catch(() => {
      null;
    });
  }
  errorCheck() {
    this.data.freq.unit = this.fastClock.getFreq().unit
    this.data.freq.value = this.fastClock.getFreq().value / (this.data.div + 1);
    setFreqRaw(this.data.freq)
    this.freqText.text(freqToString(this.data.freq));
    if (freqToNumber(this.data.freq) > this.max || freqToNumber(this.data.freq) < this.min) {
      this.err = true;
      this.rect.setAttr("fill", "red");
      plugin.addLog('error', 'SlowBus',
        `${this.label} clock output frequency range ${freqToString(numberToFreq(this.min))}-${freqToString(numberToFreq(this.max))}`
      );
    } else {
      plugin.deleteLog('error', 'SlowBus');
      this.err = false;
      this.rect.setAttr("fill", "");
    }
    for (const i in this.featureChange) {
      this.featureChange[i]()
    }
    this.cmu.errorCheck()
  }
  getFreq() {
    return JSON.parse(JSON.stringify(this.data.freq));
  }
  getEnable() {
    return true
  }
}

class PLL {
  color = getVarColor('--el-text-color-primary');
  min = 100000000;
  max = 200000000;
  parent: Konva.Group;
  group: Konva.Group;
  refdiv: Rect;
  rect: Konva.Rect;
  pd: Rect;
  fbdiv: Rect;
  vco: Rect;
  div2: Rect;
  x: number;
  y: number;
  cmu: CMU;
  refdivVText: Konva.Text;
  fbdivVText: Konva.Text;
  data: pllData;
  outputFreqText: Konva.Text;
  pllInput: Switcher;
  err = false;
  constructor(
    parent: Konva.Group,
    x: number,
    y: number,
    pllInput: Switcher,
    cmu: CMU,
    data: pllData
  ) {
    this.x = x;
    this.y = y;
    this.data = data;
    this.parent = parent;
    this.cmu = cmu;
    this.pllInput = pllInput;
    this.data.freq = this.pllInput.labels[this.pllInput.pos].freq()
    this.group = new Konva.Group({
      x: x,
      y: y,
    });

    this.refdiv = new Rect(this.group, 0, 0, "REFDIV", this.color, 110);
    this.refdivVText = new Konva.Text({
      text: `÷${this.data.refdivV}`,
      fontSize: 20,
      padding: 4,
      fontFamily: "Calibri",
      fill: getVarColor('--el-text-color-primary'),
      x: 40,
      y: 50,
    });
    this.group.add(this.refdivVText);
    this.vco = new Rect(this.group, 350, 0, "VCO", this.color, 80);
    this.div2 = new Rect(this.group, 470, 30, "÷2", this.color, 60);
    this.pd = new Rect(
      this.group,
      150,
      0,
      "PhaseDetector",
      this.color,
      160
    );
    this.fbdivVText = new Konva.Text({
      text: `x${this.data.fbdivV}`,
      fontSize: 20,
      padding: 4,
      fontFamily: "Calibri",
      fill: getVarColor('--el-text-color-primary'),
      x: 200,
      y: 110,
    });
    this.group.add(this.fbdivVText);
    this.fbdiv = new Rect(
      this.group,
      150,
      60,
      "FeadBack DIV",
      this.color,
      160
    );
    this.group.add(
      new Konva.Arrow({
        x: this.refdiv.getLeftPos().x,
        y: this.refdiv.getLeftPos().y,
        points: [-70, 0, -2, 0],
        pointerLength: 5,
        pointerWidth: 5,
        fill: "#909399",
        stroke: "#909399",
        strokeWidth: 3,
      })
    );
    this.group.add(
      new Konva.Arrow({
        x: this.refdiv.getRightPos().x,
        y: this.refdiv.getRightPos().y,
        points: [0, 0, 38, 0],
        pointerLength: 5,
        pointerWidth: 5,
        fill: "#909399",
        stroke: "#909399",
        strokeWidth: 3,
      })
    );
    this.group.add(
      new Konva.Arrow({
        x: this.refdiv.getRightPos().x,
        y: this.refdiv.getRightPos().y,
        points: [0, 0, 25, 0, 25, 60, 38, 60],
        pointerLength: 5,
        pointerWidth: 5,
        fill: "#909399",
        stroke: "#909399",
        strokeWidth: 3,
      })
    );
    this.group.add(
      new Konva.Arrow({
        x: this.pd.getRightPos().x,
        y: this.pd.getRightPos().y,
        points: [0, 0, 38, 0],
        pointerLength: 5,
        pointerWidth: 5,
        fill: "#909399",
        stroke: "#909399",
        strokeWidth: 3,
      })
    );
    this.group.add(
      new Konva.Arrow({
        x: this.div2.getLeftPos().x,
        y: this.div2.getLeftPos().y,
        points: [0, 0, -20, 0, -20, -30, -38, -30],
        pointerLength: 5,
        pointerWidth: 5,
        fill: "#909399",
        stroke: "#909399",
        strokeWidth: 3,
      })
    );
    this.group.add(
      new Konva.Arrow({
        x: this.div2.getLeftPos().x,
        y: this.div2.getLeftPos().y,
        points: [0, 0, -20, 0, -20, 30, -158, 30],
        pointerLength: 5,
        pointerWidth: 5,
        fill: "#909399",
        stroke: "#909399",
        strokeWidth: 3,
      })
    );

    this.rect = new Konva.Rect({
      x: -30,
      y: -50,
      width: 630,
      height: 200,
      dash: [2, 2],
      stroke: this.color,
      strokeWidth: 3,
      opacity: 0.3,
    });
    this.rect.on("click", () => {
      this.openEdit();
    });

    this.group.add(
      new Konva.Line({
        points: [
          this.div2.getRightPos().x,
          this.div2.getRightPos().y,
          this.div2.getRightPos().x + 70,
          this.div2.getRightPos().y,
        ],
        stroke: "#909399",
        strokeWidth: 3,
        lineJoin: "round",
      })
    );
    this.outputFreqText = new Konva.Text({
      text: freqToString(this.data.freq),
      fontSize: 14,
      padding: 4,
      fontFamily: "Calibri",
      fill: getVarColor('--el-text-color-primary'),
      x: this.div2.getRightPos().x,
      y: this.div2.getRightPos().y - 20,
    });
    this.group.add(this.outputFreqText);
    this.group.add(this.rect);

    this.rect.on("mouseenter", (evt) => {
      const stage = evt.target.getStage();
      if (stage != null) {
        stage.container().style.cursor = "pointer";
        this.rect.setAttrs({ 'strokeWidth': 4, opacity: 0.6 })
      }
    });
    this.rect.on("mouseleave", (evt) => {
      const stage = evt.target.getStage();
      if (stage != null) {
        stage.container().style.cursor = "default";
        this.rect.setAttrs({ 'strokeWidth': 2, opacity: 0.3 })
      }
    });

    for (const i in this.pllInput.labels) {
      this.pllInput.labels[i].rect.addFeatureChange(this.errorCheck.bind(this))
    }
    this.pllInput.addFeatureChange(this.errorCheck.bind(this))

    // this.errorCheck();
    this.parent.add(this.group);
  }
  openEdit() {
    const configs = ref([
      {
        type: "input",
        label: `Input Ref Frequency`,
        prop: "inputFreq",
        disable: true,
      },
      {
        type: "switch",
        label: `Enable`,
        prop: "enable",
      },
      {
        type: "switch",
        label: `PLL Deepsleep Enable`,
        prop: "pllEnable",
      },
      {
        type: "number",
        label: `Reference Clock Divider Value`,
        prop: "refdivV",
        unit: "",
        disable: false,
      },
      {
        type: "number",
        label: `Feedback Divider Value`,
        prop: "fbdivV",
        unit: "",
        disable: false,
      },
    ]);

    const input = `${this.pllInput.labels[this.pllInput.pos].label} ${freqToString(this.pllInput.labels[this.pllInput.pos].freq())}`;
    ElMessageBox({
      title: `PLL Configuration`,
      message: h(Form, {
        width: 250,
        data: {
          inputFreq: input,
          enable: this.data.enable,
          dsEnable: this.data.dsEnable,
          refdivV: this.data.refdivV,
          fbdivV: this.data.fbdivV,
        },
        config: configs,
        rules: {
          refdivV: {
            validator: (rule, value, callback) => {
                if (value >= 1 && value <= 16 && value !== 2) {
                  callback(); // validation passed
                } else {
                  callback(new Error("1-16 except 2")); // validation failed
                }
              },
            trigger: "change",
          },
          fbdivV: {
            type: "number",
            min: 10,
            max: 63,
            message: "Value from x10 to x63",
            trigger: "change",
          },
        },
        onChange: (val: any) => {
          // direction = val;
          this.data.enable = val.enable;
          this.data.fbdivV = val.fbdivV;
          this.data.refdivV = val.refdivV;
          this.data.dsEnable = val.dsEnable;
          configs.value[3].disable = !this.data.enable;
          configs.value[4].disable = !this.data.enable;

          if (!this.data.enable) {
            this.rect.setAttr("fill", "black");
          } else {
            if (this.err) {
              this.rect.setAttr("fill", "red");
            } else {
              this.rect.setAttr("fill", "");
            }
          }
          this.errorCheck();
        },
      }),
      showCancelButton: false,
      showConfirmButton: false,
    }).catch(() => {
      null;
    });
  }
  errorCheck() {
    this.fbdivVText.text(`x${this.data.fbdivV}`);
    this.refdivVText.text(`÷${this.data.refdivV}`);
    const input = this.pllInput.labels[this.pllInput.pos].freq();
    this.data.freq.unit = input.unit;
    this.data.freq.value = (input.value / (2 * this.data.refdivV)) * this.data.fbdivV;
    setFreqRaw(this.data.freq)
    this.outputFreqText.text(freqToString(this.data.freq));
    if (this.data.enable == false) {
      plugin.deleteLog('error', 'PLL');
      this.rect.setAttr("fill", "black");
      this.rect.setAttr("stroke", " #73767a"); 
    } else {
      /* finput*/
      const finput=input.value/this.data.refdivV
      this.err=false;
      if(finput>=8&&finput<=48){
        plugin.deleteLog('error', 'PLL-Finput');
      }else{
        this.err=true
        plugin.addLog('error', 'PLL-Finput','PLL-Finput from 8-48MHz');
      }

      if (freqToNumber(this.data.freq) > this.max || freqToNumber(this.data.freq) < this.min) {
        this.err = true;
        plugin.addLog('error', 'PLL', `PLL output frequency range ${freqToString(numberToFreq(this.min))}-${freqToString(numberToFreq(this.max))}`);
      } else {
        plugin.deleteLog('error', 'PLL');
      }
      if(this.err){
        this.rect.setAttr("fill", "red");
      }else{
        this.rect.setAttr("fill", "");
      }
    }
    for (const i in this.div2.featureChange) {
      this.div2.featureChange[i]()
    }
    // console.log(this.cmu)
    this.cmu.errorCheck();
  }
  getFreq() {
    return JSON.parse(JSON.stringify(this.data.freq));
  }
  getEnable() {
    return this.data.enable
  }
}


class Switcher {
  parent: Konva.Group;
  rect: Konva.Line;
  group: Konva.Group;
  //   text: Konva.Text;
  circle: Konva.Circle;
  pos: number;
  width = 30;
  x: number;
  y: number;
  labels: SwitchItem[];
  err = false;
  label = ''
  featureChange: (() => void)[] = [];
  posChange: (pos: number) => void;
  constructor(
    parent: Konva.Group,
    x: number,
    y: number,
    label: string,
    initPos: number,
    inputlabel: SwitchItem[],
    posChange: (pos: number) => void,
    wire = true
  ) {
    this.parent = parent;
    this.label = label;
    this.pos = initPos;
    this.x = x;
    this.y = y;
    this.labels = inputlabel;
    this.group = new Konva.Group({
      x: x,
      y: y,
    });
    this.posChange = posChange
    this.rect = new Konva.Line({
      points: [
        0,
        0,
        this.width,
        this.width,
        this.width,
        this.width + inputlabel.length * this.width,
        0,
        2 * this.width + inputlabel.length * this.width,
      ],
      fill: getVarColor('--el-color-primary-light-9'),
      stroke: getVarColor('--el-color-info-light-7'),
      strokeWidth: 2,
      closed: true,
    });
    this.group.add(this.rect);
    for (let i = 0; i < inputlabel.length; i++) {
      const label = inputlabel[i].label;
      const text = new Konva.Text({
        text: label,
        fontSize: 14,
        padding: 5,
        fontFamily: "Calibri",
        fill: getVarColor('--el-text-color-primary'),
        x: 0,
        y: 0,
      });
      text.setPosition({
        x: -text.width(),
        y: this.width + this.width * i - 8,
      });
      this.group.add(text);

      const opt = new Konva.Circle({
        x: this.width / 2,
        y: this.width + i * this.width + this.width / 2,
        radius: (this.width - 10) / 2,
        fill: getVarColor('white'),
        stroke: getVarColor('--el-color-primary-dark-2'),
        strokeWidth: 2,
      });
      opt.on("mouseenter", (evt) => {
        const stage = evt.target.getStage();
        if (stage != null) {
          if (inputlabel[i].getEnable() == true) {
            stage.container().style.cursor = "pointer";
          }
        }
      });
      opt.on("mouseleave", (evt) => {
        const stage = evt.target.getStage();
        if (stage != null) {
          stage.container().style.cursor = "default";
        }
      });
      opt.on("click", (evt) => {
        if (inputlabel[i].getEnable() == true) {

          if (i != this.pos) {
            this.pos = i;
            this.labels[i].rect.removeFeatureChange(this.errorCheck.bind(this))
            this.labels[this.pos].rect.addFeatureChange(this.errorCheck.bind(this))
            this.circle.setPosition({
              x: this.width / 2,
              y: this.width + this.pos * this.width + this.width / 2,
            });
            this.errorCheck()
            this.posChange(this.pos)
          }
        }

      });
      this.labels[this.pos].rect.addFeatureChange(this.errorCheck.bind(this))
      this.group.add(opt);
    }
    this.circle = new Konva.Circle({
      x: this.width / 2,
      y: this.width + this.pos * this.width + this.width / 2,
      radius: (this.width - 20) / 2,
      fill: getVarColor('--el-color-primary-dark-2'),
    });
    this.group.add(this.circle);
    this.parent.add(this.group);
    if (wire) {
      for (let i = 0; i < this.labels.length; i++) {
        this.addArrowFromRectToSwitch(this.labels[i]);
      }
    } else {
      for (let i = 0; i < this.labels.length; i++) {
        this.addArrowToSwitch(i);
      }
    }
    this.errorCheck();
  }
  getRightPos() {
    return {
      x: this.x,
      y: this.y + (this.width + this.labels.length * this.width) / 2,
    };
  }
  getLeftPos() {
    const ret: { x: number, y: number }[] = [];
    for (let i = 0; i < this.labels.length; i++) {
      ret.push({
        x: this.x,
        y: this.y + this.width + i * this.width + this.width / 2,
      });
    }
    return ret;
  }
  addArrowFromRectToSwitch(a: SwitchItem) {
    const leftPos = a.rect.getRightPos();
    this.group.add(
      new Konva.Arrow({
        x: leftPos.x - this.x,
        y: leftPos.y - this.y,
        points: a.points,
        pointerLength: 5,
        pointerWidth: 5,
        fill: a.rect.color,
        stroke: a.rect.color,
        strokeWidth: 3,
      })
    );
  }
  addArrowToSwitch(num: number) {
    const len = 50;
    const rightPos = this.getLeftPos()[num];
    this.group.add(
      new Konva.Arrow({
        x: rightPos.x - len - this.x - 5,
        y: rightPos.y - this.y,
        points: [0, 0, len, 0],
        pointerLength: 5,
        pointerWidth: 5,
        fill: this.labels[num].rect.color,
        stroke: this.labels[num].rect.color,
        strokeWidth: 3,
      })
    );
  }
  errorCheck() {
    if (this.labels[this.pos].getEnable() == false) {
      //error
      this.rect.setAttr("fill", "red");
      this.rect.setAttr("opacity", 0.3);
      plugin.addLog('error', this.label, `${this.labels[this.pos].label} has been disabled`);
      this.err = true;
    } else {
      plugin.deleteLog('error', this.label);
      this.rect.setAttr("fill", getVarColor('--el-color-primary-light-9'));
      this.rect.setAttr("opacity", 1);
      this.err = false;
    }
    for (const i in this.featureChange) {
      this.featureChange[i]()
    }
  }
  addFeatureChange(func: () => void) {
    const index = this.featureChange.indexOf(func)
    if (index == -1) {
      this.featureChange.push(func)
    }
  }
  removeFeatureChange(func: () => void) {
    const index = this.featureChange.indexOf(func)
    if (index > -1) {
      this.featureChange.splice(index, 1)
    }
  }
}

class OutPoint {
  parent: Konva.Group;
  x: number;
  y: number;
  sw: SwitchItem;
  rect: Konva.Rect;
  arrow: Konva.Arrow;
  constructor(parent: Konva.Group, x: number, y: number, sw: SwitchItem) {
    this.parent = parent;
    this.x = x;
    this.y = y;
    this.sw = sw;
    this.rect = new Konva.Rect({
      x: this.x,
      y: this.y,
      width: 10,
      height: 10,
      stroke: this.sw.rect.color,
      strokeWidth: 2,
    });

    this.arrow = new Konva.Arrow({
      x: this.x,
      y: this.y,
      points: sw.points,
      pointerLength: 5,
      pointerWidth: 5,
      fill: sw.rect.color,
      stroke: sw.rect.color,
      strokeWidth: 3,
    })
    this.parent.add(this.arrow);
    this.parent.add(
      new Konva.Text({
        text: `${this.sw.label}`,
        fontSize: 14,
        padding: 4,
        fontFamily: "Calibri",
        fill: this.sw.rect.color,
        x: this.x + 10,
        y: this.y - 5,
        align: "center",
      })
    );
    this.parent.add(this.rect);
    // this.arrow.on("mouseenter", (evt) => {
    //   this.arrow.setAttr('strokeWidth', 5)
    // });
    // this.arrow.on("mouseleave", (evt) => {
    //   this.arrow.setAttr('strokeWidth', 3)
    // });
  }

}

class CMU extends Rect {
  color = getVarColor('--el-color-warning');
  input: Switcher | null;
  data: cmuData
  getFreq: () => Freq;
  constructor(parent: Konva.Group, x: number, y: number, input: Switcher | null, label: string, data: cmuData) {
    super(parent, x, y, label, getVarColor('--el-color-warning'), 200);
    this.input = input;
    this.data = data

    new OutPoint(this.group, 1100, 20, {
      rect: this,
      points: [-900, 5, 0, 5],
      label: label,
      freq: () => {
        return {
          value: 0,
          unit: FreqUnit.Hz,
          raw: 0
        }
      },
      getEnable: () => {
        return true
      }
    });
    if (this.input != null) {
      this.group.add(
        new Konva.Arrow({
          x: 0,
          y: 25,
          points: [this.input.getRightPos().x - x + 30, this.input.getRightPos().y - y - 10, (this.input.getRightPos().x - x + 30) / 2, this.input.getRightPos().y - y - 10, (this.input.getRightPos().x - x + 30) / 2, 0, 0, 0],
          pointerLength: 5,
          pointerWidth: 5,
          fill: "#909399",
          stroke: "#909399",
          strokeWidth: 3,
        })
      );
    }
    this.rect.on('click', () => {
      this.openEdit()
    })
    // this.errorCheck()
  }
  setGetFreq(func: () => Freq) {
    this.getFreq = func;
  }
  openEdit() {
    const configs = ref([
      {
        type: "input",
        label: `Reference Clock`,
        prop: "ref",
        disable: true,
      },
      {
        type: "switch",
        label: `Enable`,
        prop: "enable",
      },
      {
        type: "switch",
        label: `Reset Enable`,
        prop: "reset",
        disable: !this.data.enable,
      },
      {
        type: "number",
        label: `Compare High`,
        prop: "compareHigh",
        disable: !this.data.enable,
        unit: ''
      },
      {
        type: "number",
        label: `Compare Low`,
        prop: "compareLow",
        disable: !this.data.enable,
        unit: ''
      },
    ])
    ElMessageBox({
      title: `${this.label} Configuration`,
      message: h(Form, {
        data: {
          ref: this.input == null ? 'SIRC' : this.input.labels[this.input.pos].label,
          enable: this.data.enable,
          reset: this.data.reset,
          compareHigh: this.data.compareHigh,
          compareLow: this.data.compareLow
        },
        width: 200,
        config: configs,
        rules: {
          compareHigh:[
            {
              type: "number",
              min: 0,
              max: 65535,
              message: `Value from 0-65535`,
              trigger: "change",
            },
            {
              type: "number",
              required: true,
              message: "Please input a compare high value",
              trigger: "change",
            },
          ],
          compareLow:[
            {
              type: "number",
              min: 0,
              max: 65535,
              message: `Value from 0-65535`,
              trigger: "change",
            },
            {
              type: "number",
              required: true,
              message: "Please input a compare low value",
              trigger: "change",
            },
          ]
        },
        onChange: (val: any) => {
          this.data.enable = val.enable;
          this.data.reset = val.reset;
          this.data.compareHigh = val.compareHigh;
          this.data.compareLow = val.compareLow
          configs.value[2].disable = !this.data.enable;
          configs.value[3].disable = !this.data.enable;
          configs.value[4].disable = !this.data.enable;
          this.errorCheck()
        },
      }),
      showCancelButton: false,
      showConfirmButton: false,
    }).catch(() => {
      null;
    });
  }
  errorCheck() {
    if (!this.data.enable) {
      this.rect.setAttr("fill", "black");
      this.rect.setAttr("stroke", " #73767a");
      plugin.deleteLog('error', this.label);
    } else {
      
        
      const freq=this.getFreq()
      /* hight should > freq.value & <= freq.value * 1.5 */
      if (this.data.compareHigh <= freq.value || this.data.compareHigh > freq.value * 1.5) {
        plugin.addLog('error', this.label,
          `compare high should > ${freqToString(freq)} & <= ${freqToString(numberToFreq(freq.raw * 1.5))}`
        );
        this.rect.setAttr("fill", 'red');
      }
      /* low should < freq.value & >= freq.value * 0.5  */
      else if (this.data.compareLow >= freq.value || this.data.compareLow < freq.value * 0.5) {
        plugin.addLog('error', this.label,
          `compare low should < ${freqToString(freq)} & >= ${freqToString(numberToFreq(freq.raw * 0.5))}`
        );
        this.rect.setAttr("fill", 'red');
      } else if (this.data.compareHigh <= this.data.compareLow) {
        plugin.addLog('error', this.label,
          `compare low should less than compare high`
        );
        this.rect.setAttr("fill", 'red');
      } else {
        plugin.deleteLog('error', this.label);
        this.rect.setAttr("fill", "");
        this.rect.setAttr("stroke", this.color);
      }

      /* Limit for warning */
      if (this.data.compareHigh > freq.value && this.data.compareHigh < Math.floor(freq.value * 1.1 + 0.99)){
        ElNotification.warning(`Recommended compare high >= ${freqToString(numberToFreq(freq.raw * 1.1))} & <= ${freqToString(numberToFreq(freq.raw * 1.5))}`)
      }
      if (this.data.compareLow < freq.value && this.data.compareLow > freq.value * 0.9){
        ElNotification.warning(`Recommended compare low <= [${freqToString(numberToFreq(freq.raw * 0.9))} & >= ${freqToString(numberToFreq(freq.raw * 0.5))}`)
      }
      
    }
   
    for (const i in this.featureChange) {
      this.featureChange[i]()
    }
  }
}

class CLK_OUT extends Rect {
  color = "#337ecc";
  input: Switcher;
  freq: Freq;
  data: clkOutData;
  err = false;
  min = 0;
  max = 30000000;
  freqText: Konva.Text;
  constructor(parent: Konva.Group, x: number, y: number, input: Switcher, data: clkOutData) {
    super(parent, x, y, "CLK_OUT", "#337ecc", 150);
    this.input = input;
    this.data = data;
    this.freq = this.input.labels[this.input.pos].freq()
    new OutPoint(this.group, 300, 20, {
      rect: this,
      points: [-150, 5, 0, 5],
      label: "SCU_CLKOUT",
      freq: this.getFreq.bind(this),
      getEnable: this.getEnable.bind(this)
    });
    this.group.add(
      new Konva.Arrow({
        x: 0,
        y: 25,
        points: [
          -170, 0,

          0, 0,
        ],
        pointerLength: 5,
        pointerWidth: 5,
        fill: "#909399",
        stroke: "#909399",
        strokeWidth: 3,
      })
    );

    this.freqText = new Konva.Text({
      text: freqToString(this.freq),
      fontSize: freqFontSize,
      padding: 4,
      fontFamily: "Calibri",
      fill: getVarColor('--el-text-color-primary'),
      x: this.width,
      y: 0,
    });
    this.group.add(this.freqText);
    this.rect.on("click", (evt) => {
      this.openEdit();
    });
    for (const i in this.input.labels) {
      this.input.labels[i].rect.addFeatureChange(this.errorCheck.bind(this))
    }
    this.input.addFeatureChange(this.errorCheck.bind(this))
    this.errorCheck();
  }
  openEdit() {
    const divOpts: { value: number, label: string }[] = [];
    for (let i = 2; i < 255; i++) {
      if (i & 0x2) {
        divOpts.push({
          value: i,
          label: `/${i}`,
        });
      }
    }
    const configs = ref([
      {
        type: "number",
        label: `Input Frequency`,
        prop: "freq",
        unit: this.input.labels[this.input.pos].freq().unit,
        disable: true,
      },
      {
        type: "switch",
        label: `Enable`,
        prop: "enable",
      },
      {
        type: "select",
        label: `Divide`,
        prop: "div",
        disable: !this.data.enable,
        opts: divOpts,
      },
    ]);
    ElMessageBox({
      title: `${this.label} Configuration`,
      message: h(Form, {
        width: 200,
        data: {
          freq: this.input.labels[this.input.pos].freq().value,
          enable: this.data.enable,
          div: this.data.div,
        },
        config: configs,
        rules: {
        },
        onChange: (val: any) => {
          // direction = val;
          this.data.enable = val.enable;
          this.data.div = val.div
          configs.value[2].disable = !this.data.enable;
          if (!this.data.enable) {
            this.rect.setAttr("fill", "black");
            this.rect.setAttr("stroke", " #73767a");
          } else {
            this.rect.setAttr("fill", "");
            this.rect.setAttr("stroke", this.color);
          }
          this.errorCheck()
        },
      }),
      showCancelButton: false,
      showConfirmButton: false,
    }).catch(() => {
      null;
    });
  }
  getFreq() {
    return this.freq;
  }
  errorCheck() {
    this.freq.unit = this.input.labels[this.input.pos].freq().unit
    this.freq.value = this.input.labels[this.input.pos].freq().value / this.data.div;
    setFreqRaw(this.freq)
    this.freqText.text(freqToString(this.freq));
    if (!this.data.enable) {
      this.err = false;
      this.rect.setAttr("fill", "black");
      plugin.deleteLog('error', 'CLK_OUT');
    } else {
      if (freqToNumber(this.freq) > this.max || freqToNumber(this.freq) < this.min) {
        this.err = true;
        this.rect.setAttr("fill", "red");
        plugin.addLog('error', 'CLK_OUT',
          `${this.label} clock output frequency range ${freqToString(numberToFreq(this.min))}-${freqToString(numberToFreq(this.max))}`
        );
      } else {
        plugin.deleteLog('error', 'CLK_OUT');
        this.err = false;
        this.rect.setAttr("fill", "");
      }
    }
    for (const i in this.featureChange) {
      this.featureChange[i]()
    }
  }
  getEnable() {
    return this.data.enable
  }
}

export class Clock {
  parent: Konva.Group | Konva.Layer;
  rect: Konva.Rect;
  group: Konva.Group;
  width = 1600;
  height = 1300;
  firc: FIRC;
  fxosc: FXOSC;
  sirc: SIRC;
  pllInput: Switcher;
  sysInput: Switcher;
  pll: PLL;
  coreClock: CoreClock;
  fastBus: FastBus;
  slowBus: SlowBus;
  cmu0Input: Switcher;
  cmu1Input: Switcher;
  cmu2Input: Switcher;
  cmu0: CMU;
  cmu1: CMU;
  cmu2: CMU;
  cmu3: CMU;
  clkOut: CLK_OUT;
  clkoutInput: Switcher;
  fircOutPoint: OutPoint;
  fxoscOutPoint: OutPoint;
  sircOutPoint: OutPoint;
  sxoscOutPoint: OutPoint;
  pllOutPoint: OutPoint;
  coreOutPoint: OutPoint;
  fastOutPoint: OutPoint;
  slowOutPoint: OutPoint;
  trng: TRNG;
  ctrlDown = false;
  scaleValue = 1.2;
  lock = false;
  lockRect: Konva.Rect|undefined
  clockName = '';
  constructor(parent: Konva.Group | Konva.Layer, data: clockData, clockName: string, lock=false) {
    plugin.setIdPrefix(clockName)
    this.clockName=clockName
    this.parent = parent;
    this.lock = lock
    const stage = this.parent.getStage() as Konva.Stage;
    this.group = new Konva.Group({
      x: 0,
      y: 100,
      draggable: true,
    });
    this.rect = new Konva.Rect({
      x: 0,
      y: 0,
      width: this.width,
      height: this.height,
      dash: [2, 2],
      stroke: getVarColor('--el-text-color-primary'),
      strokeWidth: 3,
    });

    this.group.add(this.rect);
    this.cmu3 = new CMU(this.group, 500, 900, null, 'CMU3-FXOSC', data.cmu3);
    this.fxosc = new FXOSC(this.group, 100, 200,this.cmu3,data.fxosc);
    this.cmu3.setGetFreq(this.fxosc.getFreq.bind(this.fxosc))
    this.fxosc.errorCheck()
    this.sirc = new SIRC(this.group, 100, 400, data.sirc);
    this.cmu1Input = new Switcher(
      this.group,
      300,
      750,
      'CMU1_INPUT',
      data.cmu1.ref == undefined ? 0 : data.cmu1.ref,
      [
        {
          rect: this.sirc,
          points: [],
          label: "SIRC_CLK",
          freq: this.sirc.getFreq.bind(this.sirc),
          getEnable: this.sirc.getEnable.bind(this.sirc)
        },
        {
          rect: this.fxosc,
          points: [],
          label: "FXOSC_CLK",
          freq: this.fxosc.getFreq.bind(this.fxosc),
          getEnable: this.fxosc.getEnable.bind(this.fxosc)
        },
      ],
      (pos) => { data.cmu1.ref = pos },
      false
    );
    this.cmu1 = new CMU(this.group, 500, 700, this.cmu1Input, 'CMU1-FIRC', data.cmu1);
    this.firc = new FIRC(this.group, 100, 50, this.cmu1,data.firc);
    this.cmu1.setGetFreq(this.firc.getFreq.bind(this.firc))
    this.firc.errorCheck()

    this.trng = new TRNG(this.group,300,1100)
    this.pllInput = new Switcher(
      this.group,
      300,
      60,
      'PLL_INPUT',
      data.pll.ref,
      [
        {
          rect: this.firc,
          points: [0, 0, 65, 0, 65, 30, 98, 30],
          label: "FIRC",
          freq: this.firc.getFreq.bind(this.firc),
          getEnable: this.firc.getEnable.bind(this.firc)
        },
        {
          rect: this.fxosc,
          points: [0, 0, 65, 0, 65, -90, 98, -90],
          label: "FXOSC",
          freq: this.fxosc.getFreq.bind(this.fxosc),
          getEnable: this.fxosc.getEnable.bind(this.fxosc)
        },
      ],
      (pos) => { data.pll.ref = pos },
      true
    );
    this.cmu2Input = new Switcher(
      this.group,
      300,
      900,
      'CMU2_INPUT',
      data.cmu2.ref == undefined ? 0 : data.cmu2.ref,
      [
        {
          rect: this.sirc,
          points: [],
          label: "SIRC_CLK",
          freq: this.sirc.getFreq.bind(this.sirc),
          getEnable: this.sirc.getEnable.bind(this.sirc)
        },
        {
          rect: this.fxosc,
          points: [],
          label: "FXOSC_CLK",
          freq: this.fxosc.getFreq.bind(this.fxosc),
          getEnable: this.fxosc.getEnable.bind(this.fxosc)
        },
      ],
      (pos) => { data.cmu2.ref = pos },
      false
    );
    this.cmu2 = new CMU(this.group, 500, 800, this.cmu2Input, 'CMU2-PLL', data.cmu2);
    this.pll = new PLL(
      this.group,
      this.pllInput.getRightPos().x + 100,
      this.pllInput.getRightPos().y - 10,
      this.pllInput,
      this.cmu2,
      data.pll
    );
    this.cmu2.setGetFreq(this.pll.getFreq.bind(this.pll))
    this.pll.errorCheck()
   
    this.sysInput = new Switcher(
      this.group,
      1100,
      75,
      'SYS_INPUT',
      data.coreClock.ref,
      [
        {
          rect: this.firc,
          points: [0, 0, 65, 0, 65, -50, 850, -50, 850, 45, 900 - 2, 45],
          label: "FIRC",
          freq: this.firc.getFreq.bind(this.firc),
          getEnable: this.firc.getEnable.bind(this.firc)
        },
        {
          rect: this.pll.div2,
          points: [470, 95, 570 - 2, 95],
          label: "PLL",
          freq: this.pll.getFreq.bind(this.pll),
          getEnable: this.pll.getEnable.bind(this.pll)
        },
        {
          rect: this.fxosc,
          points: [0, 0, 65, 0, 65, 150, 850, 150, 850, -45, 900 - 2, -45],
          label: "FXOSC",
          freq: this.fxosc.getFreq.bind(this.fxosc),
          getEnable: this.fxosc.getEnable.bind(this.fxosc)
        },
        {
          rect: this.sirc,
          points: [0, 0, 870, 0, 870, -215, 900 - 2, -215],
          label: "SIRC",
          freq: this.sirc.getFreq.bind(this.sirc),
          getEnable: this.sirc.getEnable.bind(this.sirc)
        },
      ],
      (pos) => { data.coreClock.ref = pos },
      true
    );
    this.coreClock = new CoreClock(this.group, 1300, 100, this.sysInput, data.coreClock);
    this.fastBus = new FastBus(this.group, 1300, 200, this.coreClock, data.fastClock);

    this.cmu0Input = new Switcher(
      this.group,
      300,
      600,
      'CMU0_INPUT',
      data.cmu0.ref == undefined ? 0 : data.cmu0.ref,
      [
        {
          rect: this.sirc,
          points: [],
          label: "SIRC_CLK",
          freq: this.sirc.getFreq.bind(this.sirc),
          getEnable: this.sirc.getEnable.bind(this.sirc)
        },
        {
          rect: this.fxosc,
          points: [],
          label: "FXOSC_CLK",
          freq: this.fxosc.getFreq.bind(this.fxosc),
          getEnable: this.fxosc.getEnable.bind(this.fxosc)
        },
      ],
      (pos) => { data.cmu0.ref = pos },
      false
    );
    this.cmu0 = new CMU(this.group, 500, 600, this.cmu0Input, 'CMU0-Slow Bus', data.cmu0);
    this.slowBus = new SlowBus(this.group, 1300, 300, this.fastBus, this.cmu0, data.slowClock);
    this.cmu0.setGetFreq(this.slowBus.getFreq.bind(this.slowBus))
    this.slowBus.errorCheck()

    this.fircOutPoint = new OutPoint(this.group, 1600, 20, {
      rect: this.firc,
      points: [-550, 5, 0, 5],
      label: "FIRC",
      freq: this.firc.getFreq.bind(this.firc),
      getEnable: this.firc.getEnable.bind(this.firc)
    });
    this.fxoscOutPoint = new OutPoint(this.group, 1600, 370, {
      rect: this.fxosc,
      points: [-550, 5, 0, 5],
      label: "FXOSC_CLK",
      freq: this.fxosc.getFreq.bind(this.fxosc),
      getEnable: this.fxosc.getEnable.bind(this.fxosc)
    });
    this.sircOutPoint = new OutPoint(this.group, 1600, 420, {
      rect: this.sirc,
      points: [-530, 5, 0, 5],
      label: "SRIC_CLK",
      freq: this.sirc.getFreq.bind(this.sirc),
      getEnable: this.sirc.getEnable.bind(this.sirc)
    });
        this.pllOutPoint = new OutPoint(this.group, 1600, 470, {
      rect: this.pll.div2,
      points: [-580, -320, -580, 5, 0, 5],
      label: "PLL_CLK",
      freq: this.pll.getFreq.bind(this.pll),
      getEnable: this.pll.getEnable.bind(this.pll)
    });
    this.coreOutPoint = new OutPoint(this.group, 1600, 120, {
      rect: this.coreClock,
      points: [-50, 5, 0, 5],
      label: "CORE_CLK",
      freq: this.coreClock.getFreq.bind(this.coreClock),
      getEnable: this.coreClock.getEnable.bind(this.coreClock)
    });
    this.fastOutPoint = new OutPoint(this.group, 1600, 220, {
      rect: this.fastBus,
      points: [-50, 5, 0, 5],
      label: "FAST_CLK",
      freq: this.fastBus.getFreq.bind(this.fastBus),
      getEnable: this.fastBus.getEnable.bind(this.fastBus)
    });

    this.slowOutPoint = new OutPoint(this.group, 1600, 320, {
      rect: this.slowBus,
      points: [-100, 5, 0, 5],
      label: "SLOW_CLK",
      freq: this.slowBus.getFreq.bind(this.slowBus),
      getEnable: this.slowBus.getEnable.bind(this.slowBus)
    });

    this.clkoutInput = new Switcher(
      this.group,
      1100,
      950,
      'CLK_OUT_INPUT',
      data.clkOut.ref,
      [
        {
          rect: this.pll.div2,
          points: [],
          label: "PLL_CLK",
          freq: this.pll.getFreq.bind(this.pll),
          getEnable: this.pll.getEnable.bind(this.pll)
        },
        {
          rect: this.firc,
          points: [],
          label: "FIRC_CLK",
          freq: this.firc.getFreq.bind(this.firc),
          getEnable: this.firc.getEnable.bind(this.firc)
        },
        {
          rect: this.sirc,
          points: [],
          label: "SIRC_CLK",
          freq: this.sirc.getFreq.bind(this.sirc),
          getEnable: this.sirc.getEnable.bind(this.sirc)
        },
        {
          rect: this.fxosc,
          points: [],
          label: "FXOSC_CLK",
          freq: this.fxosc.getFreq.bind(this.fxosc),
          getEnable: this.fxosc.getEnable.bind(this.fxosc)
        },
                {
          rect: this.coreClock,
          points: [],
          label: "CORE_CLK",
          freq: this.coreClock.getFreq.bind(this.coreClock),
          getEnable: this.coreClock.getEnable.bind(this.coreClock)
        },
        {
          rect: this.trng,
          points: [],
          label: "TRNG_CLK",
          freq: this.trng.getFreq.bind(this.trng),
          getEnable: this.trng.getEnable.bind(this.trng)
        },
      ],
      (pos) => { data.clkOut.ref = pos },
      false
    );
    this.clkOut = new CLK_OUT(this.group, 1300, 1030, this.clkoutInput, data.clkOut);
    
    





    
  
    this.parent.add(this.group);
    window.addEventListener('keyup', (e) => {
      this.ctrlDown = e.ctrlKey;
    });
    window.addEventListener('keydown', (e) => {
      this.ctrlDown = e.ctrlKey;
    });
    stage.on("wheel", (evt) => {
      if (this.ctrlDown) {
        const change = -evt.evt.deltaY / 2000;
        this.changeScale(change);
      }
    });
    this.lockChange(lock)
    this.parent.draw();
  }
  lockChange(lock){
    this.lock=lock
    if(this.lock){
      if(this.lockRect!=undefined){
        this.lockRect.destroy()
      }
      this.lockRect=new Konva.Rect({
        x: 0,
        y: 0,
        width: this.width,
        height: this.height,
        dash: [2, 2],
        stroke: getVarColor('--el-text-color-primary'),
        strokeWidth: 3,
        fill:'#909399',
        opacity: 0.4,
      });
      this.group.add(this.lockRect)
    }else{
      this.lockRect?.destroy()
      this.lockRect=undefined
    }
  }
  clearLogs()
  {
    const reg=new RegExp(`^${this.clockName}.*`)
    plugin.deleteLog('error',reg)
    plugin.deleteLog('warn',reg)
    plugin.deleteLog('info',reg)
  }
  changeScale(change: number) {
    const scale = this.group.scale();
    if(scale.x<0.2&&change<0){
      return
    }
    if(scale.x>4&&change>0){
      return
    }
    this.group.scale({
      x: scale.x + change,
      y: scale.y + change,
    });
  }
  fullScreen(){
    const stage = this.parent.getStage()
    if (stage != null) {
      let scale = stage.width() / (this.width * defaultScaleRatio);
      if (stage.height() / (this.width * defaultScaleRatio) < scale) {
        scale = stage.height() / (this.width * defaultScaleRatio)
      }
      const scaleWidth = this.width * scale
      const rx = (stage.width() / 2 - scaleWidth / 2)
      const ry = (stage.height() / 2 - scaleWidth / 2)

      this.group.setPosition({
        x: rx,
        y: ry
      })
      this.group.scale({
        x: scale,
        y: scale,
      });
    }
  }
  resize(x: number, y: number) {
    const stage = this.parent.getStage()
    if (stage != null) {
      stage.width(x)
      stage.height(y-60)
      let scale = stage.width() / (this.width * defaultScaleRatio);
      if (stage.height() / (this.width * defaultScaleRatio) < scale) {
        scale = stage.height() / (this.width * defaultScaleRatio)
      }
      const scaleWidth = this.width * scale
      const rx = (stage.width() / 2 - scaleWidth / 2)
      const ry = (stage.height() / 2 - scaleWidth / 2)

      this.group.setPosition({
        x: rx,
        y: ry
      })
      this.group.scale({
        x: scale,
        y: scale,
      });
    }
  }
}
interface clockData {
  firc: fircData,
  fxosc: fxoscData,
  sirc: sircData,
  sxosc: sxoscData,
  pll: pllData,
  coreClock: coreClockData,
  fastClock: fastClockData,
  slowClock: slowClockData,
  cmu0: cmuData,
  cmu1: cmuData,
  cmu2: cmuData,
  cmu3: cmuData,
  clkOut: clkOutData,
  periClocks: any[],
}

export const defaultClockData: clockData = {
  firc: {
    enable: true,
    dsEnable: false,
  freq: {
      value: 96,
      unit: FreqUnit.MHz,
      raw: 96000000
    }
  },
  fxosc: {
    enable: true,
    dsEnable: false,
    freq: {
      value: 24,
      unit: FreqUnit.MHz,
      raw: 24000000
    },
    gain: 5,
    mode: 0
  },
  sirc: {
    enable: true,
    dsEnable: false,
    stbEnable: false,
    pdEnable: false,
  freq: {
      value: 12,
      unit: FreqUnit.MHz,
      raw: 12000000
    }
  },
  sxosc: {
    enable: false,
    dsEnable: false,
    pdEnable: false,
    stbEnable: false,
    freq: {
      value: 32768,
      unit: FreqUnit.Hz,
      raw: 32768
    },
    gain: 0,
    mode: 0
  },
  pll: {
    enable: true,
    refdivV: 1,
    fbdivV: 10,
    ref: 1,
    freq: {
      value: 32768,
      unit: FreqUnit.Hz,
      raw: 32768
    },
    dsEnable: false,
  },
  cmu0: {
    reset: true,
    ref: 0,
    enable: true,
    compareHigh: 50,
    compareLow: 30,
  },
  cmu1: {
    reset: true,
    ref: 0,
    enable: true,
    compareHigh:120,
    compareLow: 72,
  },
  cmu2: {
    reset: true,
    ref: 0,
    enable: true,
    compareHigh:150,
    compareLow: 90,
  },
  cmu3: {
    reset: true,
    enable: true,
    ref: 0,
    compareHigh:30,
    compareLow: 18,
  },
  clkOut: {
    enable: false,
    div: 1,
    ref: 0,
  },
  coreClock: {
    div: 0,
    ref: 1,
    freq: {
      value: 48,
      unit: FreqUnit.MHz,
      raw: 48000000
    },
  },
  fastClock: {
    div: 0,
    freq: {
      value: 24,
      unit: FreqUnit.MHz,
      raw: 24000000
    },
  },
  slowClock: {
    div: 2,
    freq: {
      value: 12,
      unit: FreqUnit.MHz,
      raw: 12000000
    },
  },
  periClocks:
    [
      {
        name: 'DMA_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        div: 0,
        busClock: ['CORE_CLK']
      },
      {
        name: 'TRACE_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        div: 0,
        busClock: ['FAST_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL']
      },
      {
        name: 'EFM_CLK',
        gate: true,
        busSrc: 0, periSrc: 0, userCtrl: false,
        div: 0,
        busClock: ['SLOW_BUS_CLK']
      },
      {
        name: 'GPIO_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        div: 0,
        busClock: ['CORE_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL']
      },
      {
        name: 'PCTRLA_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        div: 0,
        busClock: ['SLOW_BUS_CLK'],
      },
      {
        name: 'PCTRLB_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        div: 0,
        busClock: ['SLOW_BUS_CLK'],
      },
      {
        name: 'PCTRLC_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        div: 0
      },
      {
        name: 'PCTRLD_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        div: 0
      },
      {
        name: 'PCTRLE_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        div: 0
      },
      {
        name: 'LINFlexD0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'LINFlexD1_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'LINFlexD2_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'SENT0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'I2C0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK',],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'I2C1_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'SPI0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'SPI1_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'SPI2_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'SPI3_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'FlexCAN0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK', 'FXOSC_CLK'],
        div: 0
      },
      {
        name: 'FlexCAN1_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK', 'FXOSC_CLK'],
        div: 0
      },
      {
        name: 'FlexCAN2_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK', 'FXOSC_CLK'],
        div: 0
      },
      {
        name: 'ADC0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'ACMP0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'PTU0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK'],
        div: 0
      },
      {
        name: 'TMU_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        div: 0
      },
      {
        name: 'eTMR0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK', 'TCLK0', 'TCLK1', 'TCLK2'],
        div: 0
      },
      {
        name: 'eTMR1_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK', 'TCLK0', 'TCLK1', 'TCLK2'],
        div: 0
      },
      {
        name: 'eTMR2_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK', 'TCLK0', 'TCLK1', 'TCLK2'],
        div: 0
      },
      {
        name: 'eTMR3_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK', 'TCLK0', 'TCLK1', 'TCLK2'],
        div: 0
      },
      {
        name: 'TMR0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        div: 0
      },
      {
        name: 'pTMR0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        div: 0
      },
      {
        name: 'lpTMR0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'RTC_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK', 'RTC_CLK_OSCIN', 'RTC_CLK_CLKIN'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'WKU_CLK',
        gate: false,
        busClock: ['SLOW_BUS_CLK'],
        busSrc: 0, periSrc: 0, userCtrl: false,
        div: 0
      },
      {
        name: 'CRC_CLK',
        gate: false,
        busClock: ['CORE_CLK'],
        busSrc: 0, periSrc: 0, userCtrl: false,
        div: 0
      },
      {
        name: 'TRNG_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        div: 0
      },
      {
        name: 'HCU_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        div: 0
      },
      {
        name: 'WDG0_CLK',
        gate: true,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK', 'SIRC_CLK', 'SXOSC_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_PLL'],
        div: 0
      },
      {
        name: 'EWDG0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        div: 0
      },
      {
        name: 'EMU0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['CORE_CLK'],
        div: 0
      },
      {
        name: 'CIM_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        div: 0
      },
      {
        name: 'SCU_CLK',
        gate: true,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        div: 0
      },
      {
        name: 'PCU_CLK',
        gate: true,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        div: 0
      },
      {
        name: 'RCU_CLK',
        gate: true,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK', 'SIRC_CLK'],
        div: 0
      },
    ]
}
export class HelpButtons {
  parent: Konva.Layer;
  buttonsGroup: Konva.Group;
  clock: any
  constructor(parent: Konva.Layer, clock: Clock) {
      this.parent = parent
      this.clock=clock
      const stage = this.parent.getStage()
      this.buttonsGroup = new Konva.Group({
          x: stage.width() / 2,
          y: 10,
          draggable: false,
          rotation: 0,
      });
      this.parent.add(this.buttonsGroup);
      this.createButton(- 100, plusIcon, () => {
        this.clock.changeScale(0.05);
      });
      this.createButton(0, fullScreenIcon, () => {
        this.clock.fullScreen();
      });
      this.createButton(100, minIcon, () => {
        this.clock.changeScale(-0.05);
      });
      
  }
  changeClock(clock){
    this.clock=clock
  }
  createButton(x: number, svg: string, action: any, disable = false, id: string | undefined = undefined) {
      const url = 'data:image/svg+xml;base64,' + window.btoa(svg);
      Konva.Image.fromURL(url, (image: any) => {
          image.draggable(false);

          image.setPosition({ x: x, y: 0 })
          image.width(30)
          image.height(30)
          if (id != undefined) {
              image.setAttr('id', id)
          }
          if (disable == false) {
              const rect = new Konva.Rect({
                  x: x - 5,
                  y: -5,
                  width: 40,
                  height: 40,
                  draggable: false,
                  fill: getVarColor("--bg-color"),
                  strokeWidth: 0,
                  stroke: getVarColor('--el-color-primary'),
                  opacity: 1
              })
              this.buttonsGroup.add(rect)
              image.on('mouseenter', () => {
                  rect.setAttr('strokeWidth', 2)
              })
              image.on('mouseleave', () => {
                  rect.setAttr('strokeWidth', 0)
              })
              image.on('click', () => {
                  if (action != null) {
                      action()
                  }

              })
          }
          this.buttonsGroup.add(image);
      });
  }
  resize(x: number) {
      const stage = this.parent.getStage()
      stage.width(x)

      this.buttonsGroup.setPosition({
          x: x / 2,
          y: 10
      })
      const stageBox = stage.container().getBoundingClientRect()
      const areaPosition = {
          x: stageBox.left + x / 2 + 150,
          y: stageBox.top + 10,
      };
    }
}