#ifndef INC_SKELETON_MANAGER_H_
#define INC_SKELETON_MANAGER_H_

#include "utils/itemsByName.h"
#include "cal3d/cal3d.h"
#include "utils/XMLParser.h"
#include "render/mesh/mesh.h"
#include "render/render_utils.h"

using namespace utils;
using namespace render;

namespace animation {

// Things that Cal3D doesn't have
class CoreModel : public CalCoreModel, public XMLParser
{
    public:
        typedef ItemsByName<CoreModel> Manager;
        static inline Manager& getManager(){return manager;}
        typedef std::vector<int>  boneIdVector;
    private:
        static Manager manager;

    private:
        std::string root_path;
        std::string name;
        void onStartElement(const std::string &elem, MKeyValue &atts);
    
    public:
        CoreModel() : CalCoreModel("<unnamed>"), name("<unnamed>") {}
        bool load(const char* name);
        
        #if defined(_DEBUG) && defined(RENDER_DEBUG_MESHES)
            boneIdVector bone_ids_to_debug;
        #endif
};
        
XMVECTOR toXMVECTOR(CalVector q);
CalVector toCalVector(XMVECTOR v);
CalQuaternion toCalQuaternion(XMVECTOR v);
XMVECTOR toXMQuaternion(CalQuaternion q);

}

#endif