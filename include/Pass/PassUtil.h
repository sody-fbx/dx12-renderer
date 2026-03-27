#pragma once

// ═══════════════════════════════════════════════════════════════════
//  PassUtil.h — Pass 공통 유틸리티
//  모든 Pass 관련 파일에서 include하는 공용 헤더
//  모든 Pass에서 공통으로 사용하거나, Pass 간 연동하는 정보를 정의
//  1. PipelineSet : RootSignature, PipelineState(PSO)
//  2. DepthTarget : DepthBuffer, Target
//  3. ShadowMap : ShadowPass에서 생성하고 ForwardPass에서 사용
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
		case ROOT_SIGNATURE_TYPE_EMPTY:   RootSignature.CreateEmpty(device);       break;
		case ROOT_SIGNATURE_TYPE_CBV:     RootSignature.CreateWithCBV(device);     break;
		case ROOT_SIGNATURE_TYPE_SHADOW:  RootSignature.CreateWithShadow(device);  break;
		case ROOT_SIGNATURE_TYPE_TEXTURE: RootSignature.CreateWithTexture(device); break;
		default: break;
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
			break;
		case SHADERTYPE::SHADOW:
			shaderPath = GetShaderPath(SHADERTYPE::SHADOW);
			Pso.CreateShadow( device
							, RootSignature.Get()
							, shaderPath
							, DXGI_FORMAT_D32_FLOAT);
			break;
		case SHADERTYPE::PBR_TEX:
			shaderPath = GetShaderPath(SHADERTYPE::PBR_TEX);
			Pso.CreateDefault( device
							 , RootSignature.Get()
							 , shaderPath, shaderPath
							 , BACK_BUFFER_FORMAT, DEPTH_FORMAT);
			break;
		default:
			break;
		}
	}
};

struct DepthTarget
{
    ComPtr<ID3D12Resource>	Buffer;
    DescriptorHeap			Heap;

    D3D12_CPU_DESCRIPTOR_HANDLE DSV() const { return Heap.GetCPUHandle(0); }
    ID3D12Resource*             Get() const { return Buffer.Get(); }
};

struct ShadowMap
{
    ComPtr<ID3D12Resource> Texture;
    DepthTarget            Depth;
    UINT                   Size = 2048;

    // 공유 SRV Heap 안에서의 슬롯 인덱스 (ForwardPass에서 읽기용)
    UINT SRVIndex = UINT_MAX;

    D3D12_CPU_DESCRIPTOR_HANDLE DSV()     const { return Depth.Heap.GetCPUHandle(0); }
    ID3D12Resource*             Get()     const { return Texture.Get(); }
    bool                        IsValid() const { return Texture != nullptr; }
};