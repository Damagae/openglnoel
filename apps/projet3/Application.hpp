#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include <glm/glm.hpp>

#include <tiny_gltf.h>

#include "CameraController.hpp"

class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:
    void computeMatrices(tinygltf::Node node, glm::mat4 matrix);
    void initShadersData();
    void initShadowData();

    glm::mat4 scaleModel(glm::mat4 matrix) {
      return glm::translate(glm::scale(matrix, m_ModelScaling), m_ModelTranslating);
    }

    void computeScaling() {
      float maxScale = 1.f;
      float scaleFactor = 1;
      float maxTranslation = 1.f;

      for (uint i = 0; i < m_ModelMatrices.size(); ++i) {
        glm::mat4 matrix = m_ModelMatrices[i];

        // std::cout << matrix << std::endl;
        // std::cout << "scale : (" << matrix[0][0] << ", " << matrix[1][1] << ", " << matrix[2][2] << ")" << std::endl;
        // std::cout << "translate : (" << matrix[3][0] << ", " << matrix[3][1] << ", " << matrix[3][2] << ")" << std::endl;

        // Scaling
        if (glm::abs(matrix[0][0]) > maxScale) {
          maxScale = glm::abs(matrix[0][0]);
        }
        else if (glm::abs(matrix[1][1]) > maxScale) {
          maxScale = glm::abs(matrix[1][1]);
        }
        else if (glm::abs(matrix[2][2]) > maxScale) {
          maxScale = glm::abs(matrix[2][2]);
        }

        // Translation
        if (glm::abs(matrix[3][0]) > maxTranslation) {
          maxTranslation = glm::abs(matrix[3][0]);
        }
        else if (glm::abs(matrix[3][1]) > maxTranslation) {
          maxTranslation = glm::abs(matrix[3][1]);
        }
        else if (glm::abs(matrix[3][2]) > maxTranslation) {
          maxTranslation = glm::abs(matrix[3][2]);
        }
      }

      // std::cout << "max scale : " << maxScale << std::endl;
      // std::cout << "max translation : " << maxTranslation * 0.1 << std::endl;

      if (maxTranslation > maxScale) {
        scaleFactor = 1.f / (maxTranslation * 0.1);
      } else {
        scaleFactor = 1.f / maxScale;
      }

      m_ModelScaling = glm::vec3(scaleFactor, scaleFactor, scaleFactor);
    }

    void computeScaling2() {
       float maxAxis = m_SceneSize[0];
       if (m_SceneSize[1] > maxAxis) { maxAxis = m_SceneSize[1]; }
       if (m_SceneSize[2] > maxAxis) { maxAxis = m_SceneSize[2]; }

       std::cout << "max axis = " << maxAxis << std::endl;
       std::cout << "Scene Size = " << m_SceneSize << std::endl;

       m_ModelScaling = glm::vec3(3000 / maxAxis);
       std::cout << "scaling = " << m_ModelScaling << std::endl;
 			 m_ModelTranslating = glm::vec3(0.f, m_SceneSize[1] * (3000 / maxAxis) * 0.005, 0.f);
    }

    static glm::mat4 quatToMatrix(glm::vec4 q) {
      float sqw = q.w * q.w;
      float sqx = q.x * q.x;
      float sqy = q.y * q.y;
      float sqz = q.z * q.z;

      // invs (inverse square length) is only required if quaternion is not already normalised
      float invs = 1 / (sqx + sqy + sqz + sqw);
      float m00 = ( sqx - sqy - sqz + sqw)*invs ; // since sqw + sqx + sqy + sqz =1/invs*invs
      float m11 = (-sqx + sqy - sqz + sqw)*invs ;
      float m22 = (-sqx - sqy + sqz + sqw)*invs ;

      float tmp1 = q.x*q.y;
      float tmp2 = q.z*q.w;
      float m10 = 2.0 * (tmp1 + tmp2)*invs ;
      float m01 = 2.0 * (tmp1 - tmp2)*invs ;

      tmp1 = q.x*q.z;
      tmp2 = q.y*q.w;
      float m20 = 2.0 * (tmp1 - tmp2)*invs ;
      float m02 = 2.0 * (tmp1 + tmp2)*invs ;
      tmp1 = q.y*q.z;
      tmp2 = q.x*q.w;
      float m21 = 2.0 * (tmp1 + tmp2)*invs ;
      float m12 = 2.0 * (tmp1 - tmp2)*invs ;

      return glm::mat4( m00, m01, m02, 0,
                        m10, m11, m12, 0,
                        m20, m21, m22, 0,
                        0,   0,   0,   1 );
    }


    static glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians)
    {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto sinTheta = glm::sin(thetaRadians);
        return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
    }

    static std::string getFilePathExtension(const std::string &FileName) {
      if (FileName.find_last_of(".") != std::string::npos)
        return FileName.substr(FileName.find_last_of(".") + 1);
      return "";
    }

    static int getMode(int tinygltfMode) {
      if (tinygltfMode == TINYGLTF_MODE_TRIANGLES) { return GL_TRIANGLES; }
      else if (tinygltfMode == TINYGLTF_MODE_TRIANGLE_STRIP) { return GL_TRIANGLE_STRIP; }
      else if (tinygltfMode == TINYGLTF_MODE_TRIANGLE_FAN) { return GL_TRIANGLE_FAN; }
      else if (tinygltfMode == TINYGLTF_MODE_POINTS) { return GL_POINTS; }
      else if (tinygltfMode == TINYGLTF_MODE_LINE) { return GL_LINES; }
      else if (tinygltfMode == TINYGLTF_MODE_LINE_LOOP) { return GL_LINE_LOOP; }
      return -1;
    }

    static glm::vec3 computeDirectionVectorUp(float phiRadians, float thetaRadians) {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto cosTheta = glm::cos(thetaRadians);
        return -glm::normalize(glm::vec3(sinPhi * cosTheta, -glm::sin(thetaRadians), cosPhi * cosTheta));
    }

    // Custom lerp function because couldn't make glm::lerp works properly
    static glm::vec3 lerp(glm::vec3 x, glm::vec3 y, glm::vec3 a) {
        return x * (glm::vec3(1.0f) - a) + y * a;
    }

    const int m_nWindowWidth = 1280;
    const int m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    // GBuffer:
    enum GBufferTextureType
    {
        GPosition = 0,
        GNormal,
        GAmbient,
        GDiffuse,
        GGlossyShininess,
        GDepth,
        Display_DirectionalLightDepthMap,
        GBufferTextureCount
    };

    const char * m_GBufferTexNames[GBufferTextureCount + 1] = { "position", "normal", "ambient", "diffuse", "glossyShininess", "depth", "directionalLightDepth", "beauty" }; // Tricks, since we cant blit depth, we use its value to draw the result of the shading pass
    const GLenum m_GBufferTextureFormat[GBufferTextureCount] = { GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT32F };
    GLuint m_GBufferTextures[GBufferTextureCount];
    GLuint m_GBufferFBO; // Framebuffer object

    GBufferTextureType m_CurrentlyDisplayed = GBufferTextureCount; // Default to beauty

    // Triangle covering the whole screen, for the shading pass:
    GLuint m_TriangleVBO = 0;
    GLuint m_TriangleVAO = 0;

    glm::vec3 m_SceneSize = glm::vec3(0.f); // Used for camera speed and projection matrix parameters
    float m_SceneSizeLength = 0.f;
    glm::vec3 m_SceneCenter = glm::vec3(0.f);

    const glm::vec3 dielectricSpecular = glm::vec3(0.04, 0.04, 0.04);
    const glm::vec3 black = glm::vec3(0, 0, 0);


    // Shading PBR functions
    const glm::vec3 getKd(tinygltf::Material material) {

      const glm::vec3 &baseColor = glm::vec3(material.values["baseColorFactor"].number_array[0], material.values["baseColorFactor"].number_array[1], material.values["baseColorFactor"].number_array[2]);
      const glm::vec3 &metallic = glm::vec3(material.values["metallicFactor"].number_array[0], material.values["metallicFactor"].number_array[1], material.values["metallicFactor"].number_array[2]);
      const glm::vec3 roughness = glm::vec3(material.values["roughnessFactor"].number_array[0], material.values["roughnessFactor"].number_array[1], material.values["roughnessFactor"].number_array[2]);

      const glm::vec3 &first = baseColor * (1 - dielectricSpecular[0]);

      const auto c = lerp(first, black, metallic);
      const auto f = lerp(dielectricSpecular, baseColor, metallic);
      const auto a = roughness * roughness;

      return (glm::vec3(1.f) - f) * c;

    }

    const glm::vec3 getKs(tinygltf::Material material) {
      const glm::vec3 &baseColor = glm::vec3(material.values["baseColorFactor"].number_array[0], material.values["baseColorFactor"].number_array[1], material.values["baseColorFactor"].number_array[2]);
      const glm::vec3 &metallic = glm::vec3(material.values["metallicFactor"].number_array[0], material.values["metallicFactor"].number_array[1], material.values["metallicFactor"].number_array[2]);
      const glm::vec3 roughness = glm::vec3(material.values["roughnessFactor"].number_array[0], material.values["roughnessFactor"].number_array[1], material.values["roughnessFactor"].number_array[2]);

      const glm::vec3 one = glm::vec3(1.f);

      const auto cdiff = lerp(baseColor * (1 - dielectricSpecular[0]), black, metallic);
      const auto f0 = lerp(dielectricSpecular, baseColor, metallic);
      const auto alpha = roughness * roughness;

      const auto f = f0 + (one - f0); // * one

      return glm::vec3(0.f);
    }

    const glm::vec3 getShininess(tinygltf::Material material) {

    }

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_AssetsRootPath;

    tinygltf::Model m_Model;
    std::map<int, glm::mat4> m_ModelMatrices;
    std::vector<int> m_MeshIds;
    std::vector<GLuint> m_TextureIds;
    std::vector<GLuint> m_SamplersIds;
    std::vector<tinygltf::Material> m_Materials;

    std::vector<GLuint> m_VAOs;
    std::vector<tinygltf::Primitive> m_Primitives;

    glm::vec3 m_ModelScaling = glm::vec3(1,1,1);
    glm::vec3 m_ModelTranslating = glm::vec3(0,0,0);

    glmlv::SimpleGeometry m_cubeGeometry;
    glmlv::SimpleGeometry m_sphereGeometry;

    GLuint m_cubeVBO = 0;
    GLuint m_cubeIBO = 0;
    GLuint m_cubeVAO = 0;
    GLuint m_cubeTextureKd = 0;

    GLuint m_sphereVBO = 0;
    GLuint m_sphereIBO = 0;
    GLuint m_sphereVAO = 0;
    GLuint m_sphereTextureKd = 0;

    GLuint m_textureSampler = 0; // Only one sampler object since we will use the sample sampling parameters for the two textures

    glmlv::GLProgram m_program;

    // glmlv::ViewController m_viewController{ m_GLFWHandle.window(), 3.f };
    CameraController m_CameraController{ m_GLFWHandle.window(), 0.008f };

    // GLSL programs
    glmlv::GLProgram m_geometryPassProgram;
    glmlv::GLProgram m_shadingPassProgram;
    glmlv::GLProgram m_displayDepthProgram;
    glmlv::GLProgram m_displayPositionProgram;
    glmlv::GLProgram m_directionalSMProgram;

    // Geometry pass uniforms
    GLint m_uModelViewProjMatrixLocation;
    GLint m_uModelViewMatrixLocation;
    GLint m_uNormalMatrixLocation;
    GLint m_uKaLocation;
    GLint m_uKdLocation;
    GLint m_uKsLocation;
    GLint m_uShininessLocation;
    GLint m_uKaSamplerLocation;
    GLint m_uKdSamplerLocation;
    GLint m_uKsSamplerLocation;
    GLint m_uShininessSamplerLocation;

    // Shading pass uniforms
    GLint m_uGBufferSamplerLocations[GDepth];
    GLint m_uDirectionalLightDirLocation;
    GLint m_uDirectionalLightIntensityLocation;
    GLint m_uPointLightPositionLocation;
    GLint m_uPointLightIntensityLocation;
    GLint m_uDirLightViewProjMatrix_shadingPass; // Suffix because the variable m_uDirLightViewProjMatrix is already used for the uniform of m_directionalSMProgram.
    GLint m_uDirLightShadowMap;
    GLint m_uDirLightShadowMapBias;
    GLint m_uDirLightShadowMapSampleCount;
    GLint m_uDirLightShadowMapSpread;

    // Display depth pass uniforms
    GLint m_uGDepthSamplerLocation;

    // Display position pass uniforms
    GLint m_uGPositionSamplerLocation;
    GLint m_uSceneSizeLocation;

    // Shadow Mapping uniform
    GLint m_uDirLightViewProjMatrix;

    // Lights
    float m_DirLightPhiAngleDegrees = 16.f;
    float m_DirLightThetaAngleDegrees = 45.f;
    glm::vec3 m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
    glm::vec3 m_DirLightColor = glm::vec3(1, 1, 1);
    float m_DirLightIntensity = 1.4f;
    float m_DirLightSMBias = 0.01f;
    int m_DirLightSMSampleCount = 16;
    float m_DirLightSMSpread = 0.0005;

    glm::vec3 m_PointLightPosition = glm::vec3(0, 1, 0);
    glm::vec3 m_PointLightColor = glm::vec3(1, 1, 1);
    float m_PointLightIntensity = 0.f;

    glm::vec3 m_CubeKd = glm::vec3(1, 0, 0);
    glm::vec3 m_SphereKd = glm::vec3(0, 1, 0);
    glm::vec3 m_ModelKd = glm::vec3(0, 1, 0);

    // Shadow mapping
    GLuint m_directionalSMTexture;
    GLuint m_directionalSMFBO;
    GLuint m_directionalSMSampler;
    int32_t m_nDirectionalSMResolution = 4096;

};
