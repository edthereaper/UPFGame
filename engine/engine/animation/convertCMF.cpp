#include "mcv_platform.h"

#include "convertCMF.h"

#include "render/render_utils.h"
using namespace render;

using namespace DirectX;

namespace utils {

bool convertCMF(CalCoreMesh* core_mesh, const char* outfile)
{
    std::vector<VertexSkin> vertices;
    std::vector<Mesh::index_t> indices;

    std::vector<Mesh::group_t> groups;

    assert(core_mesh);

    XMFLOAT3 min, max;
    bool first = true;

    int nsubmeshes = core_mesh->getCoreSubmeshCount();
    for (int sm_id = 0; sm_id < nsubmeshes; ++sm_id) {
        CalCoreSubmesh *sm = core_mesh->getCoreSubmesh(sm_id);

        // -----------------------------------------------------
        // INDEX DATA
        // -----------------------------------------------------
        UINT vtx0 = static_cast<UINT>(vertices.size());
        Mesh::index_t index0 = static_cast<Mesh::index_t>(indices.size());
        int nfaces = sm->getFaceCount();
        auto& vfaces = sm->getVectorFace();
        for (auto& fit : vfaces) {
            indices.push_back(vtx0 + fit.vertexId[0]);
            indices.push_back(vtx0 + fit.vertexId[2]);    // Swap face culling!
            indices.push_back(vtx0 + fit.vertexId[1]);
        }
        
        // -----------------------------------------------------
        // GROUPS INFO
        // -----------------------------------------------------
        Mesh::group_t group;
        group.firstIndex = index0;
        group.nIndices = (unsigned int) vfaces.size() * 3;
        groups.push_back(group);

        // All texture coords sets
        typedef std::vector<CalCoreSubmesh::TextureCoordinate> calUVsV_t;
        auto& all_tex_coords = sm->getVectorVectorTextureCoordinate();
        //assert(!all_tex_coords.empty());

        // Vertices
        int nvertexs = sm->getVertexCount();

        // Just use the first one
        calUVsV_t* tex_coords0 = nullptr;
        if (!all_tex_coords.empty()) {
            tex_coords0 = &all_tex_coords[0];
            // Sizes must match
            assert(tex_coords0->size() == nvertexs);
        }

        // For each cal3d vertex
        int cal_vtx_idx = 0;
        for (auto& vit : sm->getVectorVertex()) {

            // Build one of our skinned vertexs
            VertexSkin sv;
            memset(&sv, 0x00, sizeof(sv));

            sv.Pos = XMFLOAT3(vit.position.x, vit.position.y, vit.position.z);
            sv.Normal = XMFLOAT3(vit.normal.x, vit.normal.y, vit.normal.z);
            if (tex_coords0)
                sv.UV = XMFLOAT2((*tex_coords0)[cal_vtx_idx].u, (*tex_coords0)[cal_vtx_idx].v);

            if (first) {
                min = max = XMFLOAT3(sv.Pos.x, sv.Pos.y, sv.Pos.z);
                first = false;
            } else {
                updateMinMax(min, max, XMFLOAT3(sv.Pos.x, sv.Pos.y, sv.Pos.z));
            }

            assert(vit.vectorInfluence.size() <= 4);
            int idx = 0;
            int total_weight = 0;
            for (auto fit : vit.vectorInfluence) {
                assert(fit.boneId < 256);
                assert(fit.weight >= 0.f && fit.weight <= 1.0f);
                sv.BoneIds[idx] = static_cast<unsigned char>(fit.boneId);
                sv.Weights[idx] = static_cast<unsigned char>(fit.weight * 255.f);
                total_weight += sv.Weights[idx];
                ++idx;
            }

            // Confirm the total weight of the vertex == 255, or give the error
            // to the first vertex
            if (total_weight != 255) {
                int error = 255 - total_weight;
                assert(error > 0);
                sv.Weights[0] += error;
            }

            // Save the vertex
            vertices.push_back(sv);

            ++cal_vtx_idx;
        }
    }

    // 
    Mesh::header_t header;
    header.bytes_per_index = sizeof(Mesh::index_t);
    header.bytes_per_vertex = sizeof(VertexSkin);
    header.magic = Mesh::TAG_begin;
    header.magic_tail = header.magic;
    header.nidxs = (unsigned int) indices.size();
    header.nvertexs = (unsigned int) vertices.size();
    header.primitive_type = Mesh::TRIANGLE_LIST;
    header.version = Mesh::CURRENT_VERSION;
    header.vertex_type = VertexDecl::SKIN;
    header.bbmax[0]=max.x;
    header.bbmax[1]=max.z;
    header.bbmax[2]=max.y;
    header.bbmin[0]=min.x;
    header.bbmin[1]=min.z;
    header.bbmin[2]=min.y;

    // Virtual file
    MemoryDataSaver mds;
    mds.writePOD(header);

    computeTangentSpace<VertexSkin>(vertices, indices);
    saveChunk(mds, Mesh::TAG_vertices, vertices);
    saveChunk(mds, Mesh::TAG_indices, indices);
    saveChunk(mds, Mesh::TAG_groups, groups);

    Mesh::chunkHeader_t chunk;
    chunk.tag = Mesh::TAG_end;
    chunk.nbytes = 0;
    mds.writePOD(chunk);

    return mds.saveToFile(outfile);
}

}
