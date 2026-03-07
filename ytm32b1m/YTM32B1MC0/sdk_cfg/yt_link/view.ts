class EventEmitter {
    callbacks: any
    constructor() {
        this.callbacks = {}
    }

    on(event, cb) {
        if (!this.callbacks[event]) this.callbacks[event] = [];
        this.callbacks[event].push(cb)
    }

    emit(event, data) {
        const cbs = this.callbacks[event]
        if (cbs) {
            cbs.forEach(cb => cb(data))
        }
    }
    removeAllListeners() {
        this.callbacks = {}
    }
}


export function generateUUID() {
    return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function (c) {
        const r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8);
        return v.toString(16);
    });
}
export interface LinkItem {
    name: string,
    id: string,
    level: number,
    isFlash?: boolean,
    children?: LinkItem[],
    order?: number,
    boundary?: 'UPPER' | 'LOWER',
    InitStage?: number
    size?: string,
    copyFrom?: string,
    startAddress?: string,
    binFile?: string,
    symbolName?: string,
    x?: number,
    y?: number,
    align?: number,
    endAlign?: number,
    flags?: string,
    groupSize?: string,
    altSymbolStart?: string,
    altSymbolEnd?: string,
    wildcard?: boolean,
    init?: string,
    clear?: boolean
    removeDot?: boolean
}
export const colorMap = [
    {
        fill: '#0056B3',
        color: '#FFFFFF'
    },
    {
        fill: '#008000',
        color: '#FFFFFF'
    },
    {
        fill: '#800080',
        color: '#FFFFFF'
    },
    {
        fill: '#FFFF00',
        color: '#000000'
    },
    {
        fill: '#00FFFF',
        color: '#000000'
    }
]


export const titleMap: Record<number, string> = { 1: 'Memory', 2: 'Group', 3: 'Section', 4: 'Region' }
class Region extends joint.dia.Element {
    defaults() {
        return joint.util.defaultsDeep({
            type: 'link.region',
            attrs: {
                body: {
                    width: 'calc(w)',
                    height: 'calc(h)',
                    strokeWidth: 2,
                    stroke: '#000000',
                    fill: '#FFFFFF'
                },
                labelTop: {
                    x: 'calc(1*w)',
                    y: '10',
                    fontSize: 10,
                    fill: '#333333',
                    'font-weight': 700
                },
                label: {
                    textVerticalAnchor: 'middle',
                    textAnchor: 'middle',
                    refX: '50%',
                    refY: '50%',
                    fontSize: 12,
                    fill: '#333333',
                    textWrap: {
                        width: -10, // 相对于元素宽度的偏移
                        height: 22, // 设置为与字体大小相同的固定高度
                        ellipsis: true // 如果文本太长，以省略号结尾
                    }
                },
                labelBottom: {

                    x: 'calc(w)',
                    y: 'calc(1*h-5)',
                    fontSize: 10,
                    fill: '#333333',
                    'font-weight': 700
                },
                title: {
                    x: 'calc(0.5*w)',
                    y: '7',
                    textVerticalAnchor: 'middle',
                    textAnchor: 'middle',
                    fontSize: 8,
                    fill: '#333333'
                },
                line: {
                    x1: 0,
                    y1: 14,
                    x2: 'calc(w)',
                    y2: 14,
                    stroke: '#000000',
                    strokeWidth: 1
                },
                cornerCircle: {
                    cx: 'calc(w - 12)', // 圆心x坐标，这里假设圆形位于矩形的右上角
                    cy: 'calc(h - 12)', // 圆心y坐标
                    r: 8, // 圆的半径
                    fill: '#FF0000', // 填充颜色
                },
                cornerText: {
                    x: 'calc(w - 12)', // 文本的x坐标，使其居中于圆形
                    y: 'calc(h - 8)', // 文本的y坐标
                    text: '1', // 显示的文本
                    fontSize: 12, // 字体大小
                    textAnchor: 'middle', // 文本对齐方式，使其水平居中
                    fill: '#FFFFFF', // 字体颜色
                },
                cornerText1: {
                    x: '16', // 文本的x坐标，使其居中于圆形
                    y: 'calc(h - 4)', // 文本的y坐标
                    text: '1', // 显示的文本
                    fontSize: 12, // 字体大小

                    fill: '#FFFFFF', // 字体颜色
                },
            }
            // eslint-disable-next-line @typescript-eslint/ban-ts-comment
            /*@ts-ignore*/
        }, super.defaults);
    }
    markup = [{
        tagName: 'rect',
        selector: 'body',
    }, {
        tagName: 'text',
        selector: 'labelTop',
    }, {
        tagName: 'text',
        selector: 'label',
    }, {
        tagName: 'text',
        selector: 'labelBottom',
    },

    {
        tagName: 'text',
        selector: 'title',
    },
    {
        tagName: 'line',
        selector: 'line'
    },
    {
        tagName: 'circle',
        selector: 'cornerCircle',
    },
    {
        tagName: 'text',
        selector: 'cornerText',
    },
    {
        tagName: 'text',
        selector: 'cornerText1',
    },
    ]
}

const levelButtons = [{
    hasEditButton: false,
    hasAddButton: true,
    hasRemoveButton: false,
},
{
    hasEditButton: true,
    hasAddButton: true,
    hasRemoveButton: true,
},
{
    hasEditButton: true,
    hasAddButton: true,
    hasRemoveButton: true,
},
{
    hasEditButton: true,
    hasAddButton: true,
    hasRemoveButton: true,
},
{
    hasEditButton: true,
    hasAddButton: false,
    hasRemoveButton: true,
}
]




export class Ceil {
    paper: joint.dia.Paper
    graph: joint.dia.Graph
    rect: joint.shapes.standard.Rectangle
    events: EventEmitter = new EventEmitter()
    children: Ceil[] = []
    data: LinkItem
    title?: string
    parent?: Ceil

    constructor(paper: joint.dia.Paper, graph: joint.dia.Graph, e: LinkItem, parent?: Ceil, title?: string) {

        this.paper = paper
        this.graph = graph
        this.parent = parent
        const width = 150
        let height = 50
        this.data = e
        this.title = title
        let x = e.x 
        let y = e.y

        if(x==undefined){
            x=  (100 + e.level * 150)
        }
        if(y==undefined){
            y=  (100 + e.level * 50)
        }


        if (e.size != undefined) {
            height = Number(e.size) / 1024
            if (height < 50) {
                height = 50
            }
            if (height > 400) {
                height = 400
            }
        }

        this.rect = new Region({
            position: { x: x, y: y },
            size: { width: width, height: height },
            attrs: {
                body: {
                    fill: colorMap[e.level].fill,
                    opacity: e.isFlash ? 0.5 : 1
                },
                label: {
                    text: e.name,
                    fontSize: 16,
                    fill: colorMap[e.level].color

                },

            },
            meta: this
        });


        this.graph.addCell(this.rect);
        // (this.rect as any).on('change:position', function (cell: any) {
        //     const center = cell.getBBox();
        //     const c = cell.get('meta') as Ceil
        //     if (c) {
        //         c.data.x = center.x
        //         c.data.y = center.y
        //     }
        // });


        if (e.order != undefined) {
            this.rect.attr('cornerText/text', e.order.toString())
            //color
            this.rect.attr('cornerCircle/fill', colorMap[e.level].fill)
            this.rect.attr('cornerText/fill', colorMap[e.level].color)
            //stoke
            this.rect.attr('cornerCircle/stroke', colorMap[e.level].color)
        } else {
            this.rect.attr('cornerCircle/display', 'none')
            this.rect.attr('cornerText/display', 'none')
        }

        if (e.flags != undefined) {
            this.rect.attr('cornerText1/text', e.flags.toString())
            //color
            this.rect.attr('cornerText1/fill', colorMap[e.level].color)
            //stoke
            this.rect.attr('cornerText1/stroke', colorMap[e.level].color)
        } else {
            this.rect.attr('cornerText1/display', 'none')
        }

        if (e.level == 0) {
            let usedSize = 0
            if (e.children) {
                for (const c of e.children) {
                    usedSize += Number(c.size)
                }
            }
            const percent = (usedSize * 100 / Number(e.size)).toFixed(2) + '%'
            this.changeFlag(percent)
        }

        if (e.startAddress != undefined && e.size) {
            //set label font-weight
            this.rect.attr('labelTop', {
                text: '0x' + Number(e.startAddress).toString(16),
                fill: 'var(--el-text-color-primary)',

            })
            this.rect.attr('labelBottom', {
                text: '0x' + (Number(e.startAddress) + Number(e.size) - 1).toString(16),
                fill: 'var(--el-text-color-primary)',
            })
        } else {
            this.rect.attr('labelTop/display', 'none')
            this.rect.attr('labelBottom/display', 'none')
        }
        if (title) {
            this.rect.attr('title', {
                text: title,
                fill: colorMap[e.level].color
            })
            //make name label lower
            this.rect.attr('label/refY', '56%')


        } else {
            /* remove line */
            this.rect.attr('line/display', 'none')
        }



        if (parent) {
            /* add link */
            const link = new joint.shapes.standard.Link({
                attrs: {
                    line: {
                        stroke: 'var(--el-text-color-primary)',
                        strokeWidth: 1,
                        targetMarker: {
                            type: 'path',
                            d: 'M 5 -2.5 0 0 5 2.5 z',
                            fill: 'var(--el-text-color-primary)',
                            stroke: 'var(--el-text-color-primary)',
                        },
                    },
                },

                connector: {
                    name: 'smooth'
                }

            });
            link.target(parent.rect);
            link.source(this.rect);
            link.addTo(graph);
            parent.children.push(this)

            // if(parent.data.children==undefined){
            //     parent.data.children=[]
            // }
            // parent.data.children.push(e)
        }
        const view = this.rect.findView(paper)
        if (view) {
            const boundaryTool = new joint.elementTools.Boundary();
            const toolsList = [boundaryTool]
            if (levelButtons[e.level].hasEditButton) {
                const editButtonClass = (joint.elementTools.Button as any).extend({
                    name: 'edit-button',
                    options: {
                        markup: [{
                            tagName: 'circle',
                            selector: 'button',
                            attributes: {
                                'r': 7,
                                'fill': 'var(--el-color-warning)',
                                'cursor': 'pointer'
                            }
                        }, {
                            tagName: 'path',
                            selector: 'icon',
                            attributes: {
                                'd': ['M7 7H6a2 2 0 0 0-2 2v9a2 2 0 0 0 2 2h9a2 2 0 0 0 2-2v-1', 'M20.385 6.585a2.1 2.1 0 0 0-2.97-2.97L9 12v3h3zM16 5l3 3'],
                                'fill': 'none',
                                'stroke': '#FFFFFF',
                                'stroke-width': 2,
                                'pointer-events': 'none',
                                'transform': ['translate(-5.5,-5.5)', 'scale(0.45)'],
                            }
                        }],
                        x: '0%',
                        y: '100%',
                        offset: {
                            x: 8,
                            y: -8,
                        },
                        rotate: true,
                        action: () => {
                            this.events.emit('edit', this)
                        }
                    }
                });
                const edit = new editButtonClass();
                toolsList.push(edit)
            }
            if (levelButtons[e.level].hasAddButton) {
                if (view && levelButtons[e.level].hasAddButton) {
                    const addClass = (joint.elementTools.Button as any).extend({
                        name: 'add-button',
                        options: {
                            markup: [{
                                tagName: 'circle',
                                selector: 'button',
                                attributes: {
                                    'r': 7,
                                    'fill': 'var(--el-color-primary)',
                                    'cursor': 'pointer'
                                }
                            }, {
                                tagName: 'path',
                                selector: 'icon',
                                attributes: {
                                    'd': ['M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6z'],
                                    'fill': 'none',
                                    'stroke': '#FFFFFF',
                                    'stroke-width': 2,
                                    'pointer-events': 'none',
                                    'transform': ['translate(-5.5,-5.5)', 'scale(0.45)'],
                                }
                            }],
                            x: '0%',
                            y: '100%',
                            offset: {
                                x: 8,
                                y: -25,
                            },
                            rotate: true,
                            action: () => {
                                this.events.emit('add', this)
                            }
                        }
                    });

                    const add = new addClass();
                    toolsList.push(add)
                }
            }
            if (levelButtons[e.level].hasRemoveButton) {
                const removeTools = new joint.elementTools.Remove({
                    x: '0%',
                    y: '100%',
                    offset: {
                        x: 8,
                        y: -42,
                    },
                    action: () => {
                        this.events.emit('remove', this)
                    }
                });
                toolsList.push(removeTools)
            }


            const toolsView = new joint.dia.ToolsView({
                tools: toolsList
            });
            view.addTools(toolsView);
            view.hideTools();
        }
    }
    getId() {
        return this.data.id
    }
    on(event: 'edit' | 'add' | 'remove', cb: (ceil: Ceil) => void) {
        this.events.on(event, cb)
    }
    changeName(name: string) {
        this.data.name = name
        this.rect.attr('label/text', name)
    }
    changeAddrSize(startAddress: string, size: string) {
        this.data.startAddress = startAddress
        this.data.size = size
        this.rect.attr('labelTop/text', '0x' + Number(startAddress).toString(16))
        this.rect.attr('labelBottom/text', '0x' + (Number(startAddress) + Number(size) - 1).toString(16))
        //display
        this.rect.attr('labelTop/display', 'block')
    }
    changeTitle(title: string) {
        this.title = title
        this.rect.attr('title/text', title)
        this.rect.attr('line/display', 'block')
    }
    changeOrder(order: number) {
        this.data.order = order
        this.rect.attr('cornerText/text', order.toString())
    }
    setEnable(enable: boolean) {
        if (enable) {
            this.rect.attr('body/opacity', 1)

        } else {
            this.rect.attr('body/opacity', 0.2)
        }
    }
    changeFlag(flags: string) {
        this.data.flags = flags
        this.rect.attr('cornerText1/text', flags)
        this.rect.attr('cornerText1/display', 'block')
        this.rect.attr('cornerText1/fill', colorMap[this.data.level].color)
        this.rect.attr('cornerText1/stroke', colorMap[this.data.level].color)
    }
    destroy() {
        this.rect.remove()
        this.events.removeAllListeners()
    }
}

export class LinkView {
    paper?: joint.dia.Paper;
    graph: joint.dia.Graph;
    ceilMap: Map<string, Ceil> = new Map();
    linkMap: Map<string, {
        sourceId: string,
        linkId: string
    }> = new Map();
    itemMap: Map<string, LinkItem> = new Map();
    panning = false;
    events: EventEmitter = new EventEmitter()
    panStartPosition = { x: 0, y: 0 };
    constructor(graph: joint.dia.Graph, paper?: joint.dia.Paper) {
        this.paper = paper;
        this.graph = graph;
    }
    setPaper(paper: joint.dia.Paper) {
        this.paper = paper;
        paper.on("element:mouseenter", function (elementView) {
            elementView.showTools();
        });

        paper.on("element:mouseleave", function (elementView) {
            elementView.hideTools();
        });
        paper.on('cell:pointerup', (cellView, evt, x, y) => {
            const ceil = cellView.model
            if (ceil) {
                const center = ceil.getBBox();
                const c = ceil.get('meta') as Ceil
                if (c && c.data) {
                    c.data.x = center.x
                    c.data.y = center.y
                }
            }
        });

        // Mouse down event to start panning
        paper.on("blank:pointerdown", (event, x, y) => {
            this.panning = true;
            this.panStartPosition.x = x;
            this.panStartPosition.y = y;
            //add class to body to change the cursor icon
            document.body.classList.add("is-panning");

            // prevent text selection
            event.preventDefault();
        });

        // Mouse move event to handle panning
        paper.on(
            "cell:pointermove blank:pointermove",
            (event: any, x: number, y: number) => {
                if (this.panning && paper) {
                    // Calculate the distance the mouse has moved
                    const dx = x - this.panStartPosition.x;
                    const dy = y - this.panStartPosition.y;
                    // Move the paper by that distance
                    paper.translate(
                        paper.translate().tx + dx,
                        paper.translate().ty + dy
                    );
                }
            }
        );

        // Mouse up event to end panning
        paper.on(
            "cell:pointerup blank:pointerup",
            (event: any, x: number, y: number) => {
                if (this.panning && paper) {
                    paper.translate(
                        paper.translate().tx + (x - this.panStartPosition.x),
                        paper.translate().ty + (y - this.panStartPosition.y)
                    );
                    this.panning = false;
                    //remove class from body to change the cursor icon back
                    document.body.classList.remove("is-panning");
                }
            }
        );
    }
    refreshLevel1(parentId?: string) {
        if (!parentId) {
            return;
        }
        const parent = this.ceilMap.get(parentId);
        if (parent) {
            if (parent.data.level == 0) {
                const children = parent.children;
                if (children) {
                    const upper = children.filter((item) => item.data.boundary == "UPPER");
                    const lower = children.filter((item) => item.data.boundary == "LOWER");
                    /* sort by order, if order is same, sort by name, store the sort result in new val*/
                    const upperSort = _.sortBy(upper, ["data.order", "data.name"]);
                    let totalSIze = 0;
                    /* calculate the start address of each block */
                    let startAddress = Number(parent.data.startAddress);
                    for (let i = 0; i < upperSort.length; i++) {
                        totalSIze += Number(upperSort[i].data.size);
                        const strAddr = '0x' + startAddress.toString(16);
                        upperSort[i].data.startAddress = strAddr
                        upperSort[i].changeAddrSize(
                            strAddr,
                            upperSort[i].data.size || "0"
                        );
                        startAddress += Number(upperSort[i].data.size);
                    }

                    const lowerSort = _.sortBy(lower, ["data.order", "data.name"]);

                    startAddress =
                        Number(parent.data.startAddress) + Number(parent.data.size);
                    for (let i = lowerSort.length - 1; i >= 0; i--) {
                        totalSIze += Number(lowerSort[i].data.size);
                        startAddress -= Number(lowerSort[i].data.size);
                        const strAddr = '0x' + startAddress.toString(16);
                        lowerSort[i].data.startAddress = strAddr;
                        lowerSort[i].changeAddrSize(
                            strAddr,
                            lowerSort[i].data.size || "0"
                        );
                    }
                    /* change percent in size, with xx% format */
                    const percent = (totalSIze / Number(parent.data.size)) * 100;
                    parent.changeFlag(percent.toFixed(2) + "%");
                }
            }
        }
    }
    addCeil(link: LinkItem, parent?: Ceil,) {
        let pendingLink: any = undefined
        const ceil = new Ceil(this.paper!, this.graph, link, parent, titleMap[link.level]);
        this.ceilMap.set(ceil.getId(), ceil);
        if (link.copyFrom) {
            if (!this.itemMap.has(link.copyFrom)) {
                pendingLink = [link.copyFrom, ceil.getId()]
            } else {
                this.addLink(link.copyFrom, ceil.getId())
            }
        }
        this.itemMap.set(ceil.getId(), link)
        ceil.on('edit', (ceil) => {
            this.events.emit('edit', ceil)
        })
        ceil.on('add', (ceil) => {
            this.events.emit('add', ceil)
        })
        ceil.on('remove', (ceil) => {
            this.events.emit('remove', ceil)
        })
        return { ceil, pendingLink };
    }
    addLink(targetId: string, sourceId: string) {
        const source = this.ceilMap.get(sourceId)
        const target = this.ceilMap.get(targetId)

        if (source && target) {
            /* 虚线link*/
            const link = new joint.shapes.standard.Link({
                attrs: {
                    line: {
                        stroke: 'var(--el-text-color-primary)',
                        strokeWidth: 1,
                        strokeDasharray: '5 2', // 定义虚线的样式
                        targetMarker: {
                            type: 'path',
                            d: 'M 5 -2.5 0 0 5 2.5 z',
                            fill: 'var(--el-text-color-primary)',
                            stroke: 'var(--el-text-color-primary)',
                        },
                    },
                },

                connector: {
                    name: 'smooth'
                }

            });
            link.target(source.rect);
            link.source(target.rect);
            link.addTo(this.graph);
            this.linkMap.set(target.getId(), { sourceId: source.getId(), linkId: link.id as string })
        }
    }
    removeLink(id: string) {
        if (this.linkMap.has(id)) {
            const e = this.linkMap.get(id)
            if (e) {
                const link = this.graph.getCell(e.linkId)
                if (link) {
                    link.remove()
                }
            }
            this.linkMap.delete(id)
        }
    }
    removeCeil(id: string) {
        const ceil = this.ceilMap.get(id)
        if (ceil) {
            this.itemMap.delete(id)
            if (ceil.children) {
                const idMap = ceil.children.map((e) => e.getId())
                for (const c of idMap) {
                    this.removeCeil(c)
                }
            }
            if (ceil.data.copyFrom) {
                this.removeLink(ceil.data.copyFrom)
            }
            if (this.linkMap.has(id)) {
                const e = this.linkMap.get(id)
                if (e) {

                    const source = this.ceilMap.get(e.sourceId)
                    if (source) {
                        source.data.copyFrom = undefined
                    }
                    this.linkMap.delete(id)
                }
            }

            ceil.destroy()
            this.ceilMap.delete(id)
            if (ceil.parent) {
                const index = ceil.parent.children.findIndex((e) => e.getId() == id)
                if (index != -1) {
                    ceil.parent.children.splice(index, 1)
                }
                if (ceil.parent.data.children) {
                    const index2 = ceil.parent.data.children.findIndex((e) => e.id == id)
                    if (index2 != -1) {
                        ceil.parent.data.children.splice(index2, 1)
                    }
                }
            }
        }
    }
    buildCeilTree(links: LinkItem[], parent?: Ceil) {
        let pendingLinks: any[] = []
        for (const l of links) {
            const { ceil, pendingLink } = this.addCeil(l, parent)
            if (pendingLink) {
                pendingLinks.push(pendingLink)
            }
            if (l.children && l.children.length > 0) {
                pendingLinks = pendingLinks.concat(this.buildCeilTree(l.children, ceil))
            }
        }
        return pendingLinks
    }
    on(event: 'edit' | 'add' | 'remove', cb: (ceil: Ceil) => void) {
        this.events.on(event, cb)
    }
    clear() {
        this.ceilMap.clear()
        this.linkMap.clear()
        this.itemMap.clear()
    }
    destroy() {
        this.clear()
        this.events.removeAllListeners()
        for (const ceil of this.ceilMap.values()) {
            ceil.destroy()
        }
    }
}