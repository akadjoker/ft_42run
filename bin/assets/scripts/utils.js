const Key =
{
    NULL: 0,
    APOSTROPHE: 39,    // Key: '
    COMMA: 44,         // Key: ,
    MINUS: 45,         // Key: -
    PERIOD: 46,        // Key: .
    SLASH: 47,         // Key: /
    ZERO: 48,          // Key: 0
    ONE: 49,           // Key: 1
    TWO: 50,           // Key: 2
    THREE: 51,         // Key: 3
    FOUR: 52,          // Key: 4
    FIVE: 53,          // Key: 5
    SIX: 54,           // Key: 6
    SEVEN: 55,         // Key: 7
    EIGHT: 56,         // Key: 8
    NINE: 57,          // Key: 9
    SEMICOLON: 59,     // Key: ;
    EQUAL: 61,         // Key: =
    A: 65,             // Key: A | a
    B: 66,             // Key: B | b
    C: 67,             // Key: C | c
    D: 68,             // Key: D | d
    E: 69,             // Key: E | e
    F: 70,             // Key: F | f
    G: 71,             // Key: G | g
    H: 72,             // Key: H | h
    I: 73,             // Key: I | i
    J: 74,             // Key: J | j
    K: 75,             // Key: K | k
    L: 76,             // Key: L | l
    M: 77,             // Key: M | m
    N: 78,             // Key: N | n
    O: 79,             // Key: O | o
    P: 80,             // Key: P | p
    Q: 81,             // Key: Q | q
    R: 82,             // Key: R | r
    S: 83,             // Key: S | s
    T: 84,             // Key: T | t
    U: 85,             // Key: U | u
    V: 86,             // Key: V | v
    W: 87,             // Key: W | w
    X: 88,             // Key: X | x
    Y: 89,             // Key: Y | y
    Z: 90,             // Key: Z | z
    LEFT_BRACKET: 91,  // Key: [
    BACKSLASH: 92,     // Key: '\'
    RIGHT_BRACKET: 93, // Key: ]
    GRAVE: 96,         // Key: `
    // Function keys
    SPACE: 32,          // Key: Space
    ESCAPE: 256,        // Key: Esc
    ENTER: 257,         // Key: Enter
    TAB: 258,           // Key: Tab
    BACKSPACE: 259,     // Key: Backspace
    INSERT: 260,        // Key: Ins
    DELETE: 261,        // Key: Del
    RIGHT: 262,         // Key: Cursor right
    LEFT: 263,          // Key: Cursor left
    DOWN: 264,          // Key: Cursor down
    UP: 265,            // Key: Cursor up
    PAGE_UP: 266,       // Key: Page up
    PAGE_DOWN: 267,     // Key: Page down
    HOME: 268,          // Key: Home
    END: 269,           // Key: End
    CAPS_LOCK: 280,     // Key: Caps lock
    SCROLL_LOCK: 281,   // Key: Scroll down
    NUM_LOCK: 282,      // Key: Num lock
    PRINT_SCREEN: 283,  // Key: Print screen
    PAUSE: 284,         // Key: Pause
    F1: 290,            // Key: F1
    F2: 291,            // Key: F2
    F3: 292,            // Key: F3
    F4: 293,            // Key: F4
    F5: 294,            // Key: F5
    F6: 295,            // Key: F6
    F7: 296,            // Key: F7
    F8: 297,            // Key: F8
    F9: 298,            // Key: F9
    F10: 299,           // Key: F10
    F11: 300,           // Key: F11
    F12: 301,           // Key: F12
    LEFT_SHIFT: 340,    // Key: Shift left
    LEFT_CONTROL: 341,  // Key: Control left
    LEFT_ALT: 342,      // Key: Alt left
    LEFT_SUPER: 343,    // Key: Super left
    RIGHT_SHIFT: 344,   // Key: Shift right
    RIGHT_CONTROL: 345, // Key: Control right
    RIGHT_ALT: 346,     // Key: Alt right
    RIGHT_SUPER: 347,   // Key: Super right
    KB_MENU: 348,       // Key: KB menu
    // Keypad keys
    KP_0: 320,        // Key: Keypad 0
    KP_1: 321,        // Key: Keypad 1
    KP_2: 322,        // Key: Keypad 2
    KP_3: 323,        // Key: Keypad 3
    KP_4: 324,        // Key: Keypad 4
    KP_5: 325,        // Key: Keypad 5
    KP_6: 326,        // Key: Keypad 6
    KP_7: 327,        // Key: Keypad 7
    KP_8: 328,        // Key: Keypad 8
    KP_9: 329,        // Key: Keypad 9
    KP_DECIMAL: 330,  // Key: Keypad .
    KP_DIVIDE: 331,   // Key: Keypad /
    KP_MULTIPLY: 332, // Key: Keypad *
    KP_SUBTRACT: 333, // Key: Keypad -
    KP_ADD: 334,      // Key: Keypad +
    KP_ENTER: 335,    // Key: Keypad Enter
    KP_EQUAL: 336,    // Key: Keypad =
    // Android key buttons
    BACK: 4,        // Key: Android back button
    MENU: 82,       // Key: Android menu button
    VOLUME_UP: 24,  // Key: Android volume up button
    VOLUME_DOWN: 25 // Key: Android volume down button
};

var screenWidth = 1024;
var screenHeight = 720;


function RandomInt(min, max)
{
    min = Math.ceil(min);
    max = Math.floor(max);
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

function RandomFloat(min, max)
{
    return Math.random() * (max - min) + min;
}

function Random(max)
{
    return Math.floor(Math.random() * Math.floor(max));
}

class Batch 
{

    constructor()
    {
        this.vertex = []
        this.uv = []
    }
    
    render( x, y, width, height)
    {
        let left = 0;        let right = 1;
        let top = 0;         let bottom = 1;
        
        let x1 =x;                let y1 =y;
        let x2 =x;                let y2 =y + height;
        let x3 =x + width;        let y3 =y + height;
        let x4 = x + width; let y4 = y;

        
   
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

var batch = new Batch();

class Sprite
{

    constructor(x, y)
    {
        this.x = x;
        this.y = y;
        this.width = 32;
        this.height = 32;
        this.red   = 1.0;
        this.green = 1.0;
        this.blue  = 1.0;
        this.alpha = 1.0;
        this.texture = 0;
        this.done = false;
        this.active = true;
        this.visible = true;
    }

    render()
    {
    }

    update(dt)
    {

    }
}


class Rectangle
{
    constructor(x, y, width, height)
    {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;

    }

    contains(px, py)
    {
        return px >= this.x && px <= this.x + this.width &&
               py >= this.y && py <= this.y + this.height;
    }

    intersects(rect)
    {
        return this.x < rect.x + rect.width &&
               this.x + this.width > rect.x &&
               this.y < rect.y + rect.height &&
               this.y + this.height > rect.y;
    }

}

const Ease = {
    linear: t => t,
    easeInQuad: t => t * t,
    easeOutQuad: t => t * (2 - t),
    easeInOutQuad: t => t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t,
    
    easeInCubic: t => t * t * t,
    easeOutCubic: t => (--t) * t * t + 1,
    easeInOutCubic: t => t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1,

    easeInQuart: t => t * t * t * t,
    easeOutQuart: t => 1 - (--t) * t * t * t,
    easeInOutQuart: t => t < 0.5 ? 8 * t * t * t * t : 1 - 8 * (--t) * t * t * t,

    easeInElastic: t => {
        const c4 = (2 * Math.PI) / 3;
        return t === 0 ? 0 : t === 1 ? 1 : -Math.pow(2, 10 * t - 10) * Math.sin((t * 10 - 10.75) * c4);
    },
    easeOutElastic: t => {
        const c4 = (2 * Math.PI) / 3;
        return t === 0 ? 0 : t === 1 ? 1 : Math.pow(2, -10 * t) * Math.sin((t * 10 - 0.75) * c4) + 1;
    },
    easeInOutElastic: t => {
        const c5 = (2 * Math.PI) / 4.5;
        return t === 0
            ? 0
            : t === 1
            ? 1
            : t < 0.5
            ? -(Math.pow(2, 20 * t - 10) * Math.sin((20 * t - 11.125) * c5)) / 2
            : (Math.pow(2, -20 * t + 10) * Math.sin((20 * t - 11.125) * c5)) / 2 + 1;
    },

    easeInBack: t => {
        const c1 = 1.70158;
        const c3 = c1 + 1;
        return c3 * t * t * t - c1 * t * t;
    },
    easeOutBack: t => {
        const c1 = 1.70158;
        const c3 = c1 + 1;
        return 1 + c3 * (--t) * t * t + c1 * t * t;
    },
    easeInOutBack: t => {
        const c1 = 1.70158;
        const c2 = c1 * 1.525;
        return t < 0.5
            ? (Math.pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2
            : (Math.pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2;
    },
    

    easeInBounce: t => 1 - Ease.easeOutBounce(1 - t),
    easeOutBounce: t => {
        if (t < 1 / 2.75) {
            return 7.5625 * t * t;
        } else if (t < 2 / 2.75) {
            return 7.5625 * (t -= 1.5 / 2.75) * t + 0.75;
        } else if (t < 2.5 / 2.75) {
            return 7.5625 * (t -= 2.25 / 2.75) * t + 0.9375;
        } else {
            return 7.5625 * (t -= 2.625 / 2.75) * t + 0.984375; 
        }
    },
    
    easeInOutBounce: t => t < 0.5
        ? 0.5 * Ease.easeInBounce(2 * t)
        : 0.5 * Ease.easeOutBounce(2 * t - 1) + 0.5
    
    
    
    
    


    
};

class Tween
{
    constructor(target, property, startValue, endValue, duration, easing = Ease.linear)
    {
        this.easing = easing;
        this.target = target;
        this.property = property;
        this.startValue = startValue;
        this.endValue = endValue;
        this.duration = duration;
        this.elapsedTime = 0;
        this.isComplete = false;
        this.OnEnd= () => {};
    }

    update(deltaTime)
    {
        if (this.isComplete) return;

        this.elapsedTime += deltaTime;
        if (this.elapsedTime >= this.duration)
        {
            this.elapsedTime = this.duration;
            this.target[this.property] = this.endValue;
            this.isComplete = true;
            this.OnEnd();
        } else
        {
            const progress = this.easing(this.elapsedTime / this.duration);
            this.target[this.property] = this.startValue + (this.endValue - this.startValue) * progress;
        }
    }

}


class DelayTween
{
    constructor(duration)
    {
        this.duration = duration;
        this.elapsedTime = 0;
        this.isComplete = false;
        this.OnEnd= () => {};
    }

    update(deltaTime)
    {
        if (this.isComplete) return;

        this.elapsedTime += deltaTime;
        if (this.elapsedTime >= this.duration)
        {
            this.isComplete = true;
            this.OnEnd();
        }
    }
}

class SequentialTween
{
    constructor()
    {
        this.tweens = [];
        this.currentTweenIndex = 0;
        this.isComplete = false;
        this.OnEnd= () => {};
    }

    add(tween)
    {
        this.tweens.push(tween);
        return tween;
    }

    update(deltaTime)
    {
        if (this.isComplete) return;

        if (this.currentTweenIndex < this.tweens.length)
        {
            const currentTween = this.tweens[this.currentTweenIndex];
            currentTween.update(deltaTime);

            if (currentTween.isComplete)
            {
                this.currentTweenIndex++;
            }
        } else
        {
            this.OnEnd();
            this.isComplete = true;
            
        }
    }
}



class Screen 
{

    constructor(id)
    {
        this.id = id
        this.tweens = [];
        this.sprites = [];
  
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

    addTween(tween)
    {
        this.tweens.push(tween);
        return tween;
    }

    addSprite(sprite)
    {
        this.sprites.push(sprite);
        return sprite;
    }
    
    OnRender()
    {
        this.render()
    }
    OnRenderGui()
    {
        this.render_gui()
        this.sprites = this.sprites.filter(sprite => !sprite.done);
        this.sprites.forEach(sprite => sprite.render());

    }
    OnUpdate(dt)
    {
        this.tweens = this.tweens.filter(tween => !tween.isComplete);
        this.tweens.forEach(tween => tween.update(dt * 1000.0));
        this.sprites.forEach(sprite => sprite.update(dt));
        this.update(dt)
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
        return screen;
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
            this.currentScreen.OnUpdate(deltaTime);
        }
    }

    render()
    {
             if (this.currentScreen)
            {
                 this.currentScreen.OnRender()
                 
            }
    }

    render_gui()
    {       


            if (this.currentScreen)
            {
                if (this.transitioning)
                {
                    this.currentScreen.OnRenderGui();
                    canvas.set_alpha(this.transitionProgress);
                    canvas.set_color(1.0, 1.0, 1.0);
                    canvas.rect(0, 0, screenWidth, screenHeight, true)

                } else
                {
                        this.currentScreen.OnRenderGui();
                        if (this.mode == 1)
                        {
                            canvas.set_color(1.0, 1.0, 1.0);
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