#include "Controller.hpp"

#include <glmlv/glfw.hpp>

using namespace glm;

bool Controller::update(float elapsedTime)
{

    bool hasMoved = false;

    if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) && !m_LeftButtonPressed) {
        m_LeftButtonPressed = true;
        glfwGetCursorPos(m_pWindow, &m_LastCursorPosition.x, &m_LastCursorPosition.y);
    }
    else if (!glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) && m_LeftButtonPressed) {
        m_LeftButtonPressed = false;
    }


    if (m_LeftButtonPressed) {
        dvec2 cursorPosition;
        glfwGetCursorPos(m_pWindow, &cursorPosition.x, &cursorPosition.y);
        dvec2 delta = cursorPosition - m_LastCursorPosition;

        m_LastCursorPosition = cursorPosition;

        if (delta.x || delta.y) {
            rotateLeft(delta.x * m_fSpeed);
            rotateUp(delta.y * m_fSpeed);

            hasMoved = true;
        }
    }

    return hasMoved;
}
