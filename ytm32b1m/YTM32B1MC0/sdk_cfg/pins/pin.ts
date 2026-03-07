import Konva from "konva";
import { LabelList } from "./lqfp.ts";

export const featureItemWidth = 220
export const featureItemHeight = 20

export const pinHeight = 25;
export const pinLength = 100;

const oneFeature = true
const Green = getVarColor('--el-color-success')
const Red = 'red'
const Blue = getVarColor('--el-color-primary')

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
export interface PinDef {
    label: string;
    num: string;
    type: string;
    list: string[];
    altList: string[];
    resetState: string;
}
export class Pin {
    rect: Konva.Rect;
    label: Konva.Text;
    featureText: Konva.Text;
    userLabelText: Konva.Text;
    hoverText: Konva.Text;
    hoverRect: Konva.Rect;
    hoverItem: Konva.Group;
    group: Konva.Group;
    err = false;
    list: Konva.Group | null = null;
    parent: Konva.Layer | Konva.Group;
    feature = '';
    userLabel = '';
    alt = 0;
    featureOpened = false;
    pinBlikTimer: number | undefined = undefined;
    defautFeat: string;
    activeColor: string;
    defaultColor: string;
    pin: PinDef;
    pos: string;
    hoverX: number;
    hoverY:number;
    pinHover: Konva.Label;
    pinFeature: LabelList;
    arrow: Konva.Arrow
    constructor(
        posX: number,
        posY: number,
        parent: Konva.Layer | Konva.Group,
        pin: PinDef,
        pos: string,
        featureChange: (number: string, label: string, feature: string, alt: string) => Promise<boolean>,
        pinHoverArea: Konva.Label,
        pinFeatureArea: LabelList,
        color = "#c8c9cc",

    ) {
        this.pos = pos;
        this.hoverX = posX;
        this.hoverY = posY;
        this.pin = pin;
        this.parent = parent;
        if (pin.type == 'power') {
            this.defaultColor = this.activeColor = getVarColor('--el-color-warning')
        } else {
            this.defaultColor = this.activeColor =  getVarColor('--el-color-info-light-5')
        }
        this.defautFeat = pin.resetState;
        this.pinHover = pinHoverArea;
        this.pinFeature = pinFeatureArea;

        /* This function add a Group for Pin, and init Pin's label view area */
        this.initPinLabelArea(posX, posY);
        /* This function init the hover view and the feature view's position */
        this.initHoverPos();

        this.initPinEvent(featureChange);
        // this.initPinFeatureArea(pinHoverArea);

        this.parent.add(this.group);
    }



    initPinLabelArea(posX: number, posY: number) {
        this.group = new Konva.Group({
            x: posX,
            y: posY,
            width: pinLength,
            height: pinHeight,
            draggable: false,
            rotation: this.pos2angel(this.pos),
        });

        this.rect = new Konva.Rect({
            width: pinLength,
            height: pinHeight,
            fill: this.defaultColor,
            stroke: getVarColor('black'),
            strokeWidth: 1,
        });
        this.label = new Konva.Text({
            text: this.pin.label,
            fontSize: 18,
            fontFamily: "Calibri",
            fill: getVarColor('black'),
            width: pinLength,
            padding: 5,
            align: "center",
            id: this.pin.num,
        });
        this.group.add(this.rect);
        this.group.add(this.label);

        this.featureText = new Konva.Text({
            text: "",
            x: 0,
            y: 0,
            fontSize: 18,
            fontFamily: "Calibri",
            fill: getVarColor("black"),
            padding: 5,
            align: "center",
        });
        this.userLabelText = new Konva.Text({
            text: "",
            x: 0,
            y: 0,
            fontSize: 18,
            fontFamily: "Calibri",
            fill: "gray",
            padding: 5,
            align: "center",
        });

        this.group.add(this.featureText);
        this.group.add(this.userLabelText);
    }
    
    initHoverPos() {
        switch (this.pos) {
            case 'up':
                this.hoverY += Math.round(this.group.width());
                this.hoverX -= Math.round(this.group.height() / 2);
                break;
            case 'down':
                this.hoverX -= Math.round(this.group.height() / 2);
                break;
            case 'left':
                this.hoverY += Math.round(this.group.height() / 2);
                this.hoverX += Math.round(this.group.width());
                break;
            default:
                this.hoverY += Math.round(this.group.height() / 2);
                break;
        }
    }


    renderFeatureArea(parent: Konva.Group, featureChange: (number: string, label: string, feature: string, alt: string) => Promise<boolean>) {
        /* Render label name */
        const labelGroup = new Konva.Group({
            x:0,
            y:0
        })
        
        const ft = new Konva.Text({
            x: 0,
            y: 0,
            width: featureItemWidth - 1,
            text: this.pin.label,
            fontSize: 14,
            padding: 2,
            fontFamily: "Calibri",
            fill: getVarColor('black'),
            align: "center",
        });
        labelGroup.add(
            new Konva.Rect({
                x: 0,
                y: 0,
                width: featureItemWidth - 1,
                height: featureItemHeight - 1,
                fill: getVarColor('--el-color-info-dark-2'),
                cornerRadius: [5, 5, 0, 0],
            })
        );
        labelGroup.add(ft)
        parent.add(labelGroup);
        /* Render first feature */
        for (let i = 0; i < this.pin.list.length-1; i++) {
            const fGroup = new Konva.Group({
                x: 0,
                y: 0 + (i + 1) * 20,
            })
            const t = new Konva.Text({
                width: featureItemWidth - 1,
                text: this.pin.list[i],
                fontSize: 14,
                padding: 2,
                fontFamily: "Calibri",
                fill: getVarColor('black'),
                align: "center",
            });

            fGroup.add(
                new Konva.Rect({
                    width: featureItemWidth - 1,
                    height: featureItemHeight - 1,
                    fill: getVarColor('--el-color-info-light-5'),
                })
            );

            fGroup.add(t);

            fGroup.add(
                new Konva.Line({
                    strokeWidth: 0.5,
                    stroke: getVarColor('--el-color-info-light-3'),
                    points: [0, featureItemHeight, featureItemWidth-1, featureItemHeight],
                })
            );
            parent.add(fGroup)
        }
        /* Render last feature */
        const lastFeatureGroup = new Konva.Group({
            x: 0,
            y: 0 + this.pin.list.length * 20,
        })
        const lt = new Konva.Text({
            x: 0,
            y: 0,
            width: featureItemWidth - 1,
            text: this.pin.list[this.pin.list.length-1],
            fontSize: 14,
            padding: 2,
            fontFamily: "Calibri",
            fill: getVarColor('black'),
            align: "center",
        });

        lastFeatureGroup.add(
            new Konva.Rect({
                x: 0,
                y: 0,
                width: featureItemWidth - 1,
                height: featureItemHeight - 1,
                fill: getVarColor('--el-color-info-light-5'),
                cornerRadius: [0, 0, 5, 5]
            })
        );
        lastFeatureGroup.add(lt);
        parent.add(lastFeatureGroup);
        for (let i=1; i<parent.children.length; i++) {
            const item = parent.children[i] as Konva.Group
            if (this.alt != 0 && i-1 == this.alt) {
                item.children[0].setAttr('fill', Green)
            } else {
                item.on('mouseenter', (evt)=>{
                    const stage = evt.target.getStage()
                    if (stage != null) {
                        stage.container().style.cursor = "pointer";
                    }
                    item.children[0].setAttr('fill', getVarColor('--el-color-info-light-3'))
                })
                item.on('mouseleave', (evt)=>{
                    const stage = evt.target.getStage()
                    if (stage != null) {
                        stage.container().style.cursor = "default";
                    }
                    item.children[0].setAttr('fill', getVarColor('--el-color-info-light-5'))
                })
            }

            item.on('click', (evt)=>{
                let newFeature;
                if (evt.target != this.group) {
                    newFeature = (evt.target.parent?.children[1] as Konva.Text).text();
                } else {
                    return;
                }
                    
                evt.cancelBubble = true;
                const altIndex = this.pin.list.indexOf(newFeature)
                let resetFeature=false
                if ((newFeature == this.defautFeat) || (this.feature == newFeature)){
                    resetFeature=true
                    newFeature=''
                }
                featureChange(this.pin.num, this.pin.label, newFeature, this.pin.altList[altIndex]).then((val: boolean) => {                  
                    if (val) {
                        if (resetFeature) {
                            this.feature = '';
                            this.userLabel = '';
                            this.setFeature('', '', undefined);
                            this.alt = 0;
                            this.activeColor = this.defaultColor
                            this.rect.setAttr("fill", this.defaultColor);
                            this.label.setAttr("fill", getVarColor('black'));
                        } else {
                            if (this.err) {
                                this.activeColor = Red
                            } else {
                                if(this.activeColor == this.defaultColor) {
                                    this.activeColor = Green
                                }
                            }
                            this.setFeature(newFeature, this.userLabel, this.activeColor);
                            this.feature = newFeature
                            this.alt = altIndex

                        }
                    }
                })
                this.pinFeature.getGroup().hide();
            })
        }
    }

    initPinEvent(featureChange: (number: string, label: string, feature: string, alt: string) => Promise<boolean>) {
        this.group.on("mouseenter", (evt) => {
            const stage = evt.target.getStage()
            if (stage != null) {
                stage.container().style.cursor = "pointer";
            }
            if (this.pinHover != undefined && !this.pinHover.visible() && !this.pinFeature.getGroup().visible()) {
                this.pinHover.setAttrs({
                    x:this.hoverX,
                    y:this.hoverY
                });
                this.pinHover.children[0].setAttr('pointerDirection', this.pos);
                this.pinHover.children[1].setAttr('text', `(${this.pin.num}) - ${this.pin.label} - ${this.feature != '' ? this.feature : this.defautFeat}`)
                this.pinHover.show();
            }
            this.rect.setAttr("fill", getVarColor('--el-color-info-dark-2'));
            // this.label.setAttr("fill", getVarColor('--el-text-color-primary'));
        });
        this.group.on("mouseleave", (evt) => {
            const stage = evt.target.getStage()
            if (stage != null) {
                stage.container().style.cursor = "default";
            }
            if (this.pinHover != undefined) {
                this.pinHover.hide();
            }
            this.rect.setAttr("fill", this.activeColor);
            // this.label.setAttr("fill", getVarColor("black"));
            
        });
        this.group.on("click", (evt) => {
            this.closeHighLightPin()
            evt.cancelBubble = true;
            
            if (this.pin.type == "gpio") {
                this.pinFeature.getGroup().setAttrs({
                    x: this.hoverX,
                    y: this.hoverY
                })
                const featureGroup = new Konva.Group()
                this.renderFeatureArea(featureGroup, featureChange)
                this.pinFeature.listUpdate(featureGroup, this.pos)
                this.pinFeature.getGroup().show();
            }
        });
    }

    removeFeature() {
        this.activeColor = this.defaultColor
        this.rect.setAttr("fill", this.defaultColor);
        this.label.setAttr("fill", getVarColor('black'));
        this.feature = ''
        this.featureText.text('');
        this.userLabelText.text('');
        this.alt = 0;
    }

    setFeature(val: string, userLabel: string, userColor: string|undefined) {
        this.feature = val
        this.userLabel = userLabel
        this.alt = this.pin.list.indexOf(val);
        if (this.err) {
            this.activeColor = Red
        } else {
            if (userColor != undefined) {
                this.activeColor = userColor;
            } else {
                this.activeColor = Green;
            }
        }
        this.rect.setAttr("fill", this.activeColor);
        this.label.setAttr("fill", getVarColor('black'));
        this.featureText.text(this.feature)
        let userLabelWidth;
        if (userLabel == '') {
            this.userLabelText.text("")
            userLabelWidth = 0;
        }else {
            this.userLabelText.text("["+userLabel+"]")
            userLabelWidth = this.userLabelText.width()
        }
        
        if (this.pos == "down" || this.pos == "right") {
            this.featureText.setPosition({
                x: pinLength,
                y: 0,
            });
            this.userLabelText.setPosition({
                x: pinLength + this.featureText.width(),
                y: 0,
            });

        } else {
            this.featureText.setPosition({
                x: -userLabelWidth-this.featureText.width(),
                y: 0,
            });
            this.userLabelText.setPosition({
                x: -userLabelWidth,
                y: 0,
            });
        }
    }

    setDirection(direction:string)
    {
        if(this.arrow){
            this.arrow.destroy()
        }
        let rotation = 0;
        let offset = 0;
        if(this.pos=='up'||this.pos=='left'){
            rotation = 180;
            offset=pinLength/2+18
        }
        if(direction=='GPIO_INPUT_DIRECTION'){
            //add a arrow
            this.arrow = new Konva.Arrow({
                x: pinLength-16-offset,
                y: pinHeight/2+1,
                points: [12, 0, 0, 0],
                pointerLength: 6,
                pointerWidth: 6,
                fill: getVarColor('--el-fill-color-lighter'),
                stroke: getVarColor('--el-fill-color-lighter'),
                strokeWidth: 1,
                rotation: rotation,
            });
            this.group.add(this.arrow);
        }else if(direction=='GPIO_OUTPUT_DIRECTION'){
            //add a arrow
            this.arrow = new Konva.Arrow({
                x: pinLength-16-offset,
                y: pinHeight/2+1,
                points: [0, 0, 12, 0],
                pointerLength: 6,
                pointerWidth: 6,
                fill: getVarColor('--el-fill-color-lighter'),
                stroke: getVarColor('--el-fill-color-lighter'),
                strokeWidth: 1,
                rotation: rotation,
            });
            this.group.add(this.arrow);
        }
    }

    pos2angel(pos: string): number {
        if (pos == "up") {
            return 90;
        } else if (pos == "left") {
            return 0;
        } else if (pos == "down") {
            return 90;
        } else {
            return 0;
        }
    }
    getDefaultFeat() {
        return this.defautFeat
    }
    getChosedFeature() {
        return this.feature
    }
    highLightPin() {
        if (this.pinBlikTimer != undefined) {
            return;
        }
        let cnt = 1;
        this.rect.setAttr('fill', Blue)
        this.pinBlikTimer = window.setInterval(() => {
            if (cnt == 0) {
                cnt = 1;
                this.rect.setAttr('fill', Blue)
            } else {
                cnt = 0;
                this.rect.setAttr('fill', this.activeColor)
            }
        }, 500)
    }
    errorPin(enable: boolean) {
        this.err = enable
        if (enable) {
            this.activeColor = Red
            this.rect.setAttr('fill', this.activeColor);
        } else {
            if (this.feature != '') {
                this.activeColor = Green
            } else {
                this.activeColor = this.defaultColor
            }
            this.rect.setAttr('fill', this.activeColor)
        }
    }
    closeHighLightPin() {
        window.clearInterval(this.pinBlikTimer)
        this.pinBlikTimer = undefined;
        this.rect.setAttr('fill', this.activeColor)
    }
    setHoverText(text: string) {
        this.hoverText.setText(text);
        this.group.draw();
    }
    setRectColor(color: string) {
        this.activeColor = color;
        this.rect.setAttr("fill", color);
    }
    hideFeature() {
        this.featureOpened = false;
        this.list?.hide();
    }
}

