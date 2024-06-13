






//***************************************************************************************** */
class Button extends Sprite
{

    constructor(x, y, w, h,tex,caption)
    {
        super(x, y);
        this.width  = w;
        this.height = h;
        this.texture = tex;
        this.caption = caption;
        this.bound = new Rectangle(x, y, w, h);
        this.OnClick = () => {};
    }
    render()
    {
        this.bound.x = this.x;
        this.bound.y = this.y;
        canvas.set_color(this.red, this.green, this.blue);
        canvas.set_alpha(this.alpha);
        
        canvas.set_texture(this.texture);
    
        //console.log(mouse_x + " " + mouse_y);
    

        if (this.bound.contains(mouse_x, mouse_y))
        {
            if (mouse.down(0))
            {
                this.OnClick();
            }
            canvas.rect(this.x-2, this.y-2, this.width+2, this.height+2, false);
        }

        batch.render(this.x, this.y, this.width, this.height);
        canvas.print(this.x + (this.width / 2)-25, this.y + (this.height / 2)-12, this.caption);
    }
    
}

class ExitScreen extends Screen
{

    load()
    {
        this.transitionDuration = 2.0; 
        this.transitionProgress = 0;

        
    }
    
    update(deltaTime)
    {
        this.transitionProgress += deltaTime / this.transitionDuration;
        if (this.transitionProgress >= 1)
        {
            core.exit();
        }
    }

    render_gui()
    {
                    canvas.set_alpha(this.transitionProgress);
                    canvas.set_color(0,0,0);
                    canvas.rect(0, 0, screenWidth, screenHeight, true)
    }
}


class MenuScreen extends Screen
{

    rotate = 0;

 

  
    load()
    {

        assets.set_texture_path("assets/");



        scene.set_node_position(0, 25.0, 0.4, 0.0);
        scene.set_node_scale(0, 10.0, 1, 10.0);
        scene.set_node_rotation(0, 0, 0, 6);
        scene.node_add_model(0, 0);

 

        scene.set_entity_position(0, 6.0, -1.5, -2);
        scene.set_entity_scale(0, 0.2, 0.2, 0.2);
        scene.set_entity_rotation(0, 10, -70, 0);
        
        for (let i = 0; i < MAX_LIGHTS; i++)
        {
            scene.set_light_position(i, 1000, 1000, 1000);
            scene.set_light_color(i, 0.01, 0.01, 0.01);
            scene.set_light_intensity(i, 0.0001);
        }

        scene.set_light_position(0, 8.0, 10, 8);
        scene.set_light_intensity(0, 0.2);
        scene.set_light_color(0, 0.2, 0.2, 0.01);

        let texture = assets.load_texture("sprites/ButtonText_Small_Orange_Round.png");
        

        let spritePlay = this.addSprite(new Button(-200, 100, 150, 35,texture,"Play"));
        let spriteExit = this.addSprite(new Button(-200, 100, 150, 35, texture, "Exit"));
        
        spritePlay.OnClick = () =>
        {
            const sequentialTween = new SequentialTween();
            sequentialTween.add(new Tween(spritePlay, 'x', spritePlay.x, -200, 1000, Ease.easeInBack));
            sequentialTween.add(new Tween(spriteExit, 'x', spriteExit.x, -200, 1000, Ease.easeInBack));
            sequentialTween.OnEnd = () =>
            {
                screens.setScreen('game');
            }
            this.addTween(sequentialTween);
                
        }
        spriteExit.OnClick = () =>
        {
            const sequentialTween = new SequentialTween();
            sequentialTween.add(new Tween(spritePlay, 'x', spritePlay.x, -200, 1000, Ease.easeInBack));
            sequentialTween.add(new Tween(spriteExit, 'x', spriteExit.x, -200, 1000, Ease.easeInBack));
            sequentialTween.OnEnd = () =>
            {
                screens.setScreen('exit');
            }
            this.addTween(sequentialTween);
            
        }

        const sequentialTween = new SequentialTween();
        sequentialTween.add(new DelayTween(100)); 
        sequentialTween.add(new Tween(spritePlay, 'x', spritePlay.x, 600, 1000, Ease.easeOutElastic));
        sequentialTween.add(new DelayTween(1)); 
        sequentialTween.add(new Tween(spritePlay, 'y', spritePlay.y, 300, 1000, Ease.easeOutElastic));
        sequentialTween.add(new DelayTween(1)); 
        sequentialTween.add(new Tween(spriteExit, 'x', spriteExit.x, 600, 1000, Ease.easeOutElastic));
        sequentialTween.add(new DelayTween(1)); 
        sequentialTween.add(new Tween(spriteExit, 'y', spriteExit.y, 350, 1000, Ease.easeOutElastic));
        this.addTween(sequentialTween);



        

    }

    render_gui()
    {
        canvas.set_color(255, 0, 255);
        canvas.set_font(1);
        canvas.print(100, 100, 'live coder ;)' );

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

    reload()
    {
     
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