#ifndef PX_USER_COLLISIONMESH
#define PX_USER_COLLISIONMESH

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>
#include "utils/data_provider.h"

namespace physX_user {

class CollisionMeshLoader {
    private:
        CollisionMeshLoader()=delete;
    public:
        static physx::PxTriangleMesh* load(utils::DataProvider& dp);
        static physx::PxTriangleMesh* load(const char *name);
        static bool validateMesh(const physx::PxTriangleMeshDesc& d);
};

}

#endif