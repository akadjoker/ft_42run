






//***************************************************************************************** */
const OBJECT_TYPES = ['moeda', 'vending', 'mesa', 'cadeira','imac'];
const LANE_POSITIONS = [0,1, 2, 3,4]; 
const MAX_OBSTACLES = 15;
const  GRAVITY = -9.8;
const JUMP = 5.0;
var PlayerX = 0.0;
var PlayerZ = 0.0;


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
        
        canvas.set_texture(TextureButtonA);
    
        //console.log(mouse_x + " " + mouse_y);
    
        if (this.active) {
            if (this.bound.contains(mouse_x, mouse_y)) {
                if (mouse.down(0)) {
                    this.OnClick();
                }
                canvas.set_texture(TextureButtonB);
                //canvas.rect(this.x-2, this.y-2, this.width+2, this.height+2, false);
            }
        }

        batch.render(this.x, this.y, this.width, this.height);
        canvas.print(this.x + (this.width / 2)-25, this.y + (this.height / 2)-12, this.caption);
    }
    
}

class Menu extends Sprite
{

    constructor(x, y, w, h)
    {
        super(x, y);
        this.width  = w;
        this.height = h;
        
        this.buttonExit  = new Button(x, y, 100, 50, TextureButtonA, 'Exit');
        this.buttonClose = new Button(x, y, 100, 50, TextureButtonA, 'Play');
    }

    render()
    {
        canvas.set_texture(TextureMenu);
        canvas.set_color(1, 1, 1);
        let width = this.width;
        let height = this.height;
        batch.render(this.x-width/2, this.y-height/2, width, height);
        this.buttonExit.x = this.x - (width/4) - 50;
        this.buttonExit.y = this.y - (height/4) + 20;
        
        this.buttonClose.x = this.x - (width/4) + 98;
        this.buttonClose.y = this.y - (height/4) + 20;
        this.buttonExit.render();
        this.buttonClose.render();
    }

}





class MenuScreen extends Screen
{

    rotate = 0;

 

  
    load()
    {

        this.transitionDuration = 2.5; 
        this.transitionProgress = 1.0;
        this.doFadeIn = true;
        this.doFadeOut = false;
        this.doExit = false;
        scene.enable_3d(false);

        assets.set_texture_path("assets/");

        scene.set_node_scale(NodeEscolaA, 4.0, 3.0, 3.5);
        scene.set_node_position(NodeEscolaA, 10, escolaAPosition.y-0.1, -60);
        scene.set_node_rotation(NodeEscolaA, 0, 90, 0);
        

        scene.set_node_visible(NodeEscolaA, true);
        scene.set_node_visible(NodeFloorA, false);
    
        scene.set_node_visible(NodeEscolaB, false);
        scene.set_node_visible(NodeFloorB, false);




        scene.entity_play(PlayerID, "idle", 0, 0.25);

        scene.set_entity_position(PlayerID, 8.0, -0.5, -2);
        scene.set_entity_scale(PlayerID, 0.2, 0.2, 0.2);
        scene.set_entity_rotation(PlayerID, 0, -70, 0);
        
        for (let i = 0; i < MAX_LIGHTS; i++)
        {
            scene.set_light_position(i, 1000, 1000, 1000);
            scene.set_light_color(i, 0.01, 0.01, 0.01);
            scene.set_light_intensity(i, 0.0001);
        }

        scene.set_light_position(0, 8.0, 10, 8);
        scene.set_light_intensity(0, 0.2);
        scene.set_light_color(0, 0.2, 0.2, 0.01);

        
        

        let spritePlay = this.addSprite(new Button(600, -100, 150, 35, TextureButtonA,"Play"));
        let spriteExit = this.addSprite(new Button(600, -150, 150, 35, TextureButtonA, "Exit"));

        spritePlay.active = false;
        spriteExit.active = false;
        
        spritePlay.OnClick = () =>
        {
            spritePlay.active = false;
            spriteExit.active = false;
            this.transitionProgress=0.0;
            this.doFadeOut = true;
            const sequentialTween = new SequentialTween();
            sequentialTween.add(new Tween(spritePlay, 'y', spritePlay.y, -100, 500, Ease.easeInBack));
            sequentialTween.add(new Tween(spriteExit, 'y', spriteExit.y, -150, 500, Ease.easeInBack));
            this.addTween(sequentialTween);
                
        }
        spriteExit.OnClick = () =>
        {
            spritePlay.active = false;
            spriteExit.active = false;
            this.doFadeOut = true;
            this.doExit = true;
            const sequentialTween = new SequentialTween();
            sequentialTween.add(new Tween(spritePlay, 'y', spritePlay.y, -100, 500, Ease.easeInBack));
            sequentialTween.add(new Tween(spriteExit, 'y', spriteExit.y, -150, 500, Ease.easeInBack));
        
            this.addTween(sequentialTween);
            
        }

        const sequentialTween = new SequentialTween();
        sequentialTween.add(new DelayTween(100)); 
       // sequentialTween.add(new Tween(spritePlay, 'x', spritePlay.x, 600, 1000, Ease.easeOutElastic));
        sequentialTween.add(new Tween(spritePlay, 'y', spritePlay.y, 300, 600, Ease.easeOutElastic));
      //  sequentialTween.add(new Tween(spriteExit, 'x', spriteExit.x, 600, 1000, Ease.easeOutElastic));
        sequentialTween.add(new Tween(spriteExit, 'y', spriteExit.y, 350, 650, Ease.easeOutElastic));
        this.addTween(sequentialTween);
        sequentialTween.OnEnd = () =>
        {
            spritePlay.active = true;
            spriteExit.active = true;
        }



        

    }

    render_gui()
    {
        canvas.set_color(255, 0, 255);
        canvas.set_font(1);
        canvas.print(screenWidth - 300, screenHeight - 40, 'Luis Santos AKA DJOKER ;)');

        if (this.doFadeIn || this.doFadeOut)
        {
         
            canvas.set_alpha(this.transitionProgress);
            canvas.set_color(0, 0, 0);
            canvas.rect(0, 0, screenWidth, screenHeight, true);
        }
    

    }

    update(dt)  
    {
            if (this.doFadeIn)
            {
                this.transitionProgress -= dt / this.transitionDuration;

                if (this.transitionProgress < 0.9)
                    scene.enable_3d(true);
                if (this.transitionProgress < 0.0)
                {
                    this.doFadeIn = false;
                    this.transitionProgress = 0.0;
                    
                }
                return;
            }
        if (this.doFadeOut)
        {
            this.transitionProgress += dt / this.transitionDuration;
            if (this.transitionProgress > 1.0)
            {
                if (this.doExit)
                {
                    core.exit();
                } else
                {
                   screens.setScreen('game');
                }
                
            }
            return;
        }
        // camera.rotate_by_mouse(dt);
        // if (keyboard.down(Key.W))
        // {
        //     camera.move(0, dt);   
        // } else 
        // if (keyboard.down(Key.S))
        // {
        //     camera.move(1, dt);   
        // } else 
        // if (keyboard.down(Key.A))
        // {
        //     camera.move(2, dt);   
        // } else 
        // if (keyboard.down(Key.D))
        // {
        //     camera.move(3, dt);   
        // }
        // this.rotate += 0.5;

       


    }

    reload()
    {
     
    }
    unload()
    {
       
    }
      
}


class Player extends Entity
{
    
    load()
    {
        this.node = 0;
        this.name = "player";
        this.done = false;
        this.visible = true;
        this.position.set(8.0, -0.1, 0.0);
        this.Velocity = new Vec3(0, 0, 0);
        this.collider = new BoxColider(0, 0, 0, 1, 2.8, 1);
        this.OnGround = false;
        this.IsJumping = false;
        this.MoveSpeed = 5.0;
        this.SideSpeed = 38.0;
        this.Friction = 0.90;
        this.distance = 0.0;
        this.mode = 0;//running 1 hit 
        this.jump_type = 0;


        // hit and bounce :O
        this.hitting = false; 
        this.hitt_force = 8.0;

        // logic for jumps :P
        this.next_distance = 0.0;
        this.next_aline = 0.0;
        this.next_type = "none";

        //this.colider = new SphereColider(0, 0, 0, 1.5);
    }

    reset()
    {
        this.next_aline = 0.0;
        this.next_distance = 0.0;
        this.next_type = "none";
        this.jump_type = 0;
    }

    update(dt)
    {
        // if (keyboard.pressed(Key.P))
        // {
        //     this.hitt_force = 10.0;
        //     this.hit();
        // }
        // if (keyboard.pressed(Key.O))
        // {
        //     this.mode = 0;
        //     scene.entity_play(PlayerID, "run", 0, 0.25);
        // }


        if (this.mode === 0)
        {
            this.distance += dt * 100;

           // console.log(this.next_distance + " " + this.next_type + " " + this.next_aline);
            
            if (this.next_type === "mesa" && (this.next_distance>=-8.0 && this.next_distance<=0.5) && this.next_aline<=3.0)
            {
                this.jump_type = 1;
            } else
            if ( this.next_type === "cadeira" && (this.next_distance<=-4.0 && this.next_distance>=-14.5) && this.next_aline<=1.0)
            {
                this.jump_type = 2;
            } else 
            if ( this.next_type === "imac" && (this.next_distance<=-4.0 && this.next_distance>=-14.5) && this.next_aline<=1.0)
            {
                this.jump_type = 0;
            } else 
            {
                this.jump_type = 3;
            }

            if (keyboard.pressed(Key.SPACE))
            {
                if (this.OnGround)
                {
                    // console.log(this.next_distance + " " + this.next_type);
                    if (this.jump_type === 0)
                    {
                        scene.entity_play(PlayerID, "jump", 2, 0.05);    
                    }else
                    if (this.jump_type === 1)
                    {
                        scene.entity_play(PlayerID, "jumpOver", 2, 0.25);
                        this.Velocity.x = 0.0;
                    } else 
                    if (this.jump_type === 2)
                    {
                        this.Velocity.x *= 0.99;
                        scene.entity_play(PlayerID, "jumpUp", 2, 0.25);
                    } else
                    if (this.jump_type === 3)
                    {
                        this.Velocity.x *= 0.99;
                        scene.entity_play(PlayerID, "funJump", 2, 0.05);    
                    }

                    
                    
                    this.mode = 2;
                    this.IsJumping = true;
                
                }
            }

            if (keyboard.down(Key.A))
            {
                this.Velocity.z -= this.SideSpeed * dt;
            } else
                if (keyboard.down(Key.D))
                {
                    this.Velocity.z += this.SideSpeed * dt;
                }
            
            this.Velocity.z *= this.Friction;
        
            this.Velocity.x += 1 * this.MoveSpeed * dt;
            this.Velocity.x = Clamp(this.Velocity.x, 2.75, 10.0);

            
            


            
        } else
        if (this.mode === 1)//hit
        {
            
            this.Velocity.y = 0.0;
            this.Velocity.z = 0.0;
            let frame = scene.get_entity_frame(PlayerID);
            if (frame <= 800)
            {
                this.Velocity.x *= this.Friction;
            } else 
            {
                this.Velocity.x = 0.0;    
            }

            if (frame >= 500 && frame <= 800)
            {
                this.position.y = Lerp(this.position.y, -1.0, 0.25);
            }
            if (frame >= 2500)
            {
                this.mode = 0;
                scene.entity_play(PlayerID, "run", 0, 0.25);
            }
        } else
            if (this.mode === 2)//jump styles
            {
                let frame = scene.get_entity_frame(PlayerID);
             //   if (frame>0)
                //    console.log(frame);

                if (this.jump_type === 0)//normal jump
                {
                    this.IsJumping = false;
                    if (frame >= 100 && this.OnGround)
                    {
                        this.OnGround = false;
                        this.Velocity.y = 6.0;
                    }
                    if (frame >= 799)
                    {
                        this.OnGround = true;
                        this.mode = 0;
                        scene.entity_play(PlayerID, "run", 0, 0.15);
                        this.reset();
                    }
                } else 
                if (this.jump_type === 1)//jump over
                {
                    this.IsJumping = false;
                    if (frame >= 82 && this.OnGround)
                    {
                        this.OnGround = false;
                        this.Velocity.y = 3.5;
                        this.Velocity.x = 10.0;
                    }
                    if (frame >= 1049)
                    {
                        this.OnGround = true;
                        this.mode = 0;
                        scene.entity_play(PlayerID, "run", 0, 0.15);
                        this.reset();
                        
                    }
                } else 
                if (this.jump_type === 2)//jump side
                {
                        this.IsJumping = false;
                        if (frame >= 40 && this.OnGround)
                        {
                            this.OnGround = false;
                            this.Velocity.y = 5.5;
            
                        } else 
                        {
                            this.Velocity.y -= 9.81 * dt;
                        }
                        if (frame >= 1000)
                        {
                            this.OnGround = true;
                            this.mode = 0;
                            scene.entity_play(PlayerID, "run", 0, 0.15);
                            this.reset();
                            
                        }
                    } if (this.jump_type === 3)  //fun jump 
                    {

                        this.IsJumping = false;
                            if (frame >= 20 && this.OnGround)
                            {
                                this.OnGround = false;
                                this.Velocity.y = 3.5;
                            } 
                            if (frame >= 601)
                            {
                                this.OnGround = true;
                                this.mode = 0;
                                scene.entity_play(PlayerID, "run", 0, 0.15);
                                this.reset();
                                
                            }
                            
                    }
            
        }
        



        
        
        this.Velocity.y -= 9.81 * dt;

        this.position.x += this.Velocity.x * dt;
        this.position.z += this.Velocity.z * dt;
        this.position.y += this.Velocity.y * dt;
        

        this.position.z = Clamp(this.position.z, -5.75, 5.75);

        if (this.mode === 0 || this.mode === 2)
        {
            if (this.position.y < -0.1)
            {
                this.position.y = -0.1;
                this.Velocity.y = 0;
                this.OnGround = true;
            }
        }else 
        if (this.mode === 1)
        {
                if (this.position.y < -1.1)
                {
                    this.position.y = -1.1;
                }
         }
   
        

        this.collider.x = this.position.x;
        this.collider.y = this.position.y ;
        this.collider.z = this.position.z - 0.5;
      
        if (!MoveCamera)
        {
            camera.set_position(this.position.x - 16, 5.2, Clamp(Lerp(this.position.z, 0, dt * 2.5), -2.5, 2.0));
        }

        scene.set_entity_position(this.node, this.position.x, this.position.y, this.position.z);
    }
    render()
    {
        this.collider.render();
    }

    set_close(value, id, aline)
    {
        this.next_distance = value;
        this.next_type = id;
        this.next_aline = aline;
        
    }

    hit()
    {
        if (!this.OnGround || this.IsJumping)
            return;
       // this.position.y -= 1.0;
        //this.Velocity.y = 14.0 ;
        this.mode = 1;
        this.hitting = true;
        this.Velocity.x -= this.hitt_force * this.MoveSpeed;
        scene.entity_play(PlayerID, "hit", 2, 0.25);
    }

    collide(entity)
    {

       // console.log("collide" + entity.name);
        if (entity.name==='moeda')
        {
            doPulse = true;
            entity.kill();
            Points += 1;
        } else  //['moeda', 'vending', 'mesa', 'cadeira','imac'];
        if (entity.name==='vending')
        {
            this.hitt_force = 10.0;
           // this.hit();
        }
        if (entity.name==='mesa')
        {
         
            if (this.OnGround || !this.IsJumping)
            {
                this.hitt_force = 8.0;
                this.hit();
            }
            
          //  entity.kill();
        }
        if (entity.name==='cadeira')
        {
            if (this.OnGround || !this.IsJumping || this.jump_type !== 2)
            {
                this.hitt_force = 4.0;
                this.hit();
            }
           // entity.kill();
        }
        if (entity.name==='imac')
        {
            if (this.OnGround || !this.IsJumping)
            {
                this.hitt_force = 2.0;
                this.hit();
            }
          //  entity.kill();
        }
        
    }
}

class Light  extends Entity
{

    constructor(index,x,y,z,r,g,b,intensity)
    {
        super();
        this.node = 0;
        this.name = "light";
        this.position.set(x, y, z);
        this.red = r;
        this.green = g;
        this.blue = b;
        this.intensity = intensity;
        this.index = index;
     //   console.log("create light" + this.index);
        
    }

    update(dt)
    {
       
    }
    
    render()
    {
        if (this.index < 5)
        {
            scene.set_light_position(this.index, escolaAPosition.x - this.position.x, escolaAPosition.y + this.position.y, escolaAPosition.z - this.position.z);
            canvas.sphere( escolaAPosition.x - this.position.x, escolaAPosition.y + this.position.y, escolaAPosition.z, 0.5, 12, 8, true);
        } else
        {
            canvas.sphere( escolaBPosition.x - this.position.x, escolaBPosition.y + this.position.y, escolaBPosition.z, 0.5, 12, 8, true);
            scene.set_light_position(this.index, escolaBPosition.x-this.position.x, escolaBPosition.y+this.position.y, escolaBPosition.z-this.position.z);
        }

      
       
        scene.set_light_color(this.index, this.red, this.green, this.blue);
        scene.set_light_intensity(this.index, this.intensity);
       
        
    }
}

class LightBlink  extends Light
{

    constructor(index,x,y,z,r,g,b,intensity)
    {
        super(index,x,y,z,r,g,b,intensity);
        
        this.blinkTime = 0;
        this.nextBlinkInterval = Math.random() * 0.3 + 0.1;
        
    }

    update(deltaTime)
    {
        this.blinkTime += deltaTime;
        if (this.blinkTime >= this.nextBlinkInterval)
        {
            this.red   = Math.random() * 0.5 + 0.5;
            this.green = this.red;
            this.blue = 0.2;
            this.nextBlinkInterval = Math.random() * 0.3 + 0.1;
            this.blinkTime = 0;
        }
    }
    
   
}
class Obstacle extends Entity
{
  
    selfX = 0;
    selfZ = 0;
  
    load()
    {
        console.log("create object  " + this.name+ " " + this.node);
        
    }
    render()
    {
        this.collider.render();
    }
    update(dt)
    {
        this.selfX = this.position.x;
        this.selfZ = this.position.z;
        let distance = Distance(this.selfX, PlayerX);
        let aline    = AbsDistance(this.selfZ , PlayerZ);
    
        if (distance > -15.0 )
        {

            let player = this.game.GetById('player');
            if (player != null)
            {
                player.set_close(distance, this.name, aline);    
            }
            
        }
      //  console.log("distance " + distance);
      //  scene.set_node_position(this.node, this.position.x, this.position.y, this.position.z);
    }
}

class Coin extends Entity
{
  
 
  
    load()
    {
        console.log("create object " + this.name);
        this.angle = 0;
        
    }
    render()
    {
        this.collider.render();
    }
    update(dt)
    {
        this.angle += 10.5 * (dt * 25.0);
        scene.set_node_rotation(this.node, 0,this.angle,90);    
      
      //  scene.set_node_position(this.node, this.position.x, this.position.y, this.position.z);
    }
}

class GameScreen extends Screen
{


    player = null;
 
    lastLane = 0;
    lights = [];
    pulseElapsed = 0;
    pulseDuration = 1.0;
    pulseStep = 0;
    pulseForse = 0.0;
    isPulse = false;
    isStart = false;
    isExit = false;
    

    Pulse()
    {
        if (this.isPulse)
        {
            return;
        }
        this.isPulse = true;
        this.pulseElapsed = 0;
        this.pulseStep = 0;

    }

   

    load()
    {
        this.isStart = true;
        this.fader = 1;
        this.isExit = false;
        this.isPulse = false;
   
        this.pulseElapsed = 0;
        this.pulseStep = 0;
        this.pulseForse = 0.0;


        this.transitionDuration = 1.5; 
        this.transitionProgress = 1.0;
        this.doFadeIn = true;
        this.doFadeOut = false;
        this.isFading = true;
        
        escolaAPosition= new Vec3 (70, 0, 8);
        escolaBPosition = new Vec3(70+escolaDepth, 0, 8);
        scene.set_node_position(NodeEscolaA, escolaAPosition.x, escolaAPosition.y, escolaAPosition.z);
        scene.set_node_position(NodeEscolaB, escolaBPosition.x, escolaBPosition.y, escolaBPosition.z);
        this.isStart = true;

    // scene.set_node_position(0, 25.0, 0.0, 0.0);
    // scene.set_node_scale(0, 100.0, 1, 10.0);
    // scene.set_node_rotation(0, 0, 0, 0);
        // scene.node_add_model(0, 0);
        scene.entity_play(PlayerID, "run", 0, 0.25);
        scene.set_entity_rotation(PlayerID, 0, 90, 0);

        scene.set_node_visible(NodeEscolaA, true);
        scene.set_node_visible(NodeFloorA, true);
        scene.set_node_visible(NodeEscolaB, true);
        scene.set_node_visible(NodeFloorB, true);

        scene.set_node_scale(NodeEscolaA, 4.0, 3.0, 3.5);
        scene.set_node_position(NodeEscolaA, escolaAPosition.x, escolaAPosition.y, escolaAPosition.z);
        scene.set_node_rotation(NodeEscolaA, 0,0,0);

        
        this.menu = this.addSprite(new Menu(screenWidth / 2, -400, 300, 190));
        this.menu.buttonClose.OnClick = () =>
            {
                   
                
                let tween= new Tween(this.menu, 'y', screenHeight/2,-200, 800, Ease.easeInBack);
                this.addTween(tween);
                tween.OnEnd = () =>
                {
                    scene.entity_play(PlayerID, "run", 0, 0.25);
                    scene.set_entity_rotation(PlayerID, 0, 90, 0);
                    isPause = false;  
                    
                }
            }    
            this.menu.buttonExit.OnClick = () =>
                {
                       
                this.isExit = true;
                this.doFadeOut = true;
                        
                    let tween= new Tween(this.menu, 'y', screenHeight/2,-200, 800, Ease.easeInBack);
                    this.addTween(tween);
                   
                }    
            
         
    camera.set_position(-2, 3.5, 0);
    let c = camera.get_position();
        
        this.objectTimer = 0;
        this.objectInterval = Math.random() * 0.3 + 0.1;
        
        
        this.game = new Game();
        this.player = new Player();
        this.game.addEntity(this.player,true);



    
    for (let i = 0; i < MAX_LIGHTS; i++)
    {
        scene.set_light_position(i, 1000, 1000, 1000);
        scene.set_light_color(i, 0.0, 0.0, 0.0);
        scene.set_light_intensity(i, 0.0001);
    }
        

        let y = 10.0;
        let force = 0.6; 
        
        this.addLight(84, y, 10.2, 1.0, 1.0, 1.0, force);
        this.addLight(62, y, 8.2, 1.0, 1.0, 1.0, force);
        this.addLight(40, y, 10.2, 1.0, 1.0, 1.0, force);
        this.addLight(18, y, 8.2, 1.0, 1.0, 1.0, force);
        this.addLight(-4.5, y-0.5, 10.2, 1.0, 1.0, 1.0, force+0.8,true);

        this.addLight(84, y, 10.2, 1.0, 1.0, 1.0, force);
        this.addLight(62, y, 8.2, 1.0, 1.0, 1.0, force);
        this.addLight(40, y, 10.2, 1.0, 1.0, 1.0, force);
        this.addLight(18, y, 8.2, 1.0, 1.0, 1.0, force);
        this.addLight(-4.5, y, 10.2, 1.0, 1.0, 1.0, force);

    }

    addLight(x, y, z, r, g, b, intensity,blink=false)
    {
        let index = this.lights.length;
       
        // let light = new Light(index, escolaAPosition.x - x, escolaAPosition.y + y, escolaAPosition.z - z, r, g, b, intensity);
        if (blink)
        {
            let light = new LightBlink(index, x, y, z, r, g, b, intensity);
            this.lights.push(light);
        } else
        {
            let light = new Light(index, x, y, z, r, g, b, intensity);
            this.lights.push(light);
        }
        
    }
  

    render()
    {
        
        if (isDebug)
        {
            this.game.render();
        
            for (let i = 0; i < this.lights.length; i++)
            {
                let light = this.lights[i];
                light.render();
            }
        }
    }
    
     render_gui()
     {
            
         if (isDebug)
         {
             canvas.set_color(1, 1, 1);
             canvas.set_font(0);
             canvas.set_font_size(12);
             canvas.print(10, 20, 'Player position: ' + PlayerX.toFixed(2) + '  ' + this.player.position.z.toFixed(2))
             //canvas.print(10, 40, 'Entityes : ' + this.game.entities.length + '')
             canvas.print(10, 50, 'Nodes : ' + scene.get_total_nodes())
             canvas.print(10, 80, 'Models : ' + this.game.entities.length)
         }

         if (!isPause)
         {

             canvas.set_color(1, 1, 0);
             canvas.set_font(1);
             canvas.set_font_size(26);

         
             //let distanceInCm = this.distance.toFixed(2);
             let distanceInM = (this.player.distance / 100).toFixed(2);
             let distanceInKm = (this.player.distance / 100000).toFixed(2);

             let width = core.screen_width();
             if (this.player.distance < 100000) {
                 canvas.print((width / 2.0) - 80, 30, 'Distance ' + distanceInM + ' m ')
             } else {
                 canvas.print((width / 2.0) - 90, 30, 'Distance ' + distanceInKm + ' km ' + distanceInM + ' m ')
             }
         
             let pulse = this.pulseForse;
             canvas.set_color(1, 1, 1);
             canvas.print(width - 125, 35,  Points)
             
             canvas.set_alpha(1);

             canvas.set_texture(TextureCoint);

//             batch.render(width - 180, 30, 45, 45);
            
             
             canvas.draw_texture(TextureCoint, (width - 180)-(pulse/2.0), 20-(pulse/2.0), 55+pulse, 55+pulse);

             //TextureCoint
         } else 
         { 
             canvas.set_alpha(0.7);
             canvas.set_color(0, 0, 0);
             canvas.rect(0, 0, screenWidth, screenHeight, true)
             
             
         }

        
            if (this.doFadeIn || this.doFadeOut)
            {
             
                canvas.set_alpha(this.transitionProgress);
                canvas.set_color(0, 0, 0);
                canvas.rect(0, 0, screenWidth, screenHeight, true);
            }
         
    } 
    
    unload()
    {
       
    }

    update(dt)  
    {
            if (this.doFadeIn)
            {
                this.transitionProgress -= dt / this.transitionDuration;

                this.isFading = true;
                if (this.transitionProgress < 0.9)
                {
                    this.doFadeIn = false;
                    scene.enable_3d(true);
                }
                if (this.transitionProgress < 0.0)
                {
                    this.doFadeIn = false;
                    this.isFading = false;
                    
                }
                return;
            }
        if (this.doFadeOut)
        {
            this.transitionProgress += dt / this.transitionDuration;
            this.isFading = true;
            if (this.transitionProgress > 1.0)
            {
                core.exit();
            }
            return;
        }
        
        if (keyboard.down(Key.ESCAPE) && !isPause)
        {
            isPause = true;
            scene.entity_play(PlayerID, "idle", 0, 0.25);
            scene.set_entity_rotation(PlayerID, 0, -90, 0);
            const sequentialTween = new SequentialTween();
            sequentialTween.add(new Tween(this.menu, 'y', -200, screenHeight/2, 800, Ease.easeOutElastic));
            this.addTween(sequentialTween);
        }
   
        if (MoveCamera)
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
        }

        if (!isPause )
        {
            if (doPulse)
            {
                this.Pulse();
                doPulse = false;
            }

            if (this.isPulse)
            {
                this.pulseElapsed += dt;
                if (this.pulseElapsed < this.pulseDuration)
                {
                    this.pulseStep += 2 * dt;
                    this.pulseForse =  Math.abs(Math.sin(this.pulseStep * Math.PI) * 20);
                } else 
                {
                    this.isPulse = false;
                    this.pulseForse = 0.0;
                }

                    
            }
            
            this.removeOldObjects();
            this.game.update(dt);
            PlayerX = this.player.position.x;
            PlayerZ = this.player.position.z;
 
            this.game.collide(this.player);

            for (let i = 0; i < this.lights.length; i++)
            {
                let light = this.lights[i];
                light.update(dt);
            }

        
            if (keyboard.pressed(Key.ONE))
            {
                let o = this.createObject('mesa', this.player.position.x + 40, 0, 0);
                this.game.addEntity(o);
                // let pos = this.player.position.x + RandomFloat(30, 60);
                // if (!this.haveClosest(pos))
                //     this.generateObjects(pos);
            }


            if (keyboard.pressed(Key.TWO))
            {
                let o = this.createObject('cadeira', this.player.position.x + 40, 0, 0);
                this.game.addEntity(o);
                // let pos = this.player.position.x + RandomFloat(30, 60);
                // if (!this.haveClosest(pos))
                //     this.generateObjects(pos);
            }
    
            if (keyboard.pressed(Key.THREE))
            {
                let o = this.createObject('imac', this.player.position.x + 40, 0, 0);
                this.game.addEntity(o);
                // let pos = this.player.position.x + RandomFloat(30, 60);
                // if (!this.haveClosest(pos))
                //     this.generateObjects(pos);
            }
            if (keyboard.pressed(Key.FOUR))
                {
                    let o = this.createObject('moeda', this.player.position.x + 40, 0, 0);
                    this.game.addEntity(o);
                    // let pos = this.player.position.x + RandomFloat(30, 60);
                    // if (!this.haveClosest(pos))
                    //     this.generateObjects(pos);
                }
        


            if (this.player.position.x > (escolaAPosition.x + escolaDepth / 2) - 30)
            {
                escolaAPosition.x += escolaDepth * 2;
            }
            if (this.player.position.x > (escolaBPosition.x + escolaDepth / 2) - 30)
            {
                escolaBPosition.x += escolaDepth * 2;
            }
            scene.set_node_position(NodeEscolaA, escolaAPosition.x, escolaAPosition.y, escolaAPosition.z);
            scene.set_node_position(NodeEscolaB, escolaBPosition.x, escolaBPosition.y, escolaBPosition.z);

            scene.set_node_position(NodeFloorA, escolaAPosition.x - (escolaDepth / 2.0) + 14.0, escolaAPosition.y - 0.2, escolaAPosition.z - 8);
            scene.set_node_position(NodeFloorB, escolaBPosition.x - (escolaDepth / 2.0) + 14.0, escolaBPosition.y - 0.2, escolaBPosition.z - 8);

            this.objectTimer += dt;
            if (this.objectTimer >= this.objectInterval)
            {

                if (this.game.entities.length <= MAX_OBSTACLES)
                {
                    let pos = this.player.position.x + RandomFloat(30, 60);
                     if (!this.haveClosest(pos))
                         this.generateObjects(pos);
                }
     
                this.objectInterval = Math.random() * 0.3 + 0.1;
                this.objectTimer = 0;
            }
        } 
       
    }

    createObject(type, position, y, z)
    {
       

        
    
        if (type === 'moeda')
        {
            let object = new Coin();
            object.node = scene.create_static_node(type, true);
           
            object.position.set(position, y+3, z);
            object.collider = new SphereColider(position, y + 2, z, 0.5);
            //object.collider = new BoxColider(position, y+1.5, z-0.7, 0.4, 1.0, 1.5);
            object.name = 'moeda';
            scene.node_add_model(object.node, CoinID);
            //scene.set_node_rotation(object.node, 90, 0, 0);
            scene.set_node_scale(object.node, 0.5,0.1,0.5);
            scene.set_node_position(object.node, position, y + 2, z);

            
            return object;
                       
        }


        let object = new Obstacle();
        object.node = scene.create_static_node(type, true);
        
        if (type === 'vending')
        {
            
            object.position.set(position, y, z);
            object.collider = new BoxColider(position-1.2, y, z-1.2, 2.4, 4.0, 2.4);
            object.name = 'vending';
            scene.node_add_model(object.node, VendingID);
            scene.set_node_rotation(object.node, 90, -90, 0);
            scene.set_node_scale(object.node, 1.3, 1.4, 1.4);
            scene.set_node_position(object.node, position, y+1.80, z);
           
        
        } else if (type === 'mesa')
        {

            object.position.set(position, y, z);
            object.collider = new BoxColider(position-1.6, y, z -3.3, 3.0, 2.0, 6.5);
            object.name = 'mesa';
            scene.node_add_model(object.node, TableID);
            scene.set_node_rotation(object.node, 90, 90, 0);
            scene.set_node_scale(object.node, 0.7, 0.6, 0.3);
            scene.set_node_position(object.node, position, y-0.30, z);

        
        } else if (type === 'cadeira')
        {

            object.position.set(position, y, z);
            object.collider = new BoxColider(position-1, y-0.4, z-0.7, 1.0, 1.8, 1.5);
            object.name = 'cadeira';
            scene.node_add_model(object.node, ChairID);
            scene.set_node_rotation(object.node, 90, 90, 0);
            scene.set_node_scale(object.node, 0.2, 0.25, 0.2);
            scene.set_node_position(object.node, position, y-0.40, z);

        } else if (type === 'imac')
        {

             object.position.set(position, y, z);
             object.collider = new BoxColider(position, y, z-0.7, 0.4, 1.5, 1.5);
             object.name = 'imac';
             scene.node_add_model(object.node, MacID);
             scene.set_node_rotation(object.node, 90, -90, 0);
             scene.set_node_scale(object.node, 0.35, 0.35, 0.45);
             scene.set_node_position(object.node, position, y-0.18, z);
        }
    
       
       
        return object; 
    }

    generateInitialObjects()
    {
        for (let i = 1; i <= 25; i++)
        {
            this.generateObjects(this.player.position.x + i * 8); 
        }
    }

    haveClosest(xPosition)
    {
        for (let i = 0; i < this.game.entities.length; i++)
        {
            let entity = this.game.entities[i];
            if (entity.name ==='player') continue;
            if (xPosition - entity.position.x < 5) return true;
        }
        return false;
    }
    generateObjects(xPosition)
    {
      //  if (this.game.entities.length >= MAX_OBSTACLES) return;

        const randomObjectIndex = Math.floor(Math.random() * OBJECT_TYPES.length);
        if (randomObjectIndex > OBJECT_TYPES.length) return;
        const randomLaneIndex = Math.floor(Math.random() * LANE_POSITIONS.length);
        if (randomLaneIndex !== this.lastLane) 
        {
            this.lastLane = randomLaneIndex;    
        } else 
        {
            return;    
        }
        let z;
        if (LANE_POSITIONS[randomLaneIndex] === 0)
        {

            z = -7;
            if (OBJECT_TYPES[randomObjectIndex] === 'moeda') z = -4;
            if (OBJECT_TYPES[randomObjectIndex] === 'mesa') z = -4;
        } else if (LANE_POSITIONS[randomLaneIndex] === 1)
        {
            z = -4;
        } else if (LANE_POSITIONS[randomLaneIndex] === 2)
        {
            z = 0;
        } else if (LANE_POSITIONS[randomLaneIndex] === 3)
        {
            z = 4;
        } else if (LANE_POSITIONS[randomLaneIndex] === 4)
        {

            z = 6;
            if (OBJECT_TYPES[randomObjectIndex] === 'moeda') z = 4;
            if (OBJECT_TYPES[randomObjectIndex] === 'mesa') z = 4;
        }
            
        let object = this.createObject(OBJECT_TYPES[randomObjectIndex], xPosition, 0, z);
        this.game.addEntity(object);
      
    }
    removeOldObjects()
    {
        let toRemove = [];
        for (let i = 0; i < this.game.entities.length; i++)
        {
            let object = this.game.entities[i];
            if (object.position.x < this.player.position.x - 10)
            {
               
                toRemove.push(object);
            }
        }

        for (let i = 0; i < toRemove.length; i++)
        {
            let object = toRemove[i];
            let node = object.node;
            if (node !== -1)
            {
                this.game.remove(object);

                scene.remove_node(node);    
            } else 
            {
                console.log('node not found');
            }
           
        }
        toRemove=[];

    }
    removeAll()
    {
        this.game.clear();

    }
      
}