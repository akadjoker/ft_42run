
class Screen 
{

    constructor(id)
    {
        this.id = id
  
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

    unload()
    {
    }
    
}

class ScreenManager
{
    constructor()
    {
        this.screens = {};
        this.currentScreen = null;
        this.nextScreen = null;
        this.transitioning = false;
        this.transitionDuration = 1; 
        this.transitionProgress = 1;
        this.mode = 2;

    }

    add(screen)
    {
        this.screens[screen.id] = screen;
    }

    currentID()
    {
        if (!this.currentScreen) return "none";
        return this.currentScreen.id;
    }

    setScreen(screenId)
    {
        if (this.currentScreen )
        {
            if (this.transitioning) return;
            //if (this.currentScreen.id === screenId) return;
        }    
        if (this.screens[screenId])
        {
            if (this.currentScreen)
            {
                console.log(`Transitioning from ${this.currentScreen.id} to ${screenId}`);
                this.nextScreen = this.screens[screenId];
                this.nextScreen.load();
                this.transitioning = true;
                this.transitionProgress = 0;
                this.mode = 0;
            } else
            {
               
                console.log(`Setting screen to ${screenId}`);
                this.currentScreen = this.screens[screenId];
                this.currentScreen.load();
            }
        } else
        {
            console.error(`Screen whit ID ${screenId} not found.`);
        }
    }

    update(deltaTime)
    {
        if (this.transitioning)
        {
            this.transitionProgress += deltaTime / this.transitionDuration;
            if (this.transitionProgress >= 1)
            {
                this.transitionProgress = 1;
                this.transitioning = false;
                this.currentScreen.unload();
                this.currentScreen = this.nextScreen;
                this.nextScreen = null;
                this.mode = 1;
            }
        }
        if (this.mode == 2) 
        {
            this.transitionProgress -= deltaTime / this.transitionDuration;
            if (this.transitionProgress <= 0)
            {
                this.mode = 0;
                this.transitionProgress = 0;
            }      
        }
        if (this.mode == 1)
        {
            this.transitionProgress -= deltaTime / this.transitionDuration;
            if (this.transitionProgress <= 0)
            {
                this.mode = 0;
                this.transitionProgress = 0;
                this.transitioning = false;
            }            
        }
        if (this.currentScreen)
        {
            this.currentScreen.update(deltaTime);
        }
    }

    render()
    {
             if (this.currentScreen)
            {
                this.currentScreen.render()
            }
    }

    render_gui()
    {       


            if (this.currentScreen)
            {
                if (this.transitioning)
                {
                    this.currentScreen.render_gui();
                    canvas.set_alpha(this.transitionProgress);
                    canvas.set_color(255, 255, 255);
                    canvas.rect(0, 0, screenWidth, screenHeight, true)

                } else
                {
                        this.currentScreen.render_gui();
                        if (this.mode == 1)
                        {
                            canvas.set_color(255, 255, 255);
                            canvas.set_alpha(this.transitionProgress);
                            canvas.rect(0, 0, screenWidth, screenHeight, true)
                        }
                        if (this.mode == 2)
                        {
                            canvas.set_color(0, 0, 0);
                            canvas.set_alpha(this.transitionProgress);
                            canvas.rect(0, 0, screenWidth, screenHeight, true)
                        }
                }
        }
        
      
    }


}

var screens = new ScreenManager();

const MAX_LIGHTS = 9;


//***************************************************************************************** */



class MenuScreen extends Screen
{

    rotate = 0;
    entity = 0;
  
    load()
    {

        assets.set_texture_load_flip(true);
        assets.set_texture_path("assets/");
        assets.load_texture("model/diffuse_back.jpg");
        assets.load_texture("model/glasses.jpg");
        assets.load_texture("model/luis_eyeL_hi.jpg");
        assets.load_texture("model/luis_eyeR_hi.jpg");
        assets.load_texture("model/face.jpg");
        assets.load_texture("model/shoes.jpg");
        assets.load_texture("model/skeen.jpg");




        this.entity = scene.load_entity("assets/model/main.ah3d", "player", false);
        scene.entity_add_animation(this.entity, "assets/model/idle.anim");
        scene.entity_play(this.entity, "idle", 1, 0.25);


        scene.set_entity_texture(this.entity, 0, 1);
        scene.set_entity_texture(this.entity, 1, 2);
        scene.set_entity_texture(this.entity, 2, 3);
        scene.set_entity_texture(this.entity, 3, 4);
        scene.set_entity_texture(this.entity, 4, 5);
        scene.set_entity_texture(this.entity, 5, 6);
        scene.set_entity_texture(this.entity, 6, 7);


 

        scene.set_entity_position(this.entity, 6.0, -1.5, -2);
        scene.set_entity_scale(this.entity, 0.2, 0.2, 0.2);
        scene.set_entity_rotation(this.entity, 10, -70, 0);
        
        for (let i = 0; i < MAX_LIGHTS; i++)
        {
            scene.set_light_position(i, 1000, 1000, 1000);
            scene.set_light_color(i, 0.01, 0.01, 0.01);
            scene.set_light_intensity(i, 0.0001);
        }

        scene.set_light_position(0, 8.0, 10, 8);
        scene.set_light_intensity(0, 0.2);
        scene.set_light_color(0,0.2, 0.2, 0.01);


        

    }

    render_gui()
    {
        canvas.set_color(255, 255, 255);
        canvas.set_font(1);
        canvas.print(screenWidth / 2, screenHeight / 2, 'menu');

    }

    update(dt)  
    {
        camera.rotate_by_mouse(dt);
        if (keyboard.down(Key.W))
        {
            camera.move(0, dt);   
        } else 
        if (keyboard.down(Key.S))
        {
            camera.move(1, dt);   
        } else 
        if (keyboard.down(Key.A))
        {
            camera.move(2, dt);   
        } else 
        if (keyboard.down(Key.D))
        {
            camera.move(3, dt);   
        }
        this.rotate += 0.5;

       


    }
      
}

class GameScreen extends Screen
{
  
     load()
    {
       
        console.log('GameScreen carregando assets...');
     }
    
     render_gui()
     {
        canvas.set_font(1);
        canvas.print(screenWidth/2, screenHeight/2, 'GAME' )
     }

    update(dt)  
    {
             if (mouse.pressed(0))
            {
               
                screens.setScreen('menu');
            }
    }
      
}