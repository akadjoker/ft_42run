import { getRandomInt } from './utils.js';

class Sprites 
{

    constructor()
    {
        this.vertex = []
        this.uv = []
    }
    
    render(texture, x, y, width, height)
    {
        let left = 0;        let right = 1;
        let top = 0;         let bottom = 1;
        
        let x1 =x;                let y1 =y;
        let x2 =x;                let y2 =y + height;
        let x3 =x + width;        let y3 =y + height;
        let x4 = x + width; let y4 = y;

        canvas.set_texture(texture);
        
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

var sprites = new Sprites();



function load()
{

    let model = scene.create_cube(1, 1, 1);
    let node = scene.create_static_node("cube", false); 

    scene.set_node_postion(node, 0, 1, 1);
    scene.node_add_model(node, model);

    assets.set_texture_path("assets/");

    let img = assets.load_texture("sprites/tiles3.png");
    assets.load_texture("sprites/tiles3.png");
    
    scene.set_model_texture(model, 0, img);

    console.log(getRandomInt(100));
    assets.set_texture_path("assets/textures/");
    

}

function unload()
{
    
}

function render()
{
    canvas.grid(10, 10,true);
}

function update(dt)
{
   
}



function gui()
{
    canvas.set_color(255,255,255)
  //  canvas.draw_texture(1, 10, 10, 100, 100)

    sprites.render(1, 100, 10, 100, 100)

    
    //line 1 tri 4 quad 8

    // SetMode(QUAD);

    // TexCoord2f(texcoords[0].x, texcoords[0].y);
    // Vertex2f(coords[0].x, coords[0].y);

    // TexCoord2f(texcoords[1].x, texcoords[1].y);
    // Vertex2f(coords[1].x, coords[1].y);

    // TexCoord2f(texcoords[2].x, texcoords[2].y);
    // Vertex2f(coords[2].x, coords[2].y);

    // TexCoord2f(texcoords[3].x, texcoords[3].y);
    // Vertex2f(coords[3].x, coords[3].y);
    
    // let left = 0;
    // let right = 1;
    // let top = 0;
    // let bottom = 1;



   
    // let x = 400;
    // let y = 300;

    // let width = 100;
    // let height = 100;
    

    
    // let x1 =x;                let y1 =y;
    // let x2 =x;                let y2 =y + height;
    // let x3 =x + width;        let y3 =y + height;
    // let x4 = x + width; let y4 = y;
    
    // canvas.texcoord(left, top);
    // canvas.vertex2(x1, y1);

    // canvas.texcoord(left, bottom);
    // canvas.vertex2(x2, y2);
    
    // canvas.texcoord(right, bottom);
    // canvas.vertex2(x3, y3);
    
    // canvas.texcoord(right, top);
    // canvas.vertex2(x4, y4);

    
}