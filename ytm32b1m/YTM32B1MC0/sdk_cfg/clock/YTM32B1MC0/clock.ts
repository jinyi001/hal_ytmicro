import { ref, h, Ref } from "vue";
import Form from "./Form.vue";
import Konva from "konva";
import { ElMessageBox, ElNotification } from "element-plus";
import { Plugin } from "plugin";
import { getVarColor} from './base.ts'
const defaultScaleRatio = 1.1
const plugin = new Plugin();
const freqFontSize = 14




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


interface lpoclkData {
  enable: boolean,
  dsEnable: boolean,
  stbEnable: boolean,
  freq: Freq
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
  cmu: CMU|null;
  constructor(
    parent: Konva.Group,
    x: number,
    y: number,
    cmu: CMU|null,
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
    this.cmu?.errorCheck()
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
          gain: [
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

class TRNG extends Rect {
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
        ],
        rules: {},
        onChange: (val: any) => {
          // direction = val;
          this.data.stbEnable = val.stbEnable;
          this.data.dsEnable = val.dsEnable;
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

class LPOCLK extends Rect {
  color = getVarColor('--el-color-warning-dark-2');
  freqText: Konva.Text;
  data: lpoclkData;
  constructor(parent: Konva.Group, x: number, y: number, data: lpoclkData) {
    super(parent, x, y, "LPOCLK", getVarColor('--el-color-warning-dark-2'));
    this.data = data

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
            disable: true,
          },
          {
            type: "switch",
            label: `DeepSleep Enable`,
            prop: "dsEnable",
            disable: true,
          },
        ],
        rules: {},
        onChange: (val: any) => {
          // direction = val;
          this.data.stbEnable = val.stbEnable;
          this.data.dsEnable = val.dsEnable;
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
  }
}

class CoreClock extends Rect {
  color = getVarColor('--el-text-color-primary');
  freqText: Konva.Text;
  sw: Switcher;
  min = 0;
  max = 80000000;
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
    if(freqToNumber(this.data.freq) <= 60000000){
      this.data.flash = 1
    }else if (freqToNumber(this.data.freq) <= 80000000){
      this.data.flash = 2
    }else{
      this.data.flash = 3
    }
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
  max = 80000000;
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
  cmu: CMU|null
  constructor(
    parent: Konva.Group,
    x: number,
    y: number,
    fastClock: FastBus,
    cmu: CMU|null,
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
    this.cmu?.errorCheck()
  }
  getFreq() {
    return JSON.parse(JSON.stringify(this.data.freq));
  }
  getEnable() {
    return true
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
    new OutPoint(this.group, 1500, 20, {
      rect: this,
      points: [-1300, 5, 0, 5],
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
          compareHigh: [
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
          compareLow: [
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


      const freq = this.getFreq()
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
  lpo: LPOCLK;
  sysInput: Switcher;
  coreClock: CoreClock;
  fastBus: FastBus;
  slowBus: SlowBus;
  cmu1: CMU;
  cmu0: CMU;
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
  lockRect: Konva.Rect | undefined
  clockName = '';
  constructor(parent: Konva.Group | Konva.Layer, data: clockData, clockName: string, lock = false) {
    plugin.setIdPrefix(clockName)
    this.clockName = clockName
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
    /* all element should put below*/    
    this.sirc = new SIRC(this.group, 100, 400, data.sirc);

    this.cmu0 = new CMU(this.group, 100, 700, null, 'CMU0-FIRC', data.cmu0);
    this.firc = new FIRC(this.group, 100, 50, this.cmu0, data.firc);
    this.cmu0.setGetFreq(this.firc.getFreq.bind(this.firc))
    this.firc.errorCheck()

    this.cmu1 = new CMU(this.group, 100, 900, null, 'CMU1-FXOSC', data.cmu1);
    this.fxosc = new FXOSC(this.group, 100, 200, this.cmu1, data.fxosc);
    this.cmu1.setGetFreq(this.fxosc.getFreq.bind(this.fxosc))
    this.fxosc.errorCheck()


    this.lpo = new LPOCLK(this.group, 100, 500, data.lpo);
    this.trng = new TRNG(this.group, 300, 1100)

    
    this.sysInput = new Switcher(
      this.group,
      1100,
      75,
      'SYS_INPUT',
      data.coreClock.ref,

      [
        {
          rect: this.firc,
          points: [0, 0, 65, 0, 65, 45, 898, 45],
          label: "FIRC",
          freq: this.firc.getFreq.bind(this.firc),
          getEnable: this.firc.getEnable.bind(this.firc)
        },
        {
          rect: this.fxosc,
          points: [0, 0, 65, 0, 65, -70, 898, -70],
          label: "FXOSC",
          freq: this.fxosc.getFreq.bind(this.fxosc),
          getEnable: this.fxosc.getEnable.bind(this.fxosc)
        },
        {
          rect: this.sirc,
          points: [0, 0, 175, 0, 175, -245, 898, -245],
          label: "SIRC",
          freq: this.sirc.getFreq.bind(this.sirc),
          getEnable: this.sirc.getEnable.bind(this.sirc)
        },
      ]
      ,
      (pos) => { data.coreClock.ref = pos },
      true
    );
    this.coreClock = new CoreClock(this.group, 1300, 100, this.sysInput, data.coreClock);
    this.fastBus = new FastBus(this.group, 1300, 200, this.coreClock, data.fastClock);

    
    this.slowBus = new SlowBus(this.group, 1300, 300, this.fastBus, null, data.slowClock);
    
    this.slowBus.errorCheck()

    this.fircOutPoint = new OutPoint(this.group, 1600, 20, {
      rect: this.firc,
      points: [-1335, 55,-1335, 5, 0, 5,],
      label: "FIRC",
      freq: this.firc.getFreq.bind(this.firc),
      getEnable: this.firc.getEnable.bind(this.firc)
    });
    this.fxoscOutPoint = new OutPoint(this.group, 1600, 370, {
      rect: this.fxosc,
      points: [-1335, -145, -1335, 5, 0, 5],
      label: "FXOSC_CLK",
      freq: this.fxosc.getFreq.bind(this.fxosc),
      getEnable: this.fxosc.getEnable.bind(this.fxosc)
    });
    this.sircOutPoint = new OutPoint(this.group, 1600, 420, {
      rect: this.sirc,
      points: [-1335, 5, 0, 5],
      label: "SRIC_CLK",
      freq: this.sirc.getFreq.bind(this.sirc),
      getEnable: this.sirc.getEnable.bind(this.sirc)
    });
    this.sxoscOutPoint = new OutPoint(this.group, 1600, 520, {
      rect: this.lpo,
      points: [-1400, 5, 0, 5],
      label: "LPO_CLK",
      freq: this.lpo.getFreq.bind(this.lpo),
      getEnable: this.lpo.getEnable.bind(this.lpo)
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
          rect: this.lpo,
          points: [],
          label: "LPO_CLK",
          freq: this.lpo.getFreq.bind(this.lpo),
          getEnable: this.lpo.getEnable.bind(this.lpo)
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
  lockChange(lock) {
    this.lock = lock
    if (this.lock) {
      if (this.lockRect != undefined) {
        this.lockRect.destroy()
      }
      this.lockRect = new Konva.Rect({
        x: 0,
        y: 0,
        width: this.width,
        height: this.height,
        dash: [2, 2],
        stroke: getVarColor('--el-text-color-primary'),
        strokeWidth: 3,
        fill: '#909399',
        opacity: 0.4,
      });
      this.group.add(this.lockRect)
    } else {
      this.lockRect?.destroy()
      this.lockRect = undefined
    }
  }
  clearLogs() {
    const reg = new RegExp(`^${this.clockName}.*`)
    plugin.deleteLog('error', reg)
    plugin.deleteLog('warn', reg)
    plugin.deleteLog('info', reg)
  }
  changeScale(change: number) {
    const scale = this.group.scale();
    if(scale){
      if (scale.x < 0.2 && change < 0) {
        return
      }
      if (scale.x > 4 && change > 0) {
        return
      }
      this.group.scale({
        x: scale.x + change,
        y: scale.y + change,
      });
    }
  }
  fullScreen() {
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
      stage.height(y - 60)
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
  lpo: lpoclkData,
  coreClock: coreClockData,
  fastClock: fastClockData,
  slowClock: slowClockData,
  cmu1: cmuData,
  cmu0: cmuData,
  clkOut: clkOutData,
  periClocks: any[],
}

export const defaultClockData: clockData = {
  firc: {
    enable: true,
    dsEnable: false,
    freq: {
      value: 80,
      unit: FreqUnit.MHz,
      raw: 80000000
    }
  },
  fxosc: {
    enable: false,
    dsEnable: false,
    freq: {
      value: 24,
      unit: FreqUnit.MHz,
      raw: 24000000
    },
    gain: 6,
    mode: 0
  },
  sirc: {
    enable: true,
    dsEnable: false,
    stbEnable: false,
    pdEnable: false,
    freq: {
      value: 2,
      unit: FreqUnit.MHz,
      raw: 2000000
    }
  },
  lpo: {
    enable: true,
    dsEnable: true,
    stbEnable: true,
    freq:{
      value: 32,
      unit: FreqUnit.KHz,
      raw: 32000
    }

  },
  cmu1: {
    reset: true,
    ref: 0,
    enable: true,
    compareHigh: 30,
    compareLow: 18,
  },
  cmu0: {
    reset: true,
    enable: true,
    ref: 0,
    compareHigh: 100,
    compareLow: 60,
  },
  clkOut: {
    enable: false,
    div: 1,
    ref: 0,
  },
  coreClock: {
    div: 0,
    ref: 0,
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
    div:1,
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
        name: 'GPIO_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        div: 0,
        busClock: ['CORE_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS']
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
        name: 'UART0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
        div: 0
      },
      {
        name: 'UART1_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
        div: 0
      },
      {
        name: 'UART2_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
        div: 0
      },
      {
        name: 'I2C0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
        div: 0
      },
      {
        name: 'I2C1_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
        div: 0
      },
      {
        name: 'SPI0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
        div: 0
      },
      {
        name: 'SPI1_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
        div: 0
      },
      {
        name: 'SPI2_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
        div: 0
      },
      
      {
        name: 'FlexCAN0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK'],
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
        name: 'ADC0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
        div: 0
      },
      {
        name: 'ACMP0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
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
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
        div: 0
      },
      {
        name: 'eTMR1_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK', 'TCLK0', 'TCLK1', 'TCLK2'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
        div: 0
      },
     
      {
        name: 'MPWM0_CLK',
        gate: false,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['FAST_BUS_CLK'],
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
        div: 0
      },
      {
        name: 'CRC_CLK',
        gate: false,
        busClock: ['SLAVE_CLK'],
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
        busClock: ['SLAVE_CLK'],
        div: 0
      },
      {
        name: 'WDG0_CLK',
        gate: true,
        busSrc: 0, periSrc: 0, userCtrl: false,
        busClock: ['SLOW_BUS_CLK'],
        periClock: ['CLK_SRC_DISABLED', 'CLK_SRC_FIRC', 'CLK_SRC_SIRC', 'CLK_SRC_FXOSC', 'CLK_SRC_LPO', 'CLK_SRC_FAST_BUS'],
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
        busClock: ['FAST_BUS_CLK'],
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
      }
    ]
}
