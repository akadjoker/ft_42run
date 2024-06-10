function RandomInt(min, max)
{
    min = Math.ceil(min);
    max = Math.floor(max);
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

function RandomFloat(min, max)
{
    return Math.random() * (max - min) + min;
}

function Random(max)
{
    return Math.floor(Math.random() * Math.floor(max));
}

class Batch 
{

    constructor()
    {
        this.vertex = []
        this.uv = []
    }
    
    render( x, y, width, height)
    {
        let left = 0;        let right = 1;
        let top = 0;         let bottom = 1;
        
        let x1 =x;                let y1 =y;
        let x2 =x;                let y2 =y + height;
        let x3 =x + width;        let y3 =y + height;
        let x4 = x + width; let y4 = y;

        
   
        canvas.texcoord(left, top);
        canvas.vertex2(x1, y1);
    
        canvas.texcoord(left, bottom);
        canvas.vertex2(x2, y2);
        
        canvas.texcoord(right, bottom);
        canvas.vertex2(x3, y3);
        
        canvas.texcoord(right, top);
        canvas.vertex2(x4, y4);
    }
}

class Screen 
{

    constructor(name)
    {
        this.name = name
    }

    ready()
    {

    }


    update(dt) 
    {

    }

    render()
    {
    }

    render_gui()
    {

    }
    
}

class ScreenManager
{
    constructor()
    {
        this.screens = []
        this.current = null;
        this.new_screen = null;
    }

    add(screen)
    {
        this.screens.push(screen)
    }

    set(name)
    {
        for (let i = 0; i < this.screens.length; i++)
        {
            if (this.screens[i].name == name)
            {
                this.new_screen = this.screens[i];
                return true
            }
        }
        return false;
    }

    update(dt)
    {
        if (this.new_screen)
        {
            if (this.current)
                this.current.unload();
            this.current = this.new_screen;
            this.current.ready()
            this.new_screen = null
        }

        if (this.current)
        {
            this.current.update(dt)
        }
    }

    render()
    {
        if (this.current)
        {
            this.current.render()
        }
    }

    render_gui()
    {
        if (this.current)
        {
            this.current.render_gui()
        }
    }


}


var batch = new Batch();
var screens = new ScreenManager();