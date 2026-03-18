#pragma once

// ═══════════════════════════════════════════════════════════════════
//  ForwardPass.h
// ═══════════════════════════════════════════════════════════════════

#include "Pass/IRenderPass.h"
#include "Core/RootSignature.h"
#include "Core/PipelineState.h"
#include "Shaders/ShaderList.h"

class ForwardPass : public IRenderPass
{
public:
    void Setup(ID3D12Device* device, int width, int height) override;
    void Execute(const FrameContext& ctx) override;
    void OnResize(ID3D12Device* device, int width, int height) override;

    void AddShader( ID3D12Device* device
                  , D3D12_ROOT_PARAMETER_TYPE rootType
                  , SHADERTYPE shaderType);

private:
    void CreateDSV(ID3D12Device* device, int width, int height);

private:
    // DSV
    ComPtr<ID3D12Resource>       m_depthBuffer;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

    // PSO
    struct Shader
    {
        RootSignature  RootSig;
        PipelineState  Pso;
    };
    Shader m_shader;
};
