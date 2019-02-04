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
  const GLuint VERTEX_ATTR_POSITION = 0;
  const GLuint VERTEX_ATTR_NORMAL = 1;
  const GLuint VERTEX_ATTR_TEXCOORD = 2;

  auto viewController = glmlv::ViewController(m_GLFWHandle.window(), 3.f);

  // PATH
  const auto shadersRootPath = m_AppPath.parent_path() / "shaders";
  const auto assetsRootPath = m_AppPath.parent_path() / "assets";

  // Scene
  glmlv::SceneData scene;
  glmlv::loadObjScene(assetsRootPath / m_AppName / "models" / "sponza.obj", scene);
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
    ++scene.shapeCount;
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
  // --- Buffer Textures
  enum GBufferTextureType {
      GPosition = 0,
      GNormal, // 1
      GAmbient, // 2
      GDiffuse, // 3
      GGlossyShininess, // 4
      GDepth, // 5 // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
      GBufferTextureCount // 6
  };
  GLuint gBufferTextures[GBufferTextureCount];
  const GLenum gBufferTextureFormat[GBufferTextureCount] = { GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F };
  glGenTextures(GBufferTextureCount, gBufferTextures);
  for (auto i = 0; i < GBufferTextureCount; ++i) {
    glBindTexture(GL_TEXTURE_2D, gBufferTextures[i]);
    glTexStorage2D(GL_TEXTURE_2D, 1, gBufferTextureFormat[i], m_nWindowWidth, m_nWindowHeight);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  // --- Frame Buffer Object
  GLuint fbo;
  const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)fbo);
  for (auto i = 0; i < GBufferTextureCount - 1; ++i) {
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, gBufferTextures[i], 0);
  }
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gBufferTextures[5], 0); // GDepth
  glDrawBuffers(5, drawBuffers);
  const auto statusFBO = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
  if (statusFBO != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "FrameBuffer failed - statusFBO : " << statusFBO << std::endl;
  } else {
    std::cout << "FBO OK" << std::endl;
  }
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  // SHADDOW MAPPING -----------------------------------------------------------
  // --- Textures
  GLuint directionalSMTexture; // identifiant de texture OpenGL qui contiend la depth map selon le point de vue de la light
  GLuint directionalSMFBO; // identifiant de framebuffer OpenGL pour dessiner la depth map
  GLuint directionalSMSampler; // identifiant de sampler OpenGL pour lire la depth map depuis un shader
  int32_t nDirectionalSMResolution = 512; // résolution de la depth map

  glGenTextures(1, &directionalSMTexture);
  glBindTexture(GL_TEXTURE_2D, directionalSMTexture);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, 1, 1);
  glBindTexture(GL_TEXTURE_2D, 0);

  // --- FBO
  glGenFramebuffers(1, &directionalSMFBO);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)directionalSMFBO);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, directionalSMTexture, 0);
  glDrawBuffers(5, drawBuffers);
  const auto statusSMFBO = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
  if (statusSMFBO != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "FrameBuffer failed - statusSMFBO : " << statusSMFBO << std::endl;
  } else {
    std::cout << "SMFBO OK" << std::endl;
  }

  // --- Samplers
  glGenSamplers(1, &directionalSMSampler);
  glSamplerParameteri(directionalSMSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glSamplerParameteri(directionalSMSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glSamplerParameteri(directionalSMSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glSamplerParameteri(directionalSMSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


  // TRIANGLE -----------------------------------------------------------------
    GLuint vboTriangle;
    glGenBuffers(1, &vboTriangle);
    glBindBuffer(GL_ARRAY_BUFFER, vboTriangle);

    GLfloat dataTriangle[] = { -1, -1, 3, -1, -1, 3 };
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(dataTriangle), dataTriangle, 0);

    GLuint vaoTriangle;
    glGenVertexArrays(1, &vaoTriangle);
    glBindVertexArray(vaoTriangle);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

  // TEXTURES ------------------------------------------------------------------
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

  // --- Default Material
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

  // DEPTH TEST & MULTISAMPLE --------------------------------------------------
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  // PROGRAMS -------------------------------------------------------------------
  const auto pathToVSGeometry = shadersRootPath / m_AppName / "geometryPass.vs.glsl";
  const auto pathToFSGeometry = shadersRootPath / m_AppName / "geometryPass.fs.glsl";
  std::vector<std::experimental::filesystem::v1::__cxx11::path> shadersGeometry;
  shadersGeometry.push_back(pathToVSGeometry);
  shadersGeometry.push_back(pathToFSGeometry);
  const glmlv::GLProgram programGeometry = glmlv::compileProgram(shadersGeometry);

  const auto pathToVSShading = shadersRootPath / m_AppName / "shadingPass.vs.glsl";
  const auto pathToFSShading = shadersRootPath / m_AppName / "shadingPass.fs.glsl";
  std::vector<std::experimental::filesystem::v1::__cxx11::path> shadersShading;
  shadersShading.push_back(pathToVSShading);
  shadersShading.push_back(pathToFSShading);
  const glmlv::GLProgram programShading = glmlv::compileProgram(shadersShading);

  const auto pathToVSDirSM = shadersRootPath / m_AppName / "shadingPass.vs.glsl";
  const auto pathToFSDirSM = shadersRootPath / m_AppName / "shadingPass.fs.glsl";
  std::vector<std::experimental::filesystem::v1::__cxx11::path> shadersDirSM;
  shadersDirSM.push_back(pathToVSDirSM);
  shadersDirSM.push_back(pathToFSDirSM);
  const glmlv::GLProgram programDirSM = glmlv::compileProgram(shadersDirSM);

  // VARIABLES UNIFORMES -------------------------------------------------------
  // Récupérer les locations des variables uniform
  const auto ulMVPMatrix = programGeometry.getUniformLocation("uMVPMatrix");
  const auto ulMVMatrix = programGeometry.getUniformLocation("uMVMatrix");
  const auto ulNormalMatrix = programGeometry.getUniformLocation("uNormalMatrix");
  // --- liées aux lights
  const auto ulDirectionalLightDir = programShading.getUniformLocation("uDirectionalLightDir");
  const auto ulDirectionalLightIntensity = programShading.getUniformLocation("uDirectionalLightIntensity");
  const auto ulPointLightPosition = programShading.getUniformLocation("uPointLightPosition");
  const auto ulPointLightIntensity = programShading.getUniformLocation("uPointLightIntensity");

  const auto ulDirLightViewProjMatrixSP = programShading.getUniformLocation("uDirLightViewProjMatrix");
  const auto ulDirLightShadowMap = programShading.getUniformLocation("uDirLightShadowMap");
  const auto ulDirLightShadowMapBias = programShading.getUniformLocation("uDirLightShadowMapBias");

  // --- liées aux textures et matériaux
  const auto ulKa = programGeometry.getUniformLocation("uKa");
  const auto ulKd = programGeometry.getUniformLocation("uKd");
  const auto ulKs = programGeometry.getUniformLocation("uKs");
  const auto ulShininess = programGeometry.getUniformLocation("uShininess");
  const auto ulKaSampler = programGeometry.getUniformLocation("uKaSampler");
  const auto ulKdSampler = programGeometry.getUniformLocation("uKdSampler");
  const auto ulKsSampler = programGeometry.getUniformLocation("uKsSampler");
  const auto ulShininessSampler = programGeometry.getUniformLocation("uShininessSampler");
  // --- Gemoetry Buffer Samplers
  GLint ulGBufferSamplers[GDepth];
  ulGBufferSamplers[GPosition] = programShading.getUniformLocation("uGPosition");
  ulGBufferSamplers[GNormal] = programShading.getUniformLocation("uGNormal");
  ulGBufferSamplers[GAmbient] = programShading.getUniformLocation("uGAmbient");
  ulGBufferSamplers[GDiffuse] = programShading.getUniformLocation("uGDiffuse");
  ulGBufferSamplers[GGlossyShininess] = programShading.getUniformLocation("uGGlossyShininess");
  // --- Dir Shadow Mapping
  const auto ulDirLightViewProjMatrix = programDirSM.getUniformLocation("uDirLightViewProjMatrix");


  // LIGHTS --------------------------------------------------------------------
  float dirLightPhiAngleDegrees = 90.f;
  float dirLightThetaAngleDegrees = 45.f;
  glm::vec3 dirLightDirection = computeDirectionVector(glm::radians(dirLightPhiAngleDegrees), glm::radians(dirLightThetaAngleDegrees));
  glm::vec3 dirLightColor = glm::vec3(1, 1, 1);
  float dirLightIntensity = 1.f;

  glm::vec3 pointLightPosition = glm::vec3(0, 1, 0);
  glm::vec3 pointLightColor = glm::vec3(1, 1, 1);
  float pointLightIntensity = 5.f;

  // DISPLAY MODE --------------------------------------------------------------
  // GBufferTextureType currentlyDisplayed = GDiffuse;
  GBufferTextureType currentlyDisplayed = GBufferTextureCount;

  // ACTIVATION DIRSM ----------------------------------------------------------
   bool directionalSMDirty = true;

  // ---------------------------------------------------------------------------
  // LOOP
  // ---------------------------------------------------------------------------
  // Loop until the user closes the window
  float clearColor[3] = { 0, 0.7, 0.7 };
  glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);

  for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
  {

    static const auto computeDirectionVectorUp = [](float phiRadians, float thetaRadians) {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto cosTheta = glm::cos(thetaRadians);
        return -glm::normalize(glm::vec3(sinPhi * cosTheta, -glm::sin(thetaRadians), cosPhi * cosTheta));
    };

    const auto sceneCenter = 0.5f * (scene.bboxMax + scene.bboxMin);
    const float sceneRadius = sceneSize * 0.5f;

    const auto dirLightUpVector = computeDirectionVectorUp(glm::radians(dirLightPhiAngleDegrees), glm::radians(dirLightThetaAngleDegrees));
    const auto dirLightViewMatrix = glm::lookAt(sceneCenter + dirLightDirection * sceneRadius, sceneCenter, dirLightUpVector); // Will not work if DirLightDirection is colinear to lightUpVector
    const auto dirLightProjMatrix = glm::ortho(-sceneRadius, sceneRadius, -sceneRadius, sceneRadius, 0.01f * sceneRadius, 2.f * sceneRadius);

    if (directionalSMDirty) {
      // Calcul de la shadow map (**)

      programDirSM.use();

      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, directionalSMFBO);
      glViewport(0, 0, nDirectionalSMResolution, nDirectionalSMResolution);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUniformMatrix4fv(ulDirLightViewProjMatrix, 1, GL_FALSE, glm::value_ptr(dirLightProjMatrix * dirLightViewMatrix));

      glBindVertexArray(vaoScene);

      // We draw each shape by specifying how much indices it carries, and with an offset in the global index buffer
      for (const auto shape : shapes) {
          glDrawElements(GL_TRIANGLES, shape.indexCount, GL_UNSIGNED_INT, (const GLvoid*)(shape.indexOffset * sizeof(GLuint)));
      }

      glBindVertexArray(0);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

      directionalSMDirty = false; // Pas de calcul au prochain tour
    }

    // Rendering code -------------------------------
    const auto seconds = glfwGetTime();

    const auto fbSize = m_GLFWHandle.framebufferSize();

    // Matrices View et Projection
    const auto sceneProjMatrix = glm::perspective(70.f, float(fbSize.x) / fbSize.y, 0.01f * sceneSize, sceneSize); // near = 1% de la taille de la scene, far = 100
    const auto viewMatrix = viewController.getViewMatrix();

    programGeometry.use();

    // Bind FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo); // PAS TOUCHER

  	glViewport(0, 0, fbSize.x, fbSize.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    // Drawing -----------------------------------------------------------------
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

      // Matrices locales
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

    // Unbind VAO
    glBindVertexArray(0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Unbind FBO

    // ----------------------------------------------

    // GUI code -------------------------------------
    const auto viewportSize = m_GLFWHandle.framebufferSize();
    glViewport(0, 0, viewportSize.x, viewportSize.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (currentlyDisplayed == GBufferTextureCount)
        {
            // Shading pass
            {
              programShading.use();

              // Uniform des lights
              glUniform3fv(ulDirectionalLightDir, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(dirLightDirection), 0))));
              glUniform3fv(ulDirectionalLightIntensity, 1, glm::value_ptr(dirLightColor * dirLightIntensity));

              glUniform3fv(ulPointLightPosition, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(pointLightPosition, 1))));
              glUniform3fv(ulPointLightIntensity, 1, glm::value_ptr(pointLightColor * pointLightIntensity));

              for (int32_t i = GPosition; i < GDepth; ++i) { // 0 à 4
                  glActiveTexture(GL_TEXTURE0 + i);
                  glBindTexture(GL_TEXTURE_2D, gBufferTextures[i]);
                  glUniform1i(ulGBufferSamplers[i], i);
            }

            // Draw triangle
            glBindVertexArray(vaoTriangle);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }

      } // else {
    //   glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    //   glReadBuffer(GL_COLOR_ATTACHMENT0 + currentlyDisplayed);
    //   glBlitFramebuffer(0, 0, m_nWindowWidth, m_nWindowHeight, 0, 0, m_nWindowWidth, m_nWindowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    //   glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    // }

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

            // // Pseudo code dans le dessin de la GUI:
            // if (directional_light_change) {
            //  directionalSMDirty = true; // Il faut recalculer la shadow map
            // }

            // if (ImGui::CollapsingHeader("GBuffer")) {
            //     for (int32_t i = GPosition; i < GDepth; ++i)
            //     {
            //         const char* gBufferTexNames[GBufferTextureCount] = {"Position", "Normal", "Ambient", "Diffuse", "GlossyShininess", "Depth"};
            //         if (ImGui::RadioButton(gBufferTexNames[i], currentlyDisplayed == i)) {
            //           currentlyDisplayed = GBufferTextureType(i);
            //         }
            //     }
            // }

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
