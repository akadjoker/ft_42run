
#include <pch.h>
#include <Core.hpp>


#include <iostream>
#include <fstream>
#include <sstream>

#include <future>
#include <functional>
#include <thread>
#include <chrono>
#include <condition_variable>

#include <quickjs-libc.h>
#include <quickjs.h>

using JSFunctionMap = std::map<std::string, JSValue(*)(JSContext*, JSValueConst, int, JSValueConst*)>;

#include "Script.hpp"
#include "Scene.hpp"




int screenWidth = 1024;
int screenHeight = 720;




std::condition_variable cv;
std::mutex cv_m;
bool ready = false;


void TimerAsync(std::function<void()> callback)
{
    std::thread([callback]()
    {
        std::lock_guard<std::mutex> lk(cv_m);
        ready = true;
        cv.notify_one();
    }).detach();

    // Wait for the signal in the main thread
    std::unique_lock<std::mutex> lk(cv_m);
    cv.wait(lk, []{return ready;});

    // Call the callback in the main thread
    callback();
}

bool execute(JSContext* ctx, const char* script, const char* filename) 
{
    bool sucess = true;
    JSValue result = JS_Eval(ctx, script, strlen(script), filename, JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) 
    {
        JSValue exception = JS_GetException(ctx);
        const char* errorMessage = JS_ToCString(ctx, exception);
        Logger::Instance().Error("%s executing %s",errorMessage, filename);
        JS_FreeCString(ctx, errorMessage);
        JS_FreeValue(ctx, exception);
        sucess = false;
    }
    JS_FreeValue(ctx, result);

    return sucess;
}

std::string readFile(const std::string& filePath) 
{
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static  JSValue js_include(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
{
    if (argc != 1) 
    {
        return JS_ThrowReferenceError(ctx, "include: Wrong number of arguments (filename)");
    }
    const char* filename = JS_ToCString(ctx, argv[0]);
   
    if (System::Instance().FileExists(filename))
    {
        std::string script = readFile(filename);
        execute(ctx, script.c_str(), System::Instance().GetFileNameWithoutExt(filename));
    } else 
    {
        Logger::Instance().Error("File not found: %s", filename);

    }


    return JS_NULL;
}


JSFunctionMap globalFunctions = 
{
    {"include", js_include},
};

inline void RegisterGlobalFunctions(JSContext* ctx, JSValue global_obj) 
{
   
    for (const auto& func : globalFunctions) 
    {
        JS_SetPropertyStr(ctx, global_obj, func.first.c_str(), JS_NewCFunction(ctx, func.second, func.first.c_str(), 1));
    }
}


int main()
{

   

    Device device;
    Driver driver;
    TextureManager textureManager;
    Logger logger;
    System system;
    Scene scene;






    JSRuntime* rt = JS_NewRuntime();
    JSContext* ctx = JS_NewContext(rt);
    js_std_add_helpers(ctx, -1, NULL); 
    JSValue global_obj = JS_GetGlobalObject(ctx);




    device = Device::Instance();
    device.Create(screenWidth, screenHeight, "42 Run", true);

 

    scene.Init();

    RegisterGlobalFunctions(ctx, global_obj);
    RegisterKeyboardFunctions(ctx, global_obj);
    RegisterMouseFunctions(ctx, global_obj);
    RegisterAssetsFunctions(ctx, global_obj);
    scene.RegisterFunctions(ctx, global_obj);
    RegisterCoreFunctions(ctx, global_obj);


    bool jsPanic = false;
    // if (System::Instance().FileExists("assets/scripts/utils.js"))
    // {
    //     std::string script = readFile("assets/scripts/utils.js");
    //     execute(ctx, script.c_str(), "utils.js");
    // } else 
    // {
    //     Logger::Instance().Error("File not found: assets/scripts/utils.js");
    //     jsPanic = true;
    // }

    // if (System::Instance().FileExists("assets/scripts/game.js"))
    // {
    //     std::string script = readFile("assets/scripts/game.js");
    //     execute(ctx, script.c_str(), "game.js");
    // } else 
    // {
    //     Logger::Instance().Error("File not found: assets/scripts/game.js");
    //     jsPanic = true;
    // }

    bool UseHotReload = false;
    u64 loadTime = 0;
    u64 lastLoadTime = 0;

    u64 utilsLoadtime = System::Instance().GetFileModTime("assets/scripts/utils.js");
    u64 gameLoadtime = System::Instance().GetFileModTime("assets/scripts/game.js"); 

    u64 lastUtilsLoadtime = utilsLoadtime;
    u64 lastGameLoadtime = gameLoadtime;

    double reloadInterval = 5.0;
    double lastReloadTime = 0.0;


    if (System::Instance().FileExists("assets/scripts/main.js"))
    {
        std::string script = readFile("assets/scripts/main.js");
        if (execute(ctx, script.c_str(), "main.js"))
        {
            UseHotReload = false;
            lastLoadTime = System::Instance().GetFileModTime("assets/scripts/main.js");
            loadTime = lastLoadTime;
           
        }
    } else 
    {
        Logger::Instance().Error("File not found: assets/scripts/main.js");
        jsPanic = true;
    }



    if (!jsPanic)
    {
       scene.InitJs(ctx, global_obj);
    }



   
//   TimerAsync(std::chrono::seconds(3), [](){
//         std::cout << "Timer finished! Executing callback in main thread.\n";
//     });


    // TimerAsync(std::chrono::seconds(0), [&]()
    // {
    //          if (!jsPanic)
    //         {
    //             scene.LoadJs();
    //         }
    
    // });

    
    TimerAsync([&]()
    {
        if (!jsPanic)
        {
            scene.LoadJs();
           // scene.ReloadJs();
        }
    });


            // if (!jsPanic)
            // {
            //     scene.LoadJs();
            // }

    TextureManager::Instance().SetTexturePath("assets/textures/");
    TextureManager::Instance().FlipTextureOnLoad(true);




     while (device.Run())
    {

              float deltaTime = device.GetFrameTime();


     
            if (Keyboard::Pressed(KEY_F4))
            {
                UseHotReload = !UseHotReload;
            }

            bool forceReload = false;

            if (Keyboard::Pressed(KEY_F5))
            {
                forceReload = true;
            }
        

        if (!jsPanic)
        {
            if (UseHotReload)
            {

                lastReloadTime += deltaTime;
                if ( lastReloadTime > reloadInterval)
                {
                    loadTime = System::Instance().GetFileModTime("assets/scripts/main.js");
                    utilsLoadtime = System::Instance().GetFileModTime("assets/scripts/utils.js");
                    gameLoadtime = System::Instance().GetFileModTime("assets/scripts/game.js");


                    if ( (loadTime > lastLoadTime) || (utilsLoadtime > lastUtilsLoadtime) || (gameLoadtime > lastGameLoadtime) )
                    {
                        Logger::Instance().Warning("Hot reloading... %ld  last: %ld  utils: %ld  game: %ld",loadTime, lastLoadTime,utilsLoadtime,gameLoadtime);
                        forceReload = true;
                    }

                
                    lastReloadTime = 0.0f;
                }



            }
            if (forceReload)
            {
                 JS_FreeValue(ctx, global_obj);
                 JS_FreeContext(ctx);
                 ctx = JS_NewContext(rt);

                    
                js_std_add_helpers(ctx, -1, NULL); 
                global_obj = JS_GetGlobalObject(ctx);
                JS_FreeValue(ctx, global_obj);

                RegisterGlobalFunctions(ctx, global_obj);
                RegisterKeyboardFunctions(ctx, global_obj);
                RegisterMouseFunctions(ctx, global_obj);
                RegisterAssetsFunctions(ctx, global_obj);
                scene.RegisterFunctions(ctx, global_obj);
                RegisterCoreFunctions(ctx, global_obj);


                std::string script = readFile("assets/scripts/main.js");
                if (execute(ctx, script.c_str(), "main.js"))
                {
                    lastUtilsLoadtime = utilsLoadtime;
                    lastGameLoadtime = gameLoadtime;
                    lastLoadTime = loadTime;
                   

                    jsPanic = false;
                    scene.CloseJs ();
                    scene.InitJs(ctx, global_obj);
                    scene.ReloadJs();
                } else 
                {
                    jsPanic = true;
                }

            }
        }
        



      
        scene.Update(deltaTime);


        
        scene.Render();

        device.Flip();
    }

    if (!jsPanic)
    {
        scene.CloseJs();
    }

    JS_FreeValue(ctx, global_obj);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    
   
    scene.Release();
    device.Close();
    return 0;
}
