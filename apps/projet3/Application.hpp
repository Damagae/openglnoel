#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>

#include <glm/glm.hpp>

#include <tiny_gltf.h>

class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:
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

    const int m_nWindowWidth = 1280;
    const int m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_AssetsRootPath;

    tinygltf::Model m_Model;

    std::vector<GLuint> m_VAOs;
    std::vector<tinygltf::Primitive> m_Primitives;

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

    glmlv::ViewController m_viewController{ m_GLFWHandle.window(), 3.f };
    GLint m_uModelViewProjMatrixLocation;
    GLint m_uModelViewMatrixLocation;
    GLint m_uNormalMatrixLocation;

    GLint m_uDirectionalLightDirLocation;
    GLint m_uDirectionalLightIntensityLocation;

    GLint m_uPointLightPositionLocation;
    GLint m_uPointLightIntensityLocation;

    GLint m_uKdLocation;
    GLint m_uKdSamplerLocation;

    float m_DirLightPhiAngleDegrees = 90.f;
    float m_DirLightThetaAngleDegrees = 45.f;
    glm::vec3 m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
    glm::vec3 m_DirLightColor = glm::vec3(1, 1, 1);
    float m_DirLightIntensity = 1.f;

    glm::vec3 m_PointLightPosition = glm::vec3(0, 1, 0);
    glm::vec3 m_PointLightColor = glm::vec3(1, 1, 1);
    float m_PointLightIntensity = 5.f;

    glm::vec3 m_CubeKd = glm::vec3(1, 0, 0);
    glm::vec3 m_SphereKd = glm::vec3(0, 1, 0);
    glm::vec3 m_ModelKd = glm::vec3(0, 1, 0);
};
