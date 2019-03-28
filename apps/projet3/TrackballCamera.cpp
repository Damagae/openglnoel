#include "TrackballCamera.hpp"

// #include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <iostream>

// Move the camera towards
void TrackballCamera::moveFront(float delta)
{
	m_fDistance += delta;
}
// Rotate the camera on the X axis
void TrackballCamera::rotateLeft(float degrees)
{
	m_fAngleX += degrees;
}
// Rotate the camera on the Y axis
void TrackballCamera::rotateUp(float degrees)
{
	m_fAngleY += degrees;
}

// Returns the ViewMatrix of the camera, called each frame
glm::mat4 TrackballCamera::getViewMatrix() const
{
	// MODIFIER CETTE LIGNE POUR FOCUS SUR PACMAN : 1er parametre du glm::vec3
	glm::mat4 MVMatrix = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -m_fHauteur, -m_fDistance));
	MVMatrix = glm::rotate(MVMatrix, m_fAngleX, glm::vec3(0, 1, 0));
	MVMatrix = glm::rotate(MVMatrix, m_fAngleY, glm::vec3(1, 0, 0));

	return MVMatrix;
}

glm::mat4 TrackballCamera::getViewMatrix(glm::mat4 matrix) const
{
	// MODIFIER CETTE LIGNE POUR FOCUS SUR PACMAN : 1er parametre du glm::vec3

	glm::mat4 MVMatrix = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -m_fHauteur, -m_fDistance));
	MVMatrix = glm::rotate(MVMatrix, m_fAngleX, glm::vec3(0, 1, 0));
	MVMatrix = glm::rotate(MVMatrix, m_fAngleY, glm::vec3(1, 0, 0));

	return MVMatrix;
}

// Tells if we can zoom or not
bool TrackballCamera::zoomMax()
{
	if (m_fDistance <= 25.0)
		return true;
	else
		return false;
}
bool TrackballCamera::zoomMin()
{
	if (m_fDistance >= 50.0)
		return true;
	else
		return false;
}
