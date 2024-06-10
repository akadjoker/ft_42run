
#include "pch.h"
#include "Scene.hpp"

const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;



Scene::Scene()
{
}


Scene::~Scene()
{
}

Entity *Scene::LoadEntity(const std::string &name)
{

    Entity *e = new Entity();
    e->Load(name);
    m_entities.push_back(e);
    return e;
   
}

Model *Scene::LoadModel(const std::string &name)
{

      VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::TEXCOORD0, 2),
        VertexFormat::Element(VertexFormat::NORMAL, 3),
     //   VertexFormat::Element(VertexFormat::TANGENT, 3),
      //  VertexFormat::Element(VertexFormat::BITANGENT, 3),
    };     


    Model *m = new Model();
    if (m->Load(name,VertexFormat(elements, 3)))
    {
        m_models.push_back(m);
        return m;
    }
    
    return nullptr;
}

StaticNode *Scene::CreateStaticNode(const std::string &name)
{

    StaticNode *node = new StaticNode();
    node->SetName(name);
    m_nodes.push_back(node);
    return node;
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


VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::TEXCOORD0, 2),
        VertexFormat::Element(VertexFormat::NORMAL, 3),
     //   VertexFormat::Element(VertexFormat::TANGENT, 3),
      //  VertexFormat::Element(VertexFormat::BITANGENT, 3),
    };     

    Material * material= model->AddMaterial();
    material->texture = TextureManager::Instance().GetDefault();
    material->name = "default";


    Mesh *mesh = model->AddMesh(VertexFormat(elements, 3),0,false);

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
    mesh->CalculateTangents();
    mesh->Upload();
    m_models.push_back(model);
    return model;    
}

Model *Scene::CreatePlane(int stacks, int slices, int tilesX, int tilesY, float uvTileX , float uvTileY )
{
   
    Model *model = new Model();

   VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::TEXCOORD0, 2),
        VertexFormat::Element(VertexFormat::NORMAL, 3),
     //   VertexFormat::Element(VertexFormat::TANGENT, 3),
      //  VertexFormat::Element(VertexFormat::BITANGENT, 3),
    };     

    Material * material= model->AddMaterial();
    material->texture = TextureManager::Instance().GetDefault();
    material->name = "default";


    Mesh *mesh = model->AddMesh(VertexFormat(elements, 3),0,false);

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
    mesh->CalculateTangents();
    mesh->Upload();
    m_models.push_back(model);

    return model;

}

void Scene::Init()
{
    LoadModelShader();
    LoadSkinShader();
    LoadDepthShader();

    depth.Init(SHADOW_WIDTH, SHADOW_HEIGHT);

    StaticNode *lights = CreateStaticNode("lights");

    

    for (int i = 0; i < 10; i++)
    {
        Model *cube =  CreateCube(0.5f,0.5f,0.5f);
        const Vec3  &lightPos = m_lights[i];
        Mat4 position= Mat4::Translate(lightPos.x, lightPos.y, lightPos.z);
        cube->Transform(position);
        lights->AddModel(cube);        
    }

 


   
}


void Scene::Release()
{
    m_depthShader.Release();
    m_modelShader.Release();
    m_skinShader.Release();
    depth.Release();
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

}
static int RandomRange(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

void Scene::Render()

{
    lightPos.z = static_cast<float>(sin(Device::Instance().GetTime() * 0.5) * 10.0);
    lightPos.x = static_cast<float>(cos(Device::Instance().GetTime() * 0.5) * 10.0);
    lightPos.y = 5.0f;//static_cast<float>(sin(Device::Instance().GetTime() * 0.5) * 3.0);
    
    

    float near_plane = 1.0f;
    float far_plane = 25.0f;
    Mat4 shadowProj = Mat4::Perspective(90.0f, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
    std::vector<Mat4> shadowTransforms;

    shadowTransforms.push_back(shadowProj * Mat4::LookAt(lightPos, lightPos + Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * Mat4::LookAt(lightPos, lightPos + Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * Mat4::LookAt(lightPos, lightPos + Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * Mat4::LookAt(lightPos, lightPos + Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * Mat4::LookAt(lightPos, lightPos + Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * Mat4::LookAt(lightPos, lightPos + Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f)));


    depth.Begin();
    m_depthShader.Use();
    for (u32 i = 0; i < 6; i++)
    {
        m_depthShader.SetMatrix4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i].x);
    }
    m_depthShader.SetFloat("far_plane", far_plane);
    m_depthShader.SetFloat("lightPos", lightPos.x, lightPos.y, lightPos.z);


    
    for (u32 i = 0; i < m_nodes.size(); i++)
    {
        Mat4 m = m_nodes[i]->GetRelativeTransformation();
        m_depthShader.SetMatrix4("model", m.x);
        m_nodes[i]->RenderNoMaterial();
    }


    depth.End();
    depth.BindTexture(1);








    m_modelShader.Use();
    m_modelShader.SetMatrix4("view", viewMatrix.x);
    m_modelShader.SetMatrix4("projection", projectionMatrix.x);
    m_modelShader.SetFloat("viewPos", cameraPosition.x, cameraPosition.y, cameraPosition.z);
    m_modelShader.SetFloat("lightPos", lightPos.x, lightPos.y, lightPos.z);
    m_modelShader.SetFloat("far_plane", far_plane);

   
    //  int state = RandomRange(0, 100);
    //  float lightForce =0.0f;
    //  if (state>95)
    //  {
    //     lightForce = RandomRange(0, 1);
       
    //  }
    //   m_modelShader.SetFloat("lightColor[6]", lightForce,lightForce,lightForce);
      

    
    
    for (u32 i = 0; i < m_nodes.size(); i++)
    {
        Mat4 m = m_nodes[i]->GetRelativeTransformation();
        m_modelShader.SetMatrix4("model", m.x);
        m_nodes[i]->Render();
    }

    m_skinShader.Use();
    m_skinShader.SetMatrix4("view", viewMatrix.x);
    m_skinShader.SetMatrix4("projection", projectionMatrix.x);
   // m_skinShader.SetFloat("lightColor[6]", lightForce,lightForce,lightForce);
    m_skinShader.SetFloat("viewPos", cameraPosition.x, cameraPosition.y, cameraPosition.z);
   // m_skinShader.SetFloat("far_plane", far_plane);


    for (u32 i = 0; i < m_entities.size(); i++)
    {
        Entity *entity = m_entities[i];
        Mat4 mat = entity->GetRelativeTransformation();
        m_skinShader.SetMatrix4("model", mat.x);
        for (u32 b = 0; b < entity->bones.size(); b++)
        {
             m_skinShader.SetMatrix4(System::Instance().TextFormat("Joints[%d]", b), entity->bones[b].x); // model.bones
        }
        entity->Render();
    }
}

void Scene::Update(float elapsed)
{

   

    for (u32 i = 0; i < m_entities.size(); i++)
    {
        Entity *entity = m_entities[i];
        entity->UpdateAnimation(elapsed);
        entity->Update();
    }
}

bool Scene::LoadModelShader()
{
     const char *vShader = GLSL(
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec2 aTexCoords;
        layout(location = 2) in vec3 aNormal;


        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoords;
 
  


        uniform vec3 viewPos;

        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;
   

        void main() 
        {
            FragPos = vec3(model * vec4(aPos, 1.0));   

            TexCoords = aTexCoords;
            
            mat3 normalMatrix = transpose(inverse(mat3(model)));

            Normal = normalMatrix * aNormal;        
            
                   
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    );

    const char *fShader = GLSL(
        out vec4 FragColor;

     

        in  vec3 FragPos;
        in  vec2 TexCoords;
        in  vec3 Normal; 
      

        uniform sampler2D diffuseMap;
        uniform samplerCube shadowMap;

        uniform vec3 lightPos;
        uniform vec3 viewPos;
        uniform float far_plane;

        const bool pcf=true;




        // array of offset direction for sampling
        vec3 gridSamplingDisk[20] = vec3[]
        (
        vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
        vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
        vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
        vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
        vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
        );

float ShadowCalculation(vec3 fragPos,bool pcf)
{

    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    // test for shadows
    // float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    // float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;
    // PCF
    if (pcf)
    {
        float shadow = 0.0;
        float bias = 0.05; 
        float samples = 4.0;
        float offset = 0.1;
        for(float x = -offset; x < offset; x += offset / (samples * 0.5))
        {
            for(float y = -offset; y < offset; y += offset / (samples * 0.5))
            {
                for(float z = -offset; z < offset; z += offset / (samples * 0.5))
                {
                    float closestDepth = texture(shadowMap, fragToLight + vec3(x, y, z)).r; // use lightdir to lookup cubemap
                    closestDepth *= far_plane;   // Undo mapping [0;1]
                    if(currentDepth - bias > closestDepth)
                        shadow += 1.0;
                }
            }
        }
        shadow /= (samples * samples * samples);
         return shadow;
    } else 
    {
        float shadow = 0.0;
        float bias = 0.15;
        int samples = 20;
        float viewDistance = length(viewPos - fragPos);
        float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
        float closestDepth=0.0;
        for(int i = 0; i < samples; ++i)
        {
            closestDepth = texture(shadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
            closestDepth *= far_plane;   // undo mapping [0;1]
            if(currentDepth - bias > closestDepth)
                shadow += 1.0;
        }
        shadow /= float(samples);

        
    return shadow;
    }

    return 0.0;
  

}
    
  
         

        void main() 
        {       

            vec3 color = texture(diffuseMap, TexCoords).rgb;
            vec3 normal = normalize(Normal);
            vec3 lightColor = vec3(1.0);
            
            // ambient
            vec3 ambient = 0.4 * lightColor;
            
            // diffuse
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(lightDir, normal), 0.0);
            vec3 diffuse = diff * lightColor;

            // specular
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = 0.0;
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
            vec3 specular = spec * lightColor;    
            // calculate shadow
             float shadow =ShadowCalculation(FragPos,pcf);                                       
             vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
            
         // FragColor =DebugDepth(FragPos);

//            vec3 lighting = (ambient + diffuse + specular) * color;

            FragColor = vec4(lighting, 1.0);

        }
    );
   if (! m_modelShader.Create(vShader, fShader))
   {
    DEBUG_BREAK_IF(true);
   }
    m_modelShader.LoadDefaults();
    m_modelShader.SetInt("diffuseMap", 0);
    m_modelShader.SetInt("shadowMap", 1);
    m_modelShader.SetFloat("viewPos", 0.0f, 0.0f, 0.0f);
    


    //  m_modelShader.SetFloat("lightPos[0]", 0.0f, 5.0f, 0.0f);
    //  m_modelShader.SetFloat("lightIntensity[0]", 0.45f);
    //  m_modelShader.SetFloat("lightColor[0]", 1.0f, 1.0f, 1.0f);

    
    float y =1.5f;

    m_lights.push_back(Vec3( 0.0f, y, -28.0f));
    m_lights.push_back(Vec3( 0.0f, y, -18.0f));
    m_lights.push_back(Vec3( 0.0f, y, -7.0f));
    m_lights.push_back(Vec3( 0.0f, y, 3.0f));
    m_lights.push_back(Vec3( 0.0f, y, 10.0f));
    m_lights.push_back(Vec3( 0.0f, y, 15.0f));
    m_lights.push_back(Vec3( 0.0f, y, 24.0f));
    m_lights.push_back(Vec3( 0.0f, y, 36.0f));
    m_lights.push_back(Vec3( 0.0f, y, 47.0f));
    m_lights.push_back(Vec3( 0.0f, y, 58.0f));
    m_lights.push_back(Vec3( 0.0f, y, 68.0f));




    // m_modelShader.SetFloat("lightPos[0]", 0.0f, 2.5f, -30.0f);
    // m_modelShader.SetFloat("lightPos[1]", 0.0f, 2.5f, -18.0f);
    // m_modelShader.SetFloat("lightPos[2]", 0.0f, 2.5f, -7.0f);
    // m_modelShader.SetFloat("lightPos[3]", 0.0f, 2.5f, 3.0f);
    // m_modelShader.SetFloat("lightPos[4]", 0.0f, 2.5f, 10.0f);
    // m_modelShader.SetFloat("lightPos[5]", 0.0f, 2.5f, 15.0f);
    // m_modelShader.SetFloat("lightPos[6]", 0.0f, 2.5f, 24.0f);
    // m_modelShader.SetFloat("lightPos[7]", 0.0f, 2.5f, 36.0f);
    // m_modelShader.SetFloat("lightPos[8]", 0.0f, 2.5f, 47.0f);
    // m_modelShader.SetFloat("lightPos[9]", 0.0f, 2.5f, 58.0f);
    // m_modelShader.SetFloat("lightPos[10]", 0.0f, 2.5f, 68.0f);

 






    return false;
}

bool Scene::LoadSkinShader()
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

        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;

        const int MAX_BONES = 80;
        const int MAX_BONE_INFLUENCE = 4;
        uniform mat4 Joints[MAX_BONES];

        void main() 
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
            gl_Position = projection * viewModel * totalPosition;

            Normal = mat3(transpose(inverse(viewModel))) * totalNormal.xyz; 
            FragPos = (model * totalPosition).xyz; 


            TexCoords = aTexCoords;
        });

   
    const char *fShader = GLSL(
        out vec4 FragColor;

        in  vec3 FragPos;
        in  vec2 TexCoords;
        in  vec3 Normal; 
      

        uniform sampler2D diffuseMap;
        uniform samplerCube depthMap;

        uniform vec3 lightPos;
        uniform vec3 viewPos;
      
        uniform float far_plane;


        const bool pcf=true;
        const bool shadows=true;


        
  
         

        void main() 
        {       

            vec3 color = texture(diffuseMap, TexCoords).rgb;
            vec3 normal = normalize(Normal);
            vec3 lightColor = vec3(0.6);
            // ambient
            vec3 ambient = 0.9 * lightColor;
            // diffuse
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(lightDir, normal), 0.0);
            vec3 diffuse = diff * lightColor;
            // specular
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = 0.0;
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
            vec3 specular = spec * lightColor;    
            // calculate shadow
            // float shadow =shadows ? ShadowCalculation(fs_in.FragPos,pcf) : 0.0;                                       
            // vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
            
        //  FragColor =DebugDepth(fs_in.FragPos);

            vec3 lighting = (ambient + diffuse + specular) * color;

            FragColor = vec4(lighting, 1.0);
           

        }
    );

    if (!m_skinShader.Create(vShader, fShader))
    {
        DEBUG_BREAK_IF(true);
    }
    m_skinShader.LoadDefaults();
    m_skinShader.SetInt("diffuseMap", 0);









    m_skinShader.SetFloat("viewPos", 0.0f, 0.0f, 0.0f);

    return false;
}

bool Scene::LoadDepthShader()
{
    
        const char *vertexShaderSource = GLSL(
                layout (location = 0) in vec3 aPos;


      
            uniform mat4 model;


            void main()
            {
              
                    gl_Position =  model * vec4(aPos, 1.0);}
                
            
           );

        const char *fragmentShaderSource = GLSL(

            in vec4 FragPos;

            uniform vec3 lightPos;
            uniform float far_plane;

            void main()
            {
                float lightDistance = length(FragPos.xyz - lightPos);
                
                // map to [0;1] range by dividing by far_plane
                lightDistance = lightDistance / far_plane;
                
                // write this as modified depth
                gl_FragDepth = lightDistance;
            }
        );

        const char *geometryShaderSource = GLSL(
            layout (triangles) in;
            layout (triangle_strip, max_vertices=18) out;

            uniform mat4 shadowMatrices[6];

            out vec4 FragPos; // FragPos from GS (output per emitvertex)

            void main()
            {
                for(int face = 0; face < 6; ++face)
                {
                    gl_Layer = face; // built-in variable that specifies to which face we render.
                    for(int i = 0; i < 3; ++i) // for each triangle's vertices
                    {
                        FragPos = gl_in[i].gl_Position;
                        gl_Position = shadowMatrices[face] * FragPos;
                        EmitVertex();
                    }    
                    EndPrimitive();
                }
            } 
        );

        if (!m_depthShader.Create(vertexShaderSource, fragmentShaderSource,geometryShaderSource))
        {
            DEBUG_BREAK_IF(true);
            return false;
        }
        m_depthShader.LoadDefaults();
     //   m_depthShader.SetInt("isAnimated", 0);

        return true;
}

//*********************************************************************************
bool DepthBuffer::Init(unsigned int width, unsigned int height)
{
    this->width = width;
    this->height = height;
    
    
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

void DepthBuffer::Begin()
{
    
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
    
}

void DepthBuffer::End()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthBuffer::BindTexture(int layer )
{
    glActiveTexture(GL_TEXTURE0 + layer);
    glBindTexture(GL_TEXTURE_2D, depthMap);
}

void DepthBuffer::Release()
{
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMap);
    
}




//*********************************************************************************
bool CubeBuffer::Init(unsigned int width, unsigned int height)
{
    this->width = width;
    this->height = height;
    

    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture

    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

void CubeBuffer::Begin()
{
    
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
    
}

void CubeBuffer::End()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubeBuffer::BindTexture(int layer )
{
    glActiveTexture(GL_TEXTURE0 + layer);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
}

void CubeBuffer::Release()
{
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthCubemap);
    
}
