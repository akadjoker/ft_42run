include("assets/scripts/utils.js");



class Bunny
{
    constructor()
    {
        this.x = Random(0, 840);
        this.y = Random(0, 450);
        this.speedX = Random(-256,256)/60.0
        this.speedY = Random(-256,256)/60.0
    }

    update()
    {
        this.x += this.speedX
        this.y += this.speedY

        if (this.x < 0 || this.x > 840)
        {
            this.speedX = -this.speedX
        }
        if (this.y < 0 || this.y > 450)
        {
            this.speedY = -this.speedY
        }
        // batch.render(1, this.x, this.y,32,32);
        canvas.draw_texture(1, this.x, this.y,32,32);
    }
}



let bunnies =[]

function load()
{

    let model = scene.create_cube(1, 1, 1);
    let node = scene.create_static_node("cube", false); 

    scene.set_node_postion(node, 0, 1, 1);
    scene.node_add_model(node, model);

    assets.set_texture_path("assets/");

    let img = assets.load_texture("sprites/wabbit_alpha.png");
    
    
    scene.set_model_texture(model, 0, img);


    assets.set_texture_path("assets/textures/");
   // scene.enable_3d(false)

    canvas.load_font("assets/fonts/font1.fnt");

    
    

}

function unload()
{
    
}

function render()
{
    canvas.grid(10, 10, true);
    screens.render()
}

function update(dt)
{
 
    screens.update(dt)
    
    if (mouse.down(0))
        {
            
             for (let i = 0; i < 100; i++)
            {
                bunnies.push(new Bunny())
            }
        }
}



function gui()
{
    screens.render_gui()
    canvas.set_color(255,255,255)
    canvas.set_texture(1);
    


      for (let i = 0; i < bunnies.length; i++)
        {
            bunnies[i].update()
        }

    canvas.circle(100, 100, 23)
    canvas.rect(100, 100, 23, 23, true)
    
    canvas.set_font(1);
    
    canvas.print(10, 30, "bunnies: " + bunnies.length)

    
}