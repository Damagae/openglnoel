#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>
#include <unordered_map>
#include <map>
#include <unordered_set>

#include <tiny_gltf.h>

class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:
    void initScene(const glmlv::fs::path & objPath);
    void initShadersData();
    void CheckErrors(std::string desc);
    std::string GetFilePathExtension(const std::string &FileName);
    bool LoadShader(GLenum shaderType, GLuint &shader, const char *shaderSourceFilename);
    bool LinkShader(GLuint &prog, GLuint &vertShader, GLuint &fragShader);
    void reshapeFunc(GLFWwindow *window, int w, int h);
    void keyboardFunc(GLFWwindow *window, int key, int scancode, int action, int mods);
    void clickFunc(GLFWwindow *window, int button, int action, int mods);
    void motionFunc(GLFWwindow *window, double mouse_x, double mouse_y);
    void SetupMeshState();
    void SetupCurvesState(tinygltf::Scene &scene, GLuint progId);
    void DrawMesh(tinygltf::Model &model, const tinygltf::Mesh &mesh);
    void DrawCurves(tinygltf::Scene &scene, const tinygltf::Mesh &mesh);
    void DrawNode(tinygltf::Model &model, const tinygltf::Node &node);
    void DrawModel(tinygltf::Model &model);
    void Init();
    void PrintNodes(const tinygltf::Scene &scene);

    static glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians) {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto sinTheta = glm::sin(thetaRadians);
        return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
    }

    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ (int)m_nWindowWidth, (int)m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;

    const int width = 768;
    const int height = 768;

    tinygltf::Model m_Model;

    double prevMouseX, prevMouseY;
    bool mouseLeftPressed;
    bool mouseMiddlePressed;
    bool mouseRightPressed;
    glm::vec4 curr_quat;
    glm::vec4 prev_quat;
    glm::vec3 eye = {0.0f, 0.0f, 3.0f};
    glm::vec3 lookat = {0.0f, 0.0f, 0.0f};
    glm::vec3 up = {0.0f, 1.0f, 0.0f};

    typedef struct { GLuint vb; } GLBufferState;

    typedef struct {
      std::vector<GLuint> diffuseTex;  // for each primitive in mesh
    } GLMeshState;

    typedef struct {
      std::map<std::string, GLint> attribs;
      std::map<std::string, GLint> uniforms;
    } GLProgramState;

    typedef struct {
      GLuint vb;     // vertex buffer
      size_t count;  // byte count
    } GLCurvesState;

    std::map<int, GLBufferState> gBufferState;
    std::map<std::string, GLMeshState> gMeshState;
    std::map<int, GLCurvesState> gCurvesMesh;
    GLProgramState gGLProgramState;

    enum GBufferTextureType {
        GPosition = 0,
        GNormal,
        GAmbient,
        GDiffuse,
        GGlossyShininess,
        GDepth,
        GBufferTextureCount
    };

    GLuint m_textureSampler = 0; // Only one sampler object since we will use the same sampling parameters for all

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
