#pragma once

#include <d3d11.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <entt/entt.hpp>

#include "Types.h"
#include "Component.h"
#include "Camera.h"

using Microsoft::WRL::ComPtr;

struct Mesh
{
	ComPtr<ID3D11Buffer> VertexBuffer;
	ComPtr<ID3D11Buffer> IndexBuffer;
	UINT Stride = sizeof(Vertex);
	UINT Offset = 0;
	UINT IndexCount = 0;
};

struct Texture
{
	ComPtr<ID3D11Texture2D> Texture2D;
	ComPtr<ID3D11ShaderResourceView> SRV;
};

struct Material
{
	uint32_t AlbedoMapID;
	uint32_t NormalMapID;
	uint32_t MetalnessMapID;
	uint32_t RoughnessMapID;

	glm::vec3 Albedo = { 1.0f, 1.0f, 1.0f };
	float Metalness = 0;
	float Roughness = 1;
	bool bDataOnly = false;
};

class Renderer
{
public:
	void OnInit(HWND hwnd);
	void OnRender();

	void OnTick(bool* keys, float xChange, float yChange, float deltaTime);

	uint32_t CreateMesh(std::string filename);
	uint32_t CreateTexture(std::string filename);
	uint32_t CreateMaterial(uint32_t AMID, uint32_t NMID, uint32_t MMID, uint32_t RMID);
	uint32_t CreateMaterial(glm::vec3 albedo, float metalness, float roughness);
	entt::entity CreateEntity(glm::vec3 pos, uint32_t meshID, uint32_t materialID);

private:
	int mWindowWidth;
	int mWindowHeight;

	ComPtr<IDXGIFactory4> mFactory;

	ComPtr<ID3D11Device> mDevice;
	ComPtr<ID3D11DeviceContext> mDeviceContext;

	ComPtr<IDXGISwapChain1> mSwapchain;
	ComPtr<ID3D11Texture2D> mBackBuffer;
	ComPtr<ID3D11RenderTargetView> mBackBufferRTV;

	ComPtr<ID3D11Texture2D> mMainRenderTarget;
	ComPtr<ID3D11RenderTargetView> mMainRTV;
	ComPtr<ID3D11UnorderedAccessView> mMainUAV;

	ComPtr<ID3D11Texture2D> gDepthBuffer;
	ComPtr<ID3D11DepthStencilView> gDepthBufferView;
	ComPtr<ID3D11ShaderResourceView> gDepthBufferSRV;

	ComPtr<ID3D11Texture2D> mCopyBuffer;
	ComPtr<ID3D11UnorderedAccessView> mCopyUAV;

	ComPtr<ID3D11InputLayout> mInputLayout;
	ComPtr<ID3D11SamplerState> mSamplerState;
	ComPtr<ID3D11RasterizerState> mRasterState;
	ComPtr<ID3D11DepthStencilState> mDepthSState;

	ComPtr<ID3D11VertexShader> mForwardVS;
	ComPtr<ID3D11PixelShader> mForwardPS;
	ComPtr<ID3D11ComputeShader> mTonemapCS;

	ComPtr<ID3D11Buffer> mUniformBuffer;
	ComPtr<ID3D11Buffer> mCameraUniformBuffer;
	ComPtr<ID3D11Buffer> mMaterialUniformBuffer;
	ComPtr<ID3D11Buffer> mLightingUniformBuffer;

	Camera* mCamera;
	std::vector<Mesh> Meshes;
	std::vector<Texture> Textures;
	std::vector<Material> Materials;
	entt::registry mRegistry;
};

