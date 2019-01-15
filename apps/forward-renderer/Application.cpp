#include "Application.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

  // Cube
  // ------ VBO
  GLuint vboCube;
  glGenBuffers(1, &vboCube);
  glBindBuffer(GL_ARRAY_BUFFER, vboCube);
  glBufferStorage(GL_ARRAY_BUFFER, cube.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), cube.vertexBuffer.data(), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // ------ IBO
  GLuint iboCube;
  glGenBuffers(1, &iboCube);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCube);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, cube.indexBuffer.size() * sizeof(uint32_t), cube.indexBuffer.data(), 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // ------ VAO
  GLuint vaoCube;
  glGenVertexArrays(1, &vaoCube);
  glBindVertexArray(vaoCube);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCube);

  glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
  glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
  glEnableVertexAttribArray(VERTEX_ATTR_TEXCOORD);

  glBindBuffer(GL_ARRAY_BUFFER, vboCube);
  glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) 0);
  glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, normal));
  glVertexAttribPointer(VERTEX_ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, texCoords));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);


  // Sphere
  // ------ VBO
  GLuint vboSphere;
  glGenBuffers(1, &vboSphere);
  glBindBuffer(GL_ARRAY_BUFFER, vboSphere);
  glBufferData(GL_ARRAY_BUFFER, sphere.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), sphere.vertexBuffer.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // ------ IBO
  GLuint iboSphere;
  glGenBuffers(1, &iboSphere);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboSphere);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indexBuffer.size() * sizeof(uint32_t), sphere.indexBuffer.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // ------ VAO
  GLuint vaoSphere;
  glGenVertexArrays(1, &vaoSphere);
  glBindVertexArray(vaoSphere);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboSphere);

  glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
  glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
  glEnableVertexAttribArray(VERTEX_ATTR_TEXCOORD);

  glBindBuffer(GL_ARRAY_BUFFER, vboSphere);
  glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) 0);
  glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, normal));
  glVertexAttribPointer(VERTEX_ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, texCoords));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Depth Test
  glEnable(GL_DEPTH_TEST);

  // Program
  const auto applicationPath = glmlv::fs::path{ "/home/daphne/OpenGL/IMAC3/openglnoel-build/bin/forward-renderer" };
  const auto appName = applicationPath.stem().string();
  const auto shadersRootPath = applicationPath.parent_path() / "shaders";
  const auto pathToVS = shadersRootPath / appName / "forward.vs.glsl";
  const auto pathToFS = shadersRootPath / appName / "forward.fs.glsl";
  std::vector<std::experimental::filesystem::v1::__cxx11::path> shaders;
  shaders.push_back(pathToVS);
  shaders.push_back(pathToFS);
  const glmlv::GLProgram program = glmlv::compileProgram(shaders);

  // Récupérer les locations des variables uniform
  const auto ulMVPMatrix = program.getUniformLocation("uMVPMatrix");
  const auto ulMVMatrix = program.getUniformLocation("uMVMatrix");
  const auto ulNormalMatrix = program.getUniformLocation("uNormalMatrix");
  // --- liées aux lights
  const auto ulDirectionalLightDir = program.getUniformLocation("uDirectionalLightDir");
  const auto ulDirectionalLightIntensity = program.getUniformLocation("uDirectionalLightIntensity");
  const auto ulPointLightPosition = program.getUniformLocation("uPointLightPosition");
  const auto ulPointLightIntensity = program.getUniformLocation("uPointLightIntensity");
  const auto ulKd = program.getUniformLocation("uKd");

  program.use();

  // Matrices
  glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), 1.3f, 0.01f, 100.f); // MVPM
  glm::mat4 ViewMatrix = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); // MVM

  // Light
  glm::vec3 dlDir = {1,1,1};
  glm::vec3 dlInt = {1,1,1};
  glm::vec3 plPos = {1,1,1};
  glm::vec3 plInt = {1,1,1};
  glm::vec3 kd = {1,1,1};


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

    glm::mat4 cubeModelMatrix = glm::translate(glm::mat4(1), glm::vec3(-2, 0, 0));
    glm::mat4 sphereModelMatrix = glm::translate(glm::mat4(1), glm::vec3(2, 0, 0));
    // glm::mat4 cubeModelMatrix = glm::rotate(glm::translate(glm::mat4(1), glm::vec3(-2, 0, 0)), 0.2f * float(seconds), glm::vec3(0, 1, 0));
    // glm::mat4 sphereModelMatrix = glm::rotate(glm::translate(glm::mat4(1), glm::vec3(2, 0, 0)), 0.2f * float(seconds), glm::vec3(0, 1, 0));

    // --- Cube
    glUniformMatrix4fv(ulMVMatrix, 1, GL_FALSE, glm::value_ptr(ViewMatrix * cubeModelMatrix));
    glUniformMatrix4fv(ulMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * ViewMatrix * cubeModelMatrix));
    glUniformMatrix4fv(ulNormalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(ViewMatrix * cubeModelMatrix))));
    glUniform3f(ulDirectionalLightDir, dlDir.x,	dlDir.y,	dlDir.z);
    glUniform3f(ulDirectionalLightIntensity, dlInt.x,	dlInt.y,	dlInt.z);
    glUniform3f(ulPointLightPosition, plPos.x,	plPos.y,	plPos.z);
    glUniform3f(ulPointLightIntensity, plInt.x,	plInt.y,	plInt.z);
    glUniform3f(ulKd, kd.x,	kd.y,	kd.z);

    glBindVertexArray(vaoCube);
    glDrawElements(GL_TRIANGLES, cube.indexBuffer.size(), GL_UNSIGNED_INT, 0);

    // --- Sphere
    glUniformMatrix4fv(ulMVMatrix, 1, GL_FALSE, glm::value_ptr(ViewMatrix * sphereModelMatrix));
    glUniformMatrix4fv(ulMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * ViewMatrix * sphereModelMatrix));
    glUniformMatrix4fv(ulNormalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(ViewMatrix * sphereModelMatrix))));
    glUniform3f(ulDirectionalLightDir, dlDir.x,	dlDir.y,	dlDir.z);
    glUniform3f(ulDirectionalLightIntensity, dlInt.x,	dlInt.y,	dlInt.z);
    glUniform3f(ulPointLightPosition, plPos.x,	plPos.y,	plPos.z);
    glUniform3f(ulPointLightIntensity, plInt.x,	plInt.y,	plInt.z);
    glUniform3f(ulKd, kd.x,	kd.y,	kd.z);

    glBindVertexArray(vaoSphere);
    glDrawElements(GL_TRIANGLES, sphere.indexBuffer.size(), GL_UNSIGNED_INT, 0);

    // ----------------------------------------------

    // GUI code -------------------------------------
		glmlv::imguiNewFrame(); // Création de la fenêtre

        { // Fenêtre UI
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::SliderFloat3("Directional Light Direction", glm::value_ptr(dlDir), 0, 1);
            ImGui::SliderFloat3("Directional Light Intensity", glm::value_ptr(dlInt), 0, 1);
            ImGui::SliderFloat3("Point Light Position", glm::value_ptr(plPos), 0, 1);
            ImGui::SliderFloat3("Point Light Intensity", glm::value_ptr(plInt), 0, 1);
            ImGui::ColorPicker3("Diffuse Color", glm::value_ptr(kd));
            ImGui::End();
        }

		glmlv::imguiRenderFrame();

    // ---------------------------------------------
    glfwPollEvents(); // Poll for and process events

    auto ellapsedTime = glfwGetTime() - seconds;
    auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;

    // Interaction code ----------------------------
    if (!guiHasFocus) {
        // Put here code to handle user interactions
    }
    // ---------------------------------------------

		m_GLFWHandle.swapBuffers(); // Swap front and back buffers
  } // end of loop -------------------------------------------------------------

  // ---------------------------------------------------------------------------
  // FREE
  // ---------------------------------------------------------------------------
  glDeleteBuffers(1, &vboCube);
  glDeleteVertexArrays(1, &vaoCube);

  glDeleteBuffers(1, &vboSphere);
  glDeleteVertexArrays(1, &vaoSphere);

  return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" }

{
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

    // Put here initialization code
}
