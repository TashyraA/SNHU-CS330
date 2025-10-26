///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp
// ================
// Manages the loading and rendering of 3D scenes
//
// AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
// Created for CS-330 Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"
#include <iostream>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

namespace {
    const char* g_ModelName = "model";
    const char* g_ColorValueName = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName = "bUseTexture";
    const char* g_UseLightingName = "bUseLighting";
}

SceneManager::SceneManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_basicMeshes = new ShapeMeshes();
}

SceneManager::~SceneManager()
{
    m_pShaderManager = nullptr;
    delete m_basicMeshes;
    m_basicMeshes = nullptr;
}

bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
    int width = 0, height = 0, colorChannels = 0;
    GLuint textureID = 0;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* image = stbi_load(filename, &width, &height, &colorChannels, 0);
    if (image)
    {
        std::cout << "Successfully loaded image: " << filename << std::endl;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (colorChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (colorChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else {
            std::cout << "Unsupported image format." << std::endl;
            stbi_image_free(image);
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_textureIDs[m_loadedTextures].ID = textureID;
        m_textureIDs[m_loadedTextures].tag = tag;
        m_loadedTextures++;
        return true;
    }

    std::cout << "Failed to load image: " << filename << std::endl;
    return false;
}

void SceneManager::BindGLTextures()
{
    for (int i = 0; i < m_loadedTextures; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
    }
}

void SceneManager::DestroyGLTextures()
{
    for (int i = 0; i < m_loadedTextures; ++i)
    {
        glDeleteTextures(1, &m_textureIDs[i].ID);
    }
}

int SceneManager::FindTextureID(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; ++i)
        if (m_textureIDs[i].tag == tag)
            return m_textureIDs[i].ID;
    return -1;
}

int SceneManager::FindTextureSlot(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; ++i)
        if (m_textureIDs[i].tag == tag)
            return i;
    return -1;
}

bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
    for (const auto& mat : m_objectMaterials)
    {
        if (mat.tag == tag)
        {
            material = mat;
            return true;
        }
    }
    return false;
}

void SceneManager::SetTransformations(glm::vec3 scaleXYZ, float Xrot, float Yrot, float Zrot, glm::vec3 positionXYZ)
{
    glm::mat4 scale = glm::scale(scaleXYZ);
    glm::mat4 rotationX = glm::rotate(glm::radians(Xrot), glm::vec3(1, 0, 0));
    glm::mat4 rotationY = glm::rotate(glm::radians(Yrot), glm::vec3(0, 1, 0));
    glm::mat4 rotationZ = glm::rotate(glm::radians(Zrot), glm::vec3(0, 0, 1));
    glm::mat4 translation = glm::translate(positionXYZ);
    glm::mat4 modelView = translation * rotationX * rotationY * rotationZ * scale;

    if (m_pShaderManager)
        m_pShaderManager->setMat4Value(g_ModelName, modelView);
}

void SceneManager::SetShaderColor(float r, float g, float b, float a)
{
    glm::vec4 color(r, g, b, a);
    if (m_pShaderManager)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, false);
        m_pShaderManager->setVec4Value(g_ColorValueName, color);
    }
}

void SceneManager::SetShaderTexture(std::string textureTag)
{
    if (m_pShaderManager)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, true);
        int slot = FindTextureSlot(textureTag);
        m_pShaderManager->setSampler2DValue(g_TextureValueName, slot);
    }
}

void SceneManager::SetTextureUVScale(float u, float v)
{
    if (m_pShaderManager)
        m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
}

void SceneManager::SetShaderMaterial(std::string tag)
{
    OBJECT_MATERIAL mat;
    if (FindMaterial(tag, mat))
    {
        m_pShaderManager->setVec3Value("material.ambientColor", mat.ambientColor);
        m_pShaderManager->setFloatValue("material.ambientStrength", mat.ambientStrength);
        m_pShaderManager->setVec3Value("material.diffuseColor", mat.diffuseColor);
        m_pShaderManager->setVec3Value("material.specularColor", mat.specularColor);
        m_pShaderManager->setFloatValue("material.shininess", mat.shininess);
    }
}
/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes and textures into memory to support 3D rendering.
 ***********************************************************/
void SceneManager::PrepareScene()
{
    // Define materials and lighting setup
    DefineObjectMaterials();
    SetupSceneLights();

    // Load all basic mesh shapes used in the scene
    m_basicMeshes->LoadPlaneMesh();
    m_basicMeshes->LoadCylinderMesh();
    m_basicMeshes->LoadTorusMesh();
    m_basicMeshes->LoadSphereMesh();
    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadConeMesh();
    m_basicMeshes->LoadTaperedCylinderMesh();

    // Load texture assets and assign tags
    CreateGLTexture("../../Utilities/textures/rusticwood.jpg", "bowl");
    CreateGLTexture("../../Utilities/textures/rusticwood.jpg", "bowl_inner");
    CreateGLTexture("../../Utilities/textures/rusticwood.jpg", "rim");
    CreateGLTexture("../../Utilities/textures/rusticwood.jpg", "base");
    CreateGLTexture("../../Utilities/textures/blackwood.jpg", "blackwood");
    CreateGLTexture("../../Utilities/textures/rusticwood.jpg", "cuttingboard");
    CreateGLTexture("../../Utilities/textures/glass.jpg", "glass");

    // Bind all loaded textures to their respective slots
    BindGLTextures();
}


/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for defining the materials
 *  to be used in the scene
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
    m_objectMaterials.push_back({ "wood", 0.3f, {0.3f, 0.2f, 0.1f}, {0.55f, 0.27f, 0.07f}, {0.2f, 0.2f, 0.2f}, 12.0f });
    m_objectMaterials.push_back({ "blackwood", 0.3f, {0.2f, 0.1f, 0.05f}, {0.55f, 0.27f, 0.07f}, {0.3f, 0.2f, 0.1f}, 12.0f });
    m_objectMaterials.push_back({ "apple", 0.3f, {0.4f, 0.1f, 0.1f}, {0.85f, 0.2f, 0.2f}, {1.0f, 0.6f, 0.6f}, 32.0f });
    m_objectMaterials.push_back({ "orange", 0.3f, {0.6f, 0.3f, 0.1f}, {1.0f, 0.6f, 0.1f}, {1.0f, 0.7f, 0.3f}, 24.0f });
    m_objectMaterials.push_back({ "lemon", 0.2f, {0.8f, 0.8f, 0.2f}, {1.0f, 1.0f, 0.3f}, {0.9f, 0.9f, 0.4f}, 16.0f });
    m_objectMaterials.push_back({ "pear", 0.3f, {0.2f, 0.6f, 0.2f}, {0.3f, 0.8f, 0.3f}, {0.6f, 0.9f, 0.6f}, 20.0f });
    m_objectMaterials.push_back({ "stem", 0.2f, {0.1f, 0.3f, 0.1f}, {0.1f, 0.4f, 0.1f}, {0.2f, 0.2f, 0.2f}, 8.0f });
    m_objectMaterials.push_back({ "ceramic", 0.3f, {0.8f, 0.8f, 0.8f}, {0.9f, 0.9f, 0.9f}, {1.0f, 1.0f, 1.0f}, 40.0f });
    m_objectMaterials.push_back({ "glass", 0.2f, {0.6f, 0.5f, 0.6f}, {0.8f, 0.7f, 0.8f}, {1.0f, 1.0f, 1.0f}, 64.0f });
    m_objectMaterials.push_back({ "petal", 0.3f, {1.0f, 0.8f, 0.8f}, {1.0f, 0.6f, 0.6f}, {1.0f, 0.9f, 0.9f}, 24.0f });
    m_objectMaterials.push_back({ "center", 0.3f, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, 16.0f });
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is used for setting up the lighting
 *  for the scene
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
    m_pShaderManager->setBoolValue("bUseLighting", true);

    // Warm key light from front-right
    m_pShaderManager->setVec3Value("lightSources[0].position", 4.0f, 6.0f, 4.0f);
    m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.3f, 0.2f, 0.2f);
    m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.9f, 0.6f, 0.5f);
    m_pShaderManager->setVec3Value("lightSources[0].specularColor", 1.0f, 0.8f, 0.7f);

    // Soft white fill light from back-left
    m_pShaderManager->setVec3Value("lightSources[1].position", -4.0f, 3.0f, -3.0f);
    m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.05f, 0.05f, 0.05f);
    m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.4f, 0.4f, 0.4f);
    m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.6f, 0.6f, 0.6f);
}
/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
    glm::vec3 scaleXYZ;
    glm::vec3 positionXYZ;

    m_pShaderManager->setBoolValue("bUseLighting", true);
    m_pShaderManager->setBoolValue("bUseTexture", true);
    m_pShaderManager->setVec4Value("objectColor", glm::vec4(1.0f));

    // Background plane 
    scaleXYZ = glm::vec3(20.0f, 1.0f, 20.0f);
    positionXYZ = glm::vec3(0.0f, 10.0f, -15.0f);
    SetTransformations(scaleXYZ, 90.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderColor(0.25f, 0.23f, 0.22f, 1.0f); 
    m_basicMeshes->DrawPlaneMesh();

    // Base plane
    scaleXYZ = glm::vec3(20.0f, 1.0f, 20.0f);
    positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("blackwood");
    SetShaderMaterial("blackwood");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawPlaneMesh();

    // Bowl outer wall
    scaleXYZ = glm::vec3(3.0f, 2.0f, 3.0f);
    positionXYZ = glm::vec3(0.0f, 1.0f, -5.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("bowl");
    SetShaderMaterial("wood");
    SetTextureUVScale(2.0f, 2.0f);
    m_basicMeshes->DrawCylinderMesh();

    // Bowl inner hollow
    scaleXYZ = glm::vec3(2.9f, 0.5f, 2.9f);
    positionXYZ = glm::vec3(0.0f, 0.525f, -5.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("bowl_inner");
    SetShaderMaterial("wood");
    SetTextureUVScale(1.5f, 1.5f);
    m_basicMeshes->DrawCylinderMesh(true, true, true);

    // Bowl rim
    scaleXYZ = glm::vec3(3.05f, 0.05f, 3.05f);
    positionXYZ = glm::vec3(0.0f, 1.025f, -5.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("rim");
    SetShaderMaterial("wood");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawCylinderMesh(true, true, true);

    // Bowl base
    scaleXYZ = glm::vec3(1.2f, 0.1f, 1.2f);
    positionXYZ = glm::vec3(0.0f, 0.1f, -5.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("base");
    SetShaderMaterial("wood");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawCylinderMesh(true, true, true);

    // Fruits
    SetTransformations(glm::vec3(0.8f), 0, 0, 0, glm::vec3(-0.7f, 3.0f, -5.0f));
    SetShaderMaterial("apple");
    m_basicMeshes->DrawSphereMesh();

    SetTransformations(glm::vec3(0.9f), 0, 0, 0, glm::vec3(0.5f, 3.0f, -5.2f));
    SetShaderMaterial("orange");
    m_basicMeshes->DrawSphereMesh();

    SetTransformations(glm::vec3(1.0f, 0.8f, 0.8f), 0, 0, 0, glm::vec3(0.0f, 2.80f, -4.8f));
    SetShaderMaterial("lemon");
    m_basicMeshes->DrawSphereMesh();

    SetTransformations(glm::vec3(0.8f, 1.2f, 0.8f), 0, 0, 0, glm::vec3(0.2f, 3.0f, -5.0f));
    SetShaderMaterial("pear");
    m_basicMeshes->DrawSphereMesh();

    SetTransformations(glm::vec3(0.05f, 0.3f, 0.05f), 15.0f, 0.0f, 0.0f, glm::vec3(0.2f, 3.1f, -5.0f));
    SetShaderMaterial("stem");
    m_basicMeshes->DrawCylinderMesh(true, true, true);

    // Cutting board 
    scaleXYZ = glm::vec3(4.0f, 0.12f, 2.2f);      
    positionXYZ = glm::vec3(-4.5f, 0.06f, -4.5f);  
    SetTransformations(scaleXYZ, 0.0f, 15.0f, 0.0f, positionXYZ);
    SetShaderTexture("cuttingboard");
    SetShaderMaterial("wood");
    SetTextureUVScale(2.0f, 1.2f);
    m_basicMeshes->DrawBoxMesh();

    // Coffee mug 
    scaleXYZ = glm::vec3(0.85f, 1.2f, 0.85f);     
    positionXYZ = glm::vec3(4.2f, 0.6f, -4.2f);   
    SetTransformations(scaleXYZ, 0.0f, -25.0f, 0.0f, positionXYZ);
    SetShaderMaterial("ceramic");
    m_basicMeshes->DrawCylinderMesh(false, true, true);

    // Mug handle 
    scaleXYZ = glm::vec3(0.32f);                   
    positionXYZ = glm::vec3(4.8f, 1.1f, -4.2f);   
    SetTransformations(scaleXYZ, 0.0f, 90.0f, 0.0f, positionXYZ);
    SetShaderMaterial("ceramic");
    m_basicMeshes->DrawTorusMesh();

    // Vase 
    scaleXYZ = glm::vec3(0.8f, 2.0f, 0.8f);       
    positionXYZ = glm::vec3(-5.0f, 1.0f, -7.0f);  
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderMaterial("glass");
    m_basicMeshes->DrawTaperedCylinderMesh(true, true, true);

    // Flower positions 
    const glm::vec3 flowerPositions[3] = {
        glm::vec3(-5.1f, 3.2f, -6.9f),    
        glm::vec3(-4.8f, 3.0f, -7.2f),    
        glm::vec3(-5.4f, 3.0f, -7.2f)     
    };



    for (int i = 0; i < 3; i++) {
        scaleXYZ = glm::vec3(0.015f, 0.9f, 0.015f);
        positionXYZ = flowerPositions[i] - glm::vec3(0.0f, 0.45f, 0.0f);
        SetTransformations(scaleXYZ, -5.0f + i * 6.0f, 0.0f, 0.0f, positionXYZ);
        SetShaderMaterial("stem");
        m_basicMeshes->DrawCylinderMesh(true, true, true);

        scaleXYZ = glm::vec3(0.09f, 0.14f, 0.09f);
        positionXYZ = flowerPositions[i];
        SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
        SetShaderMaterial("petal");
        m_basicMeshes->DrawConeMesh(true);

        scaleXYZ = glm::vec3(0.04f);
        positionXYZ = flowerPositions[i] + glm::vec3(0.0f, 0.03f, 0.0f);
        SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
        SetShaderMaterial("center");
        m_basicMeshes->DrawSphereMesh();
    }
}

/********************************************
 *  SceneManager::Update()
 *
 *  This method is called every frame for
 *  updating scene geometry, animations, etc.
 ********************************************/
void SceneManager::Update()
{
    static float angle = 0.0f;
    angle += 0.01f; // Increment the rotation angle

    // Rotate the bowl on its Y axis
    glm::mat4 bowlRotation = glm::rotate(glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    m_pShaderManager->setMat4Value("bowlRotation", bowlRotation);
}
