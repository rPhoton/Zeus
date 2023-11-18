#pragma once

#include "Types.h"

class Camera
{
public:
	Camera(float WindowWidth, float WindowHeight);

	void KeyControl(bool* keys, float deltaTime);
	void MouseControl(float xChange, float yChange);

	void CalculateViewMatrix();

	CameraUB* GetMatrix() { return &mData; }

protected:
	void Update();

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	glm::mat4 view;
	glm::mat4 proj;

	CameraUB mData;

	float yaw;
	float pitch;

	float moveSpeed;
	float turnSpeed;
};

