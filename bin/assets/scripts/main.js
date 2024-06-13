include("assets/scripts/utils.js");
include("assets/scripts/game.js");

console.log("Starting...****************************");


function load()
{
  
    core.set_key_exit(Key.F2);
    
    // let model = scene.create_cube(1, 1, 1);
    // let node = scene.create_static_node("cube", false); 

    // scene.set_node_postion(node, 0, 1, 1);
    // scene.node_add_model(node, model);

    assets.set_texture_path("assets/");
    assets.set_texture_load_flip(true);
      
        // assets.load_texture("model/diffuse_back.jpg");
        // assets.load_texture("model/glasses.jpg");
        // assets.load_texture("model/luis_eyeL_hi.jpg");
        // assets.load_texture("model/luis_eyeR_hi.jpg");
        // assets.load_texture("model/face.jpg");
        // assets.load_texture("model/shoes.jpg");
        // assets.load_texture("model/skeen.jpg");




         scene.load_entity("assets/model/main.ah3d", "player", false);
        scene.entity_add_animation(0, "assets/model/idle.anim");
        scene.entity_play(0, "idle", 1, 0.25);


        // scene.set_entity_texture(0, 0, 1);
        // scene.set_entity_texture(0, 1, 2);
        // scene.set_entity_texture(0, 2, 3);
        // scene.set_entity_texture(0, 3, 4);
        // scene.set_entity_texture(0, 4, 5);
        // scene.set_entity_texture(0, 5, 6);
        // scene.set_entity_texture(0, 6, 7);


        scene.create_plane(10, 10, 5,5, 1, 1);
        scene.create_static_node("plane", true);

//    let img = assets.load_texture("sprites/wabbit_alpha.png");
    
    
  //  scene.set_model_texture(model, 0, img);


    //assets.set_texture_path("assets/textures/");
   // scene.enable_3d(false)

    
   assets.set_texture_load_flip(false);
    //canvas.load_font("assets/fonts/font3.fnt");
    canvas.load_font("assets/fonts/font1.fnt");
   // canvas.load_font("assets/fonts/font2.fnt");


   screens.add(new MenuScreen("menu"));
   screens.add(new GameScreen("game"));
   screens.add(new ExitScreen("exit"));

   screens.setScreen("menu");  


   // screens.setScreen("menu");

    
    

}

function unload()
{
    
} 

function reload()
{

    screens.add(new MenuScreen("menu"));
    screens.add(new GameScreen("game"));
    screens.add(new ExitScreen("exit"));

    screens.setScreen("menu");  

    console.log("Reloading...****************************");
  
}

function render()
{
    canvas.grid(10, 10, true);
    screens.render()
}

function update(dt)
{

    mouse_x = mouse.x();
    mouse_y = mouse.y();

  
 
    screens.update(dt);

   
   
}



function gui()
{

    screens.render_gui()
      
}