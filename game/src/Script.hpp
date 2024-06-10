
#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include "Config.hpp"
#include "Math.hpp"
#include "Texture.hpp"
#include "SceneNode.hpp"
#include "Animation.hpp"
#include "Buffer.hpp"
#include "Camera.hpp"
#include "Input.hpp"
#include <quickjs.h>



//*******************************************************************************************************
//                                  INPUT
//*******************************************************************************************************


static JSValue js_isKeyPressed(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) 
{
    int key;
    JS_ToInt32(ctx, &key,argv[0]);
   
    return JS_NewBool(ctx, Keyboard::Pressed(key));
}

static JSValue js_isKeyDown(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) 
{
   int key;
    JS_ToInt32(ctx, &key,argv[0]);
   
    return JS_NewBool(ctx, Keyboard::Down(key));
}

static JSValue js_isKeyUp(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) 
{
   int key;
    JS_ToInt32(ctx, &key,argv[0]);
   
    return JS_NewBool(ctx, Keyboard::Up(key));
}

static JSValue js_isKeyReleased(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) 
{
   int key;
    JS_ToInt32(ctx, &key,argv[0]);
    
    return JS_NewBool(ctx, Keyboard::Released(key));
}

JSFunctionMap keysFunctions = 
{
    {"pressed", js_isKeyPressed},
    {"down", js_isKeyDown},
    {"up", js_isKeyUp},
    {"released", js_isKeyReleased},
};

inline void RegisterKeyboardFunctions(JSContext* ctx, JSValue global_obj) 
{
    JSValue core = JS_NewObject(ctx);
    for (const auto& func : keysFunctions) 
    {
        JS_SetPropertyStr(ctx, core, func.first.c_str(), JS_NewCFunction(ctx, func.second, func.first.c_str(), 1));
    }
    JS_SetPropertyStr(ctx, global_obj, "keyboard", core);
}

//mouse

static JSValue js_isMouseButtonPressed(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) 
{
    int button;
    JS_ToInt32(ctx, &button,argv[0]);
    
    return JS_NewBool(ctx, Mouse::Pressed(button));
}

static JSValue js_isMouseButtonDown(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) 
{
    int button;

    JS_ToInt32(ctx, &button,argv[0]);
    
    return JS_NewBool(ctx, Mouse::Down(button));
}

static JSValue js_isMouseButtonReleased(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) 
{
    int button;
    JS_ToInt32(ctx, &button,argv[0]);
    
    return JS_NewBool(ctx, Mouse::Released(button));
}

static  JSValue js_isMouseButtonUp(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    int button;
    JS_ToInt32(ctx, &button,argv[0]);
    return JS_NewBool(ctx, Mouse::Up(button));
}



JSFunctionMap mouseFunctions = {
    {"pressed", js_isMouseButtonPressed},
    {"down", js_isMouseButtonDown},
    {"up", js_isMouseButtonUp},
    {"released", js_isMouseButtonReleased},
};

inline void RegisterMouseFunctions(JSContext* ctx, JSValue global_obj) 
{
    JSValue core = JS_NewObject(ctx);
    for (const auto& func : mouseFunctions) 
    {
        JS_SetPropertyStr(ctx, core, func.first.c_str(), JS_NewCFunction(ctx, func.second, func.first.c_str(), 1));
    }
    JS_SetPropertyStr(ctx, global_obj, "mouse", core);
}


//*******************************************************************************************************
//                                  SCRIPT ASSETS TEXTURE
//*******************************************************************************************************


static  JSValue js_assets_load_texture(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    const char* fileName = JS_ToCString(ctx, argv[0]);
    int result = 0;
    if (!TextureManager::Instance().LoadTexture(fileName))
    {
          JS_ThrowReferenceError(ctx, "assets_load_texture: fail load texture (%s)", fileName);
    } else 
    {
        result = TextureManager::Instance().GetTotalTextures();
    }
    
    JS_FreeCString(ctx, fileName);
    return JS_NewInt32(ctx, result);
}

static  JSValue js_assets_set_texture_path(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    const char* path = JS_ToCString(ctx, argv[0]);
    TextureManager::Instance().SetTexturePath(path);
    JS_FreeCString(ctx, path);
    return JS_NULL;
}

static  JSValue js_assets_set_texture_flip(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    bool flip = JS_ToBool(ctx, argv[0]);
    TextureManager::Instance().FlipTextureOnLoad(flip);
    return JS_NULL;
}


JSFunctionMap assetsTextureFunctions = 
{
    {"load_texture", js_assets_load_texture},
    {"set_texture_path", js_assets_set_texture_path},
    {"set_texture_flip_load", js_assets_set_texture_flip},
};

inline void RegisterAssetsFunctions(JSContext* ctx, JSValue global_obj) 
{
    JSValue core = JS_NewObject(ctx);
    for (const auto& func : assetsTextureFunctions) 
    {
        JS_SetPropertyStr(ctx, core, func.first.c_str(), JS_NewCFunction(ctx, func.second, func.first.c_str(), 1));
    }
    JS_SetPropertyStr(ctx, global_obj, "assets", core);
}