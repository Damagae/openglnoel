#include "Application.hpp"

#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <stack>
#include <unordered_map>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

int Application::run()
{
	// Put here code to run before rendering loop

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        // Put here rendering code
		const auto fbSize = m_GLFWHandle.framebufferSize();
		glViewport(0, 0, fbSize.x, fbSize.y);
		glClear(GL_COLOR_BUFFER_BIT);

        // GUI code:
		glmlv::imguiNewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

		glmlv::imguiRenderFrame();

        glfwPollEvents(); // Poll for and process events

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            // Put here code to handle user interactions
        }

		m_GLFWHandle.swapBuffers(); // Swap front and back buffers
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" }

{

    if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << "<path to model>" << std::endl;
      exit(-1);
    }

    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

    initScene(glmlv::fs::path{argv[1]});
    initShadersData();

    SetupMeshState();
}

void Application::initScene(const glmlv::fs::path & objPath) {

  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  std::string ext = GetFilePathExtension(objPath);

  bool ret = false;
  if (ext.compare("glb") == 0) {
    // assume binary glTF.
    ret = loader.LoadBinaryFromFile(&m_Model, &err, &warn, objPath.c_str());
  } else {
    // assume ascii glTF.
    ret = loader.LoadASCIIFromFile(&m_Model, &err, &warn, objPath.c_str());
  }

  if (!warn.empty()) {
    std::cerr << "Warn: " << warn.c_str() << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err.c_str() << std::endl;
  }
  if (!ret) {
    std::cerr << "Failed to load .glTF : " << objPath << std::endl;
    exit(-1);
  }

  // DBG
  PrintNodes(m_Model.scenes[m_Model.defaultScene > -1 ? m_Model.defaultScene : 0]);
}

void Application::initShadersData() {
  m_geometryPassProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "geometryPass.vs.glsl", m_ShadersRootPath / m_AppName / "geometryPass.fs.glsl" });

  m_uModelViewProjMatrixLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uModelViewProjMatrix");
  m_uModelViewMatrixLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uModelViewMatrix");
  m_uNormalMatrixLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uNormalMatrix");

  m_uKaLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uKa");
  m_uKdLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uKd");
  m_uKsLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uKs");
  m_uShininessLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uShininess");
  m_uKaSamplerLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uKaSampler");
  m_uKdSamplerLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uKdSampler");
  m_uKsSamplerLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uKsSampler");
  m_uShininessSamplerLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uShininessSampler");

  m_shadingPassProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "shadingPass.fs.glsl" });

  m_uGBufferSamplerLocations[GPosition] = glGetUniformLocation(m_shadingPassProgram.glId(), "uGPosition");
  m_uGBufferSamplerLocations[GNormal] = glGetUniformLocation(m_shadingPassProgram.glId(), "uGNormal");
  m_uGBufferSamplerLocations[GAmbient] = glGetUniformLocation(m_shadingPassProgram.glId(), "uGAmbient");
  m_uGBufferSamplerLocations[GDiffuse] = glGetUniformLocation(m_shadingPassProgram.glId(), "uGDiffuse");
  m_uGBufferSamplerLocations[GGlossyShininess] = glGetUniformLocation(m_shadingPassProgram.glId(), "uGGlossyShininess");

  m_uDirectionalLightDirLocation = glGetUniformLocation(m_shadingPassProgram.glId(), "uDirectionalLightDir");
  m_uDirectionalLightIntensityLocation = glGetUniformLocation(m_shadingPassProgram.glId(), "uDirectionalLightIntensity");

  m_uPointLightPositionLocation = glGetUniformLocation(m_shadingPassProgram.glId(), "uPointLightPosition");
  m_uPointLightIntensityLocation = glGetUniformLocation(m_shadingPassProgram.glId(), "uPointLightIntensity");

  m_displayDepthProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "displayDepth.fs.glsl" });

  m_uGDepthSamplerLocation = glGetUniformLocation(m_displayDepthProgram.glId(), "uGDepth");

  m_displayPositionProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "displayPosition.fs.glsl" });

  m_uGPositionSamplerLocation = glGetUniformLocation(m_displayPositionProgram.glId(), "uGPosition");
  m_uSceneSizeLocation = glGetUniformLocation(m_displayPositionProgram.glId(), "uSceneSize");
}

std::string Application::GetFilePathExtension(const std::string &FileName) {
  if (FileName.find_last_of(".") != std::string::npos)
    return FileName.substr(FileName.find_last_of(".") + 1);
  return "";
}

void Application::PrintNodes(const tinygltf::Scene &scene) {
  for (size_t i = 0; i < scene.nodes.size(); i++) {
    std::cout << "node.name : " << scene.nodes[i] << std::endl;
  }
}

void Application::SetupMeshState() {
  // Buffer
  {
    for (size_t i = 0; i < m_Model.bufferViews.size(); i++) {
      const tinygltf::BufferView &bufferView = m_Model.bufferViews[i];
      if (bufferView.target == 0) {
        std::cout << "WARN: bufferView.target is zero" << std::endl;
        continue;  // Unsupported bufferView.
      }

      const tinygltf::Buffer &buffer = m_Model.buffers[bufferView.buffer];
      GLBufferState state;
      glGenBuffers(1, &state.vb);
      glBindBuffer(bufferView.target, state.vb);
      std::cout << "buffer.size= " << buffer.data.size()
                << ", byteOffset = " << bufferView.byteOffset << std::endl;
      glBufferData(bufferView.target, bufferView.byteLength,
                   &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
      glBindBuffer(bufferView.target, 0);

      gBufferState[i] = state;
    }
  }

  // Texture
  {
    for (size_t i = 0; i < m_Model.meshes.size(); i++) {
      const tinygltf::Mesh &mesh = m_Model.meshes[i];

      gMeshState[mesh.name].diffuseTex.resize(mesh.primitives.size());
      for (size_t primId = 0; primId < mesh.primitives.size(); primId++) {
        const tinygltf::Primitive &primitive = mesh.primitives[primId];

        gMeshState[mesh.name].diffuseTex[primId] = 0;

        if (primitive.material < 0) {
          continue;
        }
        tinygltf::Material &mat = m_Model.materials[primitive.material];
        std::cout << "material.name = " << mat.name.c_str() << std::endl;
        if (mat.values.find("diffuse") != mat.values.end()) {
          std::string diffuseTexName = mat.values["diffuse"].string_value;
          if (/*std::find(std::begin(m_Model.textures), std::end(m_Model.textures), diffuseTexName)
              != std::end(m_Model.textures)*/ 1) {
            tinygltf::Texture &tex = std::find(std::begin(m_Model.textures), std::end(m_Model.textures), diffuseTexName).value;
            if (scene.images.find(tex.source) != m_Model.images.end()) {
              tinygltf::Image &image = m_Model.images[tex.source];
              GLuint texId;
              glGenTextures(1, &texId);
              glBindTexture(tex.target, texId);
              glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
              glTexParameterf(tex.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
              glTexParameterf(tex.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

              // Ignore Texture.fomat.
              GLenum format = GL_RGBA;
              if (image.component == 3) {
                format = GL_RGB;
              }
              glTexImage2D(tex.target, 0, tex.internalFormat, image.width,
                           image.height, 0, format, tex.type,
                           &image.image.at(0));

              glBindTexture(tex.target, 0);

              std::cout << "TexId = " << texId << std::endl;
              gMeshState[mesh.name].diffuseTex[primId] = texId;
            }
          }
        }
      }
    }
  }

  // glUseProgram(progId);
  // GLint vtloc = glGetAttribLocation(progId, "in_vertex");
  // GLint nrmloc = glGetAttribLocation(progId, "in_normal");
  // GLint uvloc = glGetAttribLocation(progId, "in_texcoord");
  //
  // // GLint diffuseTexLoc = glGetUniformLocation(progId, "diffuseTex");
  // GLint isCurvesLoc = glGetUniformLocation(progId, "uIsCurves");
  //
  // gGLProgramState.attribs["POSITION"] = vtloc;
  // gGLProgramState.attribs["NORMAL"] = nrmloc;
  // gGLProgramState.attribs["TEXCOORD_0"] = uvloc;
  // // gGLProgramState.uniforms["diffuseTex"] = diffuseTexLoc;
  // gGLProgramState.uniforms["isCurvesLoc"] = isCurvesLoc;
};
