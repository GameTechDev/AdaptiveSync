//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "D3D12Fullscreen.h"
#include <DirectXColors.h>
#include<DirectXColors.h>
#include<chrono>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"


double PCFreq = 0.0;
__int64 CounterStart = 0;
using namespace DirectX;

const float D3D12Fullscreen::QuadWidth = 0.0f;
const float D3D12Fullscreen::QuadHeight = 1200.0f;
const float D3D12Fullscreen::LetterboxColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
const float D3D12Fullscreen::ClearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };

const D3D12Fullscreen::Resolution D3D12Fullscreen::m_resolutionOptions[] =
{


	{ 2560u, 1440u },
   { 2560u, 1440u },
	{ 2560u, 1440u }

};
const UINT D3D12Fullscreen::m_resolutionOptionsCount = _countof(m_resolutionOptions);
UINT D3D12Fullscreen::m_resolutionIndex = 2;

D3D12Fullscreen::D3D12Fullscreen(UINT width, UINT height, std::wstring name) :
	DXSample(width, height, name),
	m_frameIndex(0),
	m_pCbvDataBegin(nullptr),
	m_sceneViewport(0.0f, 0.0f, 0.0f, 0.0f),
	m_sceneScissorRect(0, 0, 0, 0),
	m_postViewport(0.0f, 0.0f, 0.0f, 0.0f),
	m_postScissorRect(0, 0, 0, 0),
	m_rtvDescriptorSize(0),
	m_cbvSrvDescriptorSize(0),
	m_windowVisible(true),
	m_windowedMode(true),
	m_sceneConstantBufferData{},
	m_fenceValues{}
{
}

void D3D12Fullscreen::OnInit()
{
	LoadPipeline();

	LoadAssets();
	GetInitialTime();
	LoadImGui();
	LoadAdaptive();

}

void D3D12Fullscreen::LoadAdaptive() {
	ComPtr<IDXGIAdapter1> dxgiAdapter;
	ThrowIfFailed(factory->EnumAdapters1(0, &dxgiAdapter));
	UINT i = 0;
	DXGI_ADAPTER_DESC desc;
	dxgiAdapter->GetDesc(&desc);
	Intel_device = desc.VendorId == 32902;

	if (Intel_device) {
		// This has been added to the fullscreen sample. This calls the library to see adaptive sync information.
		adaptive_sync = AdaptiveSync.IsAdaptiveSyncEnabled();
		max_adaptive_refresh_rate = AdaptiveSync.AdaptiveSyncMaxRefresh();
		min_adaptive_refresh_rate = AdaptiveSync.AdaptiveSyncMinRefresh();
		adaptive_sync_supported = AdaptiveSync.AdaptiveSyncSupported();
	}
}



//Code from Errin (https://software.intel.com/en-us/blogs/2019/06/12/detect-and-enable-hdr-with-microsoft-directx-11-and-directx-12)
void D3D12Fullscreen::HDR10DisplaySetup() {

	ComPtr<IDXGIAdapter1> dxgiAdapter;
	ThrowIfFailed(factory->EnumAdapters1(0, &dxgiAdapter));
	UINT i = 0;

	ComPtr<IDXGIOutput> currentoutput = nullptr;
	ComPtr<IDXGIOutput> bestOutput = nullptr;
	float bestIntersectionArea = -1;

	while (dxgiAdapter->EnumOutputs(i, &currentoutput) != DXGI_ERROR_NOT_FOUND)
	{
		//Get the rectangle bounds of the app window
		int ax1 = m_windowBounds.left;
		int ay1 = m_windowBounds.top;
		int ax2 = m_windowBounds.right;
		int ay2 = m_windowBounds.bottom;

		//get the rectangle bounds of current output
		DXGI_OUTPUT_DESC desc;
		DXGI_OUTPUT_DESC1 desc2;
		ThrowIfFailed(currentoutput->GetDesc(&desc));

		ThrowIfFailed(currentoutput.As(&output6));
		ThrowIfFailed(output6->GetDesc1(&desc2));

		RECT r = desc.DesktopCoordinates;
		int bx1 = r.left;
		int by1 = r.top;
		int bx2 = r.right;
		int by2 = r.bottom;

		//compute intersection
		int intersectionArea = max(0, min(ax2, bx2) - max(ax1, bx1)) * max(0, min(ay1, by2) - max(ay1, by1));

		if (intersectionArea > bestIntersectionArea)
		{
			bestOutput = currentoutput;
			bestIntersectionArea = static_cast<float>(intersectionArea);
		}
		i++;
	}

	ThrowIfFailed(bestOutput.As(&output6));
	ThrowIfFailed(output6->GetDesc1(&desc1));

}

// code added to check for HDR support
bool D3D12Fullscreen::DisplayCanSupportHDR(DXGI_OUTPUT_DESC1 desc)
{
	
	return (desc.MaxLuminance >= 1000 && desc.MinLuminance <= 0.05) ||
		(desc.MaxLuminance >= 500 && desc.MinLuminance <= 0.0005);
}

//code added to set HDR
void D3D12Fullscreen::SetHDRMetaData()
{

	//check for hdr support before cofiguring swap chain desc
	HDR10DisplaySetup();
	bool supportHDR = DisplayCanSupportHDR(desc1);

	if (!m_swapChain)
		return;

	if (!supportHDR && enable_hdr)
	{
		DebugString = "HDR is not supported on this device";
		return;
	}

	if (!supportHDR || !enable_hdr)
	{
		ThrowIfFailed(m_swapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_NONE, 0, nullptr));
		enable_hdr = false;

		DebugString = "HDR Disabled";
		return;
	}


	if (supportHDR && enable_hdr)
	{
		DXGI_HDR_METADATA_HDR10 HDR10MetaData = {};
		HDR10MetaData.MaxMasteringLuminance = static_cast<UINT>(desc1.MaxLuminance * 10000.0f);
		HDR10MetaData.MinMasteringLuminance = static_cast<UINT>(desc1.MinLuminance * 10000.0f);
		HDR10MetaData.MaxContentLightLevel = static_cast<UINT16>(desc1.MaxLuminance);
		HDR10MetaData.MaxFrameAverageLightLevel = static_cast<UINT16>(desc1.MaxFullFrameLuminance);

		ThrowIfFailed(m_swapChain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(DXGI_HDR_METADATA_HDR10), &HDR10MetaData));
		DebugString = "HDR Enabled";
	}

}


//code added to implement ImGui for demo
void D3D12Fullscreen::LoadImGui()
{
#ifdef IMGUI_ENABLED
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpu(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpu(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos + ImGuiBackendFlags_HasSetMousePos;  // Enable Keyboard Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();

	ImGui::StyleColorsClassic();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(Win32Application::GetHwnd());
	ImGui_ImplDX12_Init(m_device.Get(), FrameCount,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		cpu.Offset(Descriptors::Imgui, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)),
		gpu.Offset(Descriptors::Imgui, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));

#endif //IMGUI_ENABLED
}


// Load the rendering pipeline dependencies.
void D3D12Fullscreen::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	//ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	NAME_D3D12_OBJECT(m_commandQueue);


	//initialize HDR if supported, this was added for the adatpive_sync_demo
	HDR10DisplaySetup();
	enable_hdr = DisplayCanSupportHDR(desc1);
	if (enable_hdr)
		SetHDRMetaData();


	// Describe and create the swap chain.
	// The resolution of the swap chain buffers will match the resolution of the window, enabling the
	// app to enter i  when in fullscreen mode. We will also keep a separate buffer that is not part
	// of the swap chain as an intermediate render target, whose resolution will control the rendering
	// resolution of the scene.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	// It is recommended to always use the tearing flag when it is available.
	swapChainDesc.Flags = m_tearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	if (m_tearingSupport)
	{
		// When tearing support is enabled we will handle ALT+Enter key presses in the
		// window message loop rather than let DXGI handle it by calling SetFullscreenState.
		factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER);
	}

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount + 1; // + 1 for the intermediate render target.
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));


		// Describe and create a constant buffer view (CBV) and shader resource view (SRV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
		cbvSrvHeapDesc.NumDescriptors = 7; // One CBV per frame and one SRV for the intermediate render target.
		cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_cbvSrvHeap)));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	// Create command allocators for each frame.
	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_sceneCommandAllocators[n])));
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_postCommandAllocators[n])));
	}
}






// Load the sample assets.
void D3D12Fullscreen::LoadAssets()
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// Create a root signature consisting of a descriptor table with a single CBV.
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_sceneRootSignature)));
		NAME_D3D12_OBJECT(m_sceneRootSignature);
	}

	// Create a root signature consisting of a descriptor table with a SRV and a sampler.
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];

		// We don't modify the SRV in the post-processing command list after
		// SetGraphicsRootDescriptorTable is executed on the GPU so we can use the default
		// range behavior: D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		// Allow input layout and pixel shader access and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		// Create a sampler.
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_postRootSignature)));
		NAME_D3D12_OBJECT(m_postRootSignature);
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> sceneVertexShader;
		ComPtr<ID3DBlob> scenePixelShader;
		ComPtr<ID3DBlob> postVertexShader;
		ComPtr<ID3DBlob> postPixelShader;
		ComPtr<ID3DBlob> error;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"sceneShaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &sceneVertexShader, &error));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"sceneShaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &scenePixelShader, &error));

		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"postShaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &postVertexShader, &error));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"postShaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &postPixelShader, &error));

		// Define the vertex input layouts.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		D3D12_INPUT_ELEMENT_DESC scaleInputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state objects (PSOs).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_sceneRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(sceneVertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(scenePixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_scenePipelineState)));
		NAME_D3D12_OBJECT(m_scenePipelineState);

		psoDesc.InputLayout = { scaleInputElementDescs, _countof(scaleInputElementDescs) };
		psoDesc.pRootSignature = m_postRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(postVertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(postPixelShader.Get());

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_postPipelineState)));
		NAME_D3D12_OBJECT(m_postPipelineState);
	}

	// Single-use command allocator and command list for creating resources.
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;

	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

	// Create the command lists.
	{
		ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_sceneCommandAllocators[m_frameIndex].Get(), m_scenePipelineState.Get(), IID_PPV_ARGS(&m_sceneCommandList)));
		NAME_D3D12_OBJECT(m_sceneCommandList);

		ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_postCommandAllocators[m_frameIndex].Get(), m_postPipelineState.Get(), IID_PPV_ARGS(&m_postCommandList)));
		NAME_D3D12_OBJECT(m_postCommandList);

		// Close the command lists.
		ThrowIfFailed(m_sceneCommandList->Close());
		ThrowIfFailed(m_postCommandList->Close());
	}

	LoadSizeDependentResources();
	LoadSceneResolutionDependentResources();

	// Create/update the vertex buffer.
	ComPtr<ID3D12Resource> sceneVertexBufferUpload;
	{
		// Define the geometry for a thin quad that will animate across the screen.
		const float x = QuadWidth / 2.0f;
		const float y = QuadHeight / 2.0f;

		//code was modified to go from a rectangle to a triangle for the adatpive_sync_demo
		SceneVertex quadVertices[] =
		{
			{ { -x, -y, 1.0f }, XMFLOAT4(Colors::Yellow) },
			{ { -x - 300, y, 1.0f }, XMFLOAT4(Colors::Yellow) },
			{ { x, -y, 1.0f }, XMFLOAT4(Colors::Yellow) },
			{ { x + 300, y, 1.0f }, XMFLOAT4(Colors::Yellow) }
		};

		const UINT vertexBufferSize = sizeof(quadVertices);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_sceneVertexBuffer)));

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&sceneVertexBufferUpload)));

		NAME_D3D12_OBJECT(m_sceneVertexBuffer);

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(sceneVertexBufferUpload->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, quadVertices, sizeof(quadVertices));
		sceneVertexBufferUpload->Unmap(0, nullptr);

		commandList->CopyBufferRegion(m_sceneVertexBuffer.Get(), 0, sceneVertexBufferUpload.Get(), 0, vertexBufferSize);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_sceneVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Initialize the vertex buffer views.
		m_sceneVertexBufferView.BufferLocation = m_sceneVertexBuffer->GetGPUVirtualAddress();
		m_sceneVertexBufferView.StrideInBytes = sizeof(SceneVertex);
		m_sceneVertexBufferView.SizeInBytes = vertexBufferSize;
	}

	// Create/update the fullscreen quad vertex buffer.
	ComPtr<ID3D12Resource> postVertexBufferUpload;
	{
		// Define the geometry for a fullscreen quad.
		PostVertex quadVertices[] =
		{
			{ { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },    // Bottom left.
			{ { -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },    // Top left.
			{ { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },    // Bottom right.
			{ { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } }        // Top right.
		};

		const UINT vertexBufferSize = sizeof(quadVertices);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_postVertexBuffer)));

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&postVertexBufferUpload)));

		NAME_D3D12_OBJECT(m_postVertexBuffer);

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(postVertexBufferUpload->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, quadVertices, sizeof(quadVertices));
		postVertexBufferUpload->Unmap(0, nullptr);

		commandList->CopyBufferRegion(m_postVertexBuffer.Get(), 0, postVertexBufferUpload.Get(), 0, vertexBufferSize);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_postVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Initialize the vertex buffer views.
		m_postVertexBufferView.BufferLocation = m_postVertexBuffer->GetGPUVirtualAddress();
		m_postVertexBufferView.StrideInBytes = sizeof(PostVertex);
		m_postVertexBufferView.SizeInBytes = vertexBufferSize;
	}

	// Create the constant buffer.
	{
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(SceneConstantBuffer) * FrameCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_sceneConstantBuffer)));

		NAME_D3D12_OBJECT(m_sceneConstantBuffer);

		// Describe and create constant buffer views.
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_sceneConstantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = sizeof(SceneConstantBuffer);

		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart(), 1, m_cbvSrvDescriptorSize);

		for (UINT n = 0; n < FrameCount; n++)
		{
			m_device->CreateConstantBufferView(&cbvDesc, cpuHandle);

			cbvDesc.BufferLocation += sizeof(SceneConstantBuffer);
			cpuHandle.Offset(m_cbvSrvDescriptorSize);
			m_device->CreateShaderResourceView(m_intermediateRenderTarget.Get(), nullptr, m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
		}

		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_sceneConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
		memcpy(m_pCbvDataBegin, &m_sceneConstantBufferData, sizeof(m_sceneConstantBufferData));
	}

	// Close the resource creation command list and execute it to begin the vertex buffer copy into
	// the default heap.
	ThrowIfFailed(commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValues[m_frameIndex]++;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute before continuing.
		WaitForGpu();
	}
}

void D3D12Fullscreen::LoadSizeDependentResources()
{
	UpdatePostViewAndScissor();

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);

			NAME_D3D12_OBJECT_INDEXED(m_renderTargets, n);
		}
	}


}

// Update frame-based values.
void D3D12Fullscreen::OnUpdate()
{

	//how long has it been since the last frame? use this to determine how many pixels the triangle should move
	//code was modified for the adatpive_sync_demo
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	double elapsed = (current_time.QuadPart - speed_time.QuadPart)/10000000000.0;
	speed_time = current_time;
	float translationSpeed = (float)(speed*elapsed);
	const float offsetBounds = 1.0f;

	//code added to implement ImGi for adatpive_sync_demo
#ifdef IMGUI_ENABLED
	if (1)
	{
		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		bool last_hdr = enable_hdr;
		bool last_adapt = adaptive_sync;
		ImVec2 v(0, 0);
		ImVec2 s(600.0f, 200.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);


		
		if (adaptive_sync_supported)
			vrrsupport = "Supported";
		else
			vrrsupport = "Not Supported";
			


		// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		ImGui::Begin("Adaptive Sync Control", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar); // Create a window called "Hello, world!" and append into it.
		ImGui::SetWindowPos(v);
		// ImGui::SetWindowSize(s);
		if(m_windowedMode)
			ImGui::SetWindowFontScale(1);
		else
			ImGui::SetWindowFontScale(2);


		ImGui::SliderFloat("Speed (Pixels/Second) ", &speed, 10.0f, 2000.0f);
		ImGui::SliderInt("Render Delay (ms)", &delay, 0, 30);
		ImGui::Checkbox("Vsync Enabled", &v_sync);
		ImGui::Checkbox("HDR Enabled", &enable_hdr);
		
		ImGui::Checkbox("Adaptive Sync Enabled", &adaptive_sync);
		ImGui::Text("Adaptive Refresh:  Support: %s  \nMax Refresh Rate: %i Min Refresh Rate: %i", vrrsupport, max_adaptive_refresh_rate, min_adaptive_refresh_rate);
		
		if (last_hdr != enable_hdr)
			SetHDRMetaData();

		if (last_adapt != adaptive_sync && Intel_device && adaptive_sync_supported)
		{
			if (adaptive_sync)
				AdaptiveSync.EnableAdaptiveSync();
			else
				AdaptiveSync.DisableAdaptiveSync();
		}
		else if (adaptive_sync && (!Intel_device || !adaptive_sync_supported))
		{
			adaptive_sync = false;
			DebugString = "Intel Adaptive Sync Not Supported";
		}


		ImGui::Text("Frame Rate: %.2f FPS", Frame_Rate);
		ImGui::Text("Time Per Frame: %.2f ms", (1 / Frame_Rate) * 1000);
		ImGui::Text(DebugString.c_str());


		ImGui::End();
		ImGui::PopStyleVar();


	}
#endif //IMGUI_ENABLED


	m_sceneConstantBufferData.offset.x += translationSpeed;
	if (m_sceneConstantBufferData.offset.x > offsetBounds)
	{
		m_sceneConstantBufferData.offset.x = -offsetBounds;
	}

	XMMATRIX transform = XMMatrixMultiply(
		XMMatrixOrthographicLH(static_cast<float>(m_resolutionOptions[m_resolutionIndex].Width), static_cast<float>(m_resolutionOptions[m_resolutionIndex].Height), 0.0f, 100.0f),
		XMMatrixTranslation(m_sceneConstantBufferData.offset.x, 0.0f, 0.0f));

	XMStoreFloat4x4(&m_sceneConstantBufferData.transform, XMMatrixTranspose(transform));

	UINT offset = m_frameIndex * sizeof(SceneConstantBuffer);
	memcpy(m_pCbvDataBegin + offset, &m_sceneConstantBufferData, sizeof(m_sceneConstantBufferData));
}


//function added for timings in the adatpive_sync_demo
void D3D12Fullscreen::GetInitialTime()
{
	QueryPerformanceCounter(&start_time);
	QueryPerformanceCounter(&speed_time);
	
}

//funtion added for FPS calculations for the adatpive_sync_demo
void D3D12Fullscreen::UpdateFrameRate()
{
	static int num = 120;
	LARGE_INTEGER current_time;
	if (num < 60)
		num++;
	else
	{
		num = 0;
		QueryPerformanceCounter(&current_time);
		Frame_Rate = 60.0/((current_time.QuadPart - start_time.QuadPart)/10000000.0);
		start_time = current_time;
	}
}


// Render the scene.
void D3D12Fullscreen::OnRender()
{
	if (m_windowVisible)
	{

		PIXBeginEvent(m_commandQueue.Get(), 0, L"Render");

		// Record all the commands we need to render the scene into the command lists.
		PopulateCommandLists();

		// Execute the command lists.
		ID3D12CommandList* ppCommandLists[] = { m_sceneCommandList.Get(), m_postCommandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		PIXEndEvent(m_commandQueue.Get());

		// When using sync interval 0, it is recommended to always pass the tearing
		// flag when it is supported, even when presenting in windowed mode.
		// flag when it is supported, even when presenting in windowed mode.
		// However, this flag cannot be used if the app is in fullscreen mode as a
		// result of calling SetFullscreenState.
		UINT presentFlags = (m_tearingSupport && m_windowedMode && !v_sync) ? DXGI_PRESENT_ALLOW_TEARING : 0;


		UpdateFrameRate();
		Sleep(delay);

		/*
		To  enable adaptive sync, the following funcion call is esential. The sync interval should be 0, when the interval is 1-4, vsync will be enabled and the frames will wait to be sent rather than sending immediately
		The code has been modified for the adatpive_sync_demo, when vsync is disabled, interval = 0, otherwise interval = 1
		*/

		ThrowIfFailed(m_swapChain->Present(interval, presentFlags));


		MoveToNextFrame();
	}
}

void D3D12Fullscreen::OnSizeChanged(UINT width, UINT height, bool minimized)
{
	// Determine if the swap buffers and other resources need to be resized or not.
	if ((width != m_width || height != m_height) && !minimized)
	{
		// Flush all current GPU commands.
		WaitForGpu();

		// Release the resources holding references to the swap chain (requirement of
		// IDXGISwapChain::ResizeBuffers) and reset the frame fence values to the
		// current fence value.
		for (UINT n = 0; n < FrameCount; n++)
		{
			m_renderTargets[n].Reset();
			m_fenceValues[n] = m_fenceValues[m_frameIndex];
		}

		// Resize the swap chain to the desired dimensions.
		DXGI_SWAP_CHAIN_DESC desc = {};
		m_swapChain->GetDesc(&desc);
		ThrowIfFailed(m_swapChain->ResizeBuffers(FrameCount, width, height, desc.BufferDesc.Format, desc.Flags));


		BOOL fullscreenState;
		ThrowIfFailed(m_swapChain->GetFullscreenState(&fullscreenState, nullptr));
		m_windowedMode = !fullscreenState;

		// Reset the frame index to the current back buffer index.
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Update the width, height, and aspect ratio member variables.
		UpdateForSizeChange(width, height);

		LoadSizeDependentResources();
	}

	m_windowVisible = !minimized;
}

void D3D12Fullscreen::OnDestroy()
{
	//code added for ImGui in adatpive_sync_demo
#ifdef IMGUI_ENABLED
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif //IMGUI_ENABLED


	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForGpu();


	if (!m_tearingSupport)
	{
		// Fullscreen state should always be false before exiting the app.
		ThrowIfFailed(m_swapChain->SetFullscreenState(FALSE, nullptr));
	}

	CloseHandle(m_fenceEvent);
}


// code was modified for adatpive_sync_demo, the keys can be used to control the triangle along with ImGui
void D3D12Fullscreen::OnKeyDown(UINT8 key)
{
	switch (key)
	{

		// Instrument the Space Bar to toggle between fullscreen states.
		// The window message loop callback will receive a WM_SIZE message once the
		// window is in the fullscreen state. At that point, the IDXGISwapChain should
		// be resized to match the new window size.
		//
		// NOTE: ALT+Enter will perform a similar operation; the code below is not
		// required to enable that key combination.
	case VK_SPACE:
	{
		if (m_tearingSupport)
		{
			Win32Application::ToggleFullscreenWindow();
		}
		else
		{
			BOOL fullscreenState;
			ThrowIfFailed(m_swapChain->GetFullscreenState(&fullscreenState, nullptr));
			if (FAILED(m_swapChain->SetFullscreenState(!fullscreenState, nullptr)))
			{
				// Transitions to fullscreen mode can fail when running apps over
				// terminal services or for some other unexpected reason.  Consider
				// notifying the user in some way when this happens.
				assert(false);
			}
		}
		break;
	}

	// Instrument the Right Arrow key to change the scene rendering resolution 
	// to the next resolution option. 
	case VK_RIGHT:
	{
		speed += 10;

	}
	break;

	// Instrument the Left Arrow key to change the scene rendering resolution 
	// to the previous resolution option.
	case VK_LEFT:
	{
		if (speed > 0)
			speed -= 10;

	}
	break;
	case VK_UP:
	{
		delay += 1;

	}
	break;
	case VK_DOWN:
	{
		delay -= 1;

	}
	case 86:
	{
		v_sync = !v_sync;
		if (interval)
			interval = 0;
		else
			interval = 1;
	}


	}
}

// Fill the command list with all the render commands and dependent state.
void D3D12Fullscreen::PopulateCommandLists()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_sceneCommandAllocators[m_frameIndex]->Reset());
	ThrowIfFailed(m_postCommandAllocators[m_frameIndex]->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_sceneCommandList->Reset(m_sceneCommandAllocators[m_frameIndex].Get(), m_scenePipelineState.Get()));
	ThrowIfFailed(m_postCommandList->Reset(m_postCommandAllocators[m_frameIndex].Get(), m_postPipelineState.Get()));

	// Populate m_sceneCommandList to render scene to intermediate render target.
	{

		// Set necessary state.
		m_sceneCommandList->SetGraphicsRootSignature(m_sceneRootSignature.Get());

		ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get() };
		m_sceneCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart(), m_frameIndex + 1, m_cbvSrvDescriptorSize);
		m_sceneCommandList->SetGraphicsRootDescriptorTable(0, cbvHandle);
		m_sceneCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_sceneCommandList->RSSetViewports(1, &m_sceneViewport);
		m_sceneCommandList->RSSetScissorRects(1, &m_sceneScissorRect);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), FrameCount, m_rtvDescriptorSize);
		m_sceneCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		// Record commands.



		m_sceneCommandList->ClearRenderTargetView(rtvHandle, Colors::Black, 0, nullptr);

		m_sceneCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_sceneCommandList->IASetVertexBuffers(0, 1, &m_sceneVertexBufferView);

		PIXBeginEvent(m_sceneCommandList.Get(), 0, L"Draw a triangle rectangle");
		m_sceneCommandList->DrawInstanced(4, 1, 0, 0);

		PIXEndEvent(m_sceneCommandList.Get());




	}


	ThrowIfFailed(m_sceneCommandList->Close());


	// Populate m_postCommandList to scale intermediate render target to screen.
	{
		// Set necessary state.
		m_postCommandList->SetGraphicsRootSignature(m_postRootSignature.Get());

		ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get() };
		m_postCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		// Indicate that the back buffer will be used as a render target and the
		// intermediate render target will be used as a SRV.
		D3D12_RESOURCE_BARRIER barriers[] = {
			CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
			CD3DX12_RESOURCE_BARRIER::Transition(m_intermediateRenderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		};

		m_postCommandList->ResourceBarrier(_countof(barriers), barriers);

		m_postCommandList->SetGraphicsRootDescriptorTable(0, m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());
		m_postCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_postCommandList->RSSetViewports(1, &m_postViewport);
		m_postCommandList->RSSetScissorRects(1, &m_postScissorRect);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
		m_postCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		// Record commands.
		m_postCommandList->ClearRenderTargetView(rtvHandle, Colors::Black, 0, nullptr);
		m_postCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_postCommandList->IASetVertexBuffers(0, 1, &m_postVertexBufferView);

		PIXBeginEvent(m_postCommandList.Get(), 0, L"Draw texture to screen.");
		m_postCommandList->DrawInstanced(4, 1, 0, 0);
		PIXEndEvent(m_postCommandList.Get());

		//code added for adatpive_sync_demo ImGui
#ifdef IMGUI_ENABLED
		if (1)
		{
			ImGui::Render();
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_postCommandList.Get());
		}
#endif
		// Revert resource states back to original values.
		barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		m_postCommandList->ResourceBarrier(_countof(barriers), barriers);




	}




	ThrowIfFailed(m_postCommandList->Close());

}

// Wait for pending GPU work to complete.
void D3D12Fullscreen::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]));

	// Wait until the fence has been processed.
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValues[m_frameIndex]++;
}

// Prepare to render the next frame.
void D3D12Fullscreen::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

	// Update the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

// Set up the screen viewport and scissor rect to match the current window size and scene rendering resolution.
void D3D12Fullscreen::UpdatePostViewAndScissor()
{
	float viewWidthRatio = static_cast<float>(m_resolutionOptions[m_resolutionIndex].Width) / m_width;
	float viewHeightRatio = static_cast<float>(m_resolutionOptions[m_resolutionIndex].Height) / m_height;

	float x = 1.0f;
	float y = 1.0f;

	if (viewWidthRatio < viewHeightRatio)
	{
		// The scaled image's height will fit to the viewport's height and 
		// its width will be smaller than the viewport's width.
		x = viewWidthRatio / viewHeightRatio;
	}
	else
	{
		// The scaled image's width will fit to the viewport's width and 
		// its height may be smaller than the viewport's height.
		y = viewHeightRatio / viewWidthRatio;
	}

	m_postViewport.TopLeftX = m_width * (1.0f - x) / 2.0f;
	m_postViewport.TopLeftY = m_height * (1.0f - y) / 2.0f;
	m_postViewport.Width = x * m_width;
	m_postViewport.Height = y * m_height;

	m_postScissorRect.left = static_cast<LONG>(m_postViewport.TopLeftX);
	m_postScissorRect.right = static_cast<LONG>(m_postViewport.TopLeftX + m_postViewport.Width);
	m_postScissorRect.top = static_cast<LONG>(m_postViewport.TopLeftY);
	m_postScissorRect.bottom = static_cast<LONG>(m_postViewport.TopLeftY + m_postViewport.Height);
}

// Set up appropriate views for the intermediate render target.
void D3D12Fullscreen::LoadSceneResolutionDependentResources()
{


	// Set up the scene viewport and scissor rect to match the current scene rendering resolution.
	{
		m_sceneViewport.Width = static_cast<float>(m_resolutionOptions[m_resolutionIndex].Width);
		m_sceneViewport.Height = static_cast<float>(m_resolutionOptions[m_resolutionIndex].Height);

		m_sceneScissorRect.right = static_cast<LONG>(m_resolutionOptions[m_resolutionIndex].Width);
		m_sceneScissorRect.bottom = static_cast<LONG>(m_resolutionOptions[m_resolutionIndex].Height);
	}

	// Update post-process viewport and scissor rectangle.
	UpdatePostViewAndScissor();

	// Create RTV for the intermediate render target.
	{
		D3D12_RESOURCE_DESC swapChainDesc = m_renderTargets[m_frameIndex]->GetDesc();
		const CD3DX12_CLEAR_VALUE clearValue(swapChainDesc.Format, Colors::Black);
		const CD3DX12_RESOURCE_DESC renderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			swapChainDesc.Format,
			m_resolutionOptions[m_resolutionIndex].Width,
			m_resolutionOptions[m_resolutionIndex].Height,
			1u, 1u,
			swapChainDesc.SampleDesc.Count,
			swapChainDesc.SampleDesc.Quality,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			D3D12_TEXTURE_LAYOUT_UNKNOWN, 0u);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), FrameCount, m_rtvDescriptorSize);
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&renderTargetDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			&clearValue,
			IID_PPV_ARGS(&m_intermediateRenderTarget)));
		m_device->CreateRenderTargetView(m_intermediateRenderTarget.Get(), nullptr, rtvHandle);
		NAME_D3D12_OBJECT(m_intermediateRenderTarget);
	}

	// Create SRV for the intermediate render target.
	//m_device->CreateShaderResourceView(m_intermediateRenderTarget.Get(), nullptr, m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
}



void D3D12Fullscreen::OnWindowMoved(int, int)
{
}