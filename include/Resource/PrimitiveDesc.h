#pragma once

// ═══════════════════════════════════════════════════════════════════
//  PrimitiveDesc.h — 절차적 메쉬 생성 타입 & 파라미터
// ═══════════════════════════════════════════════════════════════════

#include <cstdint>

// 절차적 생성 타입
enum class MeshPrimitive
{
	Box,
	Sphere,
	Cylinder,
	Grid,
};

// 절차적 생성 파라미터
struct PrimitiveParams
{
	float    p0 = 1.0f;   // Box: width    / Sphere: radius     / Cylinder: bottomRadius / Grid: width
	float    p1 = 1.0f;   // Box: height   / Sphere: -          / Cylinder: topRadius    / Grid: depth
	float    p2 = 1.0f;   // Box: depth    / Sphere: -          / Cylinder: height       / Grid: -
	uint32_t u0 = 20;     // Box: -        / Sphere: sliceCount / Cylinder: sliceCount   / Grid: m (열)
	uint32_t u1 = 20;     // Box: -        / Sphere: stackCount / Cylinder: stackCount   / Grid: n (행)

	static PrimitiveParams Box(float w, float h, float d)
	{
		return { w, h, d, 0, 0 };
	}

	static PrimitiveParams Sphere(float r, uint32_t slice = 20, uint32_t stack = 20)
	{
		return { r, 0.0f, 0.0f, slice, stack };
	}

	static PrimitiveParams Cylinder(float br, float tr, float h, uint32_t slice = 20, uint32_t stack = 5)
	{
		return { br, tr, h, slice, stack };
	}

	static PrimitiveParams Grid(float w, float d, uint32_t m = 20, uint32_t n = 20)
	{
		return { w, d, 0.0f, m, n };
	}
};
