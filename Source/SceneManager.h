///////////////////////////////////////////////////////////////////////////////
// SceneManager.h
// ==============
// Manages the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Created for CS-330 Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"

#include <string>
#include <vector>

/***********************************************************
 *  SceneManager
 *
 *  This class contains the code for preparing and rendering
 *  3D scenes, including the shader settings.
 ***********************************************************/
class SceneManager
{
public:
    // constructor
    SceneManager(ShaderManager* pShaderManager);
    // destructor
    ~SceneManager();

    // texture info struct
    struct TEXTURE_INFO
    {
        std::string tag;
        uint32_t ID;
    };

    // material info struct
    struct OBJECT_MATERIAL
    {
        std::string tag;
        float ambientStrength;
        glm::vec3 ambientColor;
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        float shininess;
    };

private:
    // shader and mesh managers
    ShaderManager* m_pShaderManager;
    ShapeMeshes* m_basicMeshes;

    // texture tracking
    int m_loadedTextures = 0;
    TEXTURE_INFO m_textureIDs[16];

    // material definitions
    std::vector<OBJECT_MATERIAL> m_objectMaterials;

    // texture and material setup
    bool CreateGLTexture(const char* filename, std::string tag);
    void BindGLTextures();
    void DestroyGLTextures();
    int FindTextureID(std::string tag);
    int FindTextureSlot(std::string tag);
    bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);

    // shader and transform utilities
    void SetTransformations(glm::vec3 scaleXYZ, float XrotationDegrees, float YrotationDegrees, float ZrotationDegrees, glm::vec3 positionXYZ);
    void SetShaderColor(float redColorValue, float greenColorValue, float blueColorValue, float alphaValue);
    void SetShaderTexture(std::string textureTag);
    void SetTextureUVScale(float u, float v);
    void SetShaderMaterial(std::string materialTag);

    // scene setup
    void DefineObjectMaterials();
    void SetupSceneLights();

public:
    // student-customizable methods
    void PrepareScene();
    void RenderScene();
    void Update();
};
