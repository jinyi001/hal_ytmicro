export function makeResizable(element:HTMLElement,dir='top',callback?:(a:number,b?:number)=>void,minH=0,minW=0,size=20)
{

  function childExists(className: string): boolean {
    return !!element.querySelector(`.${className}`);
  }

  if(dir=='top'){
    if (!childExists('resizable-top')) {
      const top = document.createElement('div');
      top.style.width = '100%';
      top.style.height = size + 'px';
      top.style.backgroundColor = 'transparent';
      top.style.position = 'absolute';
      top.style.top = - (size/2) + 'px';
      top.style.left = '0px';
      top.style.cursor = 'n-resize';
      top.classList.add('resizable-top');

      top.addEventListener('mousedown',resizeYNegative())

      element.appendChild(top);
    }
  }else if(dir=='bottom'){
    if (!childExists('resizable-bottom')) {
      const bottom = document.createElement('div');
      bottom.style.width = '100%';
      bottom.style.height = size + 'px';
      bottom.style.backgroundColor = 'transparent';
      bottom.style.position = 'absolute';
      bottom.style.bottom = - (size/2) + 'px';
      bottom.style.left = '0px';
      bottom.style.cursor = 'n-resize';
      bottom.classList.add('resizable-bottom');
      bottom.addEventListener('mousedown',resizeYPositive())
      
      element.appendChild(bottom);
    }
  }else if(dir=='left'){
    if (!childExists('resizable-left')) {
      const left = document.createElement('div');
      left.style.width = size + 'px';
      left.style.height = '100%';
      left.style.backgroundColor = 'transparent';
      left.style.position = 'absolute';
      left.style.top = '0px';
      left.style.left = - (size/2) + 'px';
      left.style.cursor = 'e-resize';
      left.classList.add('resizable-left');

      left.addEventListener('mousedown',resizeXNegative())
      element.appendChild(left);
    }

  }else{
    if (!childExists('resizable-right')) {
      const right = document.createElement('div');
      right.style.width = size + 'px';
      right.style.height = '100%';
      right.style.backgroundColor = 'transparent';
      right.style.position = 'absolute';
      right.style.top = '0px';
      right.style.right = - (size/2) + 'px';
      right.style.cursor = 'e-resize';
      right.classList.add('resizable-right'); 

      right.addEventListener('mousedown',resizeXPositive())

      element.appendChild(right);
    }


    // const corner4 = document.createElement('div');
    // corner4.style.width = size + 'px';
    // corner4.style.height = size + 'px';
    // corner4.style.backgroundColor = 'transparent';
    // corner4.style.position = 'absolute';
    // corner4.style.bottom = - (size/2) + 'px';
    // corner4.style.right = - (size/2) + 'px';
    // corner4.style.cursor = 'se-resize';

    // corner4.addEventListener('mousedown',resizeXPositive())
    // corner4.addEventListener('mousedown',resizeYPositive())

    // element.appendChild(corner4);


    
  }
    

    

    

    

    


   

   



    
    
    function get_int_style(key:string)
    {
        return parseInt(window.getComputedStyle(element).getPropertyValue(key));
    }

    function resizeXPositive()
    {
        let offsetX:number
        function dragMouseDown(e:MouseEvent) {
            if(e.button !== 0) return
            e = e || window.event;
            e.preventDefault();
            const {clientX} = e;
            offsetX = clientX - element.offsetLeft - get_int_style('width');
            document.addEventListener('mouseup', closeDragElement)
            document.addEventListener('mousemove', elementDrag)
          }
        
          function elementDrag(e:MouseEvent) {
                const {clientX} = e;
                let x = clientX - element.offsetLeft - offsetX
                if(x < minW) x = minW;
                if (callback){
                    callback(x)
                }else{
                    element.style.width =  x + 'px';
                }
          }
        
          function closeDragElement() {
            document.removeEventListener("mouseup", closeDragElement);  
            document.removeEventListener("mousemove", elementDrag);
          }
        return dragMouseDown
    }

    function resizeYPositive()
    {
        let offsetY:number
        function dragMouseDown(e:MouseEvent) {
            if(e.button !== 0) return
            e = e || window.event;
            e.preventDefault();
            const {clientY} = e;
            offsetY = clientY - element.offsetTop - get_int_style('height');
    
            document.addEventListener('mouseup',closeDragElement)
            document.addEventListener('mousemove',elementDrag)
          }
        
          function elementDrag(e:MouseEvent) {
                const {clientY} = e;
                let y =  clientY - element.offsetTop - offsetY;
                if(y < minH) y = minH;
                
                if (callback){
                    callback(y)
                }else{
                    element.style.height = y + 'px';
                }
          }
        
          function closeDragElement() {
            document.removeEventListener("mouseup", closeDragElement);  
            document.removeEventListener("mousemove", elementDrag);
          }
        return dragMouseDown
    }

    function resizeXNegative()
    {
        let offsetX:number
        let startX:number
        let startW:number
        let maxX:number
        function dragMouseDown(e:MouseEvent) {
            if(e.button !== 0) return
            e = e || window.event;
            e.preventDefault();
            const {clientX} = e;
            startX = get_int_style('left')
            startW = get_int_style('width')
            offsetX = clientX - startX;
            maxX = startX + startW - minW
    
            document.addEventListener('mouseup',closeDragElement)
            document.addEventListener('mousemove',elementDrag)
          }
        
          function elementDrag(e:MouseEvent) {
                const {clientX} = e;
                let x = clientX - offsetX
                let w = startW + startX - x
                if(w < minW) w = minW;
                if(x > maxX) x = maxX;
               
                if(callback){
                    callback(w,x)
                }else{
                    element.style.left = x + 'px';
                    element.style.width = w + 'px';
                }
          }
        
          function closeDragElement() {
            document.removeEventListener("mouseup", closeDragElement);  
            document.removeEventListener("mousemove", elementDrag);
          }
        return dragMouseDown
    }

    function resizeYNegative()
    {
        let offsetY:number
        let startY:number
        let startH:number
        let maxY:number
        function dragMouseDown(e:MouseEvent) {
            if(e.button !== 0) return
            e = e || window.event;
            e.preventDefault();
            const {clientY} = e;
            startY = get_int_style('top')
            startH = get_int_style('height')
            offsetY = clientY - startY;
            maxY = startY + startH - minH 
    
            document.addEventListener('mouseup',closeDragElement,false)
            document.addEventListener('mousemove',elementDrag,false)
          }
        
          function elementDrag(e:MouseEvent) {
                const {clientY} = e;
                let y =  clientY - offsetY
                let h = startH + startY - y
                if(h < minH) h = minH;
                if(y > maxY) y = maxY;
                

                if(callback){
                    callback(h,y)
                }else{
                    element.style.top = y + 'px';
                    element.style.height = h + 'px';
                }
                
          }
        
          function closeDragElement() {
            document.removeEventListener("mouseup", closeDragElement);  
            document.removeEventListener("mousemove", elementDrag);
          }
        return dragMouseDown
    }
}