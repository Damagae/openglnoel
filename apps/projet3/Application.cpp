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

	bool directionalSMDirty = true;
	bool directionalSMResolutionDirty = false;

	std::cout << "scene radius = " << m_SceneSizeLength * 0.5f << std::endl;
	std::cout << "scene center = " << m_SceneCenter * 0.5f << std::endl;

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

				const auto viewportSize = m_GLFWHandle.framebufferSize();
				const auto projMatrix = glm::perspective(70.f, float(viewportSize.x) / viewportSize.y, 0.01f, 100.f);
				const auto viewMatrix = m_CameraController.getViewMatrix();
				const auto rcpViewMatrix = m_CameraController.getRcpViewMatrix();

				m_SceneSizeLength = 10.f;
				m_SceneCenter = glm::vec3(1.f, 1.f, 1.f);

				const float sceneRadius = m_SceneSizeLength * 10.f;

				const auto dirLightUpVector = computeDirectionVectorUp(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
				const auto dirLightViewMatrix = glm::lookAt(m_SceneCenter + m_DirLightDirection * sceneRadius, m_SceneCenter, dirLightUpVector); // Will not work if m_DirLightDirection is colinear to lightUpVector
        // const auto dirLightViewMatrix = glm::lookAt(glm::vec3(0.f) + m_DirLightDirection * 100.f, glm::vec3(0.f), dirLightUpVector); // Will not work if m_DirLightDirection is colinear to lightUpVector
				const auto dirLightProjMatrix = glm::ortho(-sceneRadius, sceneRadius, -sceneRadius, sceneRadius, 0.01f * sceneRadius, 2.f * sceneRadius);

				// Shadow Map
				if (directionalSMResolutionDirty) {
                glDeleteTextures(1, &m_directionalSMTexture);

                // Realocate texture
                glGenTextures(1, &m_directionalSMTexture);
                glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
                glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, m_nDirectionalSMResolution, m_nDirectionalSMResolution);
                glBindTexture(GL_TEXTURE_2D, 0);

                // Attach new texture to FBO
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_directionalSMFBO);
                glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
                glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_directionalSMTexture, 0);

                const auto fboStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
                if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
                {
                    std::cerr << "Error on building directional shadow mapping framebuffer. Error code = " << fboStatus << std::endl;
                    throw std::runtime_error("Error on building directional shadow mapping framebuffer.");
                }

                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

                directionalSMResolutionDirty = false;
                directionalSMDirty = true; // The shadow map must also be recomputed
				}

				if (directionalSMDirty) {

						 m_directionalSMProgram.use();

						 glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_directionalSMFBO);
						 glViewport(0, 0, m_nDirectionalSMResolution, m_nDirectionalSMResolution);
						 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						 glUniformMatrix4fv(m_uDirLightViewProjMatrix, 1, GL_FALSE, glm::value_ptr(dirLightProjMatrix * dirLightViewMatrix));


						 for (uint id = 0; id < m_VAOs.size(); ++id) {
									auto vao = m_VAOs[id];
									tinygltf::Primitive primitive = m_Primitives[id];

									tinygltf::Accessor indices = m_Model.accessors[primitive.indices];
									int mode = getMode(primitive.mode);

									glBindVertexArray(vao);
							 		glDrawElements(mode, indices.count, indices.componentType, (const void*) indices.byteOffset);
						 }

						 glBindVertexArray(0);
						 glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

						 directionalSMDirty = false; // Pas de calcul au prochain tour
				}

				// Geometry Pass
				{
						m_geometryPassProgram.use();
						glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_GBufferFBO);

		        glViewport(0, 0, viewportSize.x, viewportSize.y);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						for (uint id = 0; id < m_VAOs.size(); ++id) {

							auto vao = m_VAOs[id];
							tinygltf::Primitive primitive = m_Primitives[id];

							tinygltf::Material material = m_Model.materials[primitive.material];

							const auto matrix = m_ModelMatrices[id];
							const auto modelMatrix = scaleModel(matrix);
							// const auto modelMatrix = matrix;

							const auto mvMatrix = viewMatrix * modelMatrix;
							const auto mvpMatrix = projMatrix * mvMatrix;
							const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));

							glUniformMatrix4fv(m_uModelViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
							glUniformMatrix4fv(m_uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvMatrix));
							glUniformMatrix4fv(m_uNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));

							// Same sampler for all texture units
	            glBindSampler(0, m_textureSampler);
	            glBindSampler(1, m_textureSampler);
	            glBindSampler(2, m_textureSampler);
	            glBindSampler(3, m_textureSampler);

	            // Set texture unit of each sampler
	            glUniform1i(m_uKaSamplerLocation, 0);
	            glUniform1i(m_uKdSamplerLocation, 1);
	            glUniform1i(m_uKsSamplerLocation, 2);
							glUniform1i(m_uShininessSamplerLocation, 3);

							// Diffuse color
							if (material.values.find("baseColorTexture") != material.values.end()) {
								glActiveTexture(GL_TEXTURE1);
								auto baseColorFactor = material.values["baseColorTexture"].number_array;
								if (baseColorFactor.size() >= 3) {
									glUniform3fv(m_uKdLocation, 1, glm::value_ptr(glm::vec3(baseColorFactor[0], baseColorFactor[1], baseColorFactor[2])));
								} else {
									glUniform3fv(m_uKdLocation, 1, glm::value_ptr(glm::vec3(1, 1, 1)));
								}
								glBindTexture(GL_TEXTURE_2D, m_TextureIds[material.values["baseColorTexture"].TextureIndex()]);
							}


							// Ambiant Color
							if (material.values.find("emissiveTexture") != material.values.end()) {
								glActiveTexture(GL_TEXTURE0);
								auto emissiveFactor = material.values["emissiveFactor"].number_array;
								if (emissiveFactor.size() >= 3) {
									glUniform3fv(m_uKaLocation, 1, glm::value_ptr(glm::vec3(emissiveFactor[0], emissiveFactor[1], emissiveFactor[2])));
								} else {
									glUniform3fv(m_uKaLocation, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
								}
								glBindTexture(GL_TEXTURE_2D, m_TextureIds[material.values["emissiveTexture"].TextureIndex()]);
							}


							glBindVertexArray(vao);
							tinygltf::Accessor indices = m_Model.accessors[primitive.indices];

							int mode = getMode(primitive.mode);
							glDrawElements(mode, indices.count, indices.componentType, (const void*) indices.byteOffset);

						}

						for (GLuint i : {0, 1, 2, 3})
							glBindSampler(0, m_textureSampler);

						glBindVertexArray(0);
						glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

					}

					// Shading pass
					{
							m_shadingPassProgram.use();

							glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_BeautyFBO);

							glViewport(0, 0, viewportSize.x, viewportSize.y);
							glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

							glUniform3fv(m_uDirectionalLightDirLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(m_DirLightDirection), 0))));
							glUniform3fv(m_uDirectionalLightIntensityLocation, 1, glm::value_ptr(m_DirLightColor * m_DirLightIntensity));

							glUniform3fv(m_uPointLightPositionLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(m_PointLightPosition, 1))));
							glUniform3fv(m_uPointLightIntensityLocation, 1, glm::value_ptr(m_PointLightColor * m_PointLightIntensity));

							glUniform1fv(m_uDirLightShadowMapBias, 1, &m_DirLightSMBias);
							glUniform1iv(m_uDirLightShadowMapSampleCount, 1, &m_DirLightSMSampleCount);
							glUniform1fv(m_uDirLightShadowMapSpread, 1, &m_DirLightSMSpread);

							glUniformMatrix4fv(m_uDirLightViewProjMatrix_shadingPass, 1, GL_FALSE, glm::value_ptr(dirLightProjMatrix * dirLightViewMatrix * rcpViewMatrix));

							for (int32_t i = GPosition; i < GDepth; ++i) {
									glActiveTexture(GL_TEXTURE0 + i);
									glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);

									glUniform1i(m_uGBufferSamplerLocations[i], i);
							}

							glActiveTexture(GL_TEXTURE0 + GDepth);
							glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
							glBindSampler(GDepth, m_directionalSMSampler);
							glUniform1i(m_uDirLightShadowMap, GDepth);

							glBindVertexArray(m_TriangleVAO);
							glDrawArrays(GL_TRIANGLES, 0, 3);
							glBindVertexArray(0);

							glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
					}

				// Put here rendering code
        glViewport(0, 0, viewportSize.x, viewportSize.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_CurrentlyDisplayed == GBufferTextureCount) { // Beauty

					glBindFramebuffer(GL_READ_FRAMEBUFFER, m_BeautyFBO);
					glReadBuffer(GL_COLOR_ATTACHMENT0);
					glBlitFramebuffer(0, 0, m_nWindowWidth, m_nWindowHeight, 0, 0, m_nWindowWidth, m_nWindowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

					glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        }
        else if (m_CurrentlyDisplayed == GDepth) {
            m_displayDepthProgram.use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GDepth]);

            glUniform1i(m_uGDepthSamplerLocation, 0);

            glBindVertexArray(m_TriangleVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }
				else if (m_CurrentlyDisplayed == Display_DirectionalLightDepthMap)
        {
            m_displayDepthProgram.use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);

            glUniform1i(m_uGDepthSamplerLocation, 0);

            glBindVertexArray(m_TriangleVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
				}
        else if (m_CurrentlyDisplayed == GPosition) {
            m_displayPositionProgram.use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GPosition]);

            glUniform1i(m_uGDepthSamplerLocation, 0);

            const auto rcpProjMat = glm::inverse(projMatrix);

            const glm::vec4 frustrumTopRight(1, 1, 1, 1);
            const auto frustrumTopRight_view = rcpProjMat * frustrumTopRight;

            glUniform3fv(m_uSceneSizeLocation, 1, glm::value_ptr(frustrumTopRight_view / frustrumTopRight_view.w));

            glBindVertexArray(m_TriangleVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }
        else {
            // GBuffer display
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBufferFBO);
            glReadBuffer(GL_COLOR_ATTACHMENT0 + m_CurrentlyDisplayed);
            glBlitFramebuffer(0, 0, m_nWindowWidth, m_nWindowHeight,
                0, 0, m_nWindowWidth, m_nWindowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
					}


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
                 bool angleChanged = ImGui::DragFloat("Phi Angle", &m_DirLightPhiAngleDegrees, 1.0f, 0.0f, 360.f);
                 angleChanged = ImGui::DragFloat("Theta Angle", &m_DirLightThetaAngleDegrees, 1.0f, 0.0f, 180.f) || angleChanged;

								 if (angleChanged) {
										 m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
										 directionalSMDirty = true;
								 }

								 if (ImGui::InputInt("DirShadowMap Res", &m_nDirectionalSMResolution)) {
	                    if (m_nDirectionalSMResolution <= 0) {
	                        m_nDirectionalSMResolution = 1;
											}
	                    directionalSMResolutionDirty = true;
									}

								 ImGui::InputFloat("DirShadowMap Bias", &m_DirLightSMBias);
								 ImGui::InputInt("DirShadowMap SampleCount", &m_DirLightSMSampleCount);
								 ImGui::InputFloat("DirShadowMap Spread", &m_DirLightSMSpread);
             }

             if (ImGui::CollapsingHeader("Point Light"))
             {
                 ImGui::ColorEdit3("PointLightColor", glm::value_ptr(m_PointLightColor));
                 ImGui::DragFloat("PointLightIntensity", &m_PointLightIntensity, 0.1f, 0.f, 16000.f);
                 ImGui::DragFloat3("Position", glm::value_ptr(m_PointLightPosition), 0.5f);
             }

             if (ImGui::CollapsingHeader("GBuffer"))
             {
                 for (int32_t i = GPosition; i <= GBufferTextureCount; ++i)
                 {
                     if (ImGui::RadioButton(m_GBufferTexNames[i], m_CurrentlyDisplayed == i))
                         m_CurrentlyDisplayed = GBufferTextureType(i);
                 }
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

				 // exit(0);
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
		std::cout << "# of samplers     : " << m_Model.samplers.size() << std::endl;
		std::cout << "# of cameras      : " << m_Model.cameras.size() << std::endl;


    // Fill buffers ----------------------

    std::vector<GLuint> buffers(m_Model.buffers.size());

    glGenBuffers(buffers.size(), buffers.data());

    for (uint buffer = 0; buffer < m_Model.buffers.size(); ++buffer) {
      glBindBuffer(GL_ARRAY_BUFFER, buffers[buffer]);
      glBufferStorage(GL_ARRAY_BUFFER, m_Model.buffers[buffer].data.size(), m_Model.buffers[buffer].data.data(), 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Attribute location -------------

    std::map<std::string, GLint> attribIndexOf;
    std::vector<std::string> attribNames = {"POSITION", "NORMAL", "TEXCOORD_0"};
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
		computeScaling();
		std::cout << "# of matrices : " << m_ModelMatrices.size() << std::endl;

		m_CameraController.setHauteur(10 / 2);


		// Samplers -----------------------

		for (tinygltf::Sampler sampler : m_Model.samplers) {
			GLuint samplerId;
			glGenSamplers(1, &samplerId);
			glSamplerParameteri(samplerId, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
			glSamplerParameteri(samplerId, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
			glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_S, sampler.wrapS);
			glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_T, sampler.wrapT);
			glSamplerParameteri(samplerId, GL_TEXTURE_WRAP_R, sampler.wrapR);

			m_SamplersIds.push_back(samplerId);
		}

		if (m_SamplersIds.size() > 0) {
			m_textureSampler = m_SamplersIds[0];
		}

		// Textures ------------------------

		for (tinygltf::Texture texture : m_Model.textures) {

			tinygltf::Image &image = m_Model.images[texture.source];

			std::cout << "loading texture : " << image.uri << std::endl;

			GLuint texId = 0;
      glGenTextures(1, &texId);
      glBindTexture(GL_TEXTURE_2D, texId);
			GLenum format = image.component == 3 ? GL_RGB : GL_RGBA;
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width, image.height);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, format, GL_UNSIGNED_BYTE, &image.image.at(0));

      glBindTexture(GL_TEXTURE_2D, 0);

			m_TextureIds.push_back(texId);

		}

		initShadowData();

		initShadersData();

    glEnable(GL_DEPTH_TEST);

    // Init GBuffer
		glGenTextures(GBufferTextureCount, m_GBufferTextures);


		for (int32_t i = GPosition; i < GBufferTextureCount; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, m_GBufferTextureFormat[i], m_nWindowWidth, m_nWindowHeight);
		}


		glGenFramebuffers(1, &m_GBufferFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_GBufferFBO);
    for (int32_t i = GPosition; i < GDepth; ++i)
    {
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_GBufferTextures[i], 0);
    }
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBufferTextures[GDepth], 0);

    // we will write into 5 textures from the fragment shader
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(5, drawBuffers);

    GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FB error, status: " << status << std::endl;
        throw std::runtime_error("FBO error");
    }

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		// Init beauty texture and FBO
    glGenTextures(1, &m_BeautyTexture);

    glBindTexture(GL_TEXTURE_2D, m_BeautyTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, m_nWindowWidth, m_nWindowHeight);

    glGenFramebuffers(1, &m_BeautyFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_BeautyFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BeautyTexture, 0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    {
        GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "FB error, status: " << status << std::endl;
            throw std::runtime_error("FBO error");
        }
    }

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glGenBuffers(1, &m_TriangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_TriangleVBO);

    GLfloat data[] = { -1, -1, 3, -1, -1, 3 };
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(data), data, 0);

    glGenVertexArrays(1, &m_TriangleVAO);
    glBindVertexArray(m_TriangleVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

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

		m_uDirLightViewProjMatrix_shadingPass = glGetUniformLocation(m_shadingPassProgram.glId(), "uDirLightViewProjMatrix");
    m_uDirLightShadowMap = glGetUniformLocation(m_shadingPassProgram.glId(), "uDirLightShadowMap");
    m_uDirLightShadowMapBias = glGetUniformLocation(m_shadingPassProgram.glId(), "uDirLightShadowMapBias");
    m_uDirLightShadowMapSampleCount = glGetUniformLocation(m_shadingPassProgram.glId(), "uDirLightShadowMapSampleCount");
		m_uDirLightShadowMapSpread = glGetUniformLocation(m_shadingPassProgram.glId(), "uDirLightShadowMapSpread");

    m_uDirectionalLightDirLocation = glGetUniformLocation(m_shadingPassProgram.glId(), "uDirectionalLightDir");
    m_uDirectionalLightIntensityLocation = glGetUniformLocation(m_shadingPassProgram.glId(), "uDirectionalLightIntensity");

    m_uPointLightPositionLocation = glGetUniformLocation(m_shadingPassProgram.glId(), "uPointLightPosition");
    m_uPointLightIntensityLocation = glGetUniformLocation(m_shadingPassProgram.glId(), "uPointLightIntensity");

    m_displayDepthProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "displayDepth.fs.glsl" });

    m_uGDepthSamplerLocation = glGetUniformLocation(m_displayDepthProgram.glId(), "uGDepth");

    m_displayPositionProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "displayPosition.fs.glsl" });

    m_uGPositionSamplerLocation = glGetUniformLocation(m_displayPositionProgram.glId(), "uGPosition");
		m_uSceneSizeLocation = glGetUniformLocation(m_displayPositionProgram.glId(), "uSceneSize");

		m_directionalSMProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "directionalSM.vs.glsl", m_ShadersRootPath / m_AppName / "directionalSM.fs.glsl" });
		m_uDirLightViewProjMatrix = glGetUniformLocation(m_directionalSMProgram.glId(), "uDirLightViewProjMatrix");
}

void Application::initShadowData() {

    glGenTextures(1, &m_directionalSMTexture);

    glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, m_nDirectionalSMResolution, m_nDirectionalSMResolution);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &m_directionalSMFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_directionalSMFBO);
    glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_directionalSMTexture, 0);

    const auto fboStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Error on building directional shadow mapping framebuffer. Error code = " << fboStatus << std::endl;
        throw std::runtime_error("Error on building directional shadow mapping framebuffer.");
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glGenSamplers(1, &m_directionalSMSampler);
		glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
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

		// Resizing
		bool firstLoop = true;
		float min = 0.f;
		float max = 0.f;
		if (node.mesh > -1) {

			tinygltf::Mesh mesh = m_Model.meshes[node.mesh];
			for (auto primitive : mesh.primitives) {
				tinygltf::Accessor accessor = m_Model.accessors[primitive.indices];

				if (accessor.minValues.size() > 0 && accessor.maxValues.size() > 0) {
						float primitiveMin = accessor.minValues[0];
						float primitiveMax = accessor.maxValues[0];
						if (firstLoop) {
								min = primitiveMin;
								max = primitiveMax;
								firstLoop = false;
						} else {
								if (primitiveMin < min) {
									min = primitiveMin;
								}
								if (primitiveMax > max) {
									max = primitiveMax;
								}
						}
				}
			}
			glm::vec3 meshCenter = 0.5f * (glm::vec3(max) + glm::vec3(min));
			glm::vec3 meshSize = glm::vec3(max) - glm::vec3(min);
			float meshLength = glm::length(meshSize);
			std::cout << "Mesh center = " << meshCenter << std::endl;

			float maxAxis = meshSize[0];
			if (meshSize[1] > maxAxis) { maxAxis = meshSize[1]; }
			if (meshSize[2] > maxAxis) { maxAxis = meshSize[2]; }

			// modelMatrix = glm::scale(modelMatrix, glm::vec3(1 / maxAxis));
			// modelMatrix = glm::translate(modelMatrix, glm::vec3(0.f, -meshSize[1] * 0.5, 0.f));

			if (m_SceneCenter != glm::vec3(0.f)) {
				m_SceneCenter = 0.5f * (m_SceneCenter + meshCenter);
			}
			else {
				m_SceneCenter = meshCenter;
			}

			m_SceneSize += meshSize;
			m_SceneSizeLength += meshLength;



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
