#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>

#include <glm/glm.hpp>

#include <limits>

#include <tiny_gltf.h>

class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:
    void initScene(const glmlv::fs::path & objPath);

    void initShadersData();
    void loadGLTFModel(const glmlv::fs::path & objPath);

    static void printNodes(const tinygltf::Scene &scene) {
      for (size_t i = 0; i < scene.nodes.size(); i++) {
        std::cout << "node.name : " << scene.nodes[i] << std::endl;
      }
    }

    static std::string getFilePathExtension(const std::string &FileName) {
      if (FileName.find_last_of(".") != std::string::npos)
        return FileName.substr(FileName.find_last_of(".") + 1);
      return "";
    }

    static glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians)
    {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto sinTheta = glm::sin(thetaRadians);
        return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
    }

    static std::vector<tinygltf::Texture>::iterator findIteratorInVector(std::vector<tinygltf::Texture> v, std::string s)
    {
      for (std::vector<tinygltf::Texture>::iterator it = v.begin() ; it != v.end(); ++it) {
        if ((*it).name == s) {
          return it;
        }
      }
      return v.begin();
    }

    static tinygltf::Texture findElementInVector(std::vector<tinygltf::Texture> v, std::string s)
    {
      for (int i = 0; i < v.size(); ++i) {
        if (v[i].name == s) {
          return v[i];
        }
      }
      return v[0];
    }

    static std::vector<tinygltf::Image>::iterator findIteratorInVector(std::vector<tinygltf::Image> v, int nbr)
    {
      for (std::vector<tinygltf::Image>::iterator it = v.begin() ; it != v.end(); ++it) {
        if ((*it).component == nbr) {
          return it;
        }
      }
      return v.begin();
    }

    static tinygltf::Image findElementInVector(std::vector<tinygltf::Image> v, int nbr)
    {
      for (int i = 0; i < v.size(); ++i) {
        if (v[i].component == nbr) {
          return v[i];
        }
      }
      return v[0];
    }

    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ (int) m_nWindowWidth, (int) m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;

    // GBuffer:
    enum GBufferTextureType
    {
        GPosition = 0,
        GNormal,
        GAmbient,
        GDiffuse,
        GGlossyShininess,
        GDepth,
        GBufferTextureCount
    };

    const char * m_GBufferTexNames[GBufferTextureCount + 1] = { "position", "normal", "ambient", "diffuse", "glossyShininess", "depth", "beauty" }; // Tricks, since we cant blit depth, we use its value to draw the result of the shading pass
    const GLenum m_GBufferTextureFormat[GBufferTextureCount] = { GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F };
    GLuint m_GBufferTextures[GBufferTextureCount];
    GLuint m_GBufferFBO; // Framebuffer object

    GBufferTextureType m_CurrentlyDisplayed = GBufferTextureCount; // Default to beauty

    // Triangle covering the whole screen, for the shading pass:
    GLuint m_TriangleVBO = 0;
    GLuint m_TriangleVAO = 0;

    // Scene data in GPU:
    GLuint m_SceneVBO = 0;
    GLuint m_SceneIBO = 0;
    GLuint m_SceneVAO = 0;

    typedef std::map<std::string, tinygltf::Parameter> ParameterMap;
    typedef std::map<std::string, tinygltf::Value> ExtensionMap;

    typedef struct {
      std::vector<GLuint> diffuseTex;  // for each primitive in mesh
    } GLMeshState;

    // tiny_gltf
    tinygltf::Model m_Model;
    std::map<int, GLuint> m_GBufferState; // Vertex buffer VBO
    std::map<std::string, GLMeshState> m_GMeshState;

    // Required data about the scene in CPU in order to send draw calls
    struct ShapeInfo
    {
        uint32_t indexCount; // Number of indices
        uint32_t indexOffset; // Offset in GPU index buffer
        int materialID = -1;
    };

    std::vector<ShapeInfo> m_shapes; // For each shape of the scene, its number of indices
    glm::vec3 m_SceneSize = glm::vec3(0.f); // Used for camera speed and projection matrix parameters
    float m_SceneSizeLength = 0.f;

    struct PhongMaterial
    {
        glm::vec3 Ka = glm::vec3(0); // Ambient multiplier
        glm::vec3 Kd = glm::vec3(0); // Diffuse multiplier
        glm::vec3 Ks = glm::vec3(0); // Glossy multiplier
        float shininess = 1.f; // Glossy exponent

        // OpenGL texture ids:
        GLuint KaTextureId = 0;
        GLuint KdTextureId = 0;
        GLuint KsTextureId = 0;
        GLuint shininessTextureId = 0;
    };

    GLuint m_WhiteTexture; // A white 1x1 texture
    PhongMaterial m_DefaultMaterial;
    std::vector<PhongMaterial> m_SceneMaterials;

    GLuint m_textureSampler = 0; // Only one sampler object since we will use the same sampling parameters for all textures

    // Camera
    glmlv::ViewController m_viewController{ m_GLFWHandle.window(), 3.f };

    // GLSL programs
    glmlv::GLProgram m_geometryPassProgram;
    glmlv::GLProgram m_shadingPassProgram;
    glmlv::GLProgram m_displayDepthProgram;
    glmlv::GLProgram m_displayPositionProgram;

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

    // Display depth pass uniforms
    GLint m_uGDepthSamplerLocation;

    // Display position pass uniforms
    GLint m_uGPositionSamplerLocation;
    GLint m_uSceneSizeLocation;

    // Lights
    float m_DirLightPhiAngleDegrees = 140.f;
    float m_DirLightThetaAngleDegrees = 45.f;
    glm::vec3 m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
    glm::vec3 m_DirLightColor = glm::vec3(1, 1, 1);
    float m_DirLightIntensity = 1.f;

    glm::vec3 m_PointLightPosition = glm::vec3(0, 1, 0);
    glm::vec3 m_PointLightColor = glm::vec3(1, 1, 1);
    float m_PointLightIntensity = 5.f;

};
