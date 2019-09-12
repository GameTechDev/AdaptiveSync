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

#pragma once

#include "DXSample.h"

using namespace DirectX;
#define IMGUI_ENABLED




enum CheckerboardSize
{
	Checkerboard2x2 = 0,
	Checkerboard4x4 = 1,
	Checkerboard8x8 = 2,
	Checkerboard16x16 = 3,
	Checkerboard32x32 = 4,
	Checkerboard64x64 = 5,
	CheckerboardCount
};

enum PrimativeTypes
{
	Quad = 0,
	Triangle = 1,
	PrimativeCount
};


// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

// Illustrate how to handle window resizing and fullscreen mode.
class D3D12Fullscreen : public DXSample
{
public:
    D3D12Fullscreen(UINT width, UINT height, std::wstring name);

protected:
    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
    virtual void OnWindowMoved(int, int);
    virtual void OnDestroy();
    virtual void OnKeyDown(UINT8 key);
    virtual IDXGISwapChain* GetSwapchain() { return m_swapChain.Get(); }

private:
    static const UINT FrameCount = 2;
    static const float QuadWidth;
    static const float QuadHeight;
    static const float LetterboxColor[4];
    static const float ClearColor[4];

    struct SceneVertex
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    struct PostVertex
    {
        XMFLOAT4 position;
        XMFLOAT2 uv;
    };

    struct SceneConstantBuffer
    {
        XMFLOAT4X4 transform;
        XMFLOAT4 offset;
        UINT padding[44];
    }; 
    
    struct Resolution
    {
        UINT Width;
        UINT Height;
    };

    static const Resolution m_resolutionOptions[];
    static const UINT m_resolutionOptionsCount;
    static UINT m_resolutionIndex; // Index of the current scene rendering resolution from m_resolutionOptions.

    // Pipeline objects.
    CD3DX12_VIEWPORT m_sceneViewport;
    CD3DX12_VIEWPORT m_postViewport;
    CD3DX12_RECT m_sceneScissorRect;
    CD3DX12_RECT m_postScissorRect;
    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12Resource> m_intermediateRenderTarget;
    ComPtr<ID3D12CommandAllocator> m_sceneCommandAllocators[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_postCommandAllocators[FrameCount];
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_sceneRootSignature;
    ComPtr<ID3D12RootSignature> m_postRootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_cbvSrvHeap;
    ComPtr<ID3D12PipelineState> m_scenePipelineState;
    ComPtr<ID3D12PipelineState> m_postPipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_sceneCommandList;
    ComPtr<ID3D12GraphicsCommandList> m_postCommandList;
	ComPtr<ID3D12Resource> m_texture[CheckerboardCount];
	std::vector<UINT8> GenerateTextureData(CheckerboardSize size);
    UINT m_rtvDescriptorSize;
    UINT m_cbvSrvDescriptorSize;

	//for HDR check
	ComPtr<IDXGIOutput6> output6;
	DXGI_OUTPUT_DESC1 desc1;
	ComPtr < IDXGIFactory4 > factory;

    // App resources.
    ComPtr<ID3D12Resource> m_sceneVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_sceneVertexBufferView;
    ComPtr<ID3D12Resource> m_postVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_postVertexBufferView;
    ComPtr<ID3D12Resource> m_sceneConstantBuffer;
    SceneConstantBuffer m_sceneConstantBufferData;
    UINT8* m_pCbvDataBegin;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FrameCount];

    // Track the state of the window.
    // If it's minimized the app may decide not to render frames.
    bool m_windowVisible;
    bool m_windowedMode;

    void LoadPipeline();
    void LoadAssets();
    void LoadSizeDependentResources();
    void LoadSceneResolutionDependentResources();
    void PopulateCommandLists();
    void WaitForGpu();
    void MoveToNextFrame();
    void UpdatePostViewAndScissor();
	void GetInitialTime();
	void UpdateFrameRate();
	void LoadImGui();
	void HDR10DisplaySetup();
	bool DisplayCanSupportHDR(DXGI_OUTPUT_DESC1 desc);
	void SetHDRMetaData();
	void LoadAdaptive();





	/*
	The following code was added to the fullscreen sample for the demo. 
	*/
	enum Descriptors
	{
		CheckerTexture2x2,
		CheckerTexture4x4,
		CheckerTexture8x8,
		CheckerTexture16x16,
		CheckerTexture32x32,
		CheckerTexture64x64,
#ifdef FONTS_ENABLED
		MyFont,
#endif
#ifdef IMGUI_ENABLED
		Imgui,
#endif
		Count
	};

	int m_primativeType = PrimativeTypes::Quad;
	int m_currentCheckerSize = CheckerboardSize::Checkerboard2x2;
	int m_currentShadingRate = 0;

	char* m_primativeTypes[2]{ "Rectangle", "Triangle" };
	char* m_shadingRates[5]{ "1x1", "1x2", "2x1",   "2x2",   "4x4" };
	char* m_textureSizes[6]{ "Checkerboard 2x2", "Checkerboard 4x4", "Checkerboard 8x8", "Checkerboard 16x16", "Checkerboard 32x32", "Checkerboard 64x64" };

	float speed = 500.0;
	int delay = 0;
	int del = 0;
	int interval = 0;
	bool v_sync = 0;
	double Frame_Rate;
	bool adaptive_sync = 0;
	bool enable_hdr = 0;
	int max_adaptive_refresh_rate;
	int min_adaptive_refresh_rate;
	bool adaptive_sync_supported = 0;
	LARGE_INTEGER start_time;
	LARGE_INTEGER speed_time;
	char* vrrsupport = "Not Supported";
	std::string DebugString;
	AdaptiveSyncWrapper AdaptiveSync;
	bool Intel_device;

};
