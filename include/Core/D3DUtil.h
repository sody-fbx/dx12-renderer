#pragma once

// ═══════════════════════════════════════════════════════════════════
//  D3DUtil.h — DX12 공통 유틸리티
//  모든 DX12 관련 파일에서 include하는 공용 헤더
// ═══════════════════════════════════════════════════════════════════

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <windowsx.h>
#include <wrl/client.h>       // Microsoft::WRL::ComPtr

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <string>
#include <memory>
#include <vector>
#include <array>
#include <stdexcept>
#include <cassert>
#include <unordered_map>

// ─── ComPtr ───
using Microsoft::WRL::ComPtr;

// ─── HRESULT 체크 매크로 ───
// DX12의 거의 모든 함수가 HRESULT를 반환함, 실패 시 바로 예외 처리.
#ifndef ThrowIfFailed
#define ThrowIfFailed(hr)                                           \
    do {                                                            \
        HRESULT hr_ = (hr);                                         \
        if (FAILED(hr_)) {                                          \
            char msg[256];                                          \
            sprintf_s(msg, "HRESULT failed: 0x%08X\nFile: %s\n"    \
                      "Line: %d", (unsigned)hr_, __FILE__, __LINE__);\
            throw std::runtime_error(msg);                          \
        }                                                           \
    } while(0)
#endif

// ─── 프레임 리소스 상수 ───
// 트리플 버퍼링 : GPU가 프레임 N을 렌더링하는 동안 CPU는 프레임 N+1, N+2를 준비할 수 있음.
static constexpr UINT FRAME_BUFFER_COUNT = 3;

// ─── 포맷 ───
static constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
static constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;
