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
	float clearColor[3] = { 0.9, 0.9, 0.8 };
	glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto projMatrix = glm::perspective(70.f, float(viewportSize.x) / viewportSize.y, 0.01f, 100.f);
				const auto viewMatrix = m_CameraController.getViewMatrix();

        glUniform3fv(m_uDirectionalLightDirLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(m_DirLightDirection), 0))));
        glUniform3fv(m_uDirectionalLightIntensityLocation, 1, glm::value_ptr(m_DirLightColor * m_DirLightIntensity));

        glUniform3fv(m_uPointLightPositionLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(m_PointLightPosition, 1))));
        glUniform3fv(m_uPointLightIntensityLocation, 1, glm::value_ptr(m_PointLightColor * m_PointLightIntensity));


				// const auto bindMaterial = [&](const tinygltf::Material & material)
        // {
        //     glUniform3fv(m_uKaLocation, 1, glm::value_ptr(material.Ka)); // ambiant
        //     glUniform3fv(m_uKdLocation, 1, glm::value_ptr(material.Kd)); // diffuse
        //     glUniform3fv(m_uKsLocation, 1, glm::value_ptr(material.Ks)); // specular
        //     glUniform1fv(m_uShininessLocation, 1, &material.shininess);  // shininess
				//
        //     glActiveTexture(GL_TEXTURE0);
        //     glBindTexture(GL_TEXTURE_2D, material.KaTextureId);
        //     glActiveTexture(GL_TEXTURE1);
        //     glBindTexture(GL_TEXTURE_2D, material.KdTextureId);
        //     glActiveTexture(GL_TEXTURE2);
        //     glBindTexture(GL_TEXTURE_2D, material.KsTextureId);
        //     glActiveTexture(GL_TEXTURE3);
        //     glBindTexture(GL_TEXTURE_2D, material.shininessTextureId);
				// };

        {
					glActiveTexture(GL_TEXTURE0);
	        glUniform1i(m_uKdSamplerLocation, 0); // Set the uniform to 0 because we use texture unit 0

          for (uint id = 0; id < m_VAOs.size(); ++id) {

						auto vao = m_VAOs[id];
            tinygltf::Primitive primitive = m_Primitives[id];

						tinygltf::Material material = m_Model.materials[primitive.material];

						const auto modelMatrix = glm::scale(m_ModelMatrices[id], glm::vec3(0.02f, 0.02f, 0.02f));
						// const auto modelMatrix = glm::mat4(1);

						const auto mvMatrix = viewMatrix * modelMatrix;
	          const auto mvpMatrix = projMatrix * mvMatrix;
	          const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));

	          glUniformMatrix4fv(m_uModelViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
	          glUniformMatrix4fv(m_uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvMatrix));
	          glUniformMatrix4fv(m_uNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));


						glBindSampler(0, m_SamplersIds[0]);

						// Diffuse color
				    glUniform3fv(m_uKdLocation, 1, glm::value_ptr(glm::vec3(1, 1, 1)));
						glBindTexture(GL_TEXTURE_2D, m_TextureIds[material.values["baseColorTexture"].TextureIndex()]);

            glBindVertexArray(vao);
            tinygltf::Accessor indices = m_Model.accessors[primitive.indices];

            int mode = getMode(primitive.mode);

						// mode = GL_PATCHES;

            glDrawElements(mode, indices.count, indices.componentType, (const void*) indices.byteOffset);
          }
        }

				// exit(0);

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
            m_CameraController.update(float(ellapsedTime));
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


		// for (auto node : m_Model.nodes) {
		// 	std::cout << "***" << std::endl;
		// 	std::cout << "name             : " << node.name << std::endl;
		// 	std::cout << "skin             : " << node.skin << std::endl;
		// 	std::cout << "mesh             : " << node.mesh << std::endl;
		// 	std::cout << "children size    : " << node.children.size() << std::endl;
		// 	std::cout << "rotation size    : " << node.rotation.size() << std::endl;
		// 	std::cout << "scale size       : " << node.scale.size() << std::endl;
		// 	std::cout << "translation size : " << node.translation.size() << std::endl;
		// 	std::cout << "matrix size      : " << node.matrix.size() << std::endl;
		// }

    // Fill buffers ----------------------

    std::vector<GLuint> buffers(m_Model.buffers.size());

    glGenBuffers(buffers.size(), buffers.data());

    for (uint buffer = 0; buffer < m_Model.buffers.size(); ++buffer) {
      glBindBuffer(GL_ARRAY_BUFFER, buffers[buffer]);
      glBufferStorage(GL_ARRAY_BUFFER, m_Model.buffers[buffer].data.size(), m_Model.buffers[buffer].data.data(), 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    std::cout << "# of buffers : " << buffers.size() << std::endl;

    // Attribute location -------------

    std::map<std::string, GLint> attribIndexOf;
    std::vector<std::string> attribNames = {"POSITION", "NORMAL", "TANGENT", "TEXCOORD_0", "TEXCOORD_1", "COLOR_0", "JOINTS_0", "WEIGHTS_0"};
    for (uint location = 0; location < attribNames.size(); ++location) {
      attribIndexOf.insert(std::make_pair(attribNames[location], location));
    }

    // Fill vao ------------------------

    for (auto mesh : m_Model.meshes) {

      for (auto primitive : mesh.primitives) {

        GLuint vaoId;
        glGenVertexArrays(1, &vaoId);
        glBindVertexArray(vaoId);


				// IBO
        tinygltf::Accessor indicesAccessor = m_Model.accessors[primitive.indices];
        tinygltf::BufferView bufferView = m_Model.bufferViews[indicesAccessor.bufferView];
        int bufferIndex = bufferView.buffer;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[bufferIndex]); // Bind le buffer OpenGL de la premiere boucle

        for (auto attribute : primitive.attributes) {

          for (auto key : attribNames) { // Pour chaque key d'attribut ("POSITION", "NORMAL", etc)

            if (primitive.attributes[key]) {

              tinygltf::Accessor attributeAccessor = m_Model.accessors[primitive.attributes[key]];
              bufferView = m_Model.bufferViews[attributeAccessor.bufferView];
              bufferIndex = bufferView.buffer;

              glBindBuffer(GL_ARRAY_BUFFER, buffers[bufferIndex]);
              glEnableVertexAttribArray(attribIndexOf[key]);
              glVertexAttribPointer(attribIndexOf[key], attributeAccessor.type, attributeAccessor.componentType, attributeAccessor.normalized, bufferView.byteStride, (const GLvoid *) (bufferView.byteOffset + attributeAccessor.byteOffset));

            }

          }

					glBindBuffer(GL_ARRAY_BUFFER, 0);

        }

        m_VAOs.push_back(vaoId);
        m_Primitives.push_back(primitive);

      }

    }

		std::cout << "# of vaos : " << m_VAOs.size() << std::endl;

		computeMatrices(m_Model.nodes[0], glm::mat4(1));
		std::cout << "# of matrices : " << m_ModelMatrices.size() << std::endl;

		// Textures ------------------------

		for (tinygltf::Texture texture : m_Model.textures) {

			glActiveTexture(GL_TEXTURE0);

			tinygltf::Image &image = m_Model.images[texture.source];

			GLuint texId;
      glGenTextures(1, &texId);
      glBindTexture(GL_TEXTURE_2D, texId);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			GLenum format = GL_RGBA;
			if (image.component == 3) {
					format = GL_RGB;
			}

			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width, image.height);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, format, GL_UNSIGNED_BYTE, &image.image.at(0));
      glBindTexture(GL_TEXTURE_2D, 0);

			m_TextureIds.push_back(texId);

			// Samplers ------------------------

			tinygltf::Sampler sampler = m_Model.samplers[texture.sampler];

			GLuint samplerId;
			glGenSamplers(1, &samplerId);
	    glSamplerParameteri(samplerId, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
			glSamplerParameteri(samplerId, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
			glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_S, sampler.wrapS);
			glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_T, sampler.wrapT);
	    glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_R, sampler.wrapR);

			m_SamplersIds.push_back(samplerId);

		}

		std::cout << "# of samplers : " << m_SamplersIds.size() << std::endl;



    // Note: no need to bind a sampler for modifying it: the sampler API is already direct_state_access


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

    // m_Controller.setViewMatrix(glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

}

void Application::computeMatrices(tinygltf::Node node, glm::mat4 matrix) {

	// Default
	glm::mat4 modelMatrix = matrix;


	// Si le modèle possède une matrice de transformation
	if (node.matrix.size() == 16) {
		modelMatrix = glm::mat4(node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3], node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7], node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11], node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]);
	}

	// Sinon
	else {

		// Scaling
		if (node.scale.size() == 3) {
			 const auto scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
			 modelMatrix = glm::scale(modelMatrix, scale);
		}

		// Rotation
		if (node.rotation.size() == 4) {
			const auto rotation = quatToMatrix(glm::vec4(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]));
			modelMatrix = rotation * modelMatrix;
		}

		// Translation
		if (node.translation.size() == 3) {
			const auto translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
			modelMatrix = glm::translate(modelMatrix, translation);
		}

	}

	// Application des transformation sur la matrice parente héritée
	modelMatrix = modelMatrix * matrix;

	if (node.mesh > -1) {
		// Si la transformation correspond à un mesh, on enregistre la matrice pour l'index du mesh
		m_ModelMatrices[node.mesh] = modelMatrix;
	}

	// On réitère les opérations pour tous les enfants de la node
	for (auto child : node.children) {
		computeMatrices(m_Model.nodes[child], modelMatrix);
	}

}
