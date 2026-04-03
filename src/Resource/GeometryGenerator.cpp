// ═══════════════════════════════════════════════════════════════════
//  GeometryGenerator.cpp — 기본 도형 생성
// ═══════════════════════════════════════════════════════════════════

#include "Resource/GeometryGenerator.h"

namespace GeometryGenerator
{
	MeshData<VertexCol> CreateTriangle()
	{
		MeshData<VertexCol> mesh;

		mesh.Vertices = {
			{ {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },  // 상단 (빨강)
			{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },  // 우하 (초록)
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },  // 좌하 (파랑)
		};

		mesh.Indices = { 0, 1, 2 };

		return mesh;
	}

	MeshData<VertexTex> CreateBox(float width, float height, float depth)
	{
		MeshData<VertexTex> mesh;

		float w2 = 0.5f * width;
		float h2 = 0.5f * height;
		float d2 = 0.5f * depth;

		// 24 정점 (면당 4개 × 6면) — 면마다 다른 노멀
		// Front face (+Z in LH)
		mesh.Vertices = {
			// Front
			{ {-w2, -h2, -d2}, { 0,  0, -1}, {0, 1} },
			{ {-w2, +h2, -d2}, { 0,  0, -1}, {0, 0} },
			{ {+w2, +h2, -d2}, { 0,  0, -1}, {1, 0} },
			{ {+w2, -h2, -d2}, { 0,  0, -1}, {1, 1} },
			// Back
			{ {+w2, -h2, +d2}, { 0,  0,  1}, {0, 1} },
			{ {+w2, +h2, +d2}, { 0,  0,  1}, {0, 0} },
			{ {-w2, +h2, +d2}, { 0,  0,  1}, {1, 0} },
			{ {-w2, -h2, +d2}, { 0,  0,  1}, {1, 1} },
			// Top
			{ {-w2, +h2, -d2}, { 0,  1,  0}, {0, 1} },
			{ {-w2, +h2, +d2}, { 0,  1,  0}, {0, 0} },
			{ {+w2, +h2, +d2}, { 0,  1,  0}, {1, 0} },
			{ {+w2, +h2, -d2}, { 0,  1,  0}, {1, 1} },
			// Bottom
			{ {-w2, -h2, +d2}, { 0, -1,  0}, {0, 1} },
			{ {-w2, -h2, -d2}, { 0, -1,  0}, {0, 0} },
			{ {+w2, -h2, -d2}, { 0, -1,  0}, {1, 0} },
			{ {+w2, -h2, +d2}, { 0, -1,  0}, {1, 1} },
			// Left
			{ {-w2, -h2, +d2}, {-1,  0,  0}, {0, 1} },
			{ {-w2, +h2, +d2}, {-1,  0,  0}, {0, 0} },
			{ {-w2, +h2, -d2}, {-1,  0,  0}, {1, 0} },
			{ {-w2, -h2, -d2}, {-1,  0,  0}, {1, 1} },
			// Right
			{ {+w2, -h2, -d2}, { 1,  0,  0}, {0, 1} },
			{ {+w2, +h2, -d2}, { 1,  0,  0}, {0, 0} },
			{ {+w2, +h2, +d2}, { 1,  0,  0}, {1, 0} },
			{ {+w2, -h2, +d2}, { 1,  0,  0}, {1, 1} },
		};

		mesh.Indices = {
			 0,  1,  2,   0,  2,  3,   // Front
			 4,  5,  6,   4,  6,  7,   // Back
			 8,  9, 10,   8, 10, 11,   // Top
			12, 13, 14,  12, 14, 15,   // Bottom
			16, 17, 18,  16, 18, 19,   // Left
			20, 21, 22,  20, 22, 23,   // Right
		};

		ComputeTangents(mesh.Vertices, mesh.Indices);
		return mesh;
	}

	MeshData<VertexTex> CreateSphere(float radius, UINT sliceCount, UINT stackCount)
	{
		MeshData<VertexTex> mesh;

		// 북극
		mesh.Vertices.push_back({ {0, radius, 0}, {0, 1, 0}, {0, 0} });

		float phiStep = XM_PI / stackCount;
		float thetaStep = 2.0f * XM_PI / sliceCount;

		for (UINT i = 1; i < stackCount; ++i)
		{
			float phi = i * phiStep;
			for (UINT j = 0; j <= sliceCount; ++j)
			{
				float theta = j * thetaStep;

				VertexTex v;
				v.Position.x = radius * sinf(phi) * cosf(theta);
				v.Position.y = radius * cosf(phi);
				v.Position.z = radius * sinf(phi) * sinf(theta);

				// 구의 노멀 = 정규화된 위치
				XMVECTOR p = XMLoadFloat3(&v.Position);
				XMStoreFloat3(&v.Normal, XMVector3Normalize(p));

				v.TexCoord.x = theta / (2.0f * XM_PI);
				v.TexCoord.y = phi / XM_PI;

				mesh.Vertices.push_back(v);
			}
		}

		// 남극
		mesh.Vertices.push_back({ {0, -radius, 0}, {0, -1, 0}, {0, 1} });

		// 북극 인덱스
		for (UINT i = 1; i <= sliceCount; ++i)
		{
			mesh.Indices.push_back(0);
			mesh.Indices.push_back(i + 1);
			mesh.Indices.push_back(i);
		}

		// 중간 스택
		UINT baseIndex = 1;
		UINT ringVertexCount = sliceCount + 1;
		for (UINT i = 0; i < stackCount - 2; ++i)
		{
			for (UINT j = 0; j < sliceCount; ++j)
			{
				mesh.Indices.push_back(baseIndex + i * ringVertexCount + j);
				mesh.Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
				mesh.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

				mesh.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
				mesh.Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
				mesh.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
			}
		}

		// 남극 인덱스
		UINT southPoleIndex = (UINT)mesh.Vertices.size() - 1;
		baseIndex = southPoleIndex - ringVertexCount;
		for (UINT i = 0; i < sliceCount; ++i)
		{
			mesh.Indices.push_back(southPoleIndex);
			mesh.Indices.push_back(baseIndex + i);
			mesh.Indices.push_back(baseIndex + i + 1);
		}

		ComputeTangents(mesh.Vertices, mesh.Indices);
		return mesh;
	}

	MeshData<VertexTex> CreateGrid(float width, float depth, UINT m, UINT n)
	{
		MeshData<VertexTex> mesh;

		UINT vertexCount = m * n;
		UINT faceCount = (m - 1) * (n - 1) * 2;

		float halfWidth = 0.5f * width;
		float halfDepth = 0.5f * depth;
		float dx = width / (n - 1);
		float dz = depth / (m - 1);
		float du = 1.0f / (n - 1);
		float dv = 1.0f / (m - 1);

		mesh.Vertices.resize(vertexCount);
		for (UINT i = 0; i < m; ++i)
		{
			float z = halfDepth - i * dz;
			for (UINT j = 0; j < n; ++j)
			{
				float x = -halfWidth + j * dx;
				UINT idx = i * n + j;
				mesh.Vertices[idx].Position = { x, 0.0f, z };
				mesh.Vertices[idx].Normal = { 0.0f, 1.0f, 0.0f };
				mesh.Vertices[idx].TexCoord = { j * du, i * dv };
			}
		}

		mesh.Indices.resize(faceCount * 3);
		UINT k = 0;
		for (UINT i = 0; i < m - 1; ++i)
		{
			for (UINT j = 0; j < n - 1; ++j)
			{
				mesh.Indices[k] = i * n + j;
				mesh.Indices[k + 1] = i * n + j + 1;
				mesh.Indices[k + 2] = (i + 1) * n + j;
				mesh.Indices[k + 3] = (i + 1) * n + j;
				mesh.Indices[k + 4] = i * n + j + 1;
				mesh.Indices[k + 5] = (i + 1) * n + j + 1;
				k += 6;
			}
		}

		ComputeTangents(mesh.Vertices, mesh.Indices);
		return mesh;
	}

	MeshData<VertexTex> CreateCylinder(float bottomRadius, float topRadius,
		float height, UINT sliceCount, UINT stackCount)
	{
		MeshData<VertexTex> mesh;

		float stackHeight = height / stackCount;
		float radiusStep = (topRadius - bottomRadius) / stackCount;
		UINT ringCount = stackCount + 1;

		for (UINT i = 0; i < ringCount; ++i)
		{
			float y = -0.5f * height + i * stackHeight;
			float r = bottomRadius + i * radiusStep;

			float dTheta = 2.0f * XM_PI / sliceCount;
			for (UINT j = 0; j <= sliceCount; ++j)
			{
				float c = cosf(j * dTheta);
				float s = sinf(j * dTheta);

				VertexTex v;
				v.Position = { r * c, y, r * s };
				v.TexCoord = { (float)j / sliceCount, 1.0f - (float)i / stackCount };

				// 실린더 측면 노멀 (단순화)
				XMVECTOR n = XMVector3Normalize(XMVectorSet(c, 0.0f, s, 0.0f));
				XMStoreFloat3(&v.Normal, n);

				mesh.Vertices.push_back(v);
			}
		}

		UINT ringVertexCount = sliceCount + 1;
		for (UINT i = 0; i < stackCount; ++i)
		{
			for (UINT j = 0; j < sliceCount; ++j)
			{
				mesh.Indices.push_back(i * ringVertexCount + j);
				mesh.Indices.push_back((i + 1) * ringVertexCount + j);
				mesh.Indices.push_back((i + 1) * ringVertexCount + j + 1);

				mesh.Indices.push_back(i * ringVertexCount + j);
				mesh.Indices.push_back((i + 1) * ringVertexCount + j + 1);
				mesh.Indices.push_back(i * ringVertexCount + j + 1);
			}
		}

		// Top Cap
		UINT topBaseIndex = (UINT)mesh.Vertices.size();
		float topY = 0.5f * height;
		float dTheta = 2.0f * XM_PI / sliceCount;

		for (UINT i = 0; i <= sliceCount; ++i)
		{
			float c = cosf(i * dTheta), s = sinf(i * dTheta);
			VertexTex v;
			v.Position = { topRadius * c, topY, topRadius * s };
			v.Normal = { 0.0f, 1.0f, 0.0f };
			v.TexCoord = { c * 0.5f + 0.5f, s * 0.5f + 0.5f };
			mesh.Vertices.push_back(v);
		}

		// 중심점
		VertexTex topCenter;
		topCenter.Position = { 0.0f, topY, 0.0f };
		topCenter.Normal = { 0.0f, 1.0f, 0.0f };
		topCenter.TexCoord = { 0.5f, 0.5f };
		mesh.Vertices.push_back(topCenter);
		UINT topCenterIndex = (UINT)mesh.Vertices.size() - 1;

		for (UINT i = 0; i < sliceCount; ++i)
		{
			mesh.Indices.push_back(topCenterIndex);
			mesh.Indices.push_back(topBaseIndex + i + 1);
			mesh.Indices.push_back(topBaseIndex + i);
		}

		// Bottom Cap
		UINT botBaseIndex = (UINT)mesh.Vertices.size();
		float botY = -0.5f * height;

		for (UINT i = 0; i <= sliceCount; ++i)
		{
			float c = cosf(i * dTheta), s = sinf(i * dTheta);
			VertexTex v;
			v.Position = { bottomRadius * c, botY, bottomRadius * s };
			v.Normal = { 0.0f, -1.0f, 0.0f };
			v.TexCoord = { c * 0.5f + 0.5f, s * 0.5f + 0.5f };
			mesh.Vertices.push_back(v);
		}

		// 중심점
		VertexTex botCenter;
		botCenter.Position = { 0.0f, botY, 0.0f };
		botCenter.Normal = { 0.0f, -1.0f, 0.0f };
		botCenter.TexCoord = { 0.5f, 0.5f };
		mesh.Vertices.push_back(botCenter);
		UINT botCenterIndex = (UINT)mesh.Vertices.size() - 1;

		for (UINT i = 0; i < sliceCount; ++i)
		{
			mesh.Indices.push_back(botCenterIndex);
			mesh.Indices.push_back(botBaseIndex + i);
			mesh.Indices.push_back(botBaseIndex + i + 1);
		}

		ComputeTangents(mesh.Vertices, mesh.Indices);
		return mesh;
	}

	//  UV 기반 삼각형 단위 탄젠트 계산
	//  각 삼각형의 UV 차이로 접선 방향을 구한 뒤 정점에 누적·정규화
	void ComputeTangents( std::vector<VertexTex>& vertices
	                    , const std::vector<uint32_t>& indices )
	{
		// 누적용 초기화
		for (auto& v : vertices)
			v.Tangent = { 0.0f, 0.0f, 0.0f };

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			VertexTex& v0 = vertices[indices[i    ]];
			VertexTex& v1 = vertices[indices[i + 1]];
			VertexTex& v2 = vertices[indices[i + 2]];

			XMFLOAT3 edge1 = { v1.Position.x - v0.Position.x
			                 , v1.Position.y - v0.Position.y
			                 , v1.Position.z - v0.Position.z };
			XMFLOAT3 edge2 = { v2.Position.x - v0.Position.x
			                 , v2.Position.y - v0.Position.y
			                 , v2.Position.z - v0.Position.z };

			float du1 = v1.TexCoord.x - v0.TexCoord.x;
			float dv1 = v1.TexCoord.y - v0.TexCoord.y;
			float du2 = v2.TexCoord.x - v0.TexCoord.x;
			float dv2 = v2.TexCoord.y - v0.TexCoord.y;

			float denom = du1 * dv2 - du2 * dv1;
			if (fabsf(denom) < 1e-6f) continue;   // UV 겹침 방지
			float f = 1.0f / denom;

			XMFLOAT3 tangent =
			{
				f * (dv2 * edge1.x - dv1 * edge2.x),
				f * (dv2 * edge1.y - dv1 * edge2.y),
				f * (dv2 * edge1.z - dv1 * edge2.z),
			};

			// 3 정점에 누적
			v0.Tangent.x += tangent.x;  v0.Tangent.y += tangent.y;  v0.Tangent.z += tangent.z;
			v1.Tangent.x += tangent.x;  v1.Tangent.y += tangent.y;  v1.Tangent.z += tangent.z;
			v2.Tangent.x += tangent.x;  v2.Tangent.y += tangent.y;  v2.Tangent.z += tangent.z;
		}

		// 정규화
		for (auto& v : vertices)
		{
			XMVECTOR t = XMLoadFloat3(&v.Tangent);
			if (XMVector3LengthSq(t).m128_f32[0] < 1e-6f)
				t = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);  // 퇴화 케이스 fallback
			XMStoreFloat3(&v.Tangent, XMVector3Normalize(t));
		}
	}

} // namespace GeometryGenerator
