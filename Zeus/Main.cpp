#define WIN32_LEAN_AND_MEAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <Windows.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Renderer.h"

bool Keys[1024];

double lastX;
double lastY;

double xChange;
double yChange;

bool mouseFirstMoved = true;

double deltaTime = 0.0f;
double lastTime = 0.0f;

void HandleKeys(GLFWwindow* window, int Key, int Code, int Action, int Mode);
void HandleMouse(GLFWwindow* window, double xPos, double yPos);

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	const int WIDTH = 1280;
	const int HEIGHT = 720;

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Zeus", nullptr, nullptr);

	glfwSetKeyCallback(window, HandleKeys);
	glfwSetCursorPosCallback(window, HandleMouse);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	Renderer renderer;
	renderer.OnInit(glfwGetWin32Window(window));

	int sphere = renderer.CreateMesh("Sphere.obj");

	int nrRows = 7;
	int nrColumns = 7;
	float spacing = 2.5;

	for (float i = 0; i < nrRows; ++i)
	{
		for (float j = 0; j < nrColumns; ++j)
		{
			int mat = renderer.CreateMaterial(glm::vec3(0.5, 0, 0), i / nrRows, glm::clamp(j / nrColumns, 0.05f, 1.0f));
			renderer.CreateEntity(glm::vec3(0.0f, (j - (nrColumns / 2)) * spacing, (i - (nrRows / 2)) * spacing), sphere, mat);
		}
	}

	while (!glfwWindowShouldClose(window))
	{
		double now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		renderer.OnTick(Keys, (float) xChange, (float) yChange, (float) deltaTime);

		xChange = 0.0f;
		yChange = 0.0f;

		renderer.OnRender();

		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void HandleKeys(GLFWwindow* window, int Key, int Code, int Action, int Mode)
{
	if (Key == GLFW_KEY_ESCAPE && Action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (Key >= 0 && Key < 1024)
	{
		if (Action == GLFW_PRESS)
		{
			Keys[Key] = true;
		}
		else if (Action == GLFW_RELEASE)
		{
			Keys[Key] = false;
		}
	}
}

void HandleMouse(GLFWwindow* window, double xPos, double yPos)
{
	if (mouseFirstMoved)
	{
		lastX = xPos;
		lastY = yPos;
		mouseFirstMoved = false;
	}

	xChange = xPos - lastX;
	yChange = lastY - yPos;

	lastX = xPos;
	lastY = yPos;
}