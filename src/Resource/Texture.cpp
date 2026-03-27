// ═══════════════════════════════════════════════════════════════════
//  Texture.cpp
// ═══════════════════════════════════════════════════════════════════

// stb_image: 헤더 전용 라이브러리. 반드시 하나의 .cpp에서만 IMPLEMENTATION을 정의
#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"

#include "Resource/Texture.h"

static void UploadTexture2D( ID3D12Device* device
                           , ID3D12GraphicsCommandList* cmdList
                           , const void* pixels      // RGBA8 데이터
                           , UINT width
                           , UINT height
                           , ComPtr<ID3D12Resource>& outGPU
                           , ComPtr<ID3D12Resource>& outUpload )
{
    const UINT rowPitch = width * 4;

    // GPU Default Heap Texture2D 생성
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width            = width;
    texDesc.Height           = height;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels        = 1;
    texDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc       = { 1, 0 };
    texDesc.Layout           = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    D3D12_HEAP_PROPERTIES defaultHeap = {};
    defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

    ThrowIfFailed(device->CreateCommittedResource( &defaultHeap, D3D12_HEAP_FLAG_NONE
                                                 , &texDesc, D3D12_RESOURCE_STATE_COPY_DEST
                                                 , nullptr, IID_PPV_ARGS(&outGPU)) );

    // Upload Buffer 크기 계산
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
    UINT   numRows    = 0;
    UINT64 rowBytes   = 0;
    UINT64 uploadSize = 0;
    device->GetCopyableFootprints(&texDesc, 0, 1, 0, &footprint, &numRows, &rowBytes, &uploadSize);

    D3D12_RESOURCE_DESC uploadDesc = {};
    uploadDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
    uploadDesc.Width            = uploadSize;
    uploadDesc.Height           = 1;
    uploadDesc.DepthOrArraySize = 1;
    uploadDesc.MipLevels        = 1;
    uploadDesc.SampleDesc       = { 1, 0 };
    uploadDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_HEAP_PROPERTIES uploadHeap = {};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

    ThrowIfFailed(device->CreateCommittedResource( &uploadHeap, D3D12_HEAP_FLAG_NONE
                                                 , &uploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ
                                                 , nullptr, IID_PPV_ARGS(&outUpload)) );

    // CPU -> Upload Buffer 행별 복사
    BYTE* mapped = nullptr;
    ThrowIfFailed(outUpload->Map(0, nullptr, reinterpret_cast<void**>(&mapped)));

    const BYTE* src = static_cast<const BYTE*>(pixels);
    BYTE*       dst = mapped + footprint.Offset;

    for (UINT row = 0; row < height; ++row)
    {
        memcpy(dst, src, rowPitch);
        src += rowPitch;
        dst += footprint.Footprint.RowPitch;
    }
    outUpload->Unmap(0, nullptr);

    // GPU : Upload -> Texture2D 복사
    D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
    dstLoc.pResource        = outGPU.Get();
    dstLoc.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLoc.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
    srcLoc.pResource       = outUpload.Get();
    srcLoc.Type            = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLoc.PlacedFootprint = footprint;

    cmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

    // Barrier : COPY_DEST -> PIXEL_SHADER_RESOURCE
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = outGPU.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &barrier);
}

void Texture::CreateSRV( ID3D12Device* device
                       , D3D12_CPU_DESCRIPTOR_HANDLE srvHandle )
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels       = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    device->CreateShaderResourceView(m_textureGPU.Get(), &srvDesc, srvHandle);
}

void Texture::LoadFromFile( ID3D12Device* device
                          , ID3D12GraphicsCommandList* cmdList
                          , const std::wstring& filePath
                          , D3D12_CPU_DESCRIPTOR_HANDLE srvHandle )
{
    std::string pathStr = WstrToStr(filePath);

    int width = 0, height = 0, channels = 0;
    stbi_uc* pixels = stbi_load(pathStr.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels)
    {
        OutputDebugStringA("[TextureManager] stbi_load failed: ");
        OutputDebugStringA(pathStr.c_str());
        OutputDebugStringA("\n");
        return;
    }

    UploadTexture2D(device, cmdList, pixels, (UINT)width, (UINT)height, m_textureGPU, m_uploadBuffer);

    stbi_image_free(pixels);

    CreateSRV(device, srvHandle);
}

void Texture::CreateWhite( ID3D12Device* device
                         , ID3D12GraphicsCommandList* cmdList
                         , D3D12_CPU_DESCRIPTOR_HANDLE srvHandle )
{
    // RGBA (255, 255, 255, 255) 픽셀 하나
    const uint8_t whitePixel[4] = { 255, 255, 255, 255 };

    UploadTexture2D(device, cmdList, whitePixel, 1, 1, m_textureGPU, m_uploadBuffer);

    CreateSRV(device, srvHandle);
}

void Texture::ReleaseUploadBuffer()
{ 
    m_uploadBuffer.Reset(); 
}

bool Texture::IsValid() const
{
    return m_textureGPU != nullptr;
}
