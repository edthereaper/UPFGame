#include "mcv_platform.h"
#include "mesh.h"

using namespace utils;

namespace render {

Mesh::Manager Mesh::mesh_manager = Mesh::Manager();

// Just to check we are not render a mesh we have not previously activated
const Mesh* Mesh::current_active_mesh = nullptr;

// -------------------------------------------
bool Mesh::create(
      unsigned anvertexs
    , const void* the_vertexs       // Can't be null
    , unsigned anindices
    , const index_t* the_indices     // Can be null
    , ePrimitiveType primitive_type
    , const VertexDecl* avtxs_decl
    , unsigned _stream
    , XMVECTOR newBbmin
    , XMVECTOR newBbmax
    , bool can_be_updated
    ) {
    
    // Confirm we are not already created
    assert(vb == nullptr);
    
    bbmin = newBbmin;
    bbmax = newBbmax;
    
    // Confirm the given data is valid
    assert(anvertexs > 0);
    assert(the_vertexs != nullptr);
    assert(avtxs_decl != nullptr);
    stream = _stream;
    bytesPerVertex = avtxs_decl->getBytesPerVertex(stream);
    assert(bytesPerVertex> 0);
    
    // Save number of indices and vertexs
    nindices = anindices;
    nvertexs = anvertexs;
    vtxs_decl = avtxs_decl;
    
    switch (primitive_type) {
    case POINTS: topology = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST; break;
    case LINE_LIST: topology = D3D10_PRIMITIVE_TOPOLOGY_LINELIST; break;
    case TRIANGLE_LIST: topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
    case TRIANGLE_STRIP: topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
    default:
        fatal("Primitive_type %d is not valid.", primitive_type);
    }

    // Create the VB
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.ByteWidth = bytesPerVertex * anvertexs;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    
    if (can_be_updated) {
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.Usage = D3D11_USAGE_DYNAMIC;
    } else {
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.CPUAccessFlags = 0;
    }

    // The initial contents of the VB
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = the_vertexs;
    HRESULT hr = Render::getDevice()->CreateBuffer(&bd, &InitData, &vb);
    if (FAILED(hr)) {return false;}
    
    // Create the Index Buffer if the user gives us indices
    if (the_indices) {
        assert(nindices > 0);
    
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(index_t) * nindices;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;
        InitData.pSysMem = the_indices;
        hr = Render::getDevice()->CreateBuffer(&bd, &InitData, &ib);
        if (FAILED(hr)) {return false;}
    } else {
        assert(nindices == 0);
    }
    
    return true;
}

void Mesh::destroy() {
  SAFE_RELEASE(ib);
  SAFE_RELEASE(vb);
}

component::AABB Mesh::getAABB() const {
    if (bbmin == bbmax && bbmin == zero_v) {
        return component::CAABB();
    } else {
        return component::CAABB(bbmin, bbmax);
    }
}

// --------------------------------------
void Mesh::activate() const
{
    assert(vb);

    // Activate the vertex buffer
    UINT stride = bytesPerVertex;
    assert(stride > 0);
    UINT offset = 0;
    Render::getContext()->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

    // Set primitive topology (LINES,POINTS,..)
    Render::getContext()->IASetPrimitiveTopology(topology);

    // Set index buffer
    if (ib) {
        Render::getContext()->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, 0);
    }

    current_active_mesh = this;
}

// --------------------------------------
void Mesh::render() const
{
    assert(current_active_mesh == this || "Mesh not activated");
    assert(vb);
    if ( ib ) {
        Render::getContext()->DrawIndexed(nindices, 0, 0);
    } else {
        Render::getContext()->Draw(nvertexs, 0);
    }
}

void Mesh::renderGroups(unsigned groupId0, unsigned groupIdf) const
{
    assert(groupId0 < groups.size() && groupIdf < groups.size() && groupId0 <= groupIdf);
    assert(current_active_mesh == this && "Did you forget to activate this mesh?");

    assert(vb);
    if (ib) {
        const group_t& group0 = groups[groupId0];
        auto nIndices = group0.nIndices;
        for (groupId_t groupId = groupId0+1; groupId <= groupIdf; groupId++) {
            assert(groups[groupId].firstIndex == group0.firstIndex + nIndices);
            nIndices += groups[groupId].nIndices;
        }
        Render::getContext()->DrawIndexed(nIndices, group0.firstIndex, 0);
    } else {
        const group_t& group0 = groups[groupId0];
        auto nVertices = group0.nVertices;
        for (groupId_t groupId = groupId0+1; groupId <= groupIdf; groupId++) {
            assert(groups[groupId].firstVertex == group0.firstVertex + nVertices);
            nVertices += groups[groupId].nVertices;
        }
        Render::getContext()->Draw(nVertices, group0.firstVertex);
    }
}

Mesh::fileData_t Mesh::loadFileData(DataProvider& dp)
{
    fileData_t ret;
    if (!dp.isValid()) {
        ret.success = false;
        return ret;
    }
    dp.read(ret.header);
    if (!ret.header.isValid()){
        ret.success = false;
        return ret;
    }

    bool finished = false;
    while (!finished) {
        chunkHeader_t chunk;
        dp.read(chunk);
        switch (chunk.tag) {
            case TAG_vertices:
                assert(chunk.nbytes == ret.header.nvertexs * ret.header.bytes_per_vertex);
                ret.vertices.resize(chunk.nbytes);
                dp.read(&ret.vertices[0], chunk.nbytes);
                break;

             case TAG_indices:
                assert(chunk.nbytes == ret.header.nidxs * ret.header.bytes_per_index);
                ret.indices.resize(ret.header.nidxs);
                dp.read(&ret.indices[0], chunk.nbytes);
                break;

            case TAG_groups: {
                assert(chunk.nbytes % 8 == 0);
                size_t ngroups = chunk.nbytes / 8;
                ret.groups.resize(ngroups);
                dp.read(&ret.groups[0], chunk.nbytes);
                break; }

            case TAG_end:
                finished = true;
                break;

            default:
                fatal("Invalid chunk type %08x while reading %s\n",
                    chunk.tag, dp.getName());
                break;
            }
    }

    // Convert the header.vertex_type to our vertex_declaration
    ret.vertexDecl = nullptr;
    switch (ret.header.vertex_type) {
        case VertexDecl::POSITIONS:                     ret.vertexDecl = getVertexDecl<VertexPos>(); break;
        case VertexDecl::POSITION_UV:                   ret.vertexDecl = getVertexDecl<VertexPosUV>(); break;
        case VertexDecl::POSITION_COLOR:                ret.vertexDecl = getVertexDecl<VertexPosColor>(); break;
        case VertexDecl::POSITION_UV_NORMAL:            ret.vertexDecl = getVertexDecl<VertexPosUVNormal>(); break;
        case VertexDecl::POSITION_UV_NORMAL_TANGENT:    ret.vertexDecl = getVertexDecl<VertexPosUVNormalTangent>(); break;
        case VertexDecl::SKIN:                          ret.vertexDecl = getVertexDecl<VertexSkin>(); break;
    }
    assert(ret.vertexDecl);

    ret.success = true;
    //dbg("Loaded mesh: %s with %d vertices and %d indices.\n",
    //    dp.getName(), ret.header.nvertexs, ret.header.nidxs);
    return ret;
}

// --------------------------------------
bool Mesh::load(DataProvider& dp)
{
    fileData_t data = loadFileData(dp);
    if(!data.success){return false;}
    groups = data.groups;

    return create(data.header.nvertexs
        , &data.vertices[0]
        , data.header.nidxs
        , &data.indices[0]
        , (ePrimitiveType)data.header.primitive_type
        , data.vertexDecl
        , 0
        , DirectX::XMVectorSet(data.header.bbmin[0], data.header.bbmin[1], data.header.bbmin[2],1)
        , DirectX::XMVectorSet(data.header.bbmax[0], data.header.bbmax[1], data.header.bbmax[2],1)
        , false
        );
}

bool Mesh::load(const char* name, bool defaults)
{
    char full_name[MAX_PATH];
    sprintf(full_name, "data/meshes/%s.mesh", name);
    FileDataProvider fdp(full_name);
    if (load(fdp)) {
        return true;
    } else if (defaults) {
        FileDataProvider fdpDefault("data/meshes/default.mesh");
        return load(fdpDefault);
    } else {
        return false;
    }
}

void Mesh::updateFromCPU(const void *data, size_t nBytes)
{
    if (nBytes == 0) {
        nBytes = nvertexs * bytesPerVertex;
        assert(nBytes> 0);
    }
    assert(vb);
    
    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    
    // Get CPU access to the GPU buffer
    HRESULT hr = Render::getContext()->Map(
        vb, 0, D3D11_MAP_WRITE_DISCARD,
        0, &mapped_resource);
    assert(hr == D3D_OK);
    
    // Copy from CPU to GPU
    memcpy(mapped_resource.pData, data, nBytes);
    
    // Close the map
    Render::getContext()->Unmap(vb, 0);
}

const DXGI_FORMAT __IndexFmt = (sizeof(Mesh::index_t) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

void Mesh::renderInstanced(const Mesh& instancedData, size_t nInstances) const
{
    assert(isValid());
    assert(instancedData.isValid());
    
    // Set the buffer strides.
    unsigned int strides[2];
    
    strides[0] = bytesPerVertex;                    // My stride
    strides[1] = instancedData.getBytesPerVertex();  // stride of the instance
    
    // Set the buffer offsets.
    unsigned int offsets[2] = { 0, 0 };
    
    // Set the array of pointers to the vertex and instance buffers.
    ID3D11Buffer* bufferPointers[2];
    bufferPointers[0] = vb;
    bufferPointers[1] = instancedData.vb;
    
    // Set the vertex buffer to active in the input assembler so it can be rendered.
    Render::getContext()->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
    Render::getContext()->IASetPrimitiveTopology(topology);

    if (ib) {
        assert(ib);
        // Set index buffer
        Render::getContext()->IASetIndexBuffer(ib, __IndexFmt, 0);
        Render::getContext()->DrawIndexedInstanced(nindices, (UINT)nInstances, 0, 0, 0);
    } else {
        Render::getContext()->DrawInstanced(nvertexs, (UINT)nInstances, 0, 0);
    }
}

void Mesh::renderInstanced(unsigned groupId0, unsigned groupIdf,
    const Mesh& instancedData, size_t nInstances) const
{
    assert(isValid());
    assert(instancedData.isValid());
    assert(groupId0 < groups.size() && groupIdf < groups.size() && groupId0 <= groupIdf);
    
    // Set the buffer strides.
    unsigned int strides[2];
    
    strides[0] = bytesPerVertex;                    // My stride
    strides[1] = instancedData.getBytesPerVertex();  // stride of the instance
    
    // Set the buffer offsets.
    unsigned int offsets[2] = { 0, 0 };
    
    // Set the array of pointers to the vertex and instance buffers.
    ID3D11Buffer* bufferPointers[2];
    bufferPointers[0] = vb;
    bufferPointers[1] = instancedData.vb;
    
    // Set the vertex buffer to active in the input assembler so it can be rendered.
    Render::getContext()->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
    Render::getContext()->IASetPrimitiveTopology(topology);
    current_active_mesh = nullptr; //invalidate the current activa mesh (there are two meshes)

    if (ib) {
        assert(ib);
        const group_t& group0 = groups[groupId0];
        auto nIndices = group0.nIndices;
        for (groupId_t groupId = groupId0+1; groupId <= groupIdf; groupId++) {
            assert(groups[groupId].firstIndex == group0.firstIndex + nIndices);
            nIndices += groups[groupId].nIndices;
        }
        // Set index buffer
        Render::getContext()->IASetIndexBuffer(ib, __IndexFmt, 0);
        Render::getContext()->DrawIndexedInstanced(nIndices, (UINT)nInstances, group0.firstIndex, 0, 0);
    } else {
        const group_t& group0 = groups[groupId0];
        auto nVertices = group0.nVertices;
        for (groupId_t groupId = groupId0+1; groupId <= groupIdf; groupId++) {
            assert(groups[groupId].firstVertex == group0.firstVertex + nVertices );
            nVertices += groups[groupId].nVertices;
        }
        Render::getContext()->DrawInstanced(nVertices, (UINT)nInstances, group0.firstVertex, 0);
    }
}


}


