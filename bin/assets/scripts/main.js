/*
API
SCENE

*/


function load()
{

    let model = scene.create_cube(1, 1, 1);
    let node = scene.create_static_node("cube", false); 

    scene.set_node_postion(node, 0, 1, 1);

    scene.node_add_model(node, model);
    

}

function unload()
{
    
}

function render()
{
    canvas.grid(10, 10,false);
}

function update(dt)
{
   
}

function gui()
{
    
}