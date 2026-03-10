#pragma once

// ═══════════════════════════════════════════════════════════════════
//  D3DUtil.h — DX12 공통 유틸리티
//  모든 DX12 관련 파일에서 include하는 공용 헤더
// ═══════════════════════════════════════════════════════════════════

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
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

// ─── ComPtr 별칭 ───
// DX12의 COM 오브젝트는 수동 Release 대신 ComPtr로 관리.
// ComPtr은 스마트 포인터처럼 동작 — 스코프 벗어나면 자동 Release.
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
