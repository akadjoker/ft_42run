
#include "pch.h"
#include "Scene.hpp"

// float lightLinear = 0.09f;
// float lightQuadratic = 0.032f;
// float lightIntensity =0.5f;

static u64 lastID = 0;

static VertexFormat::Element VertexElements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::TEXCOORD0, 2),
        VertexFormat::Element(VertexFormat::NORMAL, 3),
     //   VertexFormat::Element(VertexFormat::TANGENT, 3),
      //  VertexFormat::Element(VertexFormat::BITANGENT, 3),
    };   
static int VertexElemntsCount =3;

float lightLinear = 0.0334f;
float lightQuadratic = 0.0510f;
float lightIntensity =1.1679f;
bool useBloom = true;
struct Cascade
{
    float splitDepth;
    Mat4 viewProjMatrix;

};

CascadeShadow depthBuffer;
RenderQuad  quadRender;
const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
const int SHADOW_MAP_CASCADE_COUNT = 4;
Cascade cascades[SHADOW_MAP_CASCADE_COUNT];
Vec3 lightPosition = Vec3(0.5f, 4.0f,  7.5f);
TextureBuffer renderTexture;



void updateCascades(float nearClip, float farClip, const Mat4 & view, const Mat4 & proj,const Vec3 & lightPos)
	{
		float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];
     	float cascadeSplitLambda = 0.95f;

		float clipRange = farClip - nearClip;

		float minZ = nearClip;
		float maxZ = nearClip + clipRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

        for (u32 i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
        {
            float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
			float log = minZ * Pow(ratio, p);
			float uniform = minZ + range * p;
			float d = cascadeSplitLambda * (log - uniform) + uniform;
			cascadeSplits[i] = (d - nearClip) / clipRange;
        }
	
		// Calculate orthographic projection matrix for each cascade
		float lastSplitDist = 0.0;
		for (u32 i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) 
        {
			float splitDist = cascadeSplits[i];

			Vec3 frustumCorners[8] = 
            {
				Vec3(-1.0f,  1.0f, 0.0f),
				Vec3( 1.0f,  1.0f, 0.0f),
				Vec3( 1.0f, -1.0f, 0.0f),
				Vec3(-1.0f, -1.0f, 0.0f),
				Vec3(-1.0f,  1.0f,  1.0f),
				Vec3( 1.0f,  1.0f,  1.0f),
				Vec3( 1.0f, -1.0f,  1.0f),
				Vec3(-1.0f, -1.0f,  1.0f),
			};

			// Project frustum corners into world space
			Mat4 invCam =Mat4::Inverse(proj * view);   
			for (u32 j = 0; j < 8; j++) 
            {
				Vec4 invCorner = invCam * Vec4(frustumCorners[j], 1.0f);
                Vec4 div = invCorner / invCorner.w;
				frustumCorners[j] = Vec3(div.x, div.y, div.z);
			}

			for (u32 j = 0; j < 4; j++) 
            {
				Vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
				frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
				frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
			}

			// Get frustum center
			Vec3 frustumCenter = Vec3(0.0f);
			for (u32 j = 0; j < 8; j++) 
            {
				frustumCenter += frustumCorners[j];
			}
			frustumCenter /= 8.0f;

			float radius = 0.0f;
			for (u32 j = 0; j < 8; j++) 
            {
				float distance = Vec3::Length(frustumCorners[j] - frustumCenter);
				radius = Max(radius, distance);
			}
			radius = Ceil(radius * 16.0f) / 16.0f;

			Vec3 maxExtents = Vec3(radius);
			Vec3 minExtents = -maxExtents;

			Vec3 lightDir = Vec3::Normalize(-lightPos);
			Mat4 lightViewMatrix  = Mat4::LookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, Vec3(0.0f, 1.0f, 0.0f));
			Mat4 lightOrthoMatrix = Mat4::Ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);


      

			// Store split distance and matrix in cascade
			cascades[i].splitDepth = (nearClip + splitDist * clipRange) * -1.0f;
			cascades[i].viewProjMatrix = lightOrthoMatrix * lightViewMatrix;

			lastSplitDist = cascadeSplits[i];
		}
	}


Scene*  Scene::m_singleton = 0x0;


  

Scene& Scene::Instance()
{
    DEBUG_BREAK_IF(!m_singleton);
    return (*m_singleton);
}
Scene* Scene::InstancePtr()
{
    return m_singleton;

}

Scene::Scene()
{
    m_singleton = this;
}


Scene::~Scene()
{
    m_singleton = 0x0;
}

u32 Scene::AddEntity(const std::string &fileName, const std::string &name, bool castShadow)
{
    Entity *e = new Entity();
    if (e->Load(fileName))
    {
        e->id = lastID++;
        e->SetName(name);
        m_entities.push_back(e);
        e->shadow = castShadow;
        return e->id;
    }
    delete e;
    return -1;
}


u32 Scene::AddModel(const std::string &fileName, const std::string &name)
{
    Model *m = new Model();
    if (m->Load(fileName,VertexFormat(VertexElements, VertexElemntsCount)))
    {
        m->SetName(name);
        m_models.push_back(m);
        return m_models.size() - 1;
    }

    delete m;

    return -1;
}


u32 Scene::AddStaticNode(const std::string &name,bool castShadow)
{
    StaticNode *node = new StaticNode();
    node->id = lastID++;
    node->SetName(name);
    node->shadow = castShadow;
    m_nodes.push_back(node);
    return node->id;
}

bool Scene::NodeAddChild(u32 parent, u32 child)
{
    StaticNode *p = GetNode(parent);
    if (p!=nullptr)
    {
        Model *m = m_models[child];
        if (m == nullptr) return false;
        p->AddModel(m);
        return true;
    }

    // if (parent < (u32)m_nodes.size() && child < (u32)m_models.size())
    // {
    //     StaticNode *p = m_nodes[parent];
    //     if (p!=nullptr)
    //     {
    //         Model *m = m_models[child];
    //         if (m == nullptr) return false;
    //         p->AddModel(m);
    //         return true;
    //     }
    // }
    return false;
    
}

Entity *Scene::LoadEntity(const std::string &name, bool castShadow)
{

    Entity *e = new Entity();
    e->Load(name);
    m_entities.push_back(e);
    e->shadow = castShadow;
    return e;
   
}

Model *Scene::LoadModel(const std::string &name)
{
    Model *m = new Model();
    if (m->Load(name,VertexFormat(VertexElements, VertexElemntsCount)))
    {
        m_models.push_back(m);
        return m;
    }
    
    return nullptr;
}

StaticNode *Scene::CreateStaticNode(const std::string &name,bool castShadow)
{

    StaticNode *node = new StaticNode();
    node->SetName(name);
    node->shadow = castShadow;
    m_nodes.push_back(node);
  
    return node;
}

void Scene::SetLightPosition(u32 index,float x, float y, float z)
{

    if (index < MAX_LIGHTS)
    {
        m_lights[index].x = x;
        m_lights[index].y = y;
        m_lights[index].z = z;

    }

}
void Scene::SetLightColor(u32 index,float r, float g, float b)
{

    if (index < MAX_LIGHTS)
    {
        m_lights[index].r = r;
        m_lights[index].g = g;
        m_lights[index].b = b;
    }

}
void Scene::SetLightIntensity(u32 index,float intensity)
{

    if (index < MAX_LIGHTS)
    {
        m_lights[index].intensity = intensity;
    }

}
    

Model *Scene::CreateCube(float width, float height, float length)
{


    Model *model = new Model();

    float w = width/2;
    float h = height/2;
    float d = length/2;
    
    static Vec3 norms[]=
    {
		Vec3(0,0,-1),Vec3(1,0,0),Vec3(0,0,1),
		Vec3(-1,0,0),Vec3(0,1,0),Vec3(0,-1,0)
	};
	static Vec2 tex_coords[]=
    {
		Vec2(0,0),Vec2(1,0),Vec2(1,1),Vec2(0,1)
	};
	static int verts[]=
    {
		2,3,1,0,3,7,5,1,7,6,4,5,6,2,0,4,6,7,3,2,0,1,5,4
	};

    static BoundingBox box( Vec3(-w,-h,-d),Vec3(w,h,d) );

  

    Material * material= model->AddMaterial();
    material->texture = TextureManager::Instance().GetDefault();
    material->name = "default";


    Mesh *mesh = model->AddMesh(VertexFormat(VertexElements, VertexElemntsCount),0,false);

    

   for (int k=0;k<24;k+=4)
   {
        const Vec3 &normal=norms[k/4];

        for (int i=0;i<4;++i)
        {
            Vec3 pos = box.corner( verts[k+i] );
            mesh->AddVertex(pos,tex_coords[i],normal);
        }

        mesh->AddFace(k,k+1,k+2);
        mesh->AddFace(k,k+2,k+3);

   }
    //mesh->CalculateSmothNormals();
    //mesh->CalculateTangents();
    mesh->Upload();
    m_models.push_back(model);
    return model;    
}

Model *Scene::CreateQuad(float x, float y, float w, float h)
{

    Model *model = new Model();

    Material * material= model->AddMaterial();

    material->texture = TextureManager::Instance().GetDefault();

    material->name = "default";

    Mesh *mesh = model->AddMesh(VertexFormat(VertexElements, VertexElemntsCount),0,false);

    float W = w * 0.5f;
    float H = h * 0.5f;

    mesh->AddVertex(Vec3(x -W, y + H, 0.0f), Vec2(1.0f, 1.0f), Vec3(0.0f, 0.0f, -1.0f));
    mesh->AddVertex(Vec3(x +W, y + H, 0.0f), Vec2(0.0f, 1.0f), Vec3(0.0f, 0.0f, -1.0f));
    mesh->AddVertex(Vec3(x +W, y - H, 0.0f), Vec2(0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));
    mesh->AddVertex(Vec3(x -W ,y - H, 0.0f), Vec2(1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));


    mesh->AddFace(0, 1, 2);
    mesh->AddFace(0, 2, 3);

  //  mesh->CalculateTangents();
    mesh->Upload();

    m_models.push_back(model);
    mesh->m_boundingBox.Merge(mesh->GetBoundingBox());

    return model;




}



Model *Scene::CreatePlane(int stacks, int slices, int tilesX, int tilesY, float uvTileX , float uvTileY )
{
   
    Model *model = new Model();
   

    Material * material= model->AddMaterial();
    material->texture = TextureManager::Instance().GetDefault();
    material->name = "default";


    Mesh *mesh = model->AddMesh(VertexFormat(VertexElements, VertexElemntsCount),0,false);

    Vec3 center(-999999999.0f, 0.0f, -999999999.0f);



  
  for (int i = 0; i <= stacks; ++i) 
  {
        float y = static_cast<float>(i) / static_cast<float>(stacks)    ;
        for (int j = 0; j <= slices; ++j) 
        {
            float x = static_cast<float>(j) / static_cast<float>(slices) ;
            
               

                float uvX = x * uvTileX;
                float uvY = y * uvTileY;

                float pX = x * tilesX;
                float pY = y * tilesY;

                if (pX>center.x) center.x = pX;
                if (pY>center.z) center.z = pY;
                if (pX<center.x) center.x = pX;
                if (pY<center.z) center.z = pY;

             mesh->AddVertex(x * tilesX, 0.0f, y * tilesY, uvX, uvY, 0.0f, 1.0f, 0.0f);
            
        }
    }

    for (u32 i =0; i < mesh->GetVertexCount(); ++i)
    {
      Vec3 &v = mesh->positions[i];
       v.x -= center.x * 0.5f;
       v.z -= center.z * 0.5f;
    } 
  

    for (int i = 0; i < stacks; ++i) 
    {
        for (int j = 0; j < slices; ++j) 
        {
            u16 index = (slices + 1) * i + j;
            mesh->AddFace(index, index + slices + 1, index + slices + 2);
            mesh->AddFace(index, index + slices + 2, index + 1);
        }
    }
  
  
    if (mesh->normals.size() == mesh->positions.size())
    {

        for (u32 i = 0; i < mesh->GetIndexCount(); i += 3)
        {
            Plane3D plane = Plane3D(mesh->positions[mesh->indices[i]], mesh->positions[mesh->indices[i + 1]], mesh->positions[mesh->indices[i + 2]]);

           Vec3 normal = plane.normal;
            mesh->normals[mesh->indices[i]] = normal;
            mesh->normals[mesh->indices[i + 1]] = normal;
            mesh->normals[mesh->indices[i + 2]] = normal;

        }
    }
    mesh->m_boundingBox.Merge(mesh->GetBoundingBox());
   // mesh->CalculateTangents();
    mesh->Upload();
    m_models.push_back(model);

    return model;

} 

void Scene::Init()
{

    int width = Device::Instance().GetWidth();
    int height = Device::Instance().GetHeight();
    m_clearColor.Set(108,108,108,0);
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        m_lights.push_back(Light());
    }

    batch.Init(2, 1024 * 8);

    font = new Font();

    font->LoadDefaultFont();
    font->SetBatch(&batch);
    m_fonts.push_back(font);

    LoadModelShader();
    
    LoadDepthShader();

    depthBuffer.Init(SHADOW_MAP_CASCADE_COUNT,SHADOW_WIDTH, SHADOW_HEIGHT);

    quadRender.Init(width, height);

    renderTexture.Init(width, height);

    m_camera.Init(Vec3(0.0f, 0.5f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));
    m_camera.setNearClip(0.01f);
    m_camera.setFarClip(1000.0f);
    m_followCamera.Init(Vec3(0.0f, 1.0f, -5.0f));


    //  StaticNode *lights = CreateStaticNode("lights",true);

    

    // for (int i = 0; i < 10; i++)
    // {
    //     Model *cube =  CreateCube(0.5f,0.5f,0.5f);
    //     const Vec3  &lightPos = m_lights[i];
    //     Mat4 position= Mat4::Translate(lightPos.x, lightPos.y, lightPos.z);
    //     cube->Transform(position);
    //     lights->AddModel(cube);        
    //}





   
}


void Scene::Release()
{

    Logger::Instance().Info("Release scene");
    for (u32 i = 0; i < m_fonts.size(); i++)
    {
        m_fonts[i]->Release();
        delete m_fonts[i];
    }
    
    batch.Release();
    m_fonts.clear();
    m_quadShader.Release();
    m_debugShader.Release();
    depthBuffer.Release();
    m_depthShader.Release();
    m_modelShader.Release();
    renderTexture.Release();

    

    for (u32 i = 0; i < m_entities.size(); i++)
    {
        delete m_entities[i];
    }

    m_entities.clear();

    for (u32 i = 0; i < m_nodes.size(); i++)
    {
        delete m_nodes[i];
    }
    m_nodes.clear();

    for (u32 i = 0; i < m_models.size(); i++)
    {
        delete m_models[i];
    }

    m_models.clear();
    for (u32 i = 0; i < m_nodesToDelete.size(); i++)
    {
        delete m_nodesToDelete[i];
    }
    m_nodesToDelete.clear();

    

}
Entity *Scene::GetEntity(const std::string &name)
{

    for (u32 i = 0; i < m_entities.size(); i++)
    {
        if (m_entities[i]->name == name)
        {
            return m_entities[i];
        }
    }
    return nullptr;
 
}


StaticNode *Scene::GetNode(const std::string &name)
{

    for (u32 i = 0; i < m_nodes.size(); i++)
    {
        if (m_nodes[i]->name == name)
        {
            return m_nodes[i];
        }
    }
    return nullptr;
}


Model *Scene::GetModel(const std::string &name)
{

    for (u32 i = 0; i < m_models.size(); i++)
    {
        if (m_models[i]->GetName() == name)
        {
            return m_models[i];
        }
    }
    return nullptr;
}


Model *Scene::GetModel(int index)
{
    if (index >= 0 && index < m_models.size())
    {
        return m_models[index];
    }
    return nullptr;
}


Entity *Scene::GetEntity(int index)
{
   for (u32 i = 0; i < m_entities.size(); i++)
    {
        if (m_entities[i]->id == index)
        {
            return m_entities[i];
        }
    }
    return nullptr;
}


StaticNode *Scene::GetNode(int index)
{
    for (u32 i = 0; i < m_nodes.size(); i++)
    {
        if (m_nodes[i]->id == index)
        {
            return m_nodes[i];
        }
    }
    return nullptr;
}

int Scene::GetEntityByName(const std::string &name)
{

    for (u32 i = 0; i < m_entities.size(); i++)
    {
      //  LogInfo("[%d] %s - %s",i,m_entities[i]->name.c_str(),name.c_str());
        if (m_entities[i]->name == name)
        {
          //  LogInfo("FOUND [%d] %s",i,m_entities[i]->name.c_str());
            return i;
        }
    }
    return -1; 
    
}
int Scene::GetModelByName(const std::string &name)
{

    for (u32 i = 0; i < m_models.size(); i++)
    {
        if (m_models[i]->GetName() == name)
        {
            return i;
        }
    }
    return -1;

}

int Scene::GetNodeByName(const std::string &name)
{

    for (u32 i = 0; i < m_nodes.size(); i++)
    {
        if (m_nodes[i]->name == name)
        {
            return i;
        }
    }
    return -1;
}

bool Scene::RemoveModel(int index)
{
  

    if (index >= 0 && index < m_models.size())
    {
        delete m_models[index];
        m_models.erase(m_models.begin() + index);
        return true;
    }
    return false;
}
bool Scene::RemoveEntity(int index)
{
    for (u32 i = 0; i < m_entities.size(); i++)
    {
        if (m_entities[i]->id == index)
        {
            delete m_entities[i];
            m_entities.erase(m_entities.begin() + i);
            return true;
        }
    }

    // if (index >= 0 && index < m_entities.size())
    // {
    //     m_nodesToDelete.push_back(m_entities[index]);
    //     m_entities.erase(m_entities.begin() + index);
    //     return true;
    // }
    return false;
}
bool Scene::RemoveNode(int index)
{
    for (u32 i = 0; i < m_nodes.size(); i++)
    {
        if (m_nodes[i]->id == index)
        {
            delete m_nodes[i];
            m_nodes.erase(m_nodes.begin() + i);
            return true;
        }
    }

    // if (index >= 0 && index < m_nodes.size())
    // {
    //     m_nodesToDelete.push_back(m_nodes[index]);
    //     m_nodes.erase(m_nodes.begin() + index);
    //     return true;
    // }
    return false;
}

bool Scene::LoadFont(const char *fileName)
{
    Font *font = new Font();
    if (font->Load(fileName))
    {
        font->SetBatch(&batch);
        m_fonts.push_back(font);
        currentFont = m_fonts.size() - 1;
        return true;
    }
    delete font;
    return false;
}

bool Scene::SetFont(u32 index)
{
    if (index < m_fonts.size())
    {
        currentFont = index;
        return true;
    }
    return false;
}

void Scene::RemoveNodes()
{



   

    for (u32 i = 0; i < m_nodes.size(); i++)
    {
        delete m_nodes[i];
    }
    m_nodes.clear();

 


    for (u32 i = 0; i < m_nodesToDelete.size(); i++)
    {
        delete m_nodesToDelete[i];
    }
    m_nodesToDelete.clear();
}

void Scene::RemoveEntities()
{

    for (u32 i = 0; i < m_entities.size(); i++)
    {
        delete m_entities[i];
    }

    m_entities.clear();

    for (u32 i = 0; i < m_entities.size(); i++)
    {
        delete m_entities[i];
    }
    m_entities.clear();
}

void Scene::InitJs(JSContext *ctx, JSValue global_obj)
{
    this->ctx = ctx;
    this->global_obj = global_obj;
    jsPanic = false;

    js_in_functions["load"] = false;
    js_in_functions["unload"] = false;
    js_in_functions["update"] = false;
    js_in_functions["render"] = false;
    js_in_functions["collisions"] = false;
    js_in_functions["gui"] = false;
    js_in_functions["reload"] = false;
    

         JSValue js_value = JS_GetPropertyStr(ctx, global_obj, "update");
        if (JS_IsFunction(ctx, js_value)) 
        {
            js_functions["update"] = js_value;
            js_in_functions["update"] = true;
        }

        js_value = JS_GetPropertyStr(ctx, global_obj, "render");
        if (JS_IsFunction(ctx, js_value)) 
        {
            js_functions["render"] = js_value;
            js_in_functions["render"] = true;
        }

        js_value = JS_GetPropertyStr(ctx, global_obj, "gui");
        if (JS_IsFunction(ctx, js_value)) 
        {
            js_functions["gui"] = js_value;
            js_in_functions["gui"] = true;
        }

        js_value = JS_GetPropertyStr(ctx, global_obj, "collisions");
        if (JS_IsFunction(ctx, js_value)) 
        {
            js_functions["collisions"] = js_value;
            js_in_functions["collisions"] = true;
        }

        js_value = JS_GetPropertyStr(ctx, global_obj, "unload");
        if (JS_IsFunction(ctx, js_value)) 
        {
            js_functions["unload"] = js_value;
            js_in_functions["unload"] = true;
        }

        js_value = JS_GetPropertyStr(ctx, global_obj, "load");
        if (JS_IsFunction(ctx, js_value)) 
        {
            js_functions["load"] = js_value;
            js_in_functions["load"] = true;
        }

        js_value = JS_GetPropertyStr(ctx, global_obj, "reload");
        if (JS_IsFunction(ctx, js_value)) 
        {
            js_functions["reload"] = js_value;
            js_in_functions["reload"] = true;
        }
}

void Scene::LoadJs()
{
     if (js_in_functions["load"])
    {
        if (!jsPanic)
        {
            JSValue val = JS_Call(ctx, js_functions["load"], global_obj, 0, NULL);
            if (JS_IsException(val)) 
            {
                JSValue exception = JS_GetException(ctx);
                const char *error = JS_ToCString(ctx, exception);
                Logger::Instance().Error("[load] %s",error);
                JS_FreeCString(ctx, error);
                JS_FreeValue(ctx, exception);
                jsPanic = true;
            }   
            JS_FreeValue(ctx, val);      
        }
    }
}

void Scene::CloseJs()
{

    if (js_in_functions["unload"])
    {
        if (!jsPanic)
        {

            JSValue val = JS_Call(ctx, js_functions["unload"], global_obj, 0, NULL);
            if (JS_IsException(val)) 
            {
                JSValue exception = JS_GetException(ctx);
                const char *error = JS_ToCString(ctx, exception);
                Logger::Instance().Error("[unload] %s",error);
                JS_FreeCString(ctx, error);
                JS_FreeValue(ctx, exception);
                jsPanic = true;
            }   
            JS_FreeValue(ctx, val);      
        }
    }

    

    for (auto it = js_functions.begin(); it != js_functions.end(); it++)
    {
        JS_FreeValue(ctx, it->second);
    }
    ctx = nullptr;
    js_functions.clear();
    js_in_functions.clear();
    
    
}

void Scene::ReloadJs()
{

    if (js_in_functions["reload"])
    {
        if (!jsPanic)
        {
            JSValue val = JS_Call(ctx, js_functions["reload"], global_obj, 0, NULL);
            if (JS_IsException(val)) 
            {
                JSValue exception = JS_GetException(ctx);
                const char *error = JS_ToCString(ctx, exception);
                Logger::Instance().Error("[reload] %s",error);
                JS_FreeCString(ctx, error);
                JS_FreeValue(ctx, exception);
                jsPanic = true;
            }   
            JS_FreeValue(ctx, val);      
        }
    }

}


void Scene::RenderJs()
{

        if (js_in_functions["render"])
        {
            if (!jsPanic)
            {
                JSValue val = JS_Call(ctx, js_functions["render"], global_obj, 0, NULL);
                if (JS_IsException(val)) 
                {
                    JSValue exception = JS_GetException(ctx);
                    const char *error = JS_ToCString(ctx, exception);
                    Logger::Instance().Error("[render] %s",error);
                    JS_FreeCString(ctx, error);
                    JS_FreeValue(ctx, exception);
                    jsPanic = true;
                }   
                JS_FreeValue(ctx, val);      
            }
        }

}

void Scene::UpdateJs(float elapsed)
{

        if (js_in_functions["update"])
        {
            if (!jsPanic)
            {
                JSValue js_delta = JS_NewFloat64(ctx, elapsed);
                JSValue val = JS_Call(ctx, js_functions["update"], global_obj, 1, &js_delta);
                if (JS_IsException(val)) 
                {
                    JSValue exception = JS_GetException(ctx);
                    const char *error = JS_ToCString(ctx, exception);
                    Logger::Instance().Error("[update] %s",error);
                    JS_FreeCString(ctx, error);
                    JS_FreeValue(ctx, exception);
                    jsPanic = true;
                }   
                JS_FreeValue(ctx, val);      
            }
        }
}

void Scene::GuiJs()
{
        if (js_in_functions["gui"])
        {
            if (!jsPanic)
            {
                JSValue val = JS_Call(ctx, js_functions["gui"], global_obj, 0, NULL);
                if (JS_IsException(val)) 
                {
                    JSValue exception = JS_GetException(ctx);
                    const char *error = JS_ToCString(ctx, exception);
                    Logger::Instance().Error("[gui] %s",error);
                    JS_FreeCString(ctx, error);
                    JS_FreeValue(ctx, exception);
                    jsPanic = true;
                }   
                JS_FreeValue(ctx, val);      
            }
        }
    
}

void Scene::RenderLight(int index)
{

}


void Scene::Render()

{
    

    

         int width = Device::Instance().GetWidth();
        int height = Device::Instance().GetHeight();

        projectionMatrix = m_camera.GetProjectionMatrix((float)width / (float)height) ;
        viewMatrix=m_camera.GetViewMatrix();
       


    if (do3D)
    {
    
       updateCascades(m_camera.getNearClip(), m_camera.getFarClip(),viewMatrix, projectionMatrix, lightPosition);



        Mat4 lightProjection;
        Mat4 lightView;
        Mat4 lightSpaceMatrix;




        Driver::Instance().SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        Driver::Instance().SetBlend(true);
        Driver::Instance().SetBlendMode(BlendMode::BLEND);
        Driver::Instance().SetDepthTest(true);
        Driver::Instance().SetDepthClamp(true);


        m_depthShader.Use();
        glCullFace(GL_FRONT);
        depthBuffer.Begin();

            for (u32 j = 0; j < SHADOW_MAP_CASCADE_COUNT; j++)
            {

                depthBuffer.Set(j);
                lightSpaceMatrix = cascades[j].viewProjMatrix;
                m_depthShader.SetMatrix4("lightSpaceMatrix", &lightSpaceMatrix.c[0][0]);
                m_depthShader.SetInt("isStatic", 1);
                for (u32 i = 0; i < m_nodes.size(); i++)
                {
                    StaticNode *node = m_nodes[i];
                    if (!node->shadow || !node->visible) continue;
                    Mat4 m = m_nodes[i]->GetRelativeTransformation();
                    m_depthShader.SetMatrix4("model", m.x);
                    m_nodes[i]->RenderNoMaterial();
                }

                m_depthShader.SetInt("isStatic", 0);

                for (u32 i = 0; i < m_entities.size(); i++)
                {
                    Entity *entity = m_entities[i];
                    if (!entity->shadow || !entity->visible) continue;
                    Mat4 mat = entity->GetRelativeTransformation();
                    m_depthShader.SetMatrix4("model", mat.x);
                    for (u32 b = 0; b < entity->bones.size(); b++)
                    {
                        m_depthShader.SetMatrix4(System::Instance().TextFormat("Joints[%d]", b), entity->bones[b].x); // model.bones
                    }
                    entity->Render();
                }
            }

        depthBuffer.End();
        glCullFace(GL_BACK);
        
        
        Driver::Instance().SetViewport(0, 0, width, height);

        float r = m_clearColor.r / 255.0f;
        float g = m_clearColor.g / 255.0f;
        float b = m_clearColor.b / 255.0f;
        
        
        Driver::Instance().SetClearColor(r,g,b, 1.0f);
        Driver::Instance().Clear();
    

   // cameraPosition.set(-0.5f,0.5f,0.9f);
        Vec3 cameraPosition = m_camera.position;

        // lightDir = glm::normalize(glm::vec3(20.0f, 50, 20.0f));
        Vec3 lightDir = Vec3::Normalize(Vec3(-20.0f, 50, 20.0f));
       // Vec3 lightUp = Vec3(0.0f, 0.0f, 0.0f);

        m_modelShader.Use();
        m_modelShader.SetMatrix4("view", viewMatrix.x);
        m_modelShader.SetMatrix4("projection", projectionMatrix.x);
        m_modelShader.SetFloat("viewPos", cameraPosition.x, cameraPosition.y, cameraPosition.z);
        m_modelShader.SetFloat("lightDir", lightDir.x, lightDir.y, lightDir.z);
        m_modelShader.SetFloat("farPlane", m_camera.getFarClip());

        for (u32 i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
        {
            m_modelShader.SetMatrix4("cascadeshadows[" + std::to_string(i) + "].projViewMatrix", &cascades[i].viewProjMatrix.c[0][0]);
            m_modelShader.SetFloat("cascadeshadows[" + std::to_string(i) + "].splitDistance", cascades[i].splitDepth);
            m_modelShader.SetInt("shadowMap[" + std::to_string(i) + "]", i + 1);
            
    }
     





    for (int i = 0; i < m_lights.size(); i++)
    {
        const Light &light = m_lights[i];
        float lightIntensity = light.intensity;

        m_modelShader.SetFloat("lights[" + std::to_string(i) + "].Color", light.r, light.g, light.b);
        m_modelShader.SetFloat("lights[" + std::to_string(i) + "].Position", light.x, light.y, light.z);
        m_modelShader.SetFloat("lights[" + std::to_string(i) + "].Intensity", lightIntensity);
        m_modelShader.SetFloat("lights[" + std::to_string(i) + "].Linear", lightLinear);
        m_modelShader.SetFloat("lights[" + std::to_string(i) + "].Quadratic", lightQuadratic);
    }

    
    m_modelShader.SetInt("isStatic", 1);

      

    if (useBloom)   renderTexture.Begin();

   //    printf("rendering %ld nodes  %ld entities\n",m_nodes.size(), m_entities.size());

    depthBuffer.BindTextures(1);  
    
    for (u32 i = 0; i < m_nodes.size(); i++)
    {
        if (!m_nodes[i]->visible) continue;
        Mat4 m = m_nodes[i]->GetRelativeTransformation();
        m_modelShader.SetMatrix4("model", m.x);
        m_nodes[i]->Render();
    }

    m_modelShader.SetInt("isStatic", 0);
    //player don't have render shadow in body

    Driver::Instance().SetTextureId(1, 0);
    Driver::Instance().SetTextureId(2, 0);
    Driver::Instance().SetTextureId(3, 0);
    Driver::Instance().SetTextureId(4, 0);

    for (u32 i = 0; i < m_entities.size(); i++)
    {
        Entity *entity = m_entities[i];
        if (!entity->visible) continue;
        Mat4 mat = entity->GetRelativeTransformation();
        m_modelShader.SetMatrix4("model", mat.x);
        for (u32 b = 0; b < entity->bones.size(); b++)
        {
             m_modelShader.SetMatrix4(System::Instance().TextFormat("Joints[%d]", b), entity->bones[b].x); // model.bones
        }
        entity->Render();
    }


        
        Driver::Instance().SetDepthTest(true);
        Driver::Instance().SetBlend(false);
        Driver::Instance().SetCullFace(false);
        Mat4 mvp = projectionMatrix * viewMatrix;
        batch.SetMatrix(mvp);   
        batch.SetColor(255, 255, 255, 255);
        RenderJs();
        //batch.Grid(10, 10, true);
        //batch.Cube(Vec3(0.0f, 0.5f, 1.0f), 10.0f, 10.0f, 10.0f);
        batch.Render();




   if (useBloom) renderTexture.End();

  
  

    if (useBloom)
    {
     renderTexture.BindTexture(0);
     m_quadShader.Use();
     Driver::Instance().SetViewport(0, 0, width, height);
     quadRender.Render();
    }
} else 
{


        Driver::Instance().SetBlendMode(BlendMode::BLEND);
        Driver::Instance().SetViewport(0, 0, width, height);
        
        float r = m_clearColor.r / 255.0f;
        float g = m_clearColor.g / 255.0f;
        float b = m_clearColor.b / 255.0f;
        
        
        Driver::Instance().SetClearColor(r,g,b, 1.0f);
        Driver::Instance().Clear();
}

//batch 2d
if (do2D)
{
    Driver::Instance().SetDepthTest(false);
    Driver::Instance().SetBlend(true);
    Mat4 view2D = Mat4::Ortho(0.0f, width, height, 0.0f, -5.0f, 5.0f);
    batch.SetMatrix(view2D);
    GuiJs();
    batch.SetColor(255, 255, 255, 255);
  //  font->SetFontSize(12);
  //  font->Print(10, 10, System::Instance().TextFormat("FPS: %d", Device::Instance().GetFPS()));

    batch.Render();
}

    //   depthBuffer.BindTexture(1);   
    //   quadRender.Render(-0.8f,0.35f,0.4f);

    //   depthBuffer.BindTexture(2);   
    //   quadRender.Render(-0.8f,-0.1f,0.4f);


}

void Scene::Update(float elapsed)
{
    UpdateJs(elapsed);
    if (do3D)
    {

        int screenWidth = Device::Instance().GetWidth();
        int screenHeight = Device::Instance().GetHeight();
        
        projectionMatrix = m_camera.GetProjectionMatrix((float)screenWidth / (float)screenHeight) ;
        // if (Keyboard::Pressed(KEY_ENTER))
        // {
        //     m_followCameraMode = !m_followCameraMode;
        // }
        


        //followCamera.Update(deltaTime, game.player->getWorldPosition());
//        followCamera.Update( playerActor->getWorldPosition()+ Vec3(0.0f, 1.0f, 0.0f));

        viewMatrix=m_camera.GetViewMatrix();

        if (m_followCameraMode)
        {
             viewMatrix = m_followCamera.GetViewMatrix();
           //  camera.position = playerActor->getWorldPosition()+ Vec3(0.0f, 5.0f, 0.0f);
             m_camera.Yaw = 0.0f;
             m_cameraPosition = m_followCamera.GetCameraLocation();


        } else 
        {
            viewMatrix = m_camera.GetViewMatrix();
            m_cameraPosition = m_camera.position;
        }   

     //   Mat4 view = followCamera.GetViewMatrix();
      

        
   


        for (u32 i = 0; i < m_nodes.size(); i++)
        {
            if (m_nodes[i]->done)
            {
                m_nodesToDelete.push_back(m_nodes[i]);
                m_nodes.erase(m_nodes.begin() + i);
                i--;
                continue;
            }
            if (!m_nodes[i]->active) continue;
            
            m_nodes[i]->Update();
        }
    

        for (u32 i = 0; i < m_entities.size(); i++)
        {
            Entity *entity = m_entities[i];
            if (!entity->active) continue;
            entity->UpdateAnimation(elapsed);
            entity->Update();
        }

        for (u32 i = 0; i < m_nodesToDelete.size(); i++)
        {
            delete m_nodesToDelete[i];
        }
        m_nodesToDelete.clear();
    }
}

bool Scene::LoadModelShader()
{
     const char *vShader = GLSL(
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec2 aTexCoords;
        layout(location = 2) in vec3 aNormal;

        layout(location = 5) in ivec4 aBones;
        layout(location = 6) in vec4 aWeights;


        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoords;

        out vec3 outViewPosition;
        out vec4 outWorldPosition;


        uniform vec3 viewPos;

        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;

        uniform int isStatic; 
  
        const int MAX_BONES = 80;
        const int MAX_BONE_INFLUENCE = 4;
        uniform mat4 Joints[MAX_BONES];


   

        void main() 
        {

            if (isStatic==1)
            {
                vec4 initPos = vec4(aPos, 1.0);

                FragPos = vec3(model * initPos);
                Normal = transpose(inverse(mat3(model))) * aNormal;

                mat4 modelViewMatrix = view * model;
                vec4 mvPosition =  modelViewMatrix * initPos;
                outViewPosition  = mvPosition.xyz;
                outWorldPosition = model * initPos;                
                
                gl_Position = projection * view * model * initPos;


            } else 
            {
            vec4 totalPosition = vec4(0.0f);
             vec4 totalNormal = vec4(0.0f);
            for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
            {
                if (aBones[i] == -1)
                    continue;
                if (aBones[i] >= MAX_BONES)
                {
                    totalPosition = vec4(aPos, 1.0f);
                    break;
                }
                vec4 localPosition = Joints[aBones[i]] * vec4(aPos, 1.0f);
                totalPosition += localPosition * aWeights[i];
                vec4 localNormal = Joints[aBones[i]] * vec4(aNormal, 0.0f); 
                totalNormal += localNormal * aWeights[i];
            }

                mat4 viewModel = view * model;
                
                Normal = mat3(transpose(inverse(viewModel))) * totalNormal.xyz; 

                FragPos = (model * totalPosition).xyz; 

           

                
                mat4 modelViewMatrix = view * model;
                vec4 mvPosition =  modelViewMatrix * totalPosition;
                outViewPosition  = mvPosition.xyz;
                outWorldPosition = model * totalPosition;    

                gl_Position = projection * viewModel * totalPosition;    

            
            }

            TexCoords = aTexCoords;
        }
    );

     const char *fShader = GLSL(

         const int NUM_CASCADES = 4;
         const int NR_LIGHTS = 10;

         out vec4 FragColor;

         in vec3 FragPos;
         in vec2 TexCoords;
         in vec3 Normal;

         in vec3 outViewPosition;
         in vec4 outWorldPosition;

         struct CascadeShadow {
             mat4 projViewMatrix;
             float splitDistance;
         };

         struct Light {
             vec3 Position;
             vec3 Color;
             float Intensity;
             float Linear;
             float Quadratic;
         };

         uniform Light lights[NR_LIGHTS];

         uniform sampler2D diffuseTexture;
         uniform CascadeShadow cascadeshadows[NUM_CASCADES];
         uniform sampler2D shadowMap[NUM_CASCADES];

         uniform vec3 lightDir;
         uniform float farPlane;
         uniform vec3 viewPos;

         const float constant = 1.0;
         const float linear = 0.09;
         const float quadratic = 0.032;
         const float shininess = 32.0;
         const float E = 2.71828;

         float CalculateShadow(vec4 worldPosition, int idx) {
             vec2 texelSize = 1.0 / textureSize(shadowMap[idx], 0);
             vec4 shadowMapPosition = cascadeshadows[idx].projViewMatrix * worldPosition;
             vec4 projCoords = (shadowMapPosition / shadowMapPosition.w) * 0.5 + 0.5;

             float currentDepth = projCoords.z;

             // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
             if (currentDepth > 1.0)
             {
                 return 0.0;
             }

             vec3 normal = normalize(Normal);
             float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
             const float biasModifier = 0.5f;

             if (idx == NUM_CASCADES)
             {
                 bias *= 1 / (farPlane * biasModifier);
             }
             else
             {
                 bias *= 1 / (cascadeshadows[idx].splitDistance * biasModifier);
             }

             float shadow = 0.0;
             for (int x = -1; x <= 1; ++x)
             {
                 for (int y = -1; y <= 1; ++y)
                 {
                     vec2 texel = projCoords.xy + vec2(x, y) * texelSize;
                     float pcfDepth = texture(shadowMap[idx], texel).r;
                     shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
                 }
             }
             shadow /= 10.0;

             return shadow;
         }

         void main() {
             vec3 color = texture(diffuseTexture, TexCoords).rgb;
             vec3 viewDir = normalize(viewPos - FragPos);

             int cascadeIndex = 0;
             for (int i = 0; i < NUM_CASCADES - 1; i++)
             {
                 if (outViewPosition.z < cascadeshadows[i].splitDistance)
                 {
                     cascadeIndex = i + 1;
                 }
             }


             //vec3 totalDiffuse = vec3(0.0);
             //    vec3 totalSpecular= vec3(0.0);

             // luz para shadow "sol"
             vec3 normal = normalize(Normal);
             vec3 lightColor = vec3(0.4);
             vec3 ambient = 0.2 * color;
             float diff = max(dot(lightDir, normal), 0.0);
             vec3 diffuse = diff * lightColor;

             vec3 reflectDir = reflect(-lightDir, normal);
             float spec = 0.0;
             vec3 halfwayDir = normalize(lightDir + viewDir);
             spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
             vec3 specular = spec * lightColor;

             float shadow = CalculateShadow(outWorldPosition, cascadeIndex);

             vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
             // vec3 lighting = vec3(0.0);

             //    vec3 totalDiffuse = vec3(0.0);
             //    vec3 totalSpecular= vec3(0.0);

             vec3 Colors = vec3(0.0);

             for (int i = 0; i < NR_LIGHTS; ++i)
             {
                 // diffuse
                 vec3 lightDir = normalize(lights[i].Position - FragPos);
                 float diff = max(dot(Normal, lightDir), 0.0);

                 // vec3 diffuse = lights[i].Color * diff * color.rgb * lights[i].Intensity;

                 vec3 diffuse = color.rgb * diff * lights[i].Color * lights[i].Intensity;

                 // // specular
                 //  vec3 halfwayDir = normalize(lightDir + viewDir);
                 //  float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
                 //  vec3 specular =color.rgb  * spec *  lights[i].Color ;

                 // specular
                 float specularStrength = lights[i].Intensity;
                 vec3 viewDir = normalize(viewPos - FragPos);
                 vec3 reflectDir = reflect(-lightDir, Normal);
                 float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1.0);
                 vec3 specular = specularStrength * spec * color.rgb;

                 // attenuation
                 float distance = length(lights[i].Position - FragPos);

                 // if (distance > farPlane/2.0f) continue;
                 float attenuation = 1.0 / (constant + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
                 diffuse *= attenuation;
                 specular *= attenuation;

                 Colors += diffuse + specular;
             }

             vec3 finalColor = lighting + Colors;

             float fogDistance = length(viewPos - FragPos);
             //float fogDistance = gl_FragCoord.z / gl_FragCoord.w;
            
             vec3 fogColor = vec3(0.5, 0.5, 0.5);
             float fogDensity = 0.008;
             float fogFactor = 1.0 / pow(E, fogDistance * fogDensity);
            
             fogFactor = clamp(fogFactor, 0.0, 1.0);

             

            vec3 fogColorFinal = fogFactor *  finalColor + (1.0 - fogFactor) * fogColor;

             FragColor = vec4(fogColorFinal, 1.0);

             // FragColor = vec4(lighting + Colors, 1.0);
         }

     );
     if (!m_modelShader.Create(vShader, fShader))
     {
         DEBUG_BREAK_IF(true);
   }
    m_modelShader.LoadDefaults();
    m_modelShader.SetInt("diffuseTexture", 0);
    m_modelShader.SetInt("isStatic", 1);
    m_modelShader.SetFloat("viewPos", 0.0f, 0.0f, 0.0f);
    for (u32 i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
    {
        m_modelShader.SetInt("shadowMap["+std::to_string(i)+"]", i + 1);
    }


    // float y =4.5f;
    // float x =0.2f;

    // m_lights.push_back(Vec3( 0.0f, y, -38.0f));
    // m_lights.push_back(Vec3( 0.0f, y, -28.0f));
    // //m_lights.push_back(Vec3( 0.0f, y, -18.0f));
    // m_lights.push_back(Vec3( x, y, -7.0f));
    // m_lights.push_back(Vec3( x, y, 3.0f));
    // m_lights.push_back(Vec3( x, y, 10.0f));
    // m_lights.push_back(Vec3( x, y, 15.0f));
    // m_lights.push_back(Vec3( x, y, 24.0f));
    
    // m_lights.push_back(Vec3( x, y, 36.0f));//5
    // m_lights.push_back(Vec3( x, y, 47.0f));//6


    // m_lights.push_back(Vec3( x, y, 58.0f));
    // m_lights.push_back(Vec3( x, y, 68.0f));
    
    // m_lights.push_back(Vec3( 0.0f, y, 78.0f));
    // m_lights.push_back(Vec3( 0.0f, y, 88.0f));
    // m_lights.push_back(Vec3( 0.0f, y, 98.0f));
  
    
    // Vec3 color = Vec3(0.5f, 0.5f, 0.5f);


    for (int i = 0; i < m_lights.size(); i++)
    {
        m_modelShader.SetFloat("lights[" + std::to_string(i) + "].Position", 10000,1000,1000);
        m_modelShader.SetFloat("lights[" + std::to_string(i) + "].Color", 0,0,0);
        m_modelShader.SetFloat("lights[" + std::to_string(i) + "].Intensity", 0);
        m_modelShader.SetFloat("lights[" + std::to_string(i) + "].Linear", lightLinear);
        m_modelShader.SetFloat("lights[" + std::to_string(i) + "].Quadratic", lightQuadratic);

    }

    // m_modelShader.SetFloat("lights[10].Intensity", 0.25f);
    // m_modelShader.SetFloat("lights[3].Intensity", 0.25f);
    // m_modelShader.SetFloat("lights[8].Intensity", 0.95f);
 //   m_modelShader.SetFloat("lights[6].Intensity", 0.25f);
 //   m_modelShader.SetFloat("lights[6].Color", 1.0f, 1.0f, 0.0f);








    return false;
}


bool Scene::LoadDepthShader()
{

    {
    
        const char *vertexShaderSource = GLSL(
                layout (location = 0) in vec3 aPos;
                layout(location = 5) in ivec4 aBones;
                layout(location = 6) in vec4 aWeights;

      
            uniform mat4 model;
            uniform int isStatic;

            const int MAX_BONES = 80;
            const int MAX_BONE_INFLUENCE = 4;
            uniform mat4 Joints[MAX_BONES];

              uniform mat4 lightSpaceMatrix;
            

            void main()
            {
                    if (isStatic==1)
                    {
                        gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
                    }
                    else 
                    {
                        vec4 totalPosition = vec4(0.0f);
                        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
                        {
                            if (aBones[i] == -1)
                                continue;
                            if (aBones[i] >= MAX_BONES)
                            {
                                totalPosition = vec4(aPos, 1.0f);
                                break;
                            }
                            vec4 localPosition = Joints[aBones[i]] * vec4(aPos, 1.0f);
                            totalPosition += localPosition * aWeights[i];
                        }

                        gl_Position =  lightSpaceMatrix*model * totalPosition;   
                    }
            }
            
           );

        const char *fragmentShaderSource = GLSL(

              

                void main()
                {
                    
                    
                }  
        );


        LogWarning("Loading Depth Shader");

        if (!m_depthShader.Create(vertexShaderSource, fragmentShaderSource))
        {
            DEBUG_BREAK_IF(true);
            return false;
        }
        m_depthShader.LoadDefaults();
        m_depthShader.SetInt("isStatic", 1);
}
       

{
      LogWarning("Loading Debug Depth Shader");
            const char *vertexShaderSource = GLSL(
                            layout (location = 0) in vec3 aPos;
                            layout (location = 1) in vec2 aTexCoords;

                            out vec2 TexCoords;

                            void main()
                            {
                                TexCoords = aTexCoords;
                                gl_Position = vec4(aPos, 1.0);
                            }
                );

                const char *fragmentShaderSource = GLSL(
                    out vec4 FragColor;

                    in vec2 TexCoords;

                    uniform sampler2D depthMap;
                    uniform float near_plane;
                    uniform float far_plane;

                    // required when using a perspective projection matrix
                    float LinearizeDepth(float depth)
                    {
                        float z = depth * 2.0 - 1.0; // Back to NDC 
                        return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
                    }

                    void main()
                    {             
                        float depthValue = texture(depthMap, TexCoords).r;
                        // FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
                        FragColor = vec4(vec3(depthValue), 1.0); // orthographic
                    }
                    );

       if (!m_debugShader.Create(vertexShaderSource, fragmentShaderSource))
       {
           DEBUG_BREAK_IF(true);
           return false;
       }
        m_debugShader.LoadDefaults();
        m_debugShader.SetInt("depthMap", 0);
}



    {

        const char *vertexShaderSource = GLSL(
                layout (location = 0) in vec3 aPos;
                layout (location = 1) in vec2 aTexCoords;

                out vec2 TexCoords;

                void main()
                {
                    TexCoords = aTexCoords;
                    gl_Position = vec4(aPos, 1.0);
                }
        );

        const char *fragmentShaderSource = GLSL(
                out vec4 FragColor;

                in vec2 TexCoords;

                uniform sampler2D textureMap;


                    //Bloom
                const float Samples = 8.0;          // Pixels per axis; higher = bigger glow, worse performance
                const float Quality = 3.0;          // Defines size factor: Lower = smaller glow, better quality


                //blur
                float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
                float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

                vec4 Bloom(float samples, float quality);
                vec4 Blur();

                void main()
                {
                    



                    //FragColor = texture(textureMap, TexCoords);
                    FragColor = Bloom(Samples, Quality);
                }
                vec4 Bloom(float samples, float quality)
                {
                    vec2 size  = textureSize(textureMap, 0);
                    vec4 sum = vec4(0);
                    vec2 sizeFactor = vec2(1)/size*quality;
                    vec4 colDiffuse = vec4(1,1,1,1);
                    

              
                    vec4 source = texture(textureMap, TexCoords);

                    const int range = 2;            // should be = (samples - 1)/2;

                    for (int x = -range; x <= range; x++)
                    {
                        for (int y = -range; y <= range; y++)
                        {
                            sum += texture(textureMap, TexCoords + vec2(x, y)*sizeFactor);
                        }
                    }

                    
                    vec4 color = ((sum/(samples*samples)) + source)*colDiffuse;
                    return color;
                }
                vec4 Blur()
                {
                     vec3 texelColor = texture(textureMap, TexCoords).rgb*weight[0];
                     vec2 size  = textureSize(textureMap, 0);

                    for (int i = 1; i < 3; i++)
                    {
                        texelColor += texture(textureMap, TexCoords + vec2(offset[i])/size.x, 0.0).rgb*weight[i];
                        texelColor += texture(textureMap, TexCoords - vec2(offset[i])/size.x, 0.0).rgb*weight[i];
                    }

                    return   vec4(texelColor, 1.0);

                }
        );
        
        LogWarning("Loading Quad Shader");
        if (!m_quadShader.Create(vertexShaderSource, fragmentShaderSource))
        {
            DEBUG_BREAK_IF(true);
            return false;
        }
        m_quadShader.LoadDefaults();
        m_quadShader.SetInt("textureMap", 0);

    }

        return true;
}

void Scene::SetCameraPosition(const Vec3 &p)
{
   m_camera.position = p;
}

void Scene::MoveCamera(int mode, float deltaTime)
{
   
    // FORWARD,
    // BACKWARD,
    // LEFT,
    // RIGHT
    m_camera.ProcessKeyboard(mode, deltaTime);
}
void Scene::RotateCameraByMouse(float dt)
{
    m_camera.MouseView(dt);
}


void Scene::SetFollowOffset(const Vec3 &offset)
{
    m_followCamera.SetCameraOffset(offset);
}
void Scene::SetFollowCameraPosition(const Vec3 &p)
{
    m_followCamera.SetTargetPosition(p);
}



//*******************************************************************************************************
//                                  scene SCRIPT
//*******************************************************************************************************
static JSValue js_scene_load_entity(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 3)
    {
        return JS_ThrowReferenceError(ctx, "load_entity: Wrong number of arguments(fileName,name,castShadow)");
    }

    const char* fileName = JS_ToCString(ctx, argv[0]);
    const char* name = JS_ToCString(ctx, argv[1]);
    bool castShadow = JS_ToBool(ctx, argv[2]);

    u32 result =  Scene::Instance().AddEntity(fileName,name,castShadow);



    

    JS_FreeCString(ctx, name);
    JS_FreeCString(ctx, fileName);
    
    return  JS_NewInt32(ctx, result);
}

static JSValue js_scene_get_entity_id(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "get_entity_id: Wrong number of arguments(name)");
    }

    const char* name = JS_ToCString(ctx, argv[0]);

    int result =  Scene::Instance().GetEntityByName(name);

    if (result == -1)
    {
        return JS_ThrowReferenceError(ctx, "get_entity_id: Entity not found");
    }
    JS_FreeCString(ctx, name);

    return  JS_NewInt32(ctx, result);
}

static JSValue js_scene_get_node_id(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "get_node_id: Wrong number of arguments(name)");
    }

    const char* name = JS_ToCString(ctx, argv[0]);

    int result =  Scene::Instance().GetNodeByName(name);

    if (result == -1)
    {
        return JS_ThrowReferenceError(ctx, "get_node_id: Node %s not found", name);
    }
    JS_FreeCString(ctx, name);

    return  JS_NewInt32(ctx, result);
}

static JSValue js_scene_get_model_id(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "get_model_id: Wrong number of arguments(name)");
    }

    const char* name = JS_ToCString(ctx, argv[0]);

    int result =  Scene::Instance().GetModelByName(name);

    if (result == -1)
    {
        return JS_ThrowReferenceError(ctx, "get_model_id: Model[%s] not found", name);
    }
    JS_FreeCString(ctx, name);

    return  JS_NewInt32(ctx, result);
}

static JSValue js_scene_load_model(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 2)
    {
        return JS_ThrowReferenceError(ctx, "load_model: Wrong number of arguments(fileName,name)");
    }
    const char* fileName = JS_ToCString(ctx, argv[0]);
    const char* name = JS_ToCString(ctx, argv[1]);
    
    u32 result =  Scene::Instance().AddModel(fileName,name);  

    JS_FreeCString(ctx, name);
    JS_FreeCString(ctx, fileName);
    
    return  JS_NewInt32(ctx, result);
}


static JSValue js_scene_load_static_node(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 2)
    {
        return JS_ThrowReferenceError(ctx, "load_static_node: Wrong number of arguments(name,cast_Shadow)");
    }
    
    const char* name = JS_ToCString(ctx, argv[0]);
    bool castShadow = JS_ToBool(ctx, argv[1]);
    
    u32 result =  Scene::Instance().AddStaticNode(name,castShadow);

    JS_FreeCString(ctx, name);
    
    
    return  JS_NewInt32(ctx, result);
}


static JSValue js_scene_remove_model(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "remove_model: Wrong number of arguments(index)");
    }
    int index;
    JS_ToInt32(ctx, &index,argv[0]);
    bool result = Scene::Instance().RemoveModel(index);
    return  JS_NewBool(ctx, result);
}

static JSValue js_scene_remove_entity(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "remove_entity: Wrong number of arguments(index)");
    }
    int index;
    JS_ToInt32(ctx, &index,argv[0]);
    bool result = Scene::Instance().RemoveEntity(index);
    return  JS_NewBool(ctx, result);
}

static JSValue js_scene_remove_node(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "remove_node: Wrong number of arguments(index)");
    }
    int index;
    JS_ToInt32(ctx, &index,argv[0]);
    bool result = Scene::Instance().RemoveNode(index);
    if (result == false)
    {
        return JS_ThrowReferenceError(ctx, "remove_node: Node [%d] not found", index);
    }
    return  JS_NewBool(ctx, result);
}



static JSValue js_scene_create_cube(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    double width, height, length;
    if (argc != 3)
    {
         return JS_ThrowReferenceError(ctx, "create_cube: Wrong number of arguments(width,height,length)");
         
    }

    JS_ToFloat64(ctx, &width,argv[0]);
    JS_ToFloat64(ctx, &height,argv[1]);
    JS_ToFloat64(ctx, &length,argv[2]);

    Scene::Instance().CreateCube(width, height, length);

    return JS_NewInt32(ctx,Scene::Instance().GetNodeCount());
}

static JSValue js_scene_create_pane(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
   //int stacks, int slices, int tilesX, int tilesY,float uvTileX =1.0f, float uvTileY =1.0f);

   int stacks, slices, tilesX, tilesY;
   double uvTileX =1.0f, uvTileY =1.0f;

   if (argc == 6)
   {
        
        JS_ToInt32(ctx, &stacks,argv[0]);
        JS_ToInt32(ctx, &slices,argv[1]);
        JS_ToInt32(ctx, &tilesX,argv[2]);
        JS_ToInt32(ctx, &tilesY,argv[3]);
        JS_ToFloat64(ctx, &uvTileX,argv[4]);
        JS_ToFloat64(ctx, &uvTileY,argv[5]);
   } else 
   if (argc == 4)
   {
        JS_ToInt32(ctx, &stacks,argv[0]);
        JS_ToInt32(ctx, &slices,argv[1]);
        JS_ToInt32(ctx, &tilesX,argv[2]);
        JS_ToInt32(ctx, &tilesY,argv[3]);
   } else 
   {
    return JS_ThrowReferenceError(ctx, "create_plane: Wrong number of arguments(stacks,slices,tilesX,tilesY [uvTileX,uvTileY])");
   }
    Scene::Instance().CreatePlane(stacks,slices,tilesX,tilesY,uvTileX,uvTileY);
   
    return JS_NewInt32(ctx,Scene::Instance().GetNodeCount());
}

static JSValue js_scene_create_quad(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    double x,y,w,h;
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "create_quad: Wrong number of arguments(x,y,width,height)");
    }

    JS_ToFloat64(ctx, &x,argv[0]);
    JS_ToFloat64(ctx, &y,argv[1]);
    JS_ToFloat64(ctx, &w,argv[2]);
    JS_ToFloat64(ctx, &h,argv[3]);
    
    Scene::Instance().CreateQuad(x,y,w,h);
    
    return JS_NewInt32(ctx,Scene::Instance().GetNodeCount()-1);
}




static JSValue js_node_add_model(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    
    if (argc != 2)
    {
        return JS_ThrowReferenceError(ctx, "node_add_model: Wrong number of arguments(nodeIndex, modelIndex)");
    }
    int nodeIndex, modelIndex;
    JS_ToInt32(ctx, &nodeIndex,argv[0]);
    JS_ToInt32(ctx, &modelIndex,argv[1]);
    if (!Scene::Instance().NodeAddChild(nodeIndex, modelIndex))
    {
        return JS_ThrowReferenceError(ctx, "node_add_model: fail add model");
    }
    return JS_NULL;
}

static JSValue js_entity_add_animation(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    
    if (argc != 2)
    {
        return JS_ThrowReferenceError(ctx, "enttty_add_animation: Wrong number of arguments(entityIndex, filename)");
    }
    int entityIndex;
    JS_ToInt32(ctx, &entityIndex,argv[0]);
    const char* filename = JS_ToCString(ctx, argv[1]);


    Entity *entity = Scene::Instance().GetEntity(entityIndex);
    if (entity!=nullptr)
    {
       Animation* anim = entity->LoadAnimation(filename);
       if (!anim)
       {
           JS_ThrowReferenceError(ctx, "entity_add_animation: fail load animation");
       }

    } else 
    {
         JS_ThrowReferenceError(ctx, "entity_add_animation: entity[%d] not found",entityIndex);
    }
     JS_FreeCString(ctx, filename);
    return JS_NULL;
}

//name loop blend
static JSValue js_entity_play_animation(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "entity_play_animation: Wrong number of arguments(entityIndex, name, loop,blend)");
    }
    int entityIndex,mode;
    JS_ToInt32(ctx, &entityIndex,argv[0]);

    const char* name = JS_ToCString(ctx, argv[1]);

    JS_ToInt32(ctx, &mode,argv[2]);
    
    double blend = 0.25;
    JS_ToFloat64(ctx, &blend,argv[3]);
    
    Entity *entity = Scene::Instance().GetEntity(entityIndex);
    if (entity)
    {
        entity->Play(name,mode,blend);
    } else 
    {
         JS_ThrowReferenceError(ctx, "entity_play_animation: entity[%d] not found",entityIndex);
    }
    JS_FreeCString(ctx, name);
    return JS_NULL;
}


static JSValue js_entity_frame(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "get_entity_frame: Wrong number of arguments(entityIndex)");
    }
    int entityIndex;
    double frame;
    JS_ToInt32(ctx, &entityIndex,argv[0]);

    
    
    Entity *entity = Scene::Instance().GetEntity(entityIndex);
    if (entity)
    {
        frame = entity->animator->GetCurrentFrame() ;
        return JS_NewFloat64(ctx,frame);
    } else 
    {
         JS_ThrowReferenceError(ctx, "get_entity_frame: entity[%d] not found",entityIndex);
    }
    
    return JS_NULL;
}   

static JSValue js_entity_stop(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "entity_stop: Wrong number of arguments(entityIndex)");
    }
    int entityIndex;
    
    JS_ToInt32(ctx, &entityIndex,argv[0]);

    
    
    Entity *entity = Scene::Instance().GetEntity(entityIndex);
    if (entity)
    {
        entity->animator->Stop();
        } else 
    {
         JS_ThrowReferenceError(ctx, "get_entity_frame: entity[%d] not found",entityIndex);
    }
    
    return JS_NULL;
}  
static JSValue js_entity_is_ended(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "entity_is_ended: Wrong number of arguments(entityIndex)");
    }
    int entityIndex;
    
    JS_ToInt32(ctx, &entityIndex,argv[0]);

    bool result;

        Entity *entity = Scene::Instance().GetEntity(entityIndex);
    if (entity)
    {
       result = entity->animator->IsEnded();
        return JS_NewBool(ctx,result);
    } else 
    {
         JS_ThrowReferenceError(ctx, "get_entity_frame: entity[%d] not found",entityIndex);
    }
    
    return JS_NULL;
}  
static JSValue js_entity_animation_name(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "get_entity_animation_name: Wrong number of arguments(entityIndex)");
    }

    int entityIndex;
    JS_ToInt32(ctx, &entityIndex,argv[0]);

    
    
    Entity *entity = Scene::Instance().GetEntity(entityIndex);
    if (entity)
    {
        std::string name =entity->animator->GetCurrentAnimationName();
        return JS_NewString(ctx, name.c_str());
    } else 
    {
         JS_ThrowReferenceError(ctx, "get_entity_animation_name: entity[%d] not found",entityIndex);
    }
    
    return JS_NULL;
}

static JSValue js_set_entity_postion (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "set_entity_position: Wrong number of arguments(index, x, y, z)");
    }
    int nodeIndex;
    double x,y,z;
    JS_ToInt32(ctx, &nodeIndex,argv[0]);
    JS_ToFloat64(ctx, &x,argv[1]);
    JS_ToFloat64(ctx, &y,argv[2]);
    JS_ToFloat64(ctx, &z,argv[3]);

   // LogWarning("set_entity_position[%d] x[%f] y[%f] z[%f]",nodeIndex,x,y,z);

    Entity *node = Scene::Instance().GetEntity(nodeIndex);
    if (node)
    {
        node->SetPosition(x,y,z);
    } else 
    {
         JS_ThrowReferenceError(ctx, "set_entity_position: entity[%d] not found",nodeIndex);
    }

    
    return JS_NULL;
}



static JSValue js_set_entity_rotation (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "set_entity_rotation: Wrong number of arguments(Index, x, y, z)");
    }
    int nodeIndex;
    double x,y,z;
    JS_ToInt32(ctx, &nodeIndex,argv[0]);
    JS_ToFloat64(ctx, &x,argv[1]);
    JS_ToFloat64(ctx, &y,argv[2]);
    JS_ToFloat64(ctx, &z,argv[3]);  

   // LogWarning("set_entity_rotation[%d] x[%f] y[%f] z[%f]",nodeIndex,x,y,z);

    Entity *node = Scene::Instance().GetEntity(nodeIndex);
    if (node)
    {
        //node->SetRotation(0,Device::Instance().GetTicks()*0.001f,0);
        node->SetRotation(x,y,z);
    } else 
    {
         JS_ThrowReferenceError(ctx, "set_entity_rotation: entity[%d] not found",nodeIndex);
    }
    
    return JS_NULL;
}

static JSValue js_rotate_entity (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 5)
    {
        return JS_ThrowReferenceError(ctx, "rotate_entity : Wrong number of arguments(Index, angle,x, y, z)");
    }
    int nodeIndex;
    double angle,x,y,z;
    JS_ToInt32(ctx, &nodeIndex,argv[0]);
    
    JS_ToFloat64(ctx, &angle,argv[1]);
    JS_ToFloat64(ctx, &x,argv[2]);
    JS_ToFloat64(ctx, &y,argv[3]);
    JS_ToFloat64(ctx, &z,argv[4]);
    
   // LogWarning("set_entity_rotation[%d] x[%f] y[%f] z[%f]",nodeIndex,x,y,z);

    Entity *node = Scene::Instance().GetEntity(nodeIndex);
    if (node)
    {
        
        node->Rotate(angle,x,y,z);
    } else 
    {
         JS_ThrowReferenceError(ctx, "rotate_entity : entity[%d] not found",nodeIndex);
    }
    
    return JS_NULL;
}
static JSValue js_set_entity_scale (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "set_entity_scale: Wrong number of arguments(Index, x, y, z)");
    }
    int nodeIndex;
    double x,y,z;
    JS_ToInt32(ctx, &nodeIndex,argv[0]);
    JS_ToFloat64(ctx, &x,argv[1]);
    JS_ToFloat64(ctx, &y,argv[2]);
    JS_ToFloat64(ctx, &z,argv[3]);  

    Entity *node = Scene::Instance().GetEntity(nodeIndex);
    if (node)
    {
        node->SetScale(x,y,z);
    } else 
    {
         JS_ThrowReferenceError(ctx, "set_entity_scale: entity[%d] not found",nodeIndex);
    }
    
    return JS_NULL;
}
static JSValue js_set_entity_texture (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 3)
    {
        return JS_ThrowReferenceError(ctx, "set_entity_texture: Wrong number of arguments(modelIndex,materialIndex, textureIndex)");
    }
    int modelIndex, textureIndex,materilaIndex;
    JS_ToInt32(ctx, &modelIndex,argv[0]);
    JS_ToInt32(ctx, &materilaIndex,argv[1]);
    JS_ToInt32(ctx, &textureIndex,argv[2]);

    Entity *model = Scene::Instance().GetEntity(modelIndex);
    if (model)
    {
      
        
           Texture2D *texture =TextureManager::Instance().GetTexture(textureIndex);
           if (!texture)
           {
               return JS_ThrowReferenceError(ctx, "set_entity_texture:texture [%d]  not found",textureIndex);
           }
            model->SetTexture(materilaIndex,texture);

        
    } else 
    {
        return JS_ThrowReferenceError(ctx, "set_entity_texture: model [%d] not found",modelIndex);
    }
  
    
    return JS_NULL;
}
//*************************************************************************************************
//          NODE
//*************************************************************************************************
static JSValue js_scene_get_model_size(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "scene_get_model_size: Wrong number of arguments(modelIndex)");
    }
    int modelIndex;
    JS_ToInt32(ctx, &modelIndex,argv[0]);
    Model *model = Scene::Instance().GetModel(modelIndex);
    if (model)
    {
        model->CalculateBox();
        BoundingBox box =  model->GetBoundingBox();

        JSValue obj = JS_NewObject(ctx);

        double w= box.max.x - box.min.x;
        double h= box.max.y - box.min.y;
        double d= box.max.z - box.min.z;


        JS_SetPropertyStr(ctx, obj, "width", JS_NewFloat64(ctx, w));
        JS_SetPropertyStr(ctx, obj, "height", JS_NewFloat64(ctx, h));
        JS_SetPropertyStr(ctx, obj, "depth", JS_NewFloat64(ctx, d));
        return obj;

    } else 
    {
        return JS_ThrowReferenceError(ctx, "get_model_size: model [%d] not found",modelIndex);
    }
    return JS_NULL;
}
static JSValue js_set_node_postion (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "set_node_position: Wrong number of arguments(nodeIndex, x, y, z)");
    }
    int nodeIndex;
    double x,y,z;
    JS_ToInt32(ctx, &nodeIndex,argv[0]);
    JS_ToFloat64(ctx, &x,argv[1]);
    JS_ToFloat64(ctx, &y,argv[2]);
    JS_ToFloat64(ctx, &z,argv[3]);

    Node *node = Scene::Instance().GetNode(nodeIndex);
    if (node)
    {
        node->SetPosition(x,y,z);
    } else 
    {
         JS_ThrowReferenceError(ctx, "set_node_position: node[%d] not found",nodeIndex);
    }

    
    return JS_NULL;
}


static JSValue js_set_node_rotation (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "set_node_rotation: Wrong number of arguments(nodeIndex, x, y, z)");
    }
    int nodeIndex;
    double x,y,z;
    JS_ToInt32(ctx, &nodeIndex,argv[0]);
    JS_ToFloat64(ctx, &x,argv[1]);
    JS_ToFloat64(ctx, &y,argv[2]);
    JS_ToFloat64(ctx, &z,argv[3]);  

    Node *node = Scene::Instance().GetNode(nodeIndex);
    if (node)
    {
        node->SetRotation(x,y,z);
    } else 
    {
         JS_ThrowReferenceError(ctx, "set_node_rotation: node[%d] not found",nodeIndex);
    }
    
    return JS_NULL;
}


static JSValue js_set_node_scale (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "set_node_scale: Wrong number of arguments(nodeIndex, x, y, z)");
    }
    int nodeIndex;
    double x,y,z;
    JS_ToInt32(ctx, &nodeIndex,argv[0]);
    JS_ToFloat64(ctx, &x,argv[1]);
    JS_ToFloat64(ctx, &y,argv[2]);
    JS_ToFloat64(ctx, &z,argv[3]);  

    Node *node = Scene::Instance().GetNode(nodeIndex);
    if (node)
    {
        node->SetScale(x,y,z);
    } else 
    {
         JS_ThrowReferenceError(ctx, "set_node_scale: node[%d] not found",nodeIndex);
    }
    
    return JS_NULL;
}

static JSValue js_set_node_visible (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 2)
    {
        return JS_ThrowReferenceError(ctx, "set_node_visible: Wrong number of arguments(node, visible)");
    }
    int modelIndex;
    bool visible;
    JS_ToInt32(ctx, &modelIndex,argv[0]);
    visible = JS_ToBool(ctx, argv[1]);
    Node *model = Scene::Instance().GetNode(modelIndex);
    if (model)
    {
        model->visible= visible;
        
    } else 
    {
         JS_ThrowReferenceError(ctx, "set_node_visible: model[%d] not found",modelIndex);
    }
    
    return JS_NULL;

}
//*************************************************************************************************
//          MODEL
//*************************************************************************************************

static JSValue js_set_model_texture (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 3)
    {
        return JS_ThrowReferenceError(ctx, "set_model_texture: Wrong number of arguments(modelIndex,materialIndex, textureIndex)");
    }
    int modelIndex, textureIndex,materilaIndex;
    JS_ToInt32(ctx, &modelIndex,argv[0]);
    JS_ToInt32(ctx, &materilaIndex,argv[1]);
    JS_ToInt32(ctx, &textureIndex,argv[2]);

    Model *model = Scene::Instance().GetModel(modelIndex);
    if (model)
    {
        Material *material = model->GetMaterial(materilaIndex);
        if (material)
        {
           Texture2D *texture =TextureManager::Instance().GetTexture(textureIndex);
           if (!texture)
           {
               return JS_ThrowReferenceError(ctx, "set_model_texture:texture [%d]  not found",textureIndex);
           }
           material->texture = texture;
        } else 
        {
            return JS_ThrowReferenceError(ctx, "set_model_texture: material [%d] not found",materilaIndex);
        }
    } else 
    {
        return JS_ThrowReferenceError(ctx, "set_model_texture: model [%d] not found",modelIndex);
    }
  
    
    return JS_NULL;
}

static JSValue js_scale_model_texture_coords (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "scale_model_texture_coords: Wrong number of arguments(modelIndex,layer, x,y)");
    }
    int modelIndex, textureIndex;
    double x,y;
    JS_ToInt32(ctx, &modelIndex,argv[0]);
    JS_ToInt32(ctx, &textureIndex,argv[1]);

    JS_ToFloat64(ctx, &x,argv[2]);
    JS_ToFloat64(ctx, &y,argv[3]);
    

    Model *model = Scene::Instance().GetModel(modelIndex);
    if (model)
    {
        Mat4 scale = Mat4::Scale(x,y,1);
        model->TransformTextureByLayer(textureIndex,scale);       
    } else 
    {
        return JS_ThrowReferenceError(ctx, "scale_model_texture_coords: model [%d] not found",modelIndex);
    }
  
    
    return JS_NULL;
}

static JSValue js_set_model_name (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 2)
    {
        return JS_ThrowReferenceError(ctx, "set_model_name: Wrong number of arguments(modelIndex, name)");
    }
    int modelIndex;
    JS_ToInt32(ctx, &modelIndex,argv[0]);
    const char* name = JS_ToCString(ctx, argv[1]);
    Model *model = Scene::Instance().GetModel(modelIndex);
    if (model)
    {
        model->SetName(name);
    } else 
    {
         JS_ThrowReferenceError(ctx, "set_model_name: model [%d] not found",modelIndex);
    }
  
    
    JS_FreeCString(ctx, name);
    
    return JS_NULL;
}

static JSValue js_set_model_culling (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 3)
    {
        return JS_ThrowReferenceError(ctx, "set_model_culling: Wrong number of arguments(modelIndex,layer, culling)");
    }
    int modelIndex,layer;
    bool culling;
    
    JS_ToInt32(ctx, &modelIndex,argv[0]);
    JS_ToInt32(ctx, &layer,argv[1]);
    culling = JS_ToBool(ctx, argv[2]);

    Model *model = Scene::Instance().GetModel(modelIndex);
    if (model)
    {
        model->SetCullFace(layer,culling);
    } else 
    {
         JS_ThrowReferenceError(ctx, "set_model_culling: model [%d] not found",modelIndex);
    }
  
    
    return JS_NULL;
}

static JSValue js_set_model_scale (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "set_model_scale: Wrong number of arguments(modelIndex, x, y, z)");
    }
    int modelIndex;
    double x,y,z;
    JS_ToInt32(ctx, &modelIndex,argv[0]);
    JS_ToFloat64(ctx, &x,argv[1]);
    JS_ToFloat64(ctx, &y,argv[2]);
    JS_ToFloat64(ctx, &z,argv[3]);

    Model *model = Scene::Instance().GetModel(modelIndex);
    if (model)
    {
        Mat4 scale = Mat4::Scale(x,y,z);
        model->Transform(scale);
    } else 
    {
         JS_ThrowReferenceError(ctx, "set_model_scale: model [%d] not found",modelIndex);
    }
  
    
    return JS_NULL;
}

static JSValue js_set_model_position (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "set_model_position: Wrong number of arguments(modelIndex, x, y, z)");
    }
    int modelIndex;
    double x,y,z;
    JS_ToInt32(ctx, &modelIndex,argv[0]);
    JS_ToFloat64(ctx, &x,argv[1]);
    JS_ToFloat64(ctx, &y,argv[2]);
    JS_ToFloat64(ctx, &z,argv[3]);

    Model *model = Scene::Instance().GetModel(modelIndex);
    if (model)
    {
        Mat4 position = Mat4::Translate(x,y,z);
        model->Transform(position);
    } else 
    {
         JS_ThrowReferenceError(ctx, "set_model_position: model [%d] not found",modelIndex);
    }
  
    
    return JS_NULL;
}

static JSValue js_set_model_rotate (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "set_model_rotate: Wrong number of arguments(modelIndex, x, y, z)");
    }
    int modelIndex;
    double x,y,z;
    JS_ToInt32(ctx, &modelIndex,argv[0]);
    JS_ToFloat64(ctx, &x,argv[1]);
    JS_ToFloat64(ctx, &y,argv[2]);
    JS_ToFloat64(ctx, &z,argv[3]);

    Model *model = Scene::Instance().GetModel(modelIndex);
    if (model)
    {
        Mat4 rotate = Mat4::Rotate(x,y,z);
        model->Transform(rotate);
    } else 
    {
         JS_ThrowReferenceError(ctx, "set_model_rotate: model [%d] not found",modelIndex);
    }
  
    
    return JS_NULL;
}   

static JSValue js_model_update_normals (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "model_update_normals: Wrong number of arguments(modelIndex,mesh)");
    }
    int modelIndex;
    int mesh;
    JS_ToInt32(ctx, &modelIndex, argv[0]);
    JS_ToInt32(ctx, &mesh, argv[1]);

    Model *model = Scene::Instance().GetModel(modelIndex);
    if (model)
    {
        Mesh* m = model->GetMesh(mesh);
        if (m) 
        {
            m->CalculateNormals();

        } else 
        {
            JS_ThrowReferenceError(ctx, "model_update_normals: mesh [%d] not found",mesh);
        }

    } else 
    {
         JS_ThrowReferenceError(ctx, "model_update_normals: model [%d] not found",modelIndex);
    }
  
    
    return JS_NULL;
}


static JSValue js_model_update_smoth_normals (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "model_update_smoth_normals: Wrong number of arguments(modelIndex,mesh,angleWeighted)");
    }
    int modelIndex;
    int mesh;
    bool  angleWeighted;
    JS_ToInt32(ctx, &modelIndex, argv[0]);
    JS_ToInt32(ctx, &mesh, argv[1]);

    angleWeighted = JS_ToBool(ctx, argv[2]);

    



    Model *model = Scene::Instance().GetModel(modelIndex);
    if (model)
    {
        Mesh* m = model->GetMesh(mesh);
        if (m) 
        {
            m->CalculateSmothNormals(angleWeighted);

        } else 
        {
            JS_ThrowReferenceError(ctx, "model_update_smoth_normals: mesh [%d] not found",mesh);
        }

    } else 
    {
         JS_ThrowReferenceError(ctx, "model_update_smoth_normals: model [%d] not found",modelIndex);
    }
  
    
    return JS_NULL;
}


//*************************************************************************************************
//          LIGHT
//*************************************************************************************************

static JSValue js_set_light_color (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "set_light_color: Wrong number of arguments(lightIndex, r, g, b)");
    }
    int lightIndex;
    double r,g,b;
    JS_ToInt32(ctx, &lightIndex,argv[0]);
    JS_ToFloat64(ctx, &r,argv[1]);
    JS_ToFloat64(ctx, &g,argv[2]);
    JS_ToFloat64(ctx, &b,argv[3]);
     Scene::Instance().SetLightColor(lightIndex,r,g,b);
   
    return JS_NULL;
}

static JSValue js_set_light_position (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "set_light_position: Wrong number of arguments(lightIndex, x, y, z)");
    }
    int lightIndex;
    double x,y,z;
    JS_ToInt32(ctx, &lightIndex,argv[0]);
    JS_ToFloat64(ctx, &x,argv[1]);
    JS_ToFloat64(ctx, &y,argv[2]);
    JS_ToFloat64(ctx, &z,argv[3]);
     Scene::Instance().SetLightPosition(lightIndex,x,y,z);
   
    return JS_NULL;
}

static JSValue js_set_light_intensity (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 2)
    {
        return JS_ThrowReferenceError(ctx, "set_light_intensity: Wrong number of arguments(lightIndex, intensity)");
    }
    int lightIndex;
    double intensity;
    JS_ToInt32(ctx, &lightIndex,argv[0]);
    JS_ToFloat64(ctx, &intensity,argv[1]);
     Scene::Instance().SetLightIntensity(lightIndex,intensity);
   
    return JS_NULL;
}

static JSValue js_set_enbale_3d(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "enbale_3d: Wrong number of arguments(enable)");
    }
    bool do3D = JS_ToBool(ctx, argv[0]);

    Scene::Instance().Enable3D(do3D);

    return JS_NULL;
}

static JSValue js_set_clear_color(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 3)
    {
        return JS_ThrowReferenceError(ctx, "set_clear_color: Wrong number of arguments(r, g, b)");
    }
    
    int r,g,b;
    JS_ToInt32(ctx, &r,argv[0]);
    JS_ToInt32(ctx, &g,argv[1]);
    JS_ToInt32(ctx, &b,argv[2]);
    Scene::Instance().SetClearColor(r,g,b);

    

    return JS_NULL;
}
static JSValue js_set_enbale_2d(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "enbale_2d: Wrong number of arguments(enable)");
    }
    bool do2D = JS_ToBool(ctx, argv[0]);

    Scene::Instance().Enable2D(do2D);

    return JS_NULL;

}

static JSValue js_get_total_nodes(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    
    int total = Scene::Instance().GetNodeCount();
    JSValue result = JS_NewInt32(ctx, total);
    return result;
}
static JSValue js_get_total_models(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    
    int total = Scene::Instance().GetModelCount();
    JSValue result = JS_NewInt32(ctx, total);
    return result;
}

static JSValue js_get_total_entityes(JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    
    int total = Scene::Instance().GetEntityCount();
    JSValue result = JS_NewInt32(ctx, total);
    return result;

}




using JSFunctionMap = std::map<std::string, JSValue(*)(JSContext*, JSValueConst, int, JSValueConst*)>;


JSFunctionMap sceneFunctions = 
{
    {"enable_3d", js_set_enbale_3d},
    {"enable_2d", js_set_enbale_2d},
    {"set_clear_color", js_set_clear_color},
    {"load_entity", js_scene_load_entity},
    {"load_model", js_scene_load_model},
    {"get_model_size", js_scene_get_model_size},
    
    {"create_static_node", js_scene_load_static_node},

    {"get_total_nodes", js_get_total_nodes},
    {"get_total_models", js_get_total_models},
    {"get_total_entityes", js_get_total_entityes},

    {"remove_model", js_scene_remove_model},
    {"remove_entity", js_scene_remove_entity},
    {"remove_node", js_scene_remove_node},

    {"create_cube", js_scene_create_cube},
    {"create_plane", js_scene_create_pane},
    {"create_quad", js_scene_create_quad},

    {"node_add_model", js_node_add_model},

    {"entity_add_animation", js_entity_add_animation},
    {"entity_play", js_entity_play_animation},
    {"entity_stop", js_entity_stop},
    {"entity_is_ended", js_entity_is_ended},
    {"get_entity_frame", js_entity_frame},
    {"get_entity_animation_name", js_entity_animation_name},


    {"set_model_texture", js_set_model_texture},
    {"set_model_culling", js_set_model_culling},
    {"scale_model_texture_coords", js_scale_model_texture_coords},




    {"set_entity_position", js_set_entity_postion},
    {"set_entity_rotation", js_set_entity_rotation},
    {"rotate_entity", js_rotate_entity},
    {"set_entity_scale", js_set_entity_scale},
    {"set_entity_texture", js_set_entity_texture},

    {"set_node_position", js_set_node_postion},
    {"set_node_rotation", js_set_node_rotation},
    {"set_node_scale", js_set_node_scale},
    {"set_node_visible", js_set_node_visible},

    {"set_model_postion", js_set_model_position},
    {"set_model_rotation", js_set_model_rotate},
    {"set_model_scale", js_set_model_scale},
    {"set_model_name", js_set_model_name},

    {"get_entity_id", js_scene_get_entity_id},
    {"get_model_id", js_scene_get_model_id},
    {"get_node_id", js_scene_get_node_id},

    


    {"model_update_normals", js_model_update_normals},
    {"model_update_smoth_normals", js_model_update_smoth_normals},

    {"set_light_color", js_set_light_color},
    {"set_light_position", js_set_light_position},
    {"set_light_intensity", js_set_light_intensity},


  
};



static JSValue js_canvas_set_color (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 3)
    {
        return JS_ThrowReferenceError(ctx, "set_color: Wrong number of arguments(r, g, b)");
    }
    double r,g,b;
    JS_ToFloat64(ctx, &r,argv[0]);
    JS_ToFloat64(ctx, &g,argv[1]);
    JS_ToFloat64(ctx, &b,argv[2]);
    Scene::Instance().GetRenderBatch().SetColor(r,g,b);
    return JS_NULL;
}

static JSValue js_canvas_set_alpha (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)    
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "set_alpha: Wrong number of arguments(alpha)");
    }
    double alpha;
    JS_ToFloat64(ctx, &alpha,argv[0]);
    Scene::Instance().GetRenderBatch().SetAlpha(alpha);
    return JS_NULL;
}

static JSValue js_canvas_line2d (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "line2d: Wrong number of arguments(x1, y1, x2, y2)");
    }
    double x1,y1,x2,y2;
    JS_ToFloat64(ctx, &x1,argv[0]);
    JS_ToFloat64(ctx, &y1,argv[1]);
    JS_ToFloat64(ctx, &x2,argv[2]);
    JS_ToFloat64(ctx, &y2,argv[3]);
    Scene::Instance().GetRenderBatch().Line2D(x1,y1,x2,y2);
    return JS_NULL;
}

static JSValue js_canvas_circle (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    
    double x,y,r;
    bool fill=false;
    JS_ToFloat64(ctx, &x,argv[0]);
    JS_ToFloat64(ctx, &y,argv[1]);
    JS_ToFloat64(ctx, &r,argv[2]);
  
    if (argc !=3 && argc !=4)
    {
        return JS_ThrowReferenceError(ctx, "circle: Wrong number of arguments(x, y, r [fill])");
    }
 
    if (argc == 4)
    {
        fill=JS_ToBool(ctx, argv[3]);
        Scene::Instance().GetRenderBatch().Circle(x,y,r,fill);
    }
    Scene::Instance().GetRenderBatch().Circle(x,y,r,fill);

    return JS_NULL;
}

static JSValue js_canvas_rect (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    
    double x,y,w,h;
    bool fill=false;
    JS_ToFloat64(ctx, &x,argv[0]);
    JS_ToFloat64(ctx, &y,argv[1]);
    JS_ToFloat64(ctx, &w,argv[2]);
    JS_ToFloat64(ctx, &h,argv[3]);

    if (argc !=5 && argc !=5)
    {
        return JS_ThrowReferenceError(ctx, "rect: Wrong number of arguments(x, y, w, h [fill])");
    }

    if (argc == 5)
    {
        fill=JS_ToBool(ctx, argv[4]);
        Scene::Instance().GetRenderBatch().Rectangle(x,y,w,h,fill);
    }
    
    
    return JS_NULL;
}

static JSValue js_canvas_line3d (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 6)
    {
        return JS_ThrowReferenceError(ctx, "line3d: Wrong number of arguments(x1, y1, z1, x2, y2, z2)");
    }
    double x1,y1,z1,x2,y2,z2;
    JS_ToFloat64(ctx, &x1,argv[0]);
    JS_ToFloat64(ctx, &y1,argv[1]);
    JS_ToFloat64(ctx, &z1,argv[2]);
    JS_ToFloat64(ctx, &x2,argv[3]);
    JS_ToFloat64(ctx, &y2,argv[4]);
    JS_ToFloat64(ctx, &z2,argv[5]);
    Scene::Instance().GetRenderBatch().Line3D(x1,y1,z1,x2,y2,z2);
    return JS_NULL;
}
static JSValue js_canvas_grid (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    
    int slices;
    double spacing;
    bool axies = false;
    JS_ToInt32(ctx, &slices,argv[0]);
    JS_ToFloat64(ctx, &spacing,argv[1]);
    
    if (argc == 2)
    {
        Scene::Instance().GetRenderBatch().Grid(slices,spacing,axies);
    } else 
    if (argc == 3)
    {
        axies = JS_ToBool(ctx, argv[2]);
        Scene::Instance().GetRenderBatch().Grid(slices,spacing,axies);
    } else 
    {
        return JS_ThrowReferenceError(ctx, "grid: Wrong number of arguments(slices, spacing,[axies=false])");   
    }
    
    return JS_NULL;
}

static JSValue js_canvas_cube (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 6 && argc != 7)
    {
        return JS_ThrowReferenceError(ctx, "cube: Wrong number of arguments(x, y, z, width, height, depth, whire)");
    }
    double x,y,z,width,height,depth;
    
        bool wire = true;
    JS_ToFloat64(ctx, &x, argv[0]);
    JS_ToFloat64(ctx, &y,argv[1]);
    JS_ToFloat64(ctx, &z,argv[2]);
    JS_ToFloat64(ctx, &width,argv[3]);
    JS_ToFloat64(ctx, &height,argv[4]);
    JS_ToFloat64(ctx, &depth,argv[5]);

    if (argc == 7)
    {
        wire = JS_ToBool(ctx, argv[6]);
    }

    Vec3 pos(x,y,z);
    Scene::Instance().GetRenderBatch().Cube(pos,width,height,depth,wire);
    return JS_NULL;
}

static JSValue js_canvas_sphere (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 7)
    {
        return JS_ThrowReferenceError(ctx, "sphere: Wrong number of arguments(x, y, z, radius,rings,slices,whire)");
    }
    double x,y,z,radius;
    int rings,slices;
    
    JS_ToFloat64(ctx, &x,argv[0]);
    JS_ToFloat64(ctx, &y,argv[1]);
    JS_ToFloat64(ctx, &z,argv[2]);
    JS_ToFloat64(ctx, &radius,argv[3]);

    JS_ToInt32(ctx, &rings,argv[4]);
    JS_ToInt32(ctx, &slices,argv[5]);

    bool whire = JS_ToBool(ctx, argv[6]);

    Vec3 pos(x,y,z);
    Scene::Instance().GetRenderBatch().Sphere(pos,radius,rings,slices,whire);
    return JS_NULL;
}

static JSValue js_canvas_set_mode (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "set_mode: Wrong number of arguments(mode)");
    }
    int mode;
    JS_ToInt32(ctx, &mode,argv[0]);
    Scene::Instance().GetRenderBatch().SetMode(mode);
    return JS_NULL;
}

static JSValue js_canvas_vertex2f (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 2)
    {
        return JS_ThrowReferenceError(ctx, "vertex2f: Wrong number of arguments(x, y)");
    }
    double x,y;
    JS_ToFloat64(ctx, &x,argv[0]);
    JS_ToFloat64(ctx, &y,argv[1]);
    Scene::Instance().GetRenderBatch().Vertex2f(x,y);
    return JS_NULL;
}

static JSValue js_canvas_vertex3f (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 3)
    {
        return JS_ThrowReferenceError(ctx, "vertex3f: Wrong number of arguments(x, y, z)");
    }
    double x,y,z;
    JS_ToFloat64(ctx, &x,argv[0]);
    JS_ToFloat64(ctx, &y,argv[1]);
    JS_ToFloat64(ctx, &z,argv[2]);
    Scene::Instance().GetRenderBatch().Vertex3f(x,y,z);
    return JS_NULL;
}

static JSValue js_canvas_texcoord2f (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 2)
    {
        return JS_ThrowReferenceError(ctx, "texcoord2f: Wrong number of arguments(x, y)");
    }
    double x,y;
    JS_ToFloat64(ctx, &x,argv[0]);
    JS_ToFloat64(ctx, &y,argv[1]);
    Scene::Instance().GetRenderBatch().TexCoord2f(x,y);
    return JS_NULL;
}





static JSValue js_canvas_set_texture (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "set_texture: Wrong number of arguments(textureIndex)");
    }
    int textureIndex;
    JS_ToInt32(ctx, &textureIndex,argv[0]);

    Texture2D *texture =TextureManager::Instance().GetTexture(textureIndex);
    if (!texture)
    {
        return JS_ThrowReferenceError(ctx, "set_texture:texture [%d]  not found",textureIndex);
    }
    Scene::Instance().GetRenderBatch().SetTexture(texture);


    return JS_NULL;
}

static JSValue js_canvas_draw_texture (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 5)
    {
        return JS_ThrowReferenceError(ctx, "draw_texture: Wrong number of arguments(textureIndex,x,y,w,h)");
    }
    int textureIndex;
    JS_ToInt32(ctx, &textureIndex,argv[0]);
    double x,y,w,h;
    JS_ToFloat64(ctx, &x,argv[1]);
    JS_ToFloat64(ctx, &y,argv[2]);
    JS_ToFloat64(ctx, &w,argv[3]);
    JS_ToFloat64(ctx, &h,argv[4]);

    Texture2D *texture =TextureManager::Instance().GetTexture(textureIndex);
    if (!texture)
    {
        return JS_ThrowReferenceError(ctx, "set_texture:texture [%d]  not found",textureIndex);
    }
    Scene::Instance().GetRenderBatch().Quad(texture,x,y,w,h);


    return JS_NULL;
}
static JSValue js_canvas_print (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 3)
    {
        return JS_ThrowReferenceError(ctx, "print: Wrong number of arguments(text,x,y)");
    }
    const char* text= JS_ToCString(ctx, argv[2]);

   // LogWarning("text[%s] x[%f] y[%f]",text,argv[0],argv[1]);
    double x,y;
    JS_ToFloat64(ctx, &x,argv[0]);
    JS_ToFloat64(ctx, &y,argv[1]);

    Scene::Instance().GetFont()->Print(text,x,y);

    
    
    JS_FreeCString(ctx, text);

    return JS_NULL;
}

static JSValue js_canvas_set_font (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "set_font: Wrong number of arguments(fontIndex)");
    }
    int fontIndex;
    JS_ToInt32(ctx, &fontIndex,argv[0]);
    Scene::Instance().SetFont(fontIndex);
   

    return JS_NULL;
}

static JSValue js_canvas_set_font_size (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "set_font_size: Wrong number of arguments(value)");
    }
    double value;
    JS_ToFloat64(ctx, &value,argv[0]);
    Scene::Instance().GetFont()->SetFontSize(value);
    return JS_NULL;
}

static JSValue js_canvas_font_clip (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "clip_font: Wrong number of arguments(value)");
    }
    bool clip= JS_ToBool(ctx, argv[0]);

    Scene::Instance().GetFont()->EnableClip(clip);
    return JS_NULL;
}

static JSValue js_canvas_set_font_clip (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 4)
    {
        return JS_ThrowReferenceError(ctx, "set_font_clip: Wrong number of arguments(x,y,width,height)");
    }
    int x,y,width,height;
    JS_ToInt32(ctx, &x,argv[0]);
    JS_ToInt32(ctx, &y,argv[1]);
    JS_ToInt32(ctx, &width,argv[2]);
    JS_ToInt32(ctx, &height,argv[3]);
    
    Scene::Instance().GetFont()->SetClip(x,y,width,height);
    return JS_NULL;
}

static JSValue js_canvas_load_font (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "load_font: Wrong number of arguments(fileName)");
    }
    const char* text= JS_ToCString(ctx, argv[0]);

    JSValue result;
    if (Scene::Instance().LoadFont(text))
    {

        result = JS_NewInt32(ctx,Scene::Instance().GetFontsCount()-1);
    } else 
    {
        JS_ThrowReferenceError(ctx, "load_font: font [%s] not found",text);
        result = JS_NULL;
    }

    JS_FreeCString(ctx, text);


    return result;

}


JSFunctionMap canvasFunctions =
{

    {"set_color", js_canvas_set_color},
    {"set_alpha", js_canvas_set_alpha},
    
    {"line", js_canvas_line2d},
    {"circle", js_canvas_circle},
    {"rect", js_canvas_rect},

    {"line3d", js_canvas_line3d},
    {"cube", js_canvas_cube},
    {"sphere", js_canvas_sphere},
    {"grid", js_canvas_grid},

    {"set_mode", js_canvas_set_mode},
    {"vertex2", js_canvas_vertex2f},
    {"vertex3", js_canvas_vertex3f},
    {"texcoord", js_canvas_texcoord2f},

    {"set_texture", js_canvas_set_texture},
    {"draw_texture", js_canvas_draw_texture},

    {"print", js_canvas_print},
    {"set_font", js_canvas_set_font},
    {"load_font", js_canvas_load_font},
    {"set_font_size", js_canvas_set_font_size},
    {"font_clip", js_canvas_font_clip},
    {"set_font_clip", js_canvas_set_font_clip},


};


static JSValue js_move_camera (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 2)
    {
        return JS_ThrowReferenceError(ctx, "move_camera: Wrong number of arguments(mode,dt)");
    }
    int mode;
    JS_ToInt32(ctx, &mode,argv[0]);
    double dt;
    JS_ToFloat64(ctx, &dt,argv[1]);
    Scene::Instance().MoveCamera(mode,dt);
    return JS_NULL;
}

static JSValue js_rotate_camera_by_mouse (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "rotate_camera_by_mouse: Wrong number of arguments(dt)");
    }
    double dt;
    JS_ToFloat64(ctx, &dt,argv[0]);
    Scene::Instance().RotateCameraByMouse(dt);
    return JS_NULL;
}

static JSValue js_set_camera_position (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 3)
    {
        return JS_ThrowReferenceError(ctx, "set_camera_position: Wrong number of arguments(x,y,z)");
    }
    double x,y,z;
    JS_ToFloat64(ctx, &x,argv[0]);
    JS_ToFloat64(ctx, &y,argv[1]);
    JS_ToFloat64(ctx, &z,argv[2]);
    Vec3 pos(x,y,z);
    Scene::Instance().SetCameraPosition(pos);
    return JS_NULL;
}

static JSValue js_set_camera_yaw (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "set_camera_yaw: Wrong number of arguments(yaw)");
    }
    double yaw;
    JS_ToFloat64(ctx, &yaw,argv[0]);
    Scene::Instance().SetCameraYaw(yaw);
    return JS_NULL;
}

static JSValue js_set_camera_pitch (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "set_camera_pitch: Wrong number of arguments(pitch)");
    }
    double pitch;
    JS_ToFloat64(ctx, &pitch,argv[0]);
    Scene::Instance().SetCameraPitch(pitch);
    return JS_NULL;
}

static JSValue js_get_camera_position (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 0)
    {
        return JS_ThrowReferenceError(ctx, "get_camera_position: Wrong number of arguments()");
    }
    Vec3 p = Scene::Instance().GetCameraPosition();
    JSValue x = JS_NewFloat64(ctx, p.x);
    JSValue y = JS_NewFloat64(ctx, p.y);
    JSValue z = JS_NewFloat64(ctx, p.z);
    JSValue array = JS_NewArray(ctx);
    JS_SetPropertyUint32(ctx, array, 0, x);
    JS_SetPropertyUint32(ctx, array, 1, y);
    JS_SetPropertyUint32(ctx, array, 2, z);
    return array;
}

static JSValue js_set_follow_mode (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 1)
    {
        return JS_ThrowReferenceError(ctx, "set_follow_mode: Wrong number of arguments(mode)");
    }
    int mode;
    JS_ToInt32(ctx, &mode,argv[0]);
    Scene::Instance().SetFollowCameraMode(mode);
    return JS_NULL;
}

static JSValue js_set_follow_offset (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 3)
    {
        return JS_ThrowReferenceError(ctx, "set_follow_offset: Wrong number of arguments(x,y,z)");
    }
    double x, y, z;
    JS_ToFloat64(ctx, &x, argv[0]);
    JS_ToFloat64(ctx, &y, argv[1]);
    JS_ToFloat64(ctx, &z, argv[2]);
    Vec3 offset = Vec3(x, y, z);

    Scene::Instance().SetFollowOffset(offset);
    return JS_NULL;
}

static JSValue js_set_follow_camera_position (JSContext *ctx, JSValueConst , int argc, JSValueConst *argv) 
{
    if (argc != 3)
    {
        return JS_ThrowReferenceError(ctx, "set_follow_camera_position: Wrong number of arguments(x,y,z)");
    }
       double x, y, z;
    JS_ToFloat64(ctx, &x, argv[0]);
    JS_ToFloat64(ctx, &y, argv[1]);
    JS_ToFloat64(ctx, &z, argv[2]);
    Vec3 position = Vec3(x, y, z);
    Scene::Instance().SetFollowCameraPosition(position);
    return JS_NULL;
}



JSFunctionMap cameraFunctions =
{

    {"set_follow_mode", js_set_follow_mode},
    {"set_follow_offset", js_set_follow_offset},
    {"set_follow_position", js_set_follow_camera_position},
    {"get_position", js_get_camera_position},
    {"set_position", js_set_camera_position},
    {"set_yaw", js_set_camera_yaw},
    {"set_pitch", js_set_camera_pitch},
    {"rotate_by_mouse", js_rotate_camera_by_mouse},
    {"move", js_move_camera},



};

void Scene::RegisterFunctions(JSContext *ctx, JSValue global_obj)
{
     JSValue core = JS_NewObject(ctx);
     for (const auto& func : sceneFunctions) 
     {
         JS_SetPropertyStr(ctx, core, func.first.c_str(), JS_NewCFunction(ctx, func.second, func.first.c_str(), 1));
     }
    JS_SetPropertyStr(ctx, global_obj, "scene", core);

     core = JS_NewObject(ctx);
     for (const auto& func : canvasFunctions) 
     {
         JS_SetPropertyStr(ctx, core, func.first.c_str(), JS_NewCFunction(ctx, func.second, func.first.c_str(), 1));
     }
     JS_SetPropertyStr(ctx, global_obj, "canvas", core);


     core = JS_NewObject(ctx);
     for (const auto& func : cameraFunctions) 
     {
         JS_SetPropertyStr(ctx, core, func.first.c_str(), JS_NewCFunction(ctx, func.second, func.first.c_str(), 1));
     }
     JS_SetPropertyStr(ctx, global_obj, "camera", core);
}

