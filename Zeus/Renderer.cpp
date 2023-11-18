#include "Renderer.h"
#include "Utilities.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void Renderer::OnInit(HWND hwnd)
{
	UINT dxgiFactoryFlags = 0;
	UINT d3d11DeviceFlags = 0;

#ifdef _DEBUG
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

	CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory));

	D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
		NULL, d3d11DeviceFlags,
		NULL, NULL, D3D11_SDK_VERSION,
		&mDevice, nullptr, &mDeviceContext);

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Width = mWindowWidth;
	swapchainDesc.Height = mWindowHeight;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.SampleDesc.Count = 1;

	RECT rect;
	GetClientRect(hwnd, &rect);

	mWindowWidth = rect.right - rect.left;
	mWindowHeight = rect.bottom - rect.top;

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)mWindowWidth;
	viewport.Height = (float)mWindowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	mDeviceContext->RSSetViewports(1, &viewport);

	mFactory->CreateSwapChainForHwnd(mDevice.Get(), hwnd, &swapchainDesc, nullptr, nullptr, &mSwapchain);

	mSwapchain->GetBuffer(0, IID_PPV_ARGS(&mBackBuffer));
	mDevice->CreateRenderTargetView(mBackBuffer.Get(), nullptr, &mBackBufferRTV);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = mWindowWidth;
	desc.Height = mWindowHeight;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = desc.Format;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Format = desc.Format;

	mDevice->CreateTexture2D(&desc, nullptr, mMainRenderTarget.GetAddressOf());
	mDevice->CreateRenderTargetView(mMainRenderTarget.Get(), &rtvDesc, mMainRTV.GetAddressOf());
	mDevice->CreateUnorderedAccessView(mMainRenderTarget.Get(), &uavDesc, mMainUAV.GetAddressOf());

	D3D11_TEXTURE2D_DESC dSDesc{};
	dSDesc.Width = mWindowWidth;
	dSDesc.Height = mWindowHeight;
	dSDesc.MipLevels = 1;
	dSDesc.ArraySize = 1;
	dSDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	dSDesc.SampleDesc.Count = 1;
	dSDesc.SampleDesc.Quality = 0;
	dSDesc.Usage = D3D11_USAGE_DEFAULT;
	dSDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	mDevice->CreateTexture2D(&dSDesc, nullptr, &gDepthBuffer);
	mDevice->CreateDepthStencilView(gDepthBuffer.Get(), &dsvDesc, &gDepthBufferView);
	mDevice->CreateShaderResourceView(gDepthBuffer.Get(), &srvDesc, &gDepthBufferSRV);

	D3D11_TEXTURE2D_DESC copyBufferDesc = {};
	copyBufferDesc.Width = mWindowWidth;
	copyBufferDesc.Height = mWindowHeight;
	copyBufferDesc.MipLevels = 1;
	copyBufferDesc.ArraySize = 1;
	copyBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	copyBufferDesc.SampleDesc.Count = 1;
	copyBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	copyBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	copyBufferDesc.CPUAccessFlags = 0;

	D3D11_UNORDERED_ACCESS_VIEW_DESC copyUAVDesc = {};
	copyUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	copyUAVDesc.Format = copyBufferDesc.Format;

	mDevice->CreateTexture2D(&copyBufferDesc, nullptr, mCopyBuffer.GetAddressOf());
	mDevice->CreateUnorderedAccessView(mCopyBuffer.Get(), &copyUAVDesc, mCopyUAV.GetAddressOf());

	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FillMode = D3D11_FILL_SOLID;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	D3D11_DEPTH_STENCIL_DESC dssDesc = {};
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS;

	mDevice->CreateDepthStencilState(&dssDesc, &mDepthSState);
	mDevice->CreateSamplerState(&samplerDesc, &mSamplerState);
	mDevice->CreateRasterizerState(&rasterDesc, &mRasterState);

	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
	  { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  { "NORMAL",	 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  { "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	auto FVSBytecode = Read("VertexShader.cso");
	auto FPSBytecode = Read("PBR.cso");
	auto TCSBytecode = Read("TonemapCS.cso");

	mDevice->CreateVertexShader(FVSBytecode.data(), FVSBytecode.size(), nullptr, mForwardVS.GetAddressOf());
	mDevice->CreatePixelShader(FPSBytecode.data(), FPSBytecode.size(), nullptr, mForwardPS.GetAddressOf());
	mDevice->CreateComputeShader(TCSBytecode.data(), TCSBytecode.size(), nullptr, mTonemapCS.GetAddressOf());

	mDevice->CreateInputLayout(inputElementDescs, _countof(inputElementDescs),
		FVSBytecode.data(), FVSBytecode.size(), &mInputLayout);

	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = (sizeof(TransformUB) + 255) & ~255;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	mDevice->CreateBuffer(&cbDesc, nullptr, mUniformBuffer.GetAddressOf());

	cbDesc.ByteWidth = (sizeof(MaterialUB) + 255) & ~255;
	mDevice->CreateBuffer(&cbDesc, nullptr, mMaterialUniformBuffer.GetAddressOf());

	cbDesc.ByteWidth = (sizeof(LightingUB) + 255) & ~255;
	mDevice->CreateBuffer(&cbDesc, nullptr, mLightingUniformBuffer.GetAddressOf());

	mCamera = new Camera((float)mWindowWidth, (float)mWindowHeight);

	D3D11_SUBRESOURCE_DATA bufferData = {};
	cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = (sizeof(CameraUB) + 255) & ~255;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferData.pSysMem = mCamera->GetMatrix();

	mDevice->CreateBuffer(&cbDesc, &bufferData, mCameraUniformBuffer.GetAddressOf());
}

void Renderer::OnRender()
{
	mDeviceContext->VSSetShader(mForwardVS.Get(), 0, 0);
	mDeviceContext->PSSetShader(mForwardPS.Get(), 0, 0);

	mDeviceContext->IASetInputLayout(mInputLayout.Get());
	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mDeviceContext->RSSetState(mRasterState.Get());

	mDeviceContext->OMSetRenderTargets(1, mMainRTV.GetAddressOf(), gDepthBufferView.Get());
	mDeviceContext->OMSetDepthStencilState(mDepthSState.Get(), 1);

	const float clearColor[] = { 0.0f, 0.73f, 1.0f, 1.0f };
	mDeviceContext->ClearRenderTargetView(mMainRTV.Get(), clearColor);
	mDeviceContext->ClearDepthStencilView(gDepthBufferView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	mDeviceContext->VSSetConstantBuffers(0, 1, mUniformBuffer.GetAddressOf());
	mDeviceContext->VSSetConstantBuffers(1, 1, mCameraUniformBuffer.GetAddressOf());
	//mDeviceContext->PSSetConstantBuffers(0, 1, mLightingUniformBuffer.GetAddressOf());

	auto view = mRegistry.view<TransformComponent, MeshComponent>();

	for (auto entity : view)
	{
		auto [transform, mesh] = view.get<TransformComponent, MeshComponent>(entity);

		TransformUB data;
		data.Model = transform.GetTransform();
		data.iModel = glm::inverse(transform.GetTransform());

		mDeviceContext->UpdateSubresource(mUniformBuffer.Get(), 0, nullptr, &data, 0, 0);

		if (!Materials[mesh.MaterialID].bDataOnly)
		{
			mDeviceContext->PSSetShaderResources(0, 1, Textures[Materials[mesh.MaterialID].AlbedoMapID].SRV.GetAddressOf());
			mDeviceContext->PSSetShaderResources(1, 1, Textures[Materials[mesh.MaterialID].NormalMapID].SRV.GetAddressOf());
			mDeviceContext->PSSetShaderResources(2, 1, Textures[Materials[mesh.MaterialID].MetalnessMapID].SRV.GetAddressOf());
			mDeviceContext->PSSetShaderResources(3, 1, Textures[Materials[mesh.MaterialID].RoughnessMapID].SRV.GetAddressOf());
		}
		else
		{
			MaterialUB matdata;
			matdata.Albedo = Materials[mesh.MaterialID].Albedo;
			matdata.Metalness = Materials[mesh.MaterialID].Metalness;
			matdata.Roughness = Materials[mesh.MaterialID].Roughness;

			mDeviceContext->UpdateSubresource(mMaterialUniformBuffer.Get(), 0, nullptr, &matdata, 0, 0);
			mDeviceContext->PSSetConstantBuffers(1, 1, mMaterialUniformBuffer.GetAddressOf());
		}

		mDeviceContext->PSSetSamplers(0, 1, mSamplerState.GetAddressOf());

		mDeviceContext->IASetVertexBuffers(0, 1, Meshes[mesh.MeshID].VertexBuffer.GetAddressOf(),
			&Meshes[mesh.MeshID].Stride, &Meshes[mesh.MeshID].Offset);

		mDeviceContext->IASetIndexBuffer(Meshes[mesh.MeshID].IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		mDeviceContext->DrawIndexed(Meshes[mesh.MeshID].IndexCount, 0, 0);
	}

	mDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	mDeviceContext->CSSetShader(mTonemapCS.Get(), nullptr, 0);

	mDeviceContext->CSSetUnorderedAccessViews(0, 1, mMainUAV.GetAddressOf(), 0);
	mDeviceContext->CSSetUnorderedAccessViews(1, 1, mCopyUAV.GetAddressOf(), 0);

	mDeviceContext->Dispatch(mWindowWidth / 8, mWindowHeight / 8, 1);

	mDeviceContext->CopyResource(mBackBuffer.Get(), mCopyBuffer.Get());

	ID3D11UnorderedAccessView* uav_null = NULL;
	mDeviceContext->CSSetUnorderedAccessViews(0, 1, &uav_null, 0);

	mSwapchain->Present(1, 0);
}

void Renderer::OnTick(bool* keys, float xChange, float yChange, float deltaTime)
{
	mCamera->KeyControl(keys, deltaTime);
	mCamera->MouseControl(xChange, yChange);
	mCamera->CalculateViewMatrix();

	D3D11_MAPPED_SUBRESOURCE cbRes;
	mDeviceContext->Map(mCameraUniformBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &cbRes);
	CopyMemory(cbRes.pData, mCamera->GetMatrix(), sizeof(CameraUB));
	mDeviceContext->Unmap(mCameraUniformBuffer.Get(), 0);
}

uint32_t Renderer::CreateMesh(std::string filename)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str());

	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices;
	Mesh nMesh;

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex;

			vertex.Position =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.TexCoord =
			{
					   attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.Normal =
			{
				(attrib.normals[3 * index.normal_index + 0]),
				(attrib.normals[3 * index.normal_index + 1]),
				(attrib.normals[3 * index.normal_index + 2])
			};

			Vertices.push_back(vertex);
			Indices.push_back((uint32_t)Indices.size());
		}
	}

	for (size_t i = 0; i < Vertices.size(); i += 3)
	{
		glm::vec3 v0 = Vertices[i].Position;
		glm::vec3 v1 = Vertices[i + 1].Position;
		glm::vec3 v2 = Vertices[i + 2].Position;

		glm::vec2 uv0 = Vertices[i].TexCoord;
		glm::vec2 uv1 = Vertices[i + 1].TexCoord;
		glm::vec2 uv2 = Vertices[i + 2].TexCoord;

		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

		Vertices[i].Tangent = tangent;
		Vertices[i + 1].Tangent = tangent;
		Vertices[i + 2].Tangent = tangent;

		Vertices[i].BiTangent = bitangent;
		Vertices[i + 1].BiTangent = bitangent;
		Vertices[i + 2].BiTangent = bitangent;
	}

	D3D11_BUFFER_DESC VertexBufferDesc = {};
	VertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	VertexBufferDesc.ByteWidth = sizeof(Vertex) * (UINT)Vertices.size();
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = &Vertices.front();
	vInitData.SysMemPitch = 0;
	vInitData.SysMemSlicePitch = 0;

	mDevice->CreateBuffer(&VertexBufferDesc, &vInitData, nMesh.VertexBuffer.GetAddressOf());

	D3D11_BUFFER_DESC IndexBufferDesc = {};
	IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	IndexBufferDesc.ByteWidth = sizeof(unsigned int) * (UINT)Indices.size();
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = &Indices.front();
	iInitData.SysMemPitch = 0;
	iInitData.SysMemSlicePitch = 0;

	nMesh.IndexCount = (UINT)Indices.size();

	mDevice->CreateBuffer(&IndexBufferDesc, &iInitData, nMesh.IndexBuffer.GetAddressOf());

	Meshes.push_back(nMesh);

	return (uint32_t)Meshes.size() - 1;
}

uint32_t Renderer::CreateTexture(std::string filename)
{
	int TextureWidth;
	int TextureHeight;
	int TextureChannel;
	unsigned char* Pixels = stbi_load(filename.c_str(), &TextureWidth, &TextureHeight, &TextureChannel, STBI_rgb_alpha);

	Texture nTexture;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = TextureWidth;
	desc.Height = TextureHeight;
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	mDevice->CreateTexture2D(&desc, nullptr, nTexture.Texture2D.GetAddressOf());
	mDeviceContext->UpdateSubresource(nTexture.Texture2D.Get(), 0, nullptr, (const void*)Pixels, TextureWidth * 4, 0);

	//stbi_image_free(Pixels);

	mDevice->CreateShaderResourceView(nTexture.Texture2D.Get(), &srvDesc, nTexture.SRV.GetAddressOf());
	mDeviceContext->GenerateMips(nTexture.SRV.Get());

	Textures.push_back(nTexture);
	return (uint32_t)Textures.size() - 1;
}

uint32_t Renderer::CreateMaterial(uint32_t AMID, uint32_t NMID, uint32_t MMID, uint32_t RMID)
{
	Material nMat;
	nMat.AlbedoMapID = AMID;
	nMat.NormalMapID = NMID;
	nMat.MetalnessMapID = MMID;
	nMat.RoughnessMapID = RMID;

	Materials.push_back(nMat);
	return (uint32_t)Materials.size() - 1;
}

uint32_t Renderer::CreateMaterial(glm::vec3 albedo, float metalness, float roughness)
{
	Material nMat;
	nMat.AlbedoMapID = -1;
	nMat.NormalMapID = -1;
	nMat.MetalnessMapID = -1;
	nMat.RoughnessMapID = -1;

	nMat.Albedo = albedo;
	nMat.Metalness = metalness;
	nMat.Roughness = roughness;

	nMat.bDataOnly = true;

	Materials.push_back(nMat);
	return (uint32_t)Materials.size() - 1;
}

entt::entity Renderer::CreateEntity(glm::vec3 pos, uint32_t meshID, uint32_t materialID)
{
	const auto entity = mRegistry.create();
	auto& tcomponent = mRegistry.emplace<TransformComponent>(entity);
	tcomponent.Translation = pos;
	auto& mcomponent = mRegistry.emplace<MeshComponent>(entity);
	mcomponent.MeshID = meshID;
	mcomponent.MaterialID = materialID;
	return entity;
}
