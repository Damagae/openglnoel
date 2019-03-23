#include "Application.hpp"

#include <iostream>

#include <glmlv/Image2DRGBA.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

int Application::run()
{
	float clearColor[3] = { 0.5, 0.8, 0.2 };
	glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto projMatrix = glm::perspective(70.f, float(viewportSize.x) / viewportSize.y, 0.01f, 100.f);
        const auto viewMatrix = m_viewController.getViewMatrix();

        glUniform3fv(m_uDirectionalLightDirLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(m_DirLightDirection), 0))));
        glUniform3fv(m_uDirectionalLightIntensityLocation, 1, glm::value_ptr(m_DirLightColor * m_DirLightIntensity));

        glUniform3fv(m_uPointLightPositionLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(m_PointLightPosition, 1))));
        glUniform3fv(m_uPointLightIntensityLocation, 1, glm::value_ptr(m_PointLightColor * m_PointLightIntensity));

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(m_uKdSamplerLocation, 0); // Set the uniform to 0 because we use texture unit 0
        glBindSampler(0, m_textureSampler); // Tell to OpenGL what sampler we want to use on this texture unit

        // {
        //     const auto modelMatrix = glm::rotate(glm::translate(glm::mat4(1), glm::vec3(-2, 0, 0)), 0.2f * float(seconds), glm::vec3(0, 1, 0));
        //
        //     const auto mvMatrix = viewMatrix * modelMatrix;
        //     const auto mvpMatrix = projMatrix * mvMatrix;
        //     const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));
        //
        //     glUniformMatrix4fv(m_uModelViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
        //     glUniformMatrix4fv(m_uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvMatrix));
        //     glUniformMatrix4fv(m_uNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        //
        //     glUniform3fv(m_uKdLocation, 1, glm::value_ptr(m_CubeKd));
        //
        //     glBindTexture(GL_TEXTURE_2D, m_cubeTextureKd);
        //
        //     glBindVertexArray(m_cubeVAO);
        //     glDrawElements(GL_TRIANGLES, m_cubeGeometry.indexBuffer.size(), GL_UNSIGNED_INT, nullptr);
        // }
        //
        // {
        //     const auto modelMatrix = glm::rotate(glm::translate(glm::mat4(1), glm::vec3(2, 0, 0)), 0.2f * float(seconds), glm::vec3(0, 1, 0));
        //
        //     const auto mvMatrix = viewMatrix * modelMatrix;
        //     const auto mvpMatrix = projMatrix * mvMatrix;
        //     const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));
        //
        //     glUniformMatrix4fv(m_uModelViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
        //     glUniformMatrix4fv(m_uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvMatrix));
        //     glUniformMatrix4fv(m_uNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        //
        //     glUniform3fv(m_uKdLocation, 1, glm::value_ptr(m_SphereKd));
        //
        //     glBindTexture(GL_TEXTURE_2D, m_sphereTextureKd);
        //
        //     glBindVertexArray(m_sphereVAO);
        //     glDrawElements(GL_TRIANGLES, m_sphereGeometry.indexBuffer.size(), GL_UNSIGNED_INT, nullptr);
        // }

        {
          // const auto modelMatrix = glm::scale(
					// 															glm::rotate(
					// 																	glm::translate(
					// 																			glm::mat4(1), glm::vec3(2, 0, 0)),
					// 																	0.2f /** float(seconds)*/, glm::vec3(0, 1, 0)), glm::vec3(0.02f, 0.02f, 0.02f));
          // const auto modelMatrix = glm::scale(glm::rotate(glm::translate(glm::mat4(1), glm::vec3(2, 0, 0)), 0.2f /** float(seconds)*/, glm::vec3(0, 1, 0)), glm::vec3(100, 100, 100));
          // const auto modelMatrix = glm::rotate(glm::translate(glm::mat4(1), glm::vec3(2, 0, 0)), 0.2f * float(seconds), glm::vec3(0, 1, 0));

          glUniform3fv(m_uKdLocation, 1, glm::value_ptr(m_ModelKd));

          glBindTexture(GL_TEXTURE_2D, m_cubeTextureKd);

          for (uint vaoIndex = 0; vaoIndex < m_VAOs.size(); ++vaoIndex) {

						auto vao = m_VAOs[vaoIndex];
            tinygltf::Primitive primitive = m_Primitives[vaoIndex];

						auto modelMatrix = m_ModelMatrices[vaoIndex];

						const auto mvMatrix = viewMatrix * modelMatrix;
	          const auto mvpMatrix = projMatrix * mvMatrix;
	          const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));

	          glUniformMatrix4fv(m_uModelViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
	          glUniformMatrix4fv(m_uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvMatrix));
	          glUniformMatrix4fv(m_uNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));



            glBindVertexArray(vao);
            tinygltf::Accessor indices = m_Model.accessors[primitive.indices];

            int mode = getMode(primitive.mode);

						mode = GL_PATCHES;

            glDrawElements(mode, indices.count, indices.componentType, (const void*) indices.byteOffset);
          }
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindSampler(0, 0); // Unbind the sampler

        // GUI code:
		glmlv::imguiNewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            if (ImGui::ColorEdit3("clearColor", clearColor)) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }
            if (ImGui::CollapsingHeader("Directional Light"))
            {
                ImGui::ColorEdit3("DirLightColor", glm::value_ptr(m_DirLightColor));
                ImGui::DragFloat("DirLightIntensity", &m_DirLightIntensity, 0.1f, 0.f, 100.f);
                if (ImGui::DragFloat("Phi Angle", &m_DirLightPhiAngleDegrees, 1.0f, 0.0f, 360.f) ||
                    ImGui::DragFloat("Theta Angle", &m_DirLightThetaAngleDegrees, 1.0f, 0.0f, 180.f)) {
                    m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
                }
            }

            if (ImGui::CollapsingHeader("Point Light"))
            {
                ImGui::ColorEdit3("PointLightColor", glm::value_ptr(m_PointLightColor));
                ImGui::DragFloat("PointLightIntensity", &m_PointLightIntensity, 0.1f, 0.f, 16000.f);
                ImGui::InputFloat3("Position", glm::value_ptr(m_PointLightPosition));
            }

            if (ImGui::CollapsingHeader("Materials"))
            {
                ImGui::ColorEdit3("Model Kd", glm::value_ptr(m_ModelKd));
            }

            ImGui::End();
        }

		glmlv::imguiRenderFrame();

        /* Poll for and process events */
        glfwPollEvents();

        /* Swap front and back buffers*/
        m_GLFWHandle.swapBuffers();

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            m_viewController.update(float(ellapsedTime));
        }
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_AssetsRootPath { m_AppPath.parent_path() / "assets" }

{
    if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " < path to model >" << std::endl;
      exit(-1);
    }

    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file


    // Loading file ----------------

    const glmlv::fs::path objPath = glmlv::fs::path{ argv[1] };
    tinygltf::TinyGLTF loader;

    std::string err;
    std::string warn;

    std::string ext = getFilePathExtension(objPath);

    // Loading file into m_Model
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

		// Infos ---------------------

		std::cout << "# of animations   : " << m_Model.animations.size() << std::endl;
		std::cout << "# of buffers      : " << m_Model.buffers.size() << std::endl;
		std::cout << "# of bufferViews  : " << m_Model.bufferViews.size() << std::endl;
		std::cout << "# of materials    : " << m_Model.materials.size() << std::endl;
		std::cout << "# of meshes       : " << m_Model.meshes.size() << std::endl;
		std::cout << "# of nodes        : " << m_Model.nodes.size() << std::endl;
		std::cout << "# of textures     : " << m_Model.textures.size() << std::endl;
		std::cout << "# of images       : " << m_Model.images.size() << std::endl;
		std::cout << "# of scenes       : " << m_Model.scenes.size() << std::endl;

		// Node matrix computing
		// Il faut calculer les nodes, selon leur parent
		// La modelMatrix et les transformation d'un node dépendent des nodes parents
		// Il faut stocker les transformation "global" dans un dictionnaire (index du mesh, node)

		for (auto node : m_Model.nodes) {
			std::cout << "***" << std::endl;
			std::cout << "name             : " << node.name << std::endl;
			std::cout << "skin             : " << node.skin << std::endl;
			std::cout << "mesh             : " << node.mesh << std::endl;
			std::cout << "children size    : " << node.children.size() << std::endl;
			std::cout << "rotation size    : " << node.rotation.size() << std::endl;
			std::cout << "scale size       : " << node.scale.size() << std::endl;
			std::cout << "translation size : " << node.translation.size() << std::endl;
			std::cout << "matrix size      : " << node.matrix.size() << std::endl;
		}

    // Fill buffers ----------------------

    std::vector<GLuint> buffers(m_Model.buffers.size());

    glGenBuffers(buffers.size(), buffers.data());
    for (uint indexBuffer = 0; indexBuffer < m_Model.buffers.size(); ++indexBuffer) {
      glBindBuffer(GL_ARRAY_BUFFER, buffers[indexBuffer]);
      glBufferStorage(GL_ARRAY_BUFFER, m_Model.buffers[indexBuffer].data.size(), m_Model.buffers[indexBuffer].data.data(), 0);
    }

    std::cout << "Nombre de buffers : " << m_Model.buffers.size() << std::endl;

    // Attribute location -------------

    std::map<std::string, GLint> attribIndexOf;
    std::vector<std::string> attribNames = {"POSITION", "NORMAL", "TANGENT", "TEXCOORD_0", "TEXCOORD_1", "COLOR_0", "JOINTS_0", "WEIGHTS_0"};
    for (uint location = 0; location < attribNames.size(); ++location) {
      attribIndexOf.insert(std::make_pair(attribNames[location], location));
    }

    // Fill vao ------------------------
		int meshcount = 0;
    for (auto mesh : m_Model.meshes) {

      for (auto primitive : mesh.primitives) {

        GLuint vaoId;
        glGenVertexArrays(1, &vaoId);
        glBindVertexArray(vaoId);

        tinygltf::Accessor indicesAccessor = m_Model.accessors[primitive.indices];
        tinygltf::BufferView bufferView = m_Model.bufferViews[indicesAccessor.bufferView];
        int bufferIndex = bufferView.buffer;
        glBindBuffer(bufferView.target, buffers[bufferIndex]); // Bind le buffer OpenGL de la premiere boucle

        for (auto attribute : primitive.attributes) {

          for (auto key : attribNames) {

            if (primitive.attributes[key]) {

              tinygltf::Accessor attributeAccessor = m_Model.accessors[primitive.attributes[key]];
              tinygltf::BufferView attributeBufferView = m_Model.bufferViews[attributeAccessor.bufferView];
              int attributeBufferIndex = attributeBufferView.buffer;

              glBindBuffer(attributeBufferView.target, buffers[attributeBufferIndex]);
              glEnableVertexAttribArray(attribIndexOf[key]);
              int numberOfComponent = tinygltf::GetTypeSizeInBytes(attributeAccessor.type);
              glVertexAttribPointer(attribIndexOf[key], numberOfComponent, attributeAccessor.componentType, attributeAccessor.normalized, attributeBufferView.byteStride, (const GLvoid *) (bufferView.byteOffset + attributeAccessor.byteOffset));
              glBindBuffer(attributeBufferView.target, 0); // We can unbind the VBO because OpenGL has "written" in the VAO what VBO it needs to read when the VAO will be drawn

            }

          }

        }

        m_VAOs.push_back(vaoId);
        m_Primitives.push_back(primitive);

      }

    }

		computeMatrices(m_Model.nodes[0], glm::mat4(1));

    // ------------------------------------------------------------------------------------------------------------

    const GLint vboBindingIndex = 0; // Arbitrary choice between 0 and glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS)

    const GLint positionAttrLocation = 0;
    const GLint normalAttrLocation = 1;
    const GLint texCoordsAttrLocation = 2;

    glGenBuffers(1, &m_cubeVBO);
    glGenBuffers(1, &m_cubeIBO);
    glGenBuffers(1, &m_sphereVBO);
    glGenBuffers(1, &m_sphereIBO);

    m_cubeGeometry = glmlv::makeCube();
    m_sphereGeometry = glmlv::makeSphere(32);

    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    glBufferStorage(GL_ARRAY_BUFFER, m_cubeGeometry.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), m_cubeGeometry.vertexBuffer.data(), 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_sphereVBO);
    glBufferStorage(GL_ARRAY_BUFFER, m_sphereGeometry.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), m_sphereGeometry.vertexBuffer.data(), 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_cubeIBO);
    glBufferStorage(GL_ARRAY_BUFFER, m_cubeGeometry.indexBuffer.size() * sizeof(uint32_t), m_cubeGeometry.indexBuffer.data(), 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_sphereIBO);
    glBufferStorage(GL_ARRAY_BUFFER, m_sphereGeometry.indexBuffer.size() * sizeof(uint32_t), m_sphereGeometry.indexBuffer.data(), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Lets use a lambda to factorize VAO initialization:
    const auto initVAO = [positionAttrLocation, normalAttrLocation, texCoordsAttrLocation](GLuint& vao, GLuint vbo, GLuint ibo)
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // We tell OpenGL what vertex attributes our VAO is describing:
        glEnableVertexAttribArray(positionAttrLocation);
        glEnableVertexAttribArray(normalAttrLocation);
        glEnableVertexAttribArray(texCoordsAttrLocation);

        glBindBuffer(GL_ARRAY_BUFFER, vbo); // We bind the VBO because the next 3 calls will read what VBO is bound in order to know where the data is stored

        glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, position));
        glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, normal));
        glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, texCoords));

        glBindBuffer(GL_ARRAY_BUFFER, 0); // We can unbind the VBO because OpenGL has "written" in the VAO what VBO it needs to read when the VAO will be drawn

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); // Binding the IBO to GL_ELEMENT_ARRAY_BUFFER while a VAO is bound "writes" it in the VAO for usage when the VAO will be drawn

        glBindVertexArray(0);
    };

    initVAO(m_cubeVAO, m_cubeVBO, m_cubeIBO);
    initVAO(m_sphereVAO, m_sphereVBO, m_sphereIBO);

    glActiveTexture(GL_TEXTURE0); // We will work on GL_TEXTURE0 texture unit. Since the shader only use one texture at a time, we only need one texture unit
    {
        glmlv::Image2DRGBA image = glmlv::readImage(m_AssetsRootPath / m_AppName / "textures" / "cube.png");

        glGenTextures(1, &m_cubeTextureKd);
        glBindTexture(GL_TEXTURE_2D, m_cubeTextureKd);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width(), image.height());
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    {
        glmlv::Image2DRGBA image = glmlv::readImage(m_AssetsRootPath / m_AppName / "textures" / "sphere.jpg");

        glGenTextures(1, &m_sphereTextureKd);
        glBindTexture(GL_TEXTURE_2D, m_sphereTextureKd);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width(), image.height());
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Note: no need to bind a sampler for modifying it: the sampler API is already direct_state_access
    glGenSamplers(1, &m_textureSampler);
    glSamplerParameteri(m_textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnable(GL_DEPTH_TEST);

    m_program = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "forward.vs.glsl", m_ShadersRootPath / m_AppName / "forward.fs.glsl" });
    m_program.use();

    m_uModelViewProjMatrixLocation = glGetUniformLocation(m_program.glId(), "uModelViewProjMatrix");
    m_uModelViewMatrixLocation = glGetUniformLocation(m_program.glId(), "uModelViewMatrix");
    m_uNormalMatrixLocation = glGetUniformLocation(m_program.glId(), "uNormalMatrix");


    m_uDirectionalLightDirLocation = glGetUniformLocation(m_program.glId(), "uDirectionalLightDir");
    m_uDirectionalLightIntensityLocation = glGetUniformLocation(m_program.glId(), "uDirectionalLightIntensity");

    m_uPointLightPositionLocation = glGetUniformLocation(m_program.glId(), "uPointLightPosition");
    m_uPointLightIntensityLocation = glGetUniformLocation(m_program.glId(), "uPointLightIntensity");

    m_uKdLocation = glGetUniformLocation(m_program.glId(), "uKd");
    m_uKdSamplerLocation = glGetUniformLocation(m_program.glId(), "uKdSampler");

    m_viewController.setViewMatrix(glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
}

tinygltf::Node Application::findNode(tinygltf::Mesh mesh) {
		for (auto node : m_Model.nodes) {
			if (m_Model.meshes[node.mesh] == mesh) {
				return node;
			}
		}
		return m_Model.nodes[0];
}

void Application::computeMatrices(tinygltf::Node node, glm::mat4 matrix) {

	// Default
	glm::mat4 modelMatrix = glm::mat4(1);

	if (node.matrix.size() == 16) {
		modelMatrix = glm::mat4(node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3], node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7], node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11], node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]);
	}

	if (node.translation.size() == 3) {
		const auto translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
		modelMatrix = glm::translate(modelMatrix, translation);
	}

	if (node.rotation.size() == 3) {
		 const auto rotation = node.rotation.size() == 3 ? glm::vec3(node.rotation[0], node.rotation[1], node.rotation[2]) : glm::vec4(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]);
		 // modelMatrix = glm::rotate(modelMatrix, ):
	}

	if (node.rotation.size() == 4) {
		//
	}

	if (node.scale.size() == 3) {
		 const auto scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
		 modelMatrix = glm::scale(modelMatrix, scale);
	}

	modelMatrix = modelMatrix * matrix;

	if (node.mesh > 0) {
		m_ModelMatrices[node.mesh] = modelMatrix;
	}

	for (auto child : node.children) {
		computeMatrices(m_Model.nodes[child], modelMatrix);
	}

}
