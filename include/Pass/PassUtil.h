#pragma once

// ═══════════════════════════════════════════════════════════════════
//  PassUtil.h — Pass 공통 유틸리티
//  모든 Pass 관련 파일에서 include하는 공용 헤더
// ═══════════════════════════════════════════════════════════════════

#include "Pass/IRenderPass.h"

#include "Core/DescriptorHeap.h"
#include "Core/RootSignature.h"
#include "Core/PipelineState.h"

#include "Shaders/ShaderUtil.h"

struct PipelineSet
{
    RootSignature  RootSignature;
    PipelineState  Pso;

	void Create(ID3D12Device* device, ROOT_SIGNATURE_TYPE rootType, SHADERTYPE shaderType)
	{
		switch (rootType)
		{
		case ROOT_SIGNATURE_TYPE_EMPTY:
			RootSignature.CreateEmpty(device);
			break;
		case ROOT_SIGNATURE_TYPE_CBV:
			RootSignature.CreateWithCBV(device);
			break;
		default:
			break;
		}

		std::wstring shaderPath;

		switch (shaderType)
		{
		case SHADERTYPE::DEFAULT:
			shaderPath = GetShaderPath(SHADERTYPE::DEFAULT);
			Pso.CreateDefault( device
				             , RootSignature.Get()
				             , shaderPath, shaderPath
				             , BACK_BUFFER_FORMAT, DEPTH_FORMAT);
			break;
		case SHADERTYPE::TRIANGLE:
			shaderPath = GetShaderPath(SHADERTYPE::TRIANGLE);
			Pso.CreateTriangle( device
				              , RootSignature.Get()
				              , shaderPath, shaderPath
				              , BACK_BUFFER_FORMAT, DEPTH_FORMAT);
			break;
        case SHADERTYPE::LIGHT_BLPH:
            shaderPath = GetShaderPath(SHADERTYPE::LIGHT_BLPH);
			Pso.CreateDefault( device
				             , RootSignature.Get()
				             , shaderPath, shaderPath
				             , BACK_BUFFER_FORMAT, DEPTH_FORMAT);
		default:
			break;
		}
	}
};

struct DepthTarget
{
    ComPtr<ID3D12Resource> Buffer;
    DescriptorHeap Heap;

    void Create(ID3D12Device* device, int width, int height)
    {
        // Depth/Stencil Buffer
        D3D12_RESOURCE_DESC depthDesc = {};
        depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthDesc.Width = width;
        depthDesc.Height = height;
        depthDesc.DepthOrArraySize = 1;
        depthDesc.MipLevels = 1;
        depthDesc.Format = DEPTH_FORMAT;
        depthDesc.SampleDesc = { 1, 0 };
        depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = DEPTH_FORMAT;
        clearValue.DepthStencil.Depth = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;   // GPU 전용 메모리

        ThrowIfFailed(device->CreateCommittedResource( &heapProps
                                             , D3D12_HEAP_FLAG_NONE
                                             , &depthDesc
                                             , D3D12_RESOURCE_STATE_DEPTH_WRITE
                                             , &clearValue
                                             , IID_PPV_ARGS(&Buffer)));

        // DSV (Depth Stencil View) Heap
        Heap.Initialize( device
                       , D3D12_DESCRIPTOR_HEAP_TYPE_DSV
                       , 1
                       , false );

        device->CreateDepthStencilView( Buffer.Get()
                                      , nullptr
                                      , Heap.Allocate() );
    }

    void Resize( ID3D12Device* device, int width, int height)
    {
        Buffer.Reset();
        Create(device, width, height);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DSV() const
    {
        return Heap.GetCPUHandle(0);
    }

    ID3D12Resource* Get() const { return Buffer.Get(); }
};