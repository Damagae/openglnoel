#include "Controller.hpp"

#include <glmlv/glfw.hpp>

using namespace glm;

bool Controller::update(float elapsedTime)
{

    auto frontVector = -vec3(m_RcpViewMatrix[2]);
    auto leftVector = -vec3(m_RcpViewMatrix[0]);
    auto upVector = vec3(m_RcpViewMatrix[1]);
    auto position = vec3(m_RcpViewMatrix[3]);

    bool hasMoved = false;

    float lateralAngleDelta = 0.f;

    if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) && !m_LeftButtonPressed) {
        m_LeftButtonPressed = true;
        glfwGetCursorPos(m_pWindow, &m_LastCursorPosition.x, &m_LastCursorPosition.y);
    }
    else if (!glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) && m_LeftButtonPressed) {
        m_LeftButtonPressed = false;
    }

    auto newRcpViewMatrix = glm::translate(glm::mat4(1), glm::vec3(0,-10,-10));


    if (m_LeftButtonPressed) {
        dvec2 cursorPosition;
        glfwGetCursorPos(m_pWindow, &cursorPosition.x, &cursorPosition.y);
        dvec2 delta = cursorPosition - m_LastCursorPosition;

        m_LastCursorPosition = cursorPosition;

        if (delta.x || delta.y) {
            newRcpViewMatrix = rotate(newRcpViewMatrix, -0.01f * float(delta.x), vec3(0, 1, 0));
            newRcpViewMatrix = rotate(newRcpViewMatrix, -0.01f * float(delta.y), vec3(1, 0, 0));

            hasMoved = true;
        }
    }

    newRcpViewMatrix = glm::translate(newRcpViewMatrix, glm::vec3(0,-10,-10));

    frontVector = -vec3(newRcpViewMatrix[2]);
    leftVector = -vec3(newRcpViewMatrix[0]);
    upVector = cross(frontVector, leftVector);

    if (hasMoved) {
      // setViewMatrix(lookAt(position, position + frontVector, upVector));
        setViewMatrix(lookAt(position, position + frontVector, upVector));
    }

    return hasMoved;
}
