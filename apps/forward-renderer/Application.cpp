#include "Application.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int Application::run()
{
  // ---------------------------------------------------------------------------
  // INIT
  // ---------------------------------------------------------------------------
  glmlv::SimpleGeometry cube = glmlv::makeCube();
  glmlv::SimpleGeometry sphere = glmlv::makeSphere(2); // Il faudra peut-être changer la valeur
  const GLuint VERTEX_ATTR_POSITION = 0;
  const GLuint VERTEX_ATTR_NORMAL = 1;
  const GLuint VERTEX_ATTR_TEXCOORD = 2;

  // Cube
  // ------ VBO
  GLuint vboCube;
  glGenBuffers(1, &vboCube);
  // Binding du VBO sur la cible GL_ARRAY_BUFFER:
  glBindBuffer(GL_ARRAY_BUFFER, vboCube);
  glBufferData(GL_ARRAY_BUFFER, cube.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), cube.vertexBuffer.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // ------ IBO
  GLuint iboCube;
  glGenBuffers(1, &iboCube);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboCube);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube.indexBuffer.size() * sizeof(uint32_t), cube.indexBuffer.data(), GL_STATIC_DRAW);
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
  // Binding du VBO sur la cible GL_ARRAY_BUFFER:
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

  // ---------------------------------------------------------------------------
  // LOOP
  // ---------------------------------------------------------------------------
  // Loop until the user closes the window
  for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
  {
    const auto seconds = glfwGetTime();

    // Rendering code -------------------------------
  	const auto fbSize = m_GLFWHandle.framebufferSize();
  	glViewport(0, 0, fbSize.x, fbSize.y);
  	glClear(GL_COLOR_BUFFER_BIT);
    // ----------------------------------------------

    // GUI code -------------------------------------
		glmlv::imguiNewFrame(); // Création de la fenêtre

        { // Pq il y a des accolades ?
            // Fenêtre UI
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
