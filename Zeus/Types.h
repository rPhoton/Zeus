#pragma once

#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 Position;
	glm::vec2 TexCoord;
	glm::vec3 Normal;
	glm::vec3 Tangent;
	glm::vec3 BiTangent;
};

struct CameraUB
{
	glm::mat4 ViewProj;
	glm::vec3 Eye;
};

struct TransformUB
{
	glm::mat4 Model;
	glm::mat4 iModel;
};

struct MaterialUB
{
	glm::vec3 Albedo;
	float Metalness;
	float Roughness;
};

struct DirectionalLight
{
	glm::vec3 Position;
	glm::vec3 Color;
	float Intensity;
};

struct PointLight
{
	glm::vec3 Position;
	glm::vec3 Color;
	float Intensity;
	float Radius;
};

struct LightingUB
{
	DirectionalLight DirectionalLights[1];
	PointLight PointLights[4];
};