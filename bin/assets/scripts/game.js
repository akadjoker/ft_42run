
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




//***************************************************************************************** */



class MenuScreen extends Screen
{
  
    load()
    {
       
        console.log('MenuScreen carregando assets...');
    }

    render_gui()
    {
        canvas.set_font(1);
        canvas.print(screenWidth/2, screenHeight/2, 'menu' )

    }

    update(dt)  
    {
            if (mouse.pressed(0))
            {
               
                screens.setScreen('game');
            }
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