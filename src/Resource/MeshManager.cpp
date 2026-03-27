// ═══════════════════════════════════════════════════════════════════
//  MeshManager.cpp
// ═══════════════════════════════════════════════════════════════════

#include "Resource/MeshManager.h"
#include "Resource/Vertex.h"

#include <fstream>
#include <sstream>

using namespace DirectX;

void MeshManager::RequestPrimitive( const std::string& name
                                  , MeshPrimitive      type
                                  , PrimitiveParams    params )
{
    // 중복 요청 무시
    for (auto& r : m_primitiveRequests)
        if (r.name == name) return;

    m_primitiveRequests.push_back({ name, type, params });
}

void MeshManager::RequestFile( const std::string&  name
                              , const std::wstring& path )
{
    for (auto& r : m_fileRequests)
        if (r.name == name) return;

    m_fileRequests.push_back({ name, path });
}

void MeshManager::BuildAll( ID3D12Device* device
                           , ID3D12GraphicsCommandList* cmdList )
{
    for (auto& req : m_primitiveRequests)
        BuildPrimitive(device, cmdList, req);

    for (auto& req : m_fileRequests)
        BuildFile(device, cmdList, req);

    m_primitiveRequests.clear();
    m_fileRequests.clear();
    m_built = true;
}

void MeshManager::ReleaseUploadBuffers()
{
    for (auto& [name, mesh] : m_meshes)
        mesh->ReleaseUploadBuffer();
}

Mesh* MeshManager::Get(const std::string& name) const
{
    auto it = m_meshes.find(name);
    if (it != m_meshes.end())
        return it->second.get();

    OutputDebugStringA("[MeshManager] Mesh not found: ");
    OutputDebugStringA(name.c_str());
    OutputDebugStringA("\n");
    return nullptr;
}

bool MeshManager::IsReady() const
{
    return m_built;
}

void MeshManager::BuildPrimitive( ID3D12Device*             device
                                 , ID3D12GraphicsCommandList* cmdList
                                 , const PrimitiveRequest&    req )
{
    auto mesh = std::make_unique<Mesh>();
    mesh->SetName(req.name);

    const auto& p = req.params;

    switch (req.type)
    {
    case MeshPrimitive::Box:
    {
        auto data = GeometryGenerator::CreateBox(p.p0, p.p1, p.p2);
        mesh->Create(device, cmdList,
                     data.Vertices.data(), (UINT)data.Vertices.size(), sizeof(VertexTex),
                     data.Indices.data(),  (UINT)data.Indices.size(),  DXGI_FORMAT_R32_UINT);
        break;
    }
    case MeshPrimitive::Sphere:
    {
        auto data = GeometryGenerator::CreateSphere(p.p0, p.u0, p.u1);
        mesh->Create(device, cmdList,
                     data.Vertices.data(), (UINT)data.Vertices.size(), sizeof(VertexTex),
                     data.Indices.data(),  (UINT)data.Indices.size(),  DXGI_FORMAT_R32_UINT);
        break;
    }
    case MeshPrimitive::Cylinder:
    {
        auto data = GeometryGenerator::CreateCylinder(p.p0, p.p1, p.p2, p.u0, p.u1);
        mesh->Create(device, cmdList,
                     data.Vertices.data(), (UINT)data.Vertices.size(), sizeof(VertexTex),
                     data.Indices.data(),  (UINT)data.Indices.size(),  DXGI_FORMAT_R32_UINT);
        break;
    }
    case MeshPrimitive::Grid:
    {
        auto data = GeometryGenerator::CreateGrid(p.p0, p.p1, p.u0, p.u1);
        mesh->Create(device, cmdList,
                     data.Vertices.data(), (UINT)data.Vertices.size(), sizeof(VertexTex),
                     data.Indices.data(),  (UINT)data.Indices.size(),  DXGI_FORMAT_R32_UINT);
        break;
    }
    default:
        OutputDebugStringA("[MeshManager] Unknown MeshPrimitive type\n");
        return;
    }

    m_meshes[req.name] = std::move(mesh);
}

void MeshManager::BuildFile( ID3D12Device*             device
                            , ID3D12GraphicsCommandList* cmdList
                            , const FileRequest&          req )
{
    // wstring → string (UTF-8) 변환
    int len = WideCharToMultiByte(CP_UTF8, 0, req.path.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string pathStr(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, req.path.c_str(), -1, pathStr.data(), len, nullptr, nullptr);

    std::ifstream file(pathStr);
    if (!file.is_open())
    {
        OutputDebugStringA("[MeshManager] Failed to open OBJ: ");
        OutputDebugStringA(pathStr.c_str());
        OutputDebugStringA("\n");
        return;
    }

    // 인덱싱 전 원시 데이터
    std::vector<XMFLOAT3> positions;
    std::vector<XMFLOAT3> normals;
    std::vector<XMFLOAT2> texCoords;

    // 최종 버퍼 (face 파싱 시 v/t/n 조합마다 새 정점 생성)
    std::vector<VertexTex> vertices;
    std::vector<uint32_t>  indices;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "v")
        {
            XMFLOAT3 p;
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (token == "vn")
        {
            XMFLOAT3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (token == "vt")
        {
            XMFLOAT2 t;
            ss >> t.x >> t.y;
            t.y = 1.0f - t.y;   // OBJ V축은 상단 기준 → DX는 하단 기준으로 반전
            texCoords.push_back(t);
        }
        else if (token == "f")
        {
            // 삼각형 폴리곤을 가정 (Blender 등 익스포트 시 Triangulate 옵션 필요)
            // 지원 포맷: v/t/n  v//n  v/t  v
            for (int i = 0; i < 3; ++i)
            {
                std::string faceToken;
                ss >> faceToken;
                if (faceToken.empty()) break;

                int vi = 0, ti = 0, ni = 0;
                if      (sscanf_s(faceToken.c_str(), "%d/%d/%d", &vi, &ti, &ni) == 3) {}
                else if (sscanf_s(faceToken.c_str(), "%d//%d",   &vi,      &ni) == 2) {}
                else if (sscanf_s(faceToken.c_str(), "%d/%d",    &vi, &ti     ) == 2) {}
                else     sscanf_s(faceToken.c_str(), "%d",        &vi            );

                VertexTex v = {};
                if (vi > 0 && vi <= (int)positions.size()) v.Position = positions[vi - 1];
                if (ni > 0 && ni <= (int)normals.size())   v.Normal   = normals  [ni - 1];
                if (ti > 0 && ti <= (int)texCoords.size()) v.TexCoord = texCoords[ti - 1];

                indices.push_back(static_cast<uint32_t>(vertices.size()));
                vertices.push_back(v);
            }
        }
    }

    if (vertices.empty())
    {
        OutputDebugStringA("[MeshManager] OBJ has no geometry: ");
        OutputDebugStringA(req.name.c_str());
        OutputDebugStringA("\n");
        return;
    }

    auto mesh = std::make_unique<Mesh>();
    mesh->SetName(req.name);
    mesh->Create(device, cmdList,
                 vertices.data(), (UINT)vertices.size(), sizeof(VertexTex),
                 indices.data(),  (UINT)indices.size(),  DXGI_FORMAT_R32_UINT);

    m_meshes[req.name] = std::move(mesh);
}
