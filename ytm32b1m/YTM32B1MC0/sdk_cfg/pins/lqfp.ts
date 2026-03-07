import Konva from "konva";
import { Pin, PinDef, getVarColor, pinHeight, pinLength } from './pin.ts'

const defaultScaleRatio = 2
const plusIcon =
    '<svg t="1661419115069" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="2335" width="200" height="200"><path d="M1005.312 914.752l-198.528-198.464A448 448 0 1 0 0 448a448 448 0 0 0 716.288 358.784l198.4 198.4a64 64 0 1 0 90.624-90.432zM384 384H256v128h128v128h128V512h128V384H512V256H384v128z m64 384A320 320 0 1 1 448 128a320 320 0 0 1 0 640z" fill="#409EFF" p-id="2336"></path></svg>';
const minIcon =
    '<svg t="1661420224594" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="3238" width="200" height="200"><path d="M1005.312 914.752l-198.528-198.464A448 448 0 1 0 0 448a448 448 0 0 0 716.288 358.784l198.4 198.4a64 64 0 1 0 90.624-90.432zM448 767.936A320 320 0 1 1 448 128a320 320 0 0 1 0 640zM256 384v128h384V384H256z" fill="#409EFF" p-id="3239"></path></svg>';
const fullScreenIcon =
    '<svg t="1661420254009" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="4177" width="200" height="200"><path d="M213.333333 213.333333h213.333334V128H170.666667a42.666667 42.666667 0 0 0-42.666667 42.666667v256h85.333333V213.333333zM170.666667 896h256v-85.333333H213.333333v-213.333334H128v256a42.666667 42.666667 0 0 0 42.666667 42.666667z m725.333333-42.666667v-256h-85.333333v213.333334h-213.333334v85.333333h256a42.666667 42.666667 0 0 0 42.666667-42.666667zM597.333333 213.333333h213.333334v213.333334h85.333333V170.666667a42.666667 42.666667 0 0 0-42.666667-42.666667h-256v85.333333z" fill="#409EFF" p-id="4178"></path></svg>';
const searchIcon =
    '<svg t="1661420307350" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="5184" width="200" height="200"><path d="M909.6 854.5L649.9 594.8C690.2 542.7 712 479 712 412c0-80.2-31.3-155.4-87.9-212.1-56.6-56.7-132-87.9-212.1-87.9s-155.5 31.3-212.1 87.9C143.2 256.5 112 331.8 112 412c0 80.1 31.3 155.5 87.9 212.1C256.5 680.8 331.8 712 412 712c67 0 130.6-21.8 182.7-62l259.7 259.6c3.2 3.2 8.4 3.2 11.6 0l43.6-43.5c3.2-3.2 3.2-8.4 0-11.6zM570.4 570.4C528 612.7 471.8 636 412 636s-116-23.3-158.4-65.6C211.3 528 188 471.8 188 412s23.3-116.1 65.6-158.4C296 211.3 352.2 188 412 188s116.1 23.2 158.4 65.6S636 352.2 636 412s-23.3 116.1-65.6 158.4z" p-id="5185" fill="#409EFF"></path></svg>';

export const featureItemWidth = 220
export const featureItemHeight = 20

export class LQFP {
    width: number;
    height: number;
    pins: PinDef[];
    parent: Konva.Layer;
    chipGroup: Konva.Group;
    pinHoverArea: Konva.Label;
    pinFeatureArea: LabelList;
    mode: number;
    scale: number;
    showFeature = '';
    showPin = '';
    pinNodes: Record<string, Pin> = {};
    lock = false;
    lockRect:Konva.Rect|undefined;
    lockHover:Konva.Group|undefined;
    chipName: string;
    featureChange: (number: string, label: string, feature: string, alt: string) => Promise<boolean>;
    constructor(
        parent: Konva.Layer,
        chipName: string,
        pins: PinDef[],
        featureChange: (number: string, label: string, feature: string, alt: string) => Promise<boolean>,
        lock = false,
        encapsulation: string,
        cyclePosition = 1
    ) {
        this.featureChange = featureChange
        this.lock = lock
        this.pins = pins;
        this.parent = parent;
        const pinNum = pins.length;
        this.chipName = chipName;
        let width = 0;
        if (pins.length <= 48) {
            this.mode = 2;
            width = (pinNum / 4) * 2 * pinHeight + pinHeight;
        } else {
            width = (pinNum / 4) * pinHeight + 2 * pinHeight;
            this.mode = 1;
        }
        if (encapsulation == 'TSSOP') {
            width = (pinNum / 2) * 2 * pinHeight + pinHeight;
        }
        const stage = this.parent.getStage()
        this.width = width;
        this.height = encapsulation != 'TSSOP'? width: 300;
        
        let scale = stage.width() / (this.width * defaultScaleRatio);
        if (stage.height() / (this.width * defaultScaleRatio) < scale) {
            scale = stage.height() / (this.width * defaultScaleRatio)
        }
        const scaleWidth = this.width * scale
        const rx = (stage.width() / 2 - scaleWidth / 2)
        const ry = (stage.height() / 2 - scaleWidth / 2)
        this.scale = scale
        this.chipGroup = new Konva.Group({
            x: rx,
            y: ry,
            draggable: true,
            rotation: 0,
            scale: { x: scale, y: scale },
        });
        /*package*/

        this.parent.add(this.chipGroup);

        this.createBackGroundArea(encapsulation, cyclePosition);
        this.createPinHoverArea();
        this.createPinFeatureArea();
        if (encapsulation == 'TSSOP') {
            this.createSOPChip(pins);
        }else {
            this.createChip(pins);
        }
        this.pinFeatureArea.getGroup().zIndex(this.chipGroup.children.length - 1);
        this.pinHoverArea.zIndex(this.chipGroup.children.length - 2);
        this.lockChange(lock)    
        this.parent.getStage().on("click", (evt) => {
            this.closeAllExcept(evt.target as Konva.Shape);
        });
        this.parent.getStage().on("wheel", (evt) => {
            const change = -evt.evt.deltaY / 2000;
            this.changeScale(change);
        });
    }
    fullScreen() {
        const stage = this.parent.getStage()
        let scale = stage.width() / (this.width * defaultScaleRatio);
        if (stage.height() / (this.width * defaultScaleRatio) < scale) {
            scale = stage.height() / (this.width * defaultScaleRatio)
        }
        const scaleWidth = this.width * scale
        const rx = (stage.width() / 2 - scaleWidth / 2)
        const ry = (stage.height() / 2 - scaleWidth / 2)
        this.scale = scale
        this.chipGroup.setPosition({
            x: rx,
            y: ry
        })
        this.chipGroup.scale({
            x: this.scale,
            y: this.scale,
        });
    }
    resize(x: number, y: number) {
        const stage = this.parent.getStage()
        stage.width(x)
        stage.height(y)
        this.fullScreen()
        /** 
         * The fellowing scale code is used to remain the feature area size when hide log button is
         * clicked.
         */
        if (this.chipGroup != undefined) {
            const scaleX = this.chipGroup.scale()?.x || 1
            const scaleY = this.chipGroup.scale()?.y || 1
            this.pinFeatureArea.getGroup().scale({
                x: 1 / scaleX + 0.2,
                y: 1 / scaleY + 0.2,
            });
            this.pinHoverArea.scale({
                x: 1 / scaleX + 0.2,
                y: 1 / scaleY + 0.2,
            });
        }
        this.lockChange(this.lock)
    }
    
    createChip(pins: PinDef[]) {
        const lineNum = pins.length / 4;
        /* bottom */
        for (let i = lineNum * 2 - 1; i >= lineNum; i--) {
            this.createPin(
                (i - lineNum) * pinHeight * this.mode + 2 * pinHeight,
                this.width,
                "down",
                pins[i]
            );
        }
        /* right */
        for (let i = lineNum * 2; i < lineNum * 3; i++) {
            this.createPin(
                this.width,
                this.width - (i - lineNum * 2) * pinHeight * this.mode - 2 * pinHeight,
                "right",
                pins[i]
            );
        }
        /*left*/
        for (let i = lineNum - 1; i >= 0; i--) {
            this.createPin(
                -pinLength,
                i * pinHeight * this.mode + pinHeight,
                "left",
                pins[i]
            );
        }

        /* top */
        for (let i = lineNum * 3; i < lineNum * 4; i++) {
            this.createPin(
                this.width - (i - 3 * lineNum) * pinHeight * this.mode - pinHeight,
                -pinLength,
                "up",
                pins[i]
            );
        }
    }
    
    createSOPChip(pins: PinDef[]) {
        const lineNum = pins.length / 2;
        /* bottom */
        for (let i = 0; i < lineNum; i++) {
            this.createPin(
                i * pinHeight * this.mode + 2 * pinHeight,
                this.height+1,
                "down",
                pins[i]
            );
        }
        /* top */
        for (let i = lineNum; i < lineNum * 2; i++) {
            this.createPin(
                (2*lineNum-i-1) * pinHeight * this.mode + 2 * pinHeight,
                -pinLength+1,
                "up",
                pins[i]
            );
        }
    }

    createPinHoverArea() {
        this.pinHoverArea = new Konva.Label({
            x: 50,
            y: 50
        });

        // add a tag to the label
        this.pinHoverArea.add(new Konva.Tag({
            fill: getVarColor('--el-color-info-light-5'),
            stroke: getVarColor("black"),
            strokeWidth: 1,
            shadowColor: 'black',
            shadowBlur: 10,
            shadowOffset: {
                x: 10,
                y: 10
            },
            shadowOpacity: 0.2,
            lineJoin: 'round',
            pointerDirection: 'up',
            pointerWidth: 10,
            pointerHeight: 10,
            cornerRadius: 5
        }));
        
        // add text to the this.pinHoverArea
        this.pinHoverArea.add(new Konva.Text({
            fontSize: 14,
            padding: 6,
            fontFamily: "Calibri",
            fill: getVarColor("black"),
            align: "center",
        }));

        
        this.pinHoverArea.scale({
            x: 1 / this.chipGroup.scale().x + 0.2,
            y: 1 / this.chipGroup.scale().y + 0.2,
        });
    

        this.chipGroup.add(this.pinHoverArea);
        this.pinHoverArea.hide();
    }

    createPinFeatureArea() {
        this.pinFeatureArea = new LabelList();
        this.chipGroup.add(this.pinFeatureArea.getGroup());
        const scaleX = this.chipGroup.scale()?.x || 1
        const scaleY = this.chipGroup.scale()?.y || 1
        this.pinFeatureArea.getGroup().scale({
            x: 1 / scaleX + 0.2,
            y: 1 / scaleY + 0.2,
        });
        this.pinFeatureArea.getGroup().hide();
    }

    createBackGroundArea(encapsulation, cyclePosition) {
        this.chipGroup.add(
            new Konva.Rect({
                x: 0,
                y: 0,
                width: this.width,
                height: this.height + 2,
                fill: getVarColor('--el-color-info-dark-2'),
                cornerRadius: 5,
            })
        );
        /** Big rect for chip and chip partNumber */
        const base = new Konva.Rect({
            x: 10,
            y: 10,
            width: this.width - 20,
            height: this.height - 20,
            fillLinearGradientStartPoint: { x: 0, y: 0 },
            fillLinearGradientEndPoint: { x: this.width, y: this.height },
            fillLinearGradientColorStops: [0, getVarColor('--el-color-info-light-5'), 1, getVarColor('--el-color-info-dark-2')],
            stroke: "white",
            strokeWidth: 1,
        });
        /** Change cursor display value */
        base.on("mouseenter", (evt) => {
            const stage = evt.target.getStage()
            if (stage != null) {
                stage.container().style.cursor = "move";
            }

        });
        /** Change cursor display value */
        base.on("mouseleave", (evt) => {
            const stage = evt.target.getStage()
            if (stage != null) {
                stage.container().style.cursor = "default";
            }
        });
        this.chipGroup.add(base);
        
        
        // logo 140 * 114
        const width = this.width;
        const height = this.height;
        const path1 = new Konva.Path({
            data: 'M 21.645,65.844 H 18.996 C 18.379,65.91 17.75,65.949 17.117,65.949 7.664,65.949 0,58.281 0,48.832 0,40.562 5.867,33.66 13.664,32.062 c 0.629,-0.07 1.898,-0.203 3.602,-0.347 h 0.121 l -0.004,-0.008 c 10.48,-0.883 36.918,-2.094 33.965,8.453 -2.512,8.977 -5.114,14.27 -29.703,25.684 z',
            fill:getVarColor('--el-color-info-light-5'),
            scaleX: width * 0.1 / 114,
            scaleY: width * 0.1 / 114,
            x: 0,
            y: 0
        });

        const path2 = new Konva.Path({
            data: 'm 29.137,15.355 c 2.781,0 5.347,0.915 7.418,2.465 C 39.086,7.59 48.328,0 59.34,0 c 12.961,0 23.472,10.512 23.472,23.473 0,2.898 -0.527,5.679 -1.492,8.242 h 8.418 v 0.105 c 8.559,0.942 15.223,8.2 15.223,17.012 0,8.813 -6.664,16.07 -15.223,17.012 h -0.011 c -0.618,0.066 -1.247,0.105 -1.879,0.105 -0.637,0 -1.266,-0.039 -1.883,-0.105 H 72.348 c 0.011,0.265 0.019,0.527 0.019,0.797 0,10.179 -8.258,18.437 -18.437,18.437 -4.407,0 -8.453,-1.547 -11.625,-4.129 8.488,-6.054 16.957,-13.383 24.785,-22.246 0,0 24.488,-28.336 -17.664,-29.91 -10.953,-0.094 -21.453,0.363 -32.184,2.461 -0.328,-1.113 -0.504,-2.285 -0.504,-3.504 0,-6.844 5.551,-12.395 12.399,-12.395 z',
            fill:getVarColor('--el-color-info-light-5'),
            scaleX: width * 0.1 / 114,
            scaleY: width * 0.1 / 114,
            x: 0,
            y: 0
        });

        const group = new Konva.Group({
            x: width*0.85-10, /* add 10 offset to align inner rect */
            y: height*0.05+10,
            draggable:true
        });
        group.add(path1)
        group.add(path2)
        this.chipGroup.add(group);

        /* chip circle in the left top */
        const cyclePos =  encapsulation!='TSSOP'?{x: 30, y: 30}:{x: 30, y: height-30};
        this.chipGroup.add(
            new Konva.Circle({
                radius: 10,
                fill: getVarColor('--el-color-info-dark-2'),
                stroke: getVarColor("black"),
                strokeWidth: 1,
                ...cyclePos
            })
        );

        /** Chip partNumber text */
        const partNumberGroup = new Konva.Group();
        partNumberGroup.add(new Konva.Text({
            x: 0,
            y: Math.max(height*0.08, 40), /* 0.09 + 0.02 */
            text: this.chipName,
            fontSize: width*0.13,
            fontStyle: 'italic bold',
            fontFamily: 'Calibri',
            fill: 'white',
        }))
        partNumberGroup.add(new Konva.Text({
            x: 0,
            y: 0,
            text: `${this.pins.length}${encapsulation}`,
            fontSize: width*0.09,
            fontStyle: 'italic bold',
            fontFamily: 'Calibri',
            fill: 'white',
        }))
        partNumberGroup.setPosition({
            x:width*0.15-10,
            y:height*0.55 - partNumberGroup.getClientRect().height/2
        })
        this.chipGroup.add(partNumberGroup);
    }

    createPin(x: number, y: number, pos: string, pin: PinDef) {
        this.pinNodes[pin.num] = new Pin(x, y, this.chipGroup, pin, pos, this.featureChange, this.pinHoverArea, this.pinFeatureArea);
    }
    closeAllExcept(val: Konva.Shape) {
        this.closeHighLight()
        this.pinFeatureArea.getGroup().hide()
    }
    changeScale(val: number) {
        const scale = this.chipGroup.scale();
        if (scale == undefined) {
            return;
        }
        if(scale.x<0.2&&val<0){
            return
        }
        if(scale.x>4&&val>0){
            return
        }
        this.chipGroup.scale({
            x: scale.x + val,
            y: scale.y + val,
        });
        this.pinHoverArea.scale({
            x: 1/scale.x + val + 0.2,
            y: 1/scale.y + val + 0.2,
        })

        const featureScale = 1/(scale.x + val) * Math.min(1.5, Math.max(1, scale.x + val + 0.2));
    
        this.pinFeatureArea.getGroup().scale({
            x: featureScale,
            y: featureScale,
        })
        this.pinHoverArea.scale({
            x: featureScale,
            y: featureScale,
        })
    }
    getAllFeature(): string[] {
        let feature: string[] = []
        for (const i in this.pins) {
            const featureList = this.pins[i].list
            if (this.pinNodes[this.pins[i].num].defautFeat == '-') {
                const index = featureList.indexOf(this.pinNodes[this.pins[i].num].defautFeat)
                featureList.splice(index, 1)
            }
            feature = feature.concat(featureList)
        }
        return [...new Set(feature)];
    }
    getAllLabel(): string[] {
        const labels: string[] = []
        for (const i in this.pins) {
            labels.push(this.pins[i].label)
        }
        return labels;
    }
    getFeature(pinNum:number): string[] {
        return this.pins[pinNum].list;
    }
    lockChange(lock){
        this.lock=lock
        const stage=this.chipGroup.getStage()
        const layer=this.chipGroup.getLayer()
        if(this.lock&&stage!=null&&layer!=null){
            if(this.lockRect!=undefined){
                this.lockRect.destroy()
            }
            if(this.lockHover!=undefined){
                this.lockHover.destroy()
            }
            this.lockRect=new Konva.Rect({
                x: 0,
                y: 0,
                width: stage.width(),
                height: stage.height(),
                fill:getVarColor('--el-color-info-light-9'),
                opacity: 1,
            });
            layer.add(this.lockRect)
            this.lockRect.zIndex(0)

            const width = this.width;
            this.lockHover = new Konva.Group();
            const lockFeatureRect = new Konva.Shape({
                x: 0,
                y: 0,
                fill: getVarColor('--el-color-info-light-9'),
                // a Konva.Canvas renderer is passed into the sceneFunc function
                sceneFunc (context, shape) {
                    context.beginPath();
                    context.moveTo(-200, -200);
                    context.lineTo(-200, width+200);
                    context.lineTo(width+200, width+200);
                    context.lineTo(width+200, -200);
                    context.lineTo(-200, -200);
                    context.closePath();
                    context.fillStrokeShape(this);
                },
                draggable:false,
                opacity:0.05
            });
            const lockChipRect = new Konva.Shape({
                x: 0,
                y: 0,
                fill: getVarColor('--el-color-info-light-9'),
                // a Konva.Canvas renderer is passed into the sceneFunc function
                sceneFunc (context, shape) {
                    context.beginPath();
                    context.moveTo(0, 0);
                    context.lineTo(-0, width+0);
                    context.lineTo(width+0, width+0);
                    context.lineTo(width+0, -0);
                    context.lineTo(-0, -0);
                    context.closePath();
                    context.fillStrokeShape(this);
                },
                draggable:false,
                opacity:0.05
            });
            lockChipRect.on("mouseenter", (evt) => {
                const stage = evt.target.getStage()
                if (stage != null) {
                    stage.container().style.cursor = "move";
                }
    
            });
            /** Change cursor display value */
            lockChipRect.on("mouseleave", (evt) => {
                const stage = evt.target.getStage()
                if (stage != null) {
                    stage.container().style.cursor = "default";
                }
            });
            
            lockFeatureRect.on("mouseenter", (evt) => {
                const stage = evt.target.getStage()
                if (stage != null) {
                    stage.container().style.cursor = "not-allowed";
                }
    
            });
            /** Change cursor display value */
            lockFeatureRect.on("mouseleave", (evt) => {
                const stage = evt.target.getStage()
                if (stage != null) {
                    stage.container().style.cursor = "default";
                }
            });
            
            this.lockHover.add(lockFeatureRect)
            this.lockHover.add(lockChipRect)
            this.chipGroup.add(this.lockHover)

        }else{
          this.lockRect?.destroy()
          this.lockHover?.destroy()
          this.lockRect=undefined
        }
    }

    deletePinFeature(num: string) {
        this.pinNodes[num].removeFeature()
        this.pinNodes[num].setDirection('')
    }

    setPinDirection(num: string, direction: string) {
        this.pinNodes[num].setDirection(direction)
    }

    setPinFeature(num: string, feature: string, userLabel: string, userColor:string|undefined, err: boolean) {
        this.pinNodes[num].errorPin(err)
        this.pinNodes[num].setFeature(feature, userLabel, userColor)
    }
    errorPin(num: string, enable: boolean) {
        this.pinNodes[num].errorPin(enable)
    }
    highLightFeature(feature: string) {
        this.showFeature = feature;
        for (const i in this.pins) {
            const featureList = this.pins[i].list
            if (featureList.indexOf(feature) != -1) {
                this.pinNodes[this.pins[i].num].highLightPin()
            }
        }
    }
    highLightPin(name: string) {
        this.showPin = name;
        for (const i in this.pins) {
            if(this.pins[i].label==name){
                this.pinNodes[this.pins[i].num].highLightPin()
            }
        }
    }
    hoverHighLightPin(num: string) {
        this.pinNodes[parseInt(num)].highLightPin()
    }
    
    hoverClosePin(num: string) {
        this.pinNodes[parseInt(num)].closeHighLightPin()
    }

    setPinColor(num: string, color: string) {
        this.pinNodes[num].setRectColor(color)
    }

    closeHighLight() {
        for (const i in this.pins) {
            const featureList = this.pins[i].list
            if(this.showPin==this.pins[i].label){
                this.pinNodes[this.pins[i].num].closeHighLightPin()
            }
            if (featureList.indexOf(this.showFeature) != -1) {
                this.pinNodes[this.pins[i].num].closeHighLightPin()
            }
        }
    }
}

export class HelpButtons {
    parent: Konva.Layer;
    buttonsGroup: Konva.Group;
    item: any;
    item1: any;
    constructor(parent: Konva.Layer, item: any,  item2: any, chip: LQFP) {
        this.parent = parent
        this.item = item
        this.item1 = item2
        const stage = this.parent.getStage()
        this.buttonsGroup = new Konva.Group({
            x: stage.width() / 2,
            y: 10,
            draggable: false,
            rotation: 0,
        });
        this.parent.add(this.buttonsGroup);
        this.createButton(- 150, plusIcon, () => {
            chip.changeScale(0.05);
        });
        this.createButton(-50, fullScreenIcon, () => {
            chip.fullScreen();
        });
        this.createButton(50, minIcon, () => {
            chip.changeScale(-0.05);
        });
        this.createButton(150, searchIcon, null, true, "search");
        this.createSelector(stage.width() / 2 - 400, item)
        this.createSelector(stage.width() / 2 + 200, item2)
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
    createSelector(x: number, item: any) {
        const stage = this.parent.getStage()
        const stageBox = stage.container().getBoundingClientRect()
        const areaPosition = {
            x: stageBox.left + x,
            y: stageBox.top + 10,
        };
        item.style.top = areaPosition.y + 'px';
        item.style.left = areaPosition.x + 'px';
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
            x: stageBox.left + x / 2 + 200,
            y: stageBox.top + 10,
        };

        const areaPosition1 = {
            x: stageBox.left + x / 2 - 400,
            y: stageBox.top + 10,
        };
        this.item.style.top = areaPosition.y + 'px';
        this.item.style.left = areaPosition.x + 'px';

        this.item1.style.top = areaPosition1.y + 'px';
        this.item1.style.left = areaPosition1.x + 'px';
    }
}

/**
 * Konav 自带的 Label 标签只能实现功能太局限了，固定了子元素只能是 Tag 和 Text，在这里实现一个 LabelList 类
 * 实现一个待箭头的列表，用于显示 feature 的 List
 * 对应的结构：
 * Group
 * |_Tag
 * |_Group
 *   |_Rect
 *   |_Text
 */
export class LabelList {
    posX: number;
    posY: number;
    dir: string;
    group: Konva.Group;
    childGroup: Konva.Group;
    tag: Konva.Tag;
    fill: string
    constructor() {
        this.group = new Konva.Group();
        this.tag = new Konva.Tag({
            fill: getVarColor('--el-color-info-light-5'),
            stroke: getVarColor("black"),
            strokeWidth: 1,
            shadowColor: 'black',
            shadowBlur: 10,
            shadowOffset: {
                x: 10,
                y: 10
            },
            shadowOpacity: 0.2,
            lineJoin: 'round',
            pointerDirection: 'up',
            pointerWidth: 10,
            pointerHeight: 10,
            cornerRadius: 5
        });
        this.childGroup = new Konva.Group({
            x:0, 
            y:0
        });
        this.group.add(this.tag);
        this.group.add(this.childGroup);

        this.listUpdate(this.childGroup);
    }

    listUpdate(childGroup: Konva.Group, dir='up') {
        this.childGroup = childGroup;
        const width = featureItemWidth;
        const height = childGroup.children.length * featureItemHeight + 1;
        // eslint-disable-next-line @typescript-eslint/ban-ts-comment
        // @ts-ignore
        this.tag.pointerDirection(dir);

        const pointerWidth = this.tag.pointerWidth();
        const pointerHeight = this.tag.pointerHeight();
        let x = 0;
        let y = 0;
        switch (dir) {
            case 'up':
                x = width / 2;
                y = -1 * pointerHeight;
                break;
            case 'right':
                x = width + pointerWidth;
                y = height / 2;
                break;
            case 'down':
                x = width / 2;
                y = height + pointerHeight;
                break;
            case 'left':
                x = -1 * pointerWidth;
                y = height / 2;
                break;
        }

        this.tag.setAttrs({
            x: -1 * x,
            y: -1 * y,
            width: width,
            height: height,
            // fill: dir==='up'?getVarColor('--el-color-info-dark-2'):getVarColor('--el-color-info-light-5')
        });

        /* offset 0.5 for stroke width 1 */
        this.childGroup.setAttrs({
            x: -1 * x + 0.5,
            y: -1 * y + 0.5,
        });
        this.group.children[1] = childGroup;
        this.group.children = [this.tag];
        this.group.add(childGroup);

    }
    getGroup() {
        return this.group;
    }
}