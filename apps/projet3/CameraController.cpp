#include "CameraController.hpp"
#include <iostream>

using namespace glm;

double CameraController::scrollOffset = 0;

bool CameraController::update(float elapsedTime)
{

    bool hasMoved = false;

    if (scrollOffset != 0) {
      moveFront(scrollOffset * m_fSpeed);
      if (scrollOffset < 0 && scrollOffset + m_fSpeed/2 < 0) {
        scrollOffset += m_fSpeed/2;
      }
      else if (scrollOffset > 0 && scrollOffset - m_fSpeed/2 > 0) {
        scrollOffset -= m_fSpeed/2;
      }
      else {
        scrollOffset = 0;
      }
    }

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
