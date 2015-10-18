#ifndef PX_USER_COLLISIONMESH
#define PX_USER_COLLISIONMESH

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>
#include "utils/data_provider.h"

namespace physX_user {

struct staticBuffer_t {
    unsigned char* data = nullptr;
    size_t size = 0;

    staticBuffer_t(unsigned char* in, size_t size) : size(size) {
        data = new unsigned char[size];
        std::memcpy(data, in, size);
    }
    ~staticBuffer_t() {
        SAFE_DELETE(data);
        size = 0;
    }
};

class CollisionMeshLoader {
    private:
        CollisionMeshLoader()=delete;
        
        static physx::PxTriangleMesh* load(physx::PxDefaultMemoryInputData& readBuffer);
        static staticBuffer_t cook(utils::DataProvider& dp);
    public:
        static physx::PxTriangleMesh* load(const std::string& name);
        static bool validateMesh(const physx::PxTriangleMeshDesc& d);
};

}

#endif