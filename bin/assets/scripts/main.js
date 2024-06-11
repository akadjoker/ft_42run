include("assets/scripts/utils.js");
include("assets/scripts/game.js");




function load()
{
  
    
    // let model = scene.create_cube(1, 1, 1);
    // let node = scene.create_static_node("cube", false); 

    // scene.set_node_postion(node, 0, 1, 1);
    // scene.node_add_model(node, model);

    assets.set_texture_path("assets/");

//    let img = assets.load_texture("sprites/wabbit_alpha.png");
    
    
  //  scene.set_model_texture(model, 0, img);


    //assets.set_texture_path("assets/textures/");
   // scene.enable_3d(false)

    
    
    //canvas.load_font("assets/fonts/font3.fnt");
    canvas.load_font("assets/fonts/font1.fnt");
    canvas.load_font("assets/fonts/font2.fnt");


    screens.add(new MenuScreen("menu"));
    
    screens.add(new GameScreen("game"));

    screens.setScreen("menu");  


   // screens.setScreen("menu");

    
    

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

   
   
}



function gui()
{
   
   // canvas.set_color(255,255,255)
    


    // canvas.circle(100, 100, 23)
    // canvas.rect(100, 100, 23, 23, true)
    
    // canvas.set_font(1);
    
    // canvas.print(10, 30,  "Screen: " + screens.currentID())

    screens.render_gui()
    
}