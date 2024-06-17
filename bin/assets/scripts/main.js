include("assets/scripts/utils.js");
include("assets/scripts/game.js");

console.log("Starting...****************************");
var PlayerID = 0;
var FloorID = 0;
var ChairID = 0;
var EscolaID = 0;
var MacID = 0;
var TableID = 0;
var VendingID = 0;
var CoinID = 0;

var NodeEscolaA = 0;
var NodeEscolaB = 0;

var NodeFloorA = 0;
var NodeFloorB = 0;

var escolaAPosition = new Vec3 (70, 0, 8);
var escolaBPosition = null;
var escolaDepth = 1;
var Points = 0;

var CubeID = 0;

var TextureCoint = 0;
var TextureButtonA = 0;
var TextureButtonB = 0;
var TextureMenu= 0; 

var MoveCamera = false;

var isPause = false;
var isDebug = false;
var doPulse = false;

function load()
{
  
    core.set_key_exit(Key.F2);
    
    // let model = scene.create_cube(1, 1, 1);
    // let node = scene.create_static_node("cube", false); 

    // scene.set_node_postion(node, 0, 1, 1);
    // scene.node_add_model(node, model);

    assets.set_texture_path("assets/");
    assets.set_texture_load_flip(true);
      
        assets.load_texture("model/diffuse_back.jpg");
        assets.load_texture("model/glasses.jpg");
        assets.load_texture("model/luis_eyeL_hi.jpg");
        assets.load_texture("model/luis_eyeR_hi.jpg");
        assets.load_texture("model/face.jpg");
        assets.load_texture("model/shoes.jpg");
        assets.load_texture("model/skeen.jpg");
    let escolaTexture = assets.load_texture("textures/textura.jpg");
    let vendingTexture = assets.load_texture("textures/vending.jpg");
    let tableTexture = assets.load_texture("textures/wood.jpg");
  //  let macTexture = assets.load_texture("textures/metal.jpg");
  //  let macTexture2 = assets.load_texture("textures/b2c.png");
    let chairTexture = assets.load_texture("textures/rubber.jpeg");
    let floorTexture = assets.load_texture("textures/tiles.jpg"); 
    //let coinTexture = assets.load_texture("textures/coin.jpg");

    assets.set_texture_load_flip(false);
    TextureCoint = assets.load_texture("sprites/coin.png");
    TextureButtonA = assets.load_texture("sprites/Button.png");
    TextureButtonB = assets.load_texture("sprites/Rect.png");
    TextureMenu = assets.load_texture("sprites/menu.png");
 



        PlayerID=scene.load_entity("assets/model/main.ah3d", "player", true);
    scene.entity_add_animation(PlayerID, "assets/model/idle.anim");
    scene.entity_add_animation(PlayerID, "assets/model/run.anim");
    scene.entity_add_animation(PlayerID, "assets/model/jump.anim");
    scene.entity_add_animation(PlayerID, "assets/model/hit.anim");
    scene.entity_add_animation(PlayerID, "assets/model/jumpOver.anim");
    scene.entity_add_animation(PlayerID, "assets/model/jumpUp.anim");
    scene.entity_add_animation(PlayerID, "assets/model/funJump.anim");

    //mplay mode 0 lool 1 pingpong 2 one shot
    
    scene.entity_play(PlayerID, "run", 0, 0.25);
    

    
    



        scene.set_entity_texture(PlayerID, 0, 1);
        scene.set_entity_texture(PlayerID, 1, 2);
        scene.set_entity_texture(PlayerID, 2, 3);
        scene.set_entity_texture(PlayerID, 3, 4);
        scene.set_entity_texture(PlayerID, 4, 5);
        scene.set_entity_texture(PlayerID, 5, 6);
        scene.set_entity_texture(PlayerID, 6, 7);


    FloorID  = scene.create_plane(10, 10, 5,5, 30, 8);
    NodeFloorA = scene.create_static_node("planeA", false);
    NodeFloorB = scene.create_static_node("planeB", false);
    
    EscolaID  = scene.load_model("assets/escola/escola.h3d", "escola");
    ChairID   = scene.load_model("assets/escola/chair.h3d", "chair");
    MacID     = scene.load_model("assets/escola/mac.h3d", "mac");
    TableID   = scene.load_model("assets/escola/table.h3d", "table");
    VendingID = scene.load_model("assets/escola/vending.h3d", "vending");
    CoinID = scene.load_model("assets/escola/coin.h3d", "coin");
    

    scene.set_model_texture(EscolaID, 0, escolaTexture);
    scene.set_model_texture(EscolaID, 1, escolaTexture);
    scene.set_model_texture(EscolaID, 2, escolaTexture);

    scene.set_model_texture(ChairID, 0, chairTexture);
    scene.set_model_texture(MacID, 0, escolaTexture);
    scene.set_model_texture(MacID, 1, escolaTexture);
    scene.set_model_texture(TableID, 0, tableTexture);
    scene.set_model_texture(VendingID, 0, vendingTexture);
    scene.set_model_texture(CoinID, 0, TextureCoint);

    scene.set_model_texture(FloorID, 0, floorTexture);
    

    CubeID = scene.create_cube(1, 1, 1);


    let escolaSize = scene.get_model_size(EscolaID);
   // console.log("escola size: " + escolaSize.width + " " + escolaSize.height + " " + escolaSize.depth);

    escolaDepth = escolaSize.width * 4.0;

  //  console.log("escola depth: " + escolaDepth);


    scene.set_model_culling(EscolaID, 1, false);
  //  scene.set_model_culling(EscolaID, 1, false);
  //  scene.set_model_culling(EscolaID, 2, false);
 //   scene.set_model_culling(EscolaID, 3, false);
   // scene.scale_model_texture_coords(EscolaID, 1, 10.0, 10.0);


    // let node = scene.create_static_node("chair", true);
    // scene.set_node_position(node, 10, 0, 0);
    // scene.node_add_model(node, ChairID);
    // scene.set_node_rotation(node, 90, 90, 0);
    // scene.set_node_scale(node, 0.2, 0.2, 0.2);


    // node = scene.create_static_node("table", true);
    // scene.set_node_position(node, 10, 0, 0);
    // scene.node_add_model(node, TableID);
    // scene.set_node_rotation(node, 90, 90, 0);
    // scene.set_node_scale(node, 0.3, 0.3, 0.3);

    // node = scene.create_static_node("mac", true);
    // scene.set_node_position(node, 10, 0, 0);
    // scene.node_add_model(node, MacID);
    // scene.set_node_rotation(node, 90, 90, 0);
    // scene.set_node_scale(node, 0.35, 0.35, 0.35);


    // node = scene.create_static_node("vending", true);
    // scene.set_node_position(node, 10,2, 0);
    // scene.node_add_model(node, VendingID);
    // scene.set_node_rotation(node, 90, 90, 0);
    // scene.set_node_scale(node, 1.2, 1.25, 1.2);

    NodeEscolaA = scene.create_static_node("escolaA", false);
    escolaAPosition= new Vec3 (70, 0, 8);


    scene.node_add_model(NodeEscolaA, EscolaID);
    scene.set_node_scale(NodeEscolaA, 4.0, 3.0, 3.5);

    NodeEscolaB = scene.create_static_node("escolaB", false);
    escolaBPosition = new Vec3(70+escolaDepth, 0, 8);
    scene.node_add_model(NodeEscolaB, EscolaID);
    scene.set_node_scale(NodeEscolaB, 4.0, 3.0, 3.5);
    
               
    scene.set_node_position(NodeEscolaA, escolaAPosition.x, escolaAPosition.y, escolaAPosition.z);
    scene.set_node_position(NodeEscolaB, escolaBPosition.x, escolaBPosition.y, escolaBPosition.z);

     

    
   assets.set_texture_load_flip(false);
    //canvas.load_font("assets/fonts/font3.fnt");
    canvas.load_font("assets/fonts/font1.fnt");
    // canvas.load_font("assets/fonts/font2.fnt");
    
    scene.set_node_position(NodeFloorA, escolaAPosition.x - (escolaDepth / 2.0) + 14.0, escolaAPosition.y - 0.2, escolaAPosition.z - 8);
    scene.set_node_position(NodeFloorB, escolaBPosition.x - (escolaDepth / 2.0) + 14.0, escolaBPosition.y - 0.2, escolaBPosition.z - 8);


    scene.set_node_scale(NodeFloorA, 21.625, 1, 5.0);
    scene.node_add_model(NodeFloorA, FloorID);


    scene.set_node_scale(NodeFloorB, 21.625, 1, 5.0);
    scene.node_add_model(NodeFloorB, FloorID);


    scene.set_entity_position(PlayerID, 6.0, -1.5, -2);
    scene.set_entity_scale(PlayerID, 0.2, 0.2, 0.2);
    scene.set_entity_rotation(PlayerID, 0, 90, 0);
    
    for (let i = 0; i < MAX_LIGHTS; i++)
    {
        scene.set_light_position(i, 1000, 1000, 1000);
        scene.set_light_color(i, 0.01, 0.01, 0.01);
        scene.set_light_intensity(i, 0.0001);
    }

    scene.set_light_position(0, 8.0, 10, 8);
    scene.set_light_intensity(0, 0.2);
    scene.set_light_color(0, 0.2, 0.2, 0.01);

    scene.set_node_visible(NodeEscolaA, true);
    scene.set_node_visible(NodeFloorA, true);

    scene.set_node_visible(NodeEscolaB, true);
    scene.set_node_visible(NodeFloorB, true);
    
      


    // PlayerID = scene.get_entity_id("player");
    
    //  ChairID = scene.get_model_id("chair");
    //  EscolaID = scene.get_model_id("escola");
    //  MacID = scene.get_model_id("mac");
    //  TableID = scene.get_model_id("table");
    //  VendingID = scene.get_model_id("vending");
    
    //  NodeEscolaA = scene.get_node_id("escolaA");
    //  NodeEscolaB = scene.get_node_id("escolaB");

   screens.add(new MenuScreen("menu"));
   screens.add(new GameScreen("game"));

   screens.setScreen("menu");    


   // screens.setScreen("menu");



}

function unload()
{
    
} 

function reload()
{
    EscolaID = scene.get_model_id("escola");
    let escolaSize = scene.get_model_size(EscolaID);
    escolaDepth = escolaSize.width * 4.0;
    escolaAPosition= new Vec3 (70, 0, 8);
    escolaBPosition = new Vec3(70 + escolaDepth, 0, 8);
    

    // PlayerID = scene.get_entity_id("player");
    
    //  ChairID = scene.get_model_id("chair");

    //  MacID = scene.get_model_id("mac");
    //  TableID = scene.get_model_id("table");
    //  VendingID = scene.get_model_id("vending");
    
    //  NodeEscolaA = scene.get_node_id("escolaA");
    //  NodeEscolaB = scene.get_node_id("escolaB");
      
   

    screens.add(new MenuScreen("menu"));
    screens.add(new GameScreen("game"));


    screens.setScreen("menu");  



    console.log("Reloading...");
  
}

function render()
{
   
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