#include "mcv_platform.h"
#include "CollisionMesh.h"

#include "render/mesh/mesh.h"

using namespace physx;
using namespace utils;
using namespace render;
using index_t = render::Mesh::index_t;

#include "PhysX_USER/PhysicsManager.h"
#include "PhysX/Include/PxToolkit/include/PxTkStream.h"
#define Physics physX_user::PhysicsManager::get()

namespace physX_user {

physx::PxTriangleMesh * CollisionMeshLoader::load(const char *name)
{
	char full_name[MAX_PATH];

	sprintf(full_name, "data/meshes/%s.mesh", name);
	FileDataProvider fdp(full_name);

	if (!fdp.isValid()) {
        return nullptr;
    } else {
	    return (load(fdp));
    }

}

#define _VALIDATE_MESH
bool CollisionMeshLoader::validateMesh(const PxTriangleMeshDesc& d)
{
    static const size_t MAX_VERTICES = 1<<(sizeof(index_t)*8);
    index_t* indices = (index_t*)d.triangles.data;
    auto count = d.triangles.count;
    auto maxVertices = d.points.count;
    bool ok = true;
    if (maxVertices > MAX_VERTICES) {
        dbg("Mesh has too many vertices! %d vertices, max %d.\n", maxVertices, MAX_VERTICES);
    }
    for (unsigned i=0; i<count; i++) {
        auto a = indices[3*i+0];
        auto b = indices[3*i+1];
        auto c = indices[3*i+2];
        if (a > maxVertices) {
            dbg("Indice a=%d not valid (max=%d) (triangle %d = %d, %d, %d).\n",
                a, maxVertices, i, a,b,c);
            ok = false;
        }
        if (b > maxVertices) {
            dbg("Indice b=%d not valid (max=%d) (triangle %d = %d, %d, %d).\n",
                a, maxVertices, i, a,b,c);
            ok = false;
        }
        if (c > maxVertices) {
            dbg("Indice c=%d not valid (max=%d) (triangle %d = %d, %d, %d).\n",
                a, maxVertices, i, a,b,c);
            ok = false;
        }
        //if (a == b || b == c || a == c) {
        //    dbg("Triangle %d shares vertices (triangle %d = %d, %d, %d).\n", i, i, a,b,c);
        //}
    }
    return ok;
}

physx::PxTriangleMesh* CollisionMeshLoader::load(DataProvider& dp)
{
    auto data = render::Mesh::loadFileData(dp);
    
    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = data.header.nvertexs;
    meshDesc.points.stride = data.header.bytes_per_vertex;
    meshDesc.points.data = data.vertices.data();
    
    meshDesc.triangles.count = data.header.nidxs/3;
    meshDesc.triangles.stride = 3 * sizeof(index_t);
    meshDesc.triangles.data = data.indices.data();
    
    meshDesc.flags = PxMeshFlag::eFLIPNORMALS | PxMeshFlag::e16_BIT_INDICES;
    
    PxDefaultMemoryOutputStream writeBuffer;
    #if defined(_DEBUG) && defined(_VALIDATE_MESH)
        validateMesh(meshDesc);
    #endif
    bool status = Physics.gCooking->cookTriangleMesh(meshDesc, writeBuffer);
    
    assert(status);
    
    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    PxTriangleMesh *trianguleMesh = Physics.gPhysicsSDK->createTriangleMesh(readBuffer);
    
    return trianguleMesh;
}

}