#include "Application.hpp"

#include <iostream>
#include <unordered_set>
#include <algorithm>

#include <imgui.h>
#include <glmlv/Image2DRGBA.hpp>
#include <glmlv/scene_loading.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

int Application::run()
{
  // ---------------------------------------------------------------------------
  // INITIALIZATION
  // ---------------------------------------------------------------------------
  const glmlv::SimpleGeometry cube = glmlv::makeCube();
  const glmlv::SimpleGeometry sphere = glmlv::makeSphere(32); // Il faudra peut-être changer la valeur
  const GLuint VERTEX_ATTR_POSITION = 0;
  const GLuint VERTEX_ATTR_NORMAL = 1;
  const GLuint VERTEX_ATTR_TEXCOORD = 2;

  auto viewController = glmlv::ViewController(m_GLFWHandle.window(), 3.f);

  // PATH
  const auto applicationPath = glmlv::fs::path{ "/home/daphne/OpenGL/IMAC3/openglnoel-build/bin/forward-renderer" };
  const auto appName = applicationPath.stem().string();
  const auto shadersRootPath = applicationPath.parent_path() / "shaders";
  const auto assetsRootPath = applicationPath.parent_path() / "assets";
  const auto pathToVS = shadersRootPath / appName / "forward.vs.glsl";
  const auto pathToFS = shadersRootPath / appName / "forward.fs.glsl";

  // Scene
  glmlv::SceneData scene;
  glmlv::loadObjScene(assetsRootPath / appName / "models" / "sponza.obj", scene);
  const auto sceneSize = glm::length(scene.bboxMax - scene.bboxMin);
  viewController.setSpeed(sceneSize * 0.1f); // 10% de la scene parcouru par seconde

  std::cout << "# of shapes    : " << scene.shapeCount << std::endl;
  std::cout << "# of materials : " << scene.materials.size() << std::endl;
  std::cout << "# of vertex    : " << scene.vertexBuffer.size() << std::endl;
  std::cout << "# of triangles    : " << scene.indexBuffer.size() / 3 << std::endl;
  std::cerr << "bbox : " << scene.bboxMin << ", " << scene.bboxMax << std::endl;

  // ------ VBO
  GLuint vboScene;
  glGenBuffers(1, &vboScene);
  glBindBuffer(GL_ARRAY_BUFFER, vboScene);
  glBufferData(GL_ARRAY_BUFFER, scene.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), scene.vertexBuffer.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // ------ IBO
  GLuint iboScene;
  glGenBuffers(1, &iboScene);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboScene);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, scene.indexBuffer.size() * sizeof(uint32_t), scene.indexBuffer.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // ------ Init shape info
  std::vector<ShapeInfo> shapes;
  uint32_t indexOffset = 0;
  for (auto shapeID = 0; shapeID < scene.indexCountPerShape.size(); ++shapeID) {
    shapes.emplace_back();
    auto & shape = shapes.back();
    shape.indexCount = scene.indexCountPerShape[shapeID];
    shape.indexOffset = indexOffset;
    shape.materialID = scene.materialIDPerShape[shapeID];
    shape.localToWorldMatrix = scene.localToWorldMatrixPerShape[shapeID];
    indexOffset += shape.indexCount;
  }

  // ------ VAO
  GLuint vaoScene;
  glGenVertexArrays(1, &vaoScene);
  glBindVertexArray(vaoScene);

  glEnableVertexAttribArray(VERTEX_ATTR_POSITION); // 0
  glEnableVertexAttribArray(VERTEX_ATTR_NORMAL); // 1
  glEnableVertexAttribArray(VERTEX_ATTR_TEXCOORD); // 2

  glBindBuffer(GL_ARRAY_BUFFER, vboScene);

  glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, position));
  glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, normal));
  glVertexAttribPointer(VERTEX_ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, texCoords));

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboScene);

  glBindVertexArray(0);

  // TEXTURES
  // --- White Texture
  GLuint whiteTex;
  glGenTextures(1, &whiteTex);
  glBindTexture(GL_TEXTURE_2D, whiteTex);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, 1, 1);
  glm::vec4 white(1.f, 1.f, 1.f, 1.f);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, &white);
  glBindTexture(GL_TEXTURE_2D, 0);

  // --- Scene textures
  std::vector<GLuint> sceneTextures;
  for (const auto & texture : scene.textures) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, texture.width(), texture.height());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    sceneTextures.emplace_back(tex);
  }

  // --- Materials
  std::vector<glmlv::SceneData::PhongMaterial> sceneMaterials;
  for (const auto & material : scene.materials) {
      glmlv::SceneData::PhongMaterial newMaterial;
      newMaterial.Ka = material.Ka;
      newMaterial.Kd = material.Kd;
      newMaterial.Ks = material.Ks;
      newMaterial.shininess = material.shininess;
      newMaterial.KaTextureId = material.KaTextureId >= 0 ? sceneTextures[material.KaTextureId] : whiteTex;
      newMaterial.KdTextureId = material.KdTextureId >= 0 ? sceneTextures[material.KdTextureId] : whiteTex;
      newMaterial.KsTextureId = material.KsTextureId >= 0 ? sceneTextures[material.KsTextureId] : whiteTex;
      newMaterial.shininessTextureId = material.shininessTextureId >= 0 ? sceneTextures[material.shininessTextureId] : whiteTex;

      sceneMaterials.emplace_back(newMaterial);
  }

  glmlv::SceneData::PhongMaterial defaultMat;
  defaultMat.Ka = glm::vec3(0);
  defaultMat.Kd = glm::vec3(1);
  defaultMat.Ks = glm::vec3(1);
  defaultMat.shininess = 32.f;
  defaultMat.KaTextureId = whiteTex;
  defaultMat.KdTextureId = whiteTex;
  defaultMat.KsTextureId = whiteTex;
  defaultMat.shininessTextureId = whiteTex;

  // --- Samplers
  // Note: no need to bind a sampler for modifying it: the sampler API is already direct_state_access
  GLuint textureSampler = 0;
  glGenSamplers(1, &textureSampler);
  glSamplerParameteri(textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glSamplerParameteri(textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // DEPTH TEST
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  // PROGRAM
  std::vector<std::experimental::filesystem::v1::__cxx11::path> shaders;
  shaders.push_back(pathToVS);
  shaders.push_back(pathToFS);
  const glmlv::GLProgram program = glmlv::compileProgram(shaders);
  program.use();

  // VARIABLES UNIFORM
  // Récupérer les locations des variables uniform
  const auto ulMVPMatrix = program.getUniformLocation("uMVPMatrix");
  const auto ulMVMatrix = program.getUniformLocation("uMVMatrix");
  const auto ulNormalMatrix = program.getUniformLocation("uNormalMatrix");
  // --- liées aux lights
  const auto ulDirectionalLightDir = program.getUniformLocation("uDirectionalLightDir");
  const auto ulDirectionalLightIntensity = program.getUniformLocation("uDirectionalLightIntensity");
  const auto ulPointLightPosition = program.getUniformLocation("uPointLightPosition");
  const auto ulPointLightIntensity = program.getUniformLocation("uPointLightIntensity");
  // --- liées aux textures et matériaux
  const auto ulKa = program.getUniformLocation("uKa");
  const auto ulKd = program.getUniformLocation("uKd");
  const auto ulKs = program.getUniformLocation("uKs");
  const auto ulShininess = program.getUniformLocation("uShininess");

  const auto ulKaSampler = program.getUniformLocation("uKaSampler");
  const auto ulKdSampler = program.getUniformLocation("uKdSampler");
  const auto ulKsSampler = program.getUniformLocation("uKsSampler");
  const auto ulShininessSampler = program.getUniformLocation("uShininessSampler");

  // LIGHTS
  float dirLightPhiAngleDegrees = 90.f;
  float dirLightThetaAngleDegrees = 45.f;
  glm::vec3 dirLightDirection = computeDirectionVector(glm::radians(dirLightPhiAngleDegrees), glm::radians(dirLightThetaAngleDegrees));
  glm::vec3 dirLightColor = glm::vec3(1, 1, 1);
  float dirLightIntensity = 1.f;

  glm::vec3 pointLightPosition = glm::vec3(0, 1, 0);
  glm::vec3 pointLightColor = glm::vec3(1, 1, 1);
  float pointLightIntensity = 5.f;

  // ---------------------------------------------------------------------------
  // LOOP
  // ---------------------------------------------------------------------------
  // Loop until the user closes the window
  float clearColor[3] = { 0.5, 0.8, 0.2 };
  glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);

  for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
  {
    const auto seconds = glfwGetTime();

    // Rendering code -------------------------------
  	const auto fbSize = m_GLFWHandle.framebufferSize();
  	glViewport(0, 0, fbSize.x, fbSize.y);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Matrices
    const auto sceneProjMatrix = glm::perspective(70.f, float(fbSize.x) / fbSize.y, 0.01f * sceneSize, sceneSize); // near = 1% de la taille de la scene, far = 100
    const auto viewMatrix = viewController.getViewMatrix();

    // Directional Light
    glUniform3fv(ulDirectionalLightDir, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(dirLightDirection), 0))));
    glUniform3fv(ulDirectionalLightIntensity, 1, glm::value_ptr(dirLightColor * dirLightIntensity));

    // Point light
    glUniform3fv(ulPointLightPosition, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(pointLightPosition, 1))));
    glUniform3fv(ulPointLightIntensity, 1, glm::value_ptr(pointLightColor * pointLightIntensity));

    // Samplers
    for (GLuint i : {0, 1, 2, 3}) {
      glBindSampler(i, textureSampler);
    }

    // Set texture unit of each sampler
    glUniform1i(ulKaSampler, 0);
    glUniform1i(ulKdSampler, 1);
    glUniform1i(ulKsSampler, 2);
    glUniform1i(ulShininessSampler, 3);

    // Bind VAO
    glBindVertexArray(vaoScene);

    // Materials
    const glmlv::SceneData::PhongMaterial * currentMaterial = nullptr;

    // Drawing
    // For each shape
    for (const auto shape: shapes) {
      // Récupération du material
       const auto & material = shape.materialID >= 0 ? sceneMaterials[shape.materialID] : defaultMat;

      // On bind si c'est pas le même material que précédement
      if (currentMaterial != &material) {
        glUniform3fv(ulKa, 1, glm::value_ptr(material.Ka));
        glUniform3fv(ulKd, 1, glm::value_ptr(material.Kd));
        glUniform3fv(ulKs, 1, glm::value_ptr(material.Ks));
        glUniform1fv(ulShininess, 1, &material.shininess);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.KaTextureId);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, material.KdTextureId);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, material.KsTextureId);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, material.shininessTextureId);
        currentMaterial = &material;
      }

      // Matrices
      const auto modelMatrix = shape.localToWorldMatrix;

			const auto mvMatrix = viewMatrix * modelMatrix;
			const auto mvpMatrix = sceneProjMatrix * mvMatrix;
			const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));

      glUniformMatrix4fv(ulMVMatrix, 1, GL_FALSE, glm::value_ptr(mvMatrix));
      glUniformMatrix4fv(ulMVPMatrix, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
      glUniformMatrix4fv(ulNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));

      glDrawElements(GL_TRIANGLES, shape.indexCount, GL_UNSIGNED_INT, (const GLvoid*) (shape.indexOffset * sizeof(GLuint)));
    }

    // Unbind samplers
    for (GLuint i : {0, 1, 2, 3}) {
      glBindSampler(i, 0);
    }

    // ----------------------------------------------

    // GUI code -------------------------------------
		glmlv::imguiNewFrame(); // Création de la fenêtre

        { // Fenêtre UI
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            if (ImGui::ColorEdit3("clearColor", clearColor)) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }
            if (ImGui::Button("Sort shapes wrt materialID")) {
                std::sort(begin(shapes), end(shapes), [&](auto lhs, auto rhs) {
                    return lhs.materialID < rhs.materialID;
                });
            }
            if (ImGui::CollapsingHeader("Directional Light")) {
                ImGui::ColorEdit3("DirLightColor", glm::value_ptr(dirLightColor));
                ImGui::DragFloat("DirLightIntensity", &dirLightIntensity, 0.1f, 0.f, 100.f);
                if (ImGui::DragFloat("Phi Angle", &dirLightPhiAngleDegrees, 1.0f, 0.0f, 360.f) ||
                    ImGui::DragFloat("Theta Angle", &dirLightThetaAngleDegrees, 1.0f, 0.0f, 180.f)) {
                    dirLightDirection = computeDirectionVector(glm::radians(dirLightPhiAngleDegrees), glm::radians(dirLightThetaAngleDegrees));
                }
            }

            if (ImGui::CollapsingHeader("Point Light")) {
                ImGui::ColorEdit3("PointLightColor", glm::value_ptr(pointLightColor));
                ImGui::DragFloat("PointLightIntensity", &pointLightIntensity, 0.1f, 0.f, 16000.f);
                ImGui::InputFloat3("Position", glm::value_ptr(pointLightPosition));
            }

            ImGui::End();
        }

		glmlv::imguiRenderFrame();

    // ---------------------------------------------
    glfwPollEvents(); // Poll for and process events

    m_GLFWHandle.swapBuffers(); // Swap front and back buffers

    auto ellapsedTime = glfwGetTime() - seconds;
    auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;

    // Interaction code ----------------------------
    if (!guiHasFocus) {
        // Put here code to handle user interactions
        viewController.update(float(ellapsedTime));
    }
    // ---------------------------------------------

  } // end of loop -------------------------------------------------------------

  // ---------------------------------------------------------------------------
  // FREE
  // ---------------------------------------------------------------------------
    glDeleteBuffers(1, &vboScene);
    glDeleteVertexArrays(1, &vaoScene);

  return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" }

{
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

}
