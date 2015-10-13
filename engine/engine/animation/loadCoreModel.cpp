#include "mcv_platform.h"
#include "skeleton_manager.h"

#include "cal3d/coretrack.h"
#include "cal3d/corekeyframe.h"

#include "convertCMF.h"
using namespace utils;

using namespace DirectX;

namespace animation {

void CoreModel::onStartElement(const std::string &elem, MKeyValue &atts)
{
    if (elem == "skeleton") {
        // Load .csf
        std::string csf = root_path + name + ".csf";
        bool is_ok = loadCoreSkeleton(csf);
        assert(is_ok);
        
        auto bones = getCoreSkeleton()->getVectorCoreBone();        
        float scale_factor = atts.getFloat("scale", 1.0f);
        scale(scale_factor);

    } else if (elem == "mesh") {
        std::string name = atts["name"];
        std::string smesh = "data/meshes/" + name + ".mesh";
        FileDataProvider fdp(smesh.c_str());
        if(!fdp.isValid()) {
            // Load .cmf
            std::string cmf = root_path + name + ".cmf";
            int mesh_id = loadCoreMesh(cmf);
            assert(mesh_id >= 0);
            bool ok = convertCMF(getCoreMesh(mesh_id), smesh.c_str());
            assert(ok);
        }
    } else if (elem == "anim") {
        std::string name = atts["name"];
        std::string caf = root_path + name + ".caf";
        int id = loadCoreAnimation(caf);
        addAnimationName(name, id);
        assert(id >= 0);

    } else if (elem == "dump_bone") {
        int id = atts.getInt("id", -1);
        if ( id != -1 ) {
			#if defined(_DEBUG) && defined(RENDER_DEBUG_MESHES)
				bone_ids_to_debug.push_back(id);
			#endif
        }
    } else if (elem == "dump_anim") {
        int the_anim_id = atts.getInt("id", 0);
        int the_bone_id = atts.getInt("bone", 0);
        CalCoreAnimation* core_anim = getCoreAnimation(the_anim_id);
        // get the list of core tracks of above core animation
        std::list<CalCoreTrack *>& listCoreTrack = core_anim->getListCoreTrack();
        // loop through all core tracks of the core animation
        for (auto i = listCoreTrack.begin(); i != listCoreTrack.end(); ++i) {
            int bone_id = (*i)->getCoreBoneId();
            if (bone_id == the_bone_id) {
                float anim_duration = core_anim->getDuration();
                for (float t = 0.0f; t < anim_duration; t += 1.f / 30.f) {
                    CalVector trans;
                    CalQuaternion q;
                    (*i)->getState(t, trans, q);
            
                    XMVECTOR dxq = toXMQuaternion(q);
                    XMMATRIX m = DirectX::XMMatrixRotationQuaternion(dxq);
                    XMVECTOR front = m.r[0];
                    float yaw = getYawFromVector(front);
                    dbg("At time %f Trans = %f %f %f q=%f %f %f %f Yaw:%f\n", t,
                        trans.x, trans.y, trans.z,
                        q.x, q.y, q.z, q.w,
                        rad2deg( yaw ));
                }
            }
        }
        
    }    
}

bool CoreModel::load(const char* newName)
{
    name = newName;
    root_path = "data/skeletons/" + name + "/";
    
    CalLoader::setLoadingMode(LOADER_ROTATE_X_AXIS|LOADER_INVERT_V_COORD);
    
    char full_name[MAX_PATH];
    sprintf(full_name, "%s%s.xml", root_path.c_str(), name.c_str());
    return xmlParseFile(full_name);
}

}
