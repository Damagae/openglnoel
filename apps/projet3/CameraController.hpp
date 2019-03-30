#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glmlv/glfw.hpp>

struct GLFWwindow;

class CameraController {

  private:
    GLFWwindow* m_pWindow = nullptr;
    float m_fSpeed = 0.f;
    bool m_LeftButtonPressed = false;
    bool m_RightButtonPressed = false;
    glm::dvec2 m_LastCursorPositionLeft;
    glm::dvec2 m_LastCursorPositionRight;

    glm::mat4 m_ViewMatrix = glm::mat4(1);
    glm::mat4 m_RcpViewMatrix = glm::mat4(1);

    float m_fDistance = 8.f;	// Distance par rapport au centre de la scène
    float m_fHauteur = 2.f;		// Hauteur de la caméra par rapport au centre de la scène
  	float m_fDecalage = 0.f;		// Hauteur de la caméra par rapport au centre de la scène
  	float m_fAngleX = 0.f;		// Angle effectuée par la caméra autour de l'axe X de la scène
  	float m_fAngleY = 0.f;		// Angle effectuée par la caméra autour de l'axe Y de la scène

    glm::dvec2 m_PreviousDelta;
    glm::dvec2 m_Delta;

  	// Returns true if zoom is max/min, false if not
    bool zoomMin(float delta) {
    	return (m_fDistance + delta <= 2.0);
    }

    bool zoomMax(float delta) {
    	return (m_fDistance + delta >= 50.0);
    }

  public:
    static double scrollOffset;

    CameraController(GLFWwindow* window, float speed = 1.f) :
        m_pWindow(window), m_fSpeed(speed) {
          glfwSetScrollCallback(window, zoomCB);
        }

    static void zoomCB(GLFWwindow* window, double xoffset, double yoffset) {
        scrollOffset += yoffset;
    }

    void setHauteur(float hauteur) {
      m_fHauteur = hauteur;
    }

    void setSpeed(float speed) { m_fSpeed = speed; }

    float getSpeed() const { return m_fSpeed; }

    void increaseSpeed(float delta) {
        m_fSpeed += delta;
        m_fSpeed = glm::max(m_fSpeed, 0.f);
    }

    float getCameraSpeed() const { return m_fSpeed; }

    bool update(float elapsedTime);


    glm::mat4& getViewMatrix() {
        // m_ViewMatrix = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -m_fHauteur, -m_fDistance));
        m_ViewMatrix = glm::translate(glm::mat4(1.f), glm::vec3(m_fDecalage, -m_fHauteur + m_fHauteur / 2.f, -m_fDistance));
      	m_ViewMatrix = glm::rotate(m_ViewMatrix, m_fAngleX, glm::vec3(0, 1, 0));
      	m_ViewMatrix = glm::rotate(m_ViewMatrix, m_fAngleY, glm::vec3(1, 0, 0));
        m_ViewMatrix = glm::translate(m_ViewMatrix, glm::vec3(m_fDecalage, - m_fHauteur / 2.f, 0.f));

      	return m_ViewMatrix;
    }

    glm::mat4& getRcpViewMatrix() {
        m_RcpViewMatrix = glm::inverse(m_ViewMatrix);
        return m_RcpViewMatrix;
    }

    // Move the camera towards
    //permettant d'avancer / reculer la caméra. Lorsque delta est positif la caméra doit avancer, sinon elle doit reculer.
    void moveFront(float delta)
    {
      if (!zoomMin(delta) && !zoomMax(delta)) {
        m_fDistance += delta;
      }
    }

    void moveCenterX(float delta) {
        m_fDecalage += delta;
    }

    void moveCenterY(float delta) {
        m_fHauteur += delta;
    }

    // Rotate the camera on the X axis
    // permettant de tourner latéralement autour du centre de vision.
    void rotateLeft(float degrees)
    {
    	m_fAngleX += degrees;
    }

    // Rotate the camera on the Y axis
    // permettant de tourner verticalement autour du centre de vision.
    void rotateUp(float degrees)
    {
    	m_fAngleY += degrees;
    }
};
