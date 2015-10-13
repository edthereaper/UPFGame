#ifndef RENDER_MESH_COMPONENT_H_
#define RENDER_MESH_COMPONENT_H_

#include "utils/XMLParser.h"

#include "mesh.h"
#include "../texture/material.h"

#include "handles/handle.h"
#include "components/color.h"

namespace render {

	//#define DEBUG_TRACK_MESHKEY

	/** COMPONENT CMesh
	* XML Attributes
	*  name - loads name.mesh
	*/
	class CMesh {
	public:

//		static const Mesh::groupId_t MAX_MATERIALS = 12;
		static const Mesh::groupId_t MAX_MATERIALS = 4;
		struct key_t {
		    public:
			    static const unsigned NO_COLOR = 0;
			    static const unsigned NO_SELFI = 0;
			    static const unsigned BAD_ID = ~0;

			    const Material* material = nullptr;
			    const Material* switchMat = nullptr;
			    component::Color tint = NO_COLOR;
			    component::Color selfIllumination = NO_SELFI;
			    Mesh::groupId_t group0 = key_t::BAD_ID;
			    Mesh::groupId_t groupf = key_t::BAD_ID;
    #if defined(_DEBUG) && defined (DEBUG_TRACK_MESHKEY)
			    bool __debug = false;
                inline void setDebugTrack(bool b = true) {__debug = b;}
    #else
                inline void setDebugTrack(bool b = true) {}
    #endif

		};

	private:
		Mesh* mesh = nullptr;
        float motionBlur = 1;
        float motionBlurModifier = 1;
        float selfIlluminationClamp = 1;
        float diffuseAsSelfIllumination = 0;
        float paintableAmount = 0;
		bool visible = true;
		unsigned nGroups = 0;
		key_t keys[MAX_MATERIALS];

	public:
		~CMesh();

        CMesh()=default;
        CMesh(const CMesh& copy) : mesh(copy.mesh), visible(copy.visible), nGroups(copy.nGroups) {
            for (int i = 0; i<MAX_MATERIALS; i++) {keys[i] = copy.keys[i];}
        }
        CMesh(CMesh&& move) : mesh(move.mesh), visible(move.visible), nGroups(move.nGroups) {
            move.mesh = nullptr;
            for (int i = 0; i<MAX_MATERIALS; i++) {
                keys[i] = move.keys[i];
                move.keys[i] = key_t();
            }
        }

		inline void setVisible(bool b = true) { visible = b; }
		inline bool isVisible() const { return visible; }

		//for procedurally generated meshes
		void addKey(const key_t& key, unsigned firstVertex = 0, unsigned nVertices = 0);

		inline Mesh* getMesh() const { return mesh; }
		void setMesh(Mesh* newMesh);
        void removeMesh();

		inline void update(float) {}
		void init();

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);

		inline utils::range_t<const key_t*> iterateKeys() const{
			return utils::range_t<const key_t*>(std::begin(keys), std::end(keys));
		}

		inline utils::range_t<key_t*> iterateKeys() {
			return utils::range_t<key_t*>(std::begin(keys), std::end(keys));
		}

		/* Load a mesh from a name, to a given entity.
		Loads the default mesh in case of failure.

		Use this to load meshes from code!
		*/
		static void load(const std::string name, component::Handle ownerEntity,
			const std::string defName = "default");

		void setMaterial(const Material* mat);
		void setMaterial(const Material* mat, unsigned keyId);
        
        inline float getMotionBlurAmount() const {return motionBlur * motionBlurModifier;}
        inline float getSelfIlluminationClamp() const {return selfIlluminationClamp;}
        inline float getMotionBlurModifier() const {return motionBlurModifier;}
        inline float getDiffuseAsSelfIllumination() const {return diffuseAsSelfIllumination;}
        inline float getPaintableAmount() const {return paintableAmount;}


        inline void setSelfIlluminationClamp(float f) {selfIlluminationClamp = f;}
        inline void setMotionBlurModifier(float f) {motionBlurModifier = f;}
        inline void setDiffuseAsSelfIllumination(float f) {diffuseAsSelfIllumination = f;}
        inline void setPaintableAmount(float f) {paintableAmount = f;}

        bool switchMaterials();

        inline bool hasRandomMaterials() {
            for(const auto& k : keys) {
                if (k.material != nullptr && k.material->isRandom()) {
                    return true;
                }
            }
            return false;
        }
	};

}

#endif