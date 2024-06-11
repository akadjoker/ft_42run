
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



// template<class _Rep, class _Period>
// std::future<void> TimerAsync(std::chrono::duration<_Rep, _Period> duration, std::function<void()> callback)
// {
//     return std::async(std::launch::async, [duration, callback]()
//     {
        
//             std::this_thread::sleep_for(duration);
//             callback();

//     });
// }


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

void execute(JSContext* ctx, const char* script, const char* filename) 
{
    JSValue result = JS_Eval(ctx, script, strlen(script), filename, JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) 
    {
        JSValue exception = JS_GetException(ctx);
        const char* errorMessage = JS_ToCString(ctx, exception);
        Logger::Instance().Error(errorMessage);
        JS_FreeCString(ctx, errorMessage);
        JS_FreeValue(ctx, exception);
    }
    JS_FreeValue(ctx, result);
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

    // std::cout<<"Timer Start" << std::endl;
    // auto future = TimerAsync(std::chrono::seconds(0), [&]()
    // {
    //     std::cout<<"Timer async" << std::endl;
    
    // });

    // while (true)
    // {
    //    // std::cout<<"processing" << std::endl;
     
    //     auto status = future.wait_for(std::chrono::seconds(0));
    //     if (status == std::future_status::ready)
    //     {
    //               std::cout<<"Timer break" << std::endl;
    //         break;
    //     }
    // }
//       std::cout<<"Timer Finished" << std::endl;

//     return 0;
// }

    Device device;
    Driver driver;
    TextureManager textureManager;
    Logger logger;
    System system;
    Scene scene;





    //Camera camera(Vec3(-20.0f, 10.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));
    //Camera camera(Vec3(0.0f, 0.5f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));
    // float lastX = screenWidth / 2.0f;
    // float lastY = screenHeight / 2.0f;
    // bool firstMouse = true;
   // bool followCameraMode = false;

    // FollowCamera followCamera(Vec3(0.0f, 8.0f, -8.0f), 0.99f);
    //FixCamera followCamera(Vec3(0.0f, 1.0f, -5.0f));



    JSRuntime* rt = JS_NewRuntime();
    JSContext* ctx = JS_NewContext(rt);
    js_std_add_helpers(ctx, -1, NULL); 
    JSValue global_obj = JS_GetGlobalObject(ctx);




    device = Device::Instance();
    device.Create(screenWidth, screenHeight, "Engine", true);

 

    scene.Init();

    RegisterGlobalFunctions(ctx, global_obj);
    RegisterKeyboardFunctions(ctx, global_obj);
    RegisterMouseFunctions(ctx, global_obj);
    RegisterAssetsFunctions(ctx, global_obj);
    scene.RegisterFunctions(ctx, global_obj);
    RegisterCoreFunctions(ctx, global_obj);


    bool jsPanic = false;

    if (System::Instance().FileExists("assets/scripts/main.js"))
    {
        std::string script = readFile("assets/scripts/main.js");
        execute(ctx, script.c_str(), "main.js");
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
        }
    });


            // if (!jsPanic)
            // {
            //     scene.LoadJs();
            // }

    TextureManager::Instance().SetTexturePath("assets/textures/");
    TextureManager::Instance().FlipTextureOnLoad(true);

//     Model *cube =  scene.CreateCube(1.5f,1.5f,1.5f);
//     StaticNode *node = scene.CreateStaticNode("cube",false);
//     node->AddModel(cube);


   
//    Model *plane =  scene.CreatePlane(20, 20, 8, 4, 40.0f, 120.0f);
   
//    StaticNode * planeNode = scene.CreateStaticNode("floor",false);
//    planeNode->scale.set(8.0f, 1.0f, 60.0f);   
//    planeNode->AddModel(plane);




//   Model *escola = scene.LoadModel("assets/escola/escola.h3d");

//    Vec3 escolaSize = escola->GetSize();


//    escola->SetTexture(0, TextureManager::Instance().Load("textura.jpg"));
//    escola->SetTexture(1, TextureManager::Instance().Load("textura.jpg"));
//    escola->SetTexture(2, TextureManager::Instance().Load("textura.jpg"));

// //    escola->SetTextureNormal(0, TextureManager::Instance().Load("NormalMap.png"));
// //    escola->SetTextureNormal(1, TextureManager::Instance().Load("NormalMap.png"));
// //    escola->SetTextureNormal(2, TextureManager::Instance().Load("NormalMap.png"));


//    escola->SetCullFace(0,false);
//    escola->SetCullFace(1,false);
//    escola->SetCullFace(2,false);
   
   

//   StaticNode *escolaNodeA = scene.CreateStaticNode("scalA");
//    escolaNodeA->AddModel(escola);
//    escolaNodeA->scale.set(2.0f, 2.0f, 2.0f);
//    escolaNodeA->position.set(5.0f, 0.12f, escolaSize.x);
//    float angle=90.0f;
//    escolaNodeA->orientation.setEuler(0,ToRadians(angle),0);
//    escolaNodeA->Update();


//   StaticNode  *escolaNodeB = scene.CreateStaticNode("scalB");
//    escolaNodeB->AddModel(escola);
//    escolaNodeB->scale.set(2.0f, 2.0f, 2.0f);
//    escolaNodeB->position.set(5.0f, 0.12f, -escolaSize.x);
//    escolaNodeB->orientation.setEuler(0,ToRadians(angle),0);
//    escolaNodeB->Update();




     while (device.Run())
    {
        

        

                //  int xposIn, yposIn;
                //  u32 IsMouseDown = SDL_GetMouseState(&xposIn, &yposIn);
                //   if ( IsMouseDown & SDL_BUTTON(SDL_BUTTON_LEFT) )
                //   {
                //           float xpos = static_cast<float>(xposIn);
                //           float ypos = static_cast<float>(yposIn);

                //           if (firstMouse)
                //           {
                //               lastX = xpos;
                //               lastY = ypos;
                //               firstMouse = false;
                //           }

                //           float xoffset = xpos - lastX;
                //           float yoffset = lastY - ypos; 

                //           lastX = xpos;
                //           lastY = ypos;

                //        scene.GetCamera().ProcessMouseMovement(xoffset, yoffset);
                //   }
                //   else
                //   {
                //       firstMouse = true;
                //   }

    
         float deltaTime = device.GetFrameTime();

        // if (Keyboard::Down(KEY_W))
        // {
        //     camera.ProcessKeyboard(FORWARD, deltaTime);
         
        //  } 
        //  if (Keyboard::Down(KEY_S))
        //  {
        //      camera.ProcessKeyboard(BACKWARD, deltaTime);

        //  }
        //  if (Keyboard::Down(KEY_A))
        //  {
		// 	 camera.ProcessKeyboard(LEFT, deltaTime);

		//  }
    
        //  if (Keyboard::Down(KEY_D))
        //  {
        //      camera.ProcessKeyboard(RIGHT, deltaTime);

        //  }





        // Driver::Instance().SetBlend(true);
        // Driver::Instance().SetBlendMode(BlendMode::BLEND);
        // Driver::Instance().SetDepthTest(true);
        // Driver::Instance().SetDepthClamp(true);
        // Driver::Instance().SetCullFace(true);
        // Driver::Instance().SetClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // Driver::Instance().Clear();
        


      
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
