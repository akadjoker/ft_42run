
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
#include <quickjs.h>

const int MAX_LIGHTS = 9;


class   Scene
{
public:



    static Scene& Instance();
    static Scene* InstancePtr();

    u32 AddEntity(const std::string &fileName,const std::string &name,bool castShadow=false);
    u32 AddModel(const std::string &fileName,const std::string &name);
    u32 AddStaticNode(const std::string &name,bool castShadow=false);

    bool NodeAddChild(u32 parent, u32 child);


    Entity * LoadEntity(const std::string &name,bool castShadow=false);
    Model  * LoadModel(const std::string &name);

    StaticNode *CreateStaticNode(const std::string &name="Node",bool castShadow=false);

    Model *CreateCube (float width, float height, float length);
    Model *CreatePlane(int stacks, int slices, int tilesX, int tilesY,float uvTileX =1.0f, float uvTileY =1.0f);
    Model *CreateQuad(float x, float y, float w, float h);
   
    void RegisterFunctions(JSContext *ctx, JSValue global_obj);

    void SetLightPosition(u32 index,float x, float y, float z);
    void SetLightColor(u32 index,float r, float g, float b);
    void SetLightIntensity(u32 index,float intensity);
    


    void Init();

    void Render();

    void Update(float elapsed);


    void Release();

    void SetViewMatrix(const Mat4 &m) { viewMatrix = m; }
    void SetProjectionMatrix(const Mat4 &m) { projectionMatrix = m; }
    void SetCameraPosition(const Vec3 &p) { cameraPosition = p; }
    Vec3 GetCameraPosition() const { return cameraPosition; }

    Entity     *GetEntity(const std::string &name);
    Model      *GetModel(const std::string &name);
    StaticNode *GetNode(const std::string &name);

    Entity     *GetEntity(int index);
    Model      *GetModel(int index);
    StaticNode *GetNode(int index);

    bool RemoveModel(int index);
    bool RemoveEntity(int index);
    bool RemoveNode(int index);

    RenderBatch &GetRenderBatch() { return batch; }
    Font *GetFont() { return m_fonts[currentFont]; };

    bool LoadFont(const char* fileName);

    bool SetFont(u32 index);

    u32 GetEntityCount() const { return (u32)m_entities.size(); }//animations nodes  
    u32 GetNodeCount()   const { return (u32)m_nodes.size(); } //static nodes with mesh/models
    u32 GetModelCount()  const { return (u32)m_models.size(); }//mesh

    u32 GetFontsCount()  const { return (u32)m_fonts.size(); }


    void RemoveNodes();
    void RemoveEntities();

    void Enable3D(bool enable) { do3D = enable; }
    void Enable2D(bool enable) { do2D = enable; }

    void InitJs( JSContext* ctx , JSValue global_obj);
    void LoadJs();
    void CloseJs();
    void RenderJs();
    void UpdateJs(float elapsed);
    void GuiJs();

    
    

         Scene();
        ~Scene();

private:
    static Scene* m_singleton;
    struct Light
    {
        float x, y, z;
        float r, g, b;
        float intensity;
        Light()
        {
            x = 0.0f;
            y = 0.0f;
            z = 0.0f;
            r = 0.5f;
            g = 0.5f;
            b = 0.5f;
            intensity = 0.25f;
        }
    };

    std::vector<Light> m_lights;
    std::vector<Entity*> m_entities;
    std::vector<Node*> m_nodesToDelete;
    std::vector<StaticNode*> m_nodes;
    std::vector<Model*> m_models;
    std::vector<Font*> m_fonts;
    u32 currentFont{0};

    Shader m_modelShader;
    Shader m_depthShader;
    Shader m_debugShader;
    Shader m_quadShader;

    RenderBatch batch;
    Font *font;

    Mat4 viewMatrix;
    Mat4 projectionMatrix;
    Vec3 cameraPosition;
    Vec3 lightPos;
    bool do3D{true};
    bool do2D{true};

    std::unordered_map<std::string, JSValue> js_functions;
    std::unordered_map<std::string, bool>    js_in_functions;
    JSValue global_obj;
    JSContext *ctx;
    bool jsPanic;

private:
   


    bool LoadModelShader();
    bool LoadDepthShader();
    void RenderLight(int index);
};