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
        glfwGetCursorPos(m_pWindow, &m_LastCursorPositionLeft.x, &m_LastCursorPositionLeft.y);
    }
    else if (!glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) && m_LeftButtonPressed) {
        m_LeftButtonPressed = false;
    }

    if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_RIGHT) && !m_RightButtonPressed) {
        m_RightButtonPressed = true;
        glfwGetCursorPos(m_pWindow, &m_LastCursorPositionRight.x, &m_LastCursorPositionRight.y);
    }
    else if (!glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_RIGHT) && m_RightButtonPressed) {
        m_RightButtonPressed = false;
    }


    if (m_LeftButtonPressed) {
        dvec2 cursorPosition;
        glfwGetCursorPos(m_pWindow, &cursorPosition.x, &cursorPosition.y);
        dvec2 delta = cursorPosition - m_LastCursorPositionLeft;

        m_LastCursorPositionLeft = cursorPosition;

        if (delta.x || delta.y) {
            rotateLeft(delta.x * m_fSpeed);
            rotateUp(delta.y * m_fSpeed);

            hasMoved = true;
        }

    }

    else if (m_RightButtonPressed) {
      dvec2 cursorPosition;
      glfwGetCursorPos(m_pWindow, &cursorPosition.x, &cursorPosition.y);
      dvec2 delta = cursorPosition - m_LastCursorPositionRight;

      m_LastCursorPositionRight = cursorPosition;

      if (delta.x || delta.y) {
          moveCenterX(delta.x * m_fSpeed);
          moveCenterY(delta.y * m_fSpeed);

          hasMoved = true;
      }

    }




    return hasMoved;
}
