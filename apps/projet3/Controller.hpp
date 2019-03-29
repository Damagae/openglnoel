#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct GLFWwindow;

class Controller {

  private:
    GLFWwindow* m_pWindow = nullptr;
    float m_fSpeed = 0.f;
    bool m_LeftButtonPressed = false;
    glm::dvec2 m_LastCursorPosition;

    glm::mat4 m_ViewMatrix = glm::mat4(1);
    glm::mat4 m_RcpViewMatrix = glm::mat4(1);

    float m_fDistance = 8.f;	// Distance par rapport au centre de la scène
  	float m_fHauteur = 2.f;		// Hauteur de la caméra par rapport au centre de la scène
  	float m_fAngleX = 0.f;		// Angle effectuée par la caméra autour de l'axe X de la scène
  	float m_fAngleY = 0.f;		// Angle effectuée par la caméra autour de l'axe Y de la scène

  	// Returns true if zoom is max/min, false if not
    bool zoomMax() {
    	return (m_fDistance <= 25.0);
    }

    bool zoomMin() {
    	return (m_fDistance >= 50.0);
    }


  public:
    Controller(GLFWwindow* window, float speed = 1.f) :
        m_pWindow(window), m_fSpeed(speed) { }

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
        m_ViewMatrix = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -m_fHauteur + 2.f, -m_fDistance));
      	m_ViewMatrix = glm::rotate(m_ViewMatrix, m_fAngleX, glm::vec3(0, 1, 0));
      	m_ViewMatrix = glm::rotate(m_ViewMatrix, m_fAngleY, glm::vec3(1, 0, 0));
        m_ViewMatrix = glm::translate(m_ViewMatrix, glm::vec3(0.f, - 2.f, 0.f));

      	return m_ViewMatrix;
    }

    const glm::mat4& getRcpViewMatrix() const { return m_RcpViewMatrix; }

    // Move the camera towards
    //permettant d'avancer / reculer la caméra. Lorsque delta est positif la caméra doit avancer, sinon elle doit reculer.
    void moveFront(float delta)
    {
    	m_fDistance += delta;
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