#include "Camera.h"
#include "KeyCodes.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float WindowWidth, float WindowHeight)
{
	position = glm::vec3(4.0f, 0.0f, 0.0f);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	yaw = 180.0f;
	pitch = 0.0f;
	front = glm::vec3(0.0f, 0.0f, -1.0f);
	moveSpeed = 5.0f;
	turnSpeed = 0.2f;

	proj = glm::perspectiveFovRH_ZO(glm::radians(45.0f), WindowWidth, WindowHeight, 0.1f, 1000.0f);

	Update();
}

void Camera::KeyControl(bool* keys, float deltaTime)
{
	float velocity = moveSpeed * deltaTime;

	if (keys[VK_KEY_W])
	{
		position += front * velocity;
	}

	if (keys[VK_KEY_S])
	{
		position -= front * velocity;
	}

	if (keys[VK_KEY_A])
	{
		position -= right * velocity;
	}

	if (keys[VK_KEY_D])
	{
		position += right * velocity;
	}
}

void Camera::MouseControl(float xChange, float yChange)
{
	xChange *= turnSpeed;
	yChange *= turnSpeed;

	yaw += xChange;
	pitch += yChange;

	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}

	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}

	Update();
}

void Camera::CalculateViewMatrix()
{
	view = glm::lookAtRH(position, position + front, up);
	mData.Eye = position;
	mData.ViewProj = proj * view;
}

void Camera::Update()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}
