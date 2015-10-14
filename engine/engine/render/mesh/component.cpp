#include "mcv_platform.h"
#include "component.h"

#include "handles/entity.h"
#include "handles/prefab.h"
#include "components/transform.h"
#include "components/color.h"
#include "utils/data_provider.h"

using namespace component;
using namespace utils;

//#define _DEBUG_RENDER

#include "../renderManager.h"

namespace render {

CMesh::~CMesh()
{
	if (mesh != nullptr) {
        mesh = nullptr;
        Mesh::getManager().destroy(mesh);
		RenderManager::deleteKeys(Handle(this).getOwner());
	}
}

void CMesh::removeMesh()
{
	if (mesh != nullptr) {
        for (auto&i : keys) {
            i = key_t();
        }
		RenderManager::deleteKeys(Handle(this).getOwner());
        mesh = nullptr;
	}
}

void CMesh::setMesh(Mesh* newMesh)
{
    if (newMesh != mesh) {
        removeMesh();
    	mesh = newMesh;
    }
}

void CMesh::init()
{
	if (mesh != nullptr) {
		RenderManager::addKeys(Handle(this).getOwner());
	}
}

void CMesh::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
{
    static const auto defaultMat = Material::getManager().getByName("default");
    static std::string meshName;
	if (elem == "Mesh") {
        if (atts.has("name")) {
            meshName = atts.getString("name", "default");
            mesh = Mesh::getManager().getByName(meshName);
        }
        motionBlur = atts.getFloat("motionBlur", motionBlur);
        selfIlluminationClamp = atts.getFloat("selfIlluminationClamp", selfIlluminationClamp);
        diffuseAsSelfIllumination = atts.getFloat("diffuseAsSelfIllumination", diffuseAsSelfIllumination);
        paintableAmount = atts.getFloat("paintableAmount", paintableAmount);
        visible = atts.getBool("visible", visible);
	} else if (elem == "submesh") {

        Mesh::groupId_t group = atts.getInt("id", -1);
		assert(group >= 0);
        Material* material = nullptr;
        if (atts.has("material")) {
            std::string matName = atts.getString("material");
            material = Material::getManager().getByName(matName);
            #ifdef _DEBUG
                if (matName == "default") {
                    dbg("Mesh %s uses default material.\n", meshName.c_str());
                }
            #endif
            if (material == nullptr) {
                material = defaultMat;
                #ifdef _DEBUG
                    dbg("Mesh %s uses missing material %s.\n", meshName.c_str(), matName.c_str());
                #endif
            }
        } else {
            material = defaultMat;
        }
        assert(material != nullptr);
        Material* switchMat = nullptr;
        if (atts.has("switchMat")) {
            auto& matMan = Material::getManager();
            switchMat = matMan.getByName(atts.getString("switchMat", "black"));
            if (switchMat == nullptr) {
                switchMat = matMan.getByName("black");
            }
        }

        if (nGroups > 0 &&                          //There are other groups
            switchMat == nullptr &&                 //And I am not special
            keys[nGroups-1].groupf == group-1 &&    //And the last group ends right before this submesh
            keys[nGroups-1].material == material && //And we both use the same material
            keys[nGroups-1].switchMat == nullptr    //And the last group didn't switch materials
            ) {
            //Chain key
            keys[nGroups-1].groupf = group;
        } else {
            //Create a new key
		    assert(nGroups<MAX_MATERIALS && "Number of supported materials is limited!\n");
		    key_t& submesh = keys[nGroups];
		    submesh.material = material;
		    submesh.switchMat = switchMat;
		    submesh.group0 = submesh.groupf = group;
		    submesh.selfIllumination = submesh.tint = 0;
		    nGroups++;
        }
	}
}

void CMesh::load(const std::string name, Handle ownerEntity, const std::string defName)
{
	assert(ownerEntity.isValid());
	std::string path = "data/prefabs/mesh/" + name;
	if (DirLister::isDir(path)) {
		auto files = DirLister::listDir(path);
		assert(files.size() > 0);
		std::string filenamefile(files[utils::rand_uniform((int)files.size() - 1)]);
		filenamefile.erase(filenamefile.find_last_of('.'));
		load(name + "/" + filenamefile, ownerEntity);
	}
	else {
		std::string str("mesh/" + name);
		if (!PrefabManager::get().prefabricateComponents(str.c_str(), ownerEntity)) {
			load(defName, ownerEntity);
		}
	}

}

void CMesh::addKey(const key_t& key, unsigned first, unsigned n)
{
	for (auto& k : keys) {
		if (k.group0 == key_t::BAD_ID || k.groupf == key_t::BAD_ID) {
			k = key;
			mesh->groups.push_back(
				Mesh::group_t(
				first,
				n == 0 ?
				(((mesh->ib == nullptr) ? mesh->nvertexs : mesh->nindices) - first) :
				n
				));
			return;
		}
	}
	assert(false && "No keys remaining.");
}

void CMesh::setMaterial(const Material* mat)
{
	for (auto& key : iterateKeys()) {
		key.material = mat;
	}
    Handle owner = Handle(this).getOwner();
	RenderManager::deleteKeys(owner);
	RenderManager::addKeys(owner);
}

bool CMesh::switchMaterials()
{
    bool switched = false;
	for (auto& key : iterateKeys()) {
        auto smat = key.switchMat;
        if (smat != nullptr) {
            auto mat = key.material;
            key.material = smat;
            key.switchMat = mat;
            switched = true;
        }
	}
    if (switched) {
        Handle owner = Handle(this).getOwner();
	    RenderManager::deleteKeys(owner);
	    RenderManager::addKeys(owner);
    }
    return switched;
}

void CMesh::setMaterial(const Material* mat, unsigned keyId)
{
	assert(keyId < MAX_MATERIALS);
	keys[keyId].material = mat;
    Handle owner = Handle(this).getOwner();
	RenderManager::deleteKeys(owner);
	RenderManager::addKeys(owner);
}

}
