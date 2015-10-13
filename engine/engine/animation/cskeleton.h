#ifndef ANIMATION_CSKELETON_H_
#define ANIMATION_CSKELETON_H_

#include "mcv_platform.h"

#include "animationPlugger.h"

#include "render/render_utils.h"
#include "render/camera/culling.h"

#include "level/level.h"

class CalModel;
class CalBone;

namespace animation {
    
class CSkeleton : public level::SpatiallyIndexed {
    public:
        static float globalFactor;
    private:
		CalModel*  model = nullptr;
        int currentMainCycle = -1;
        float animSpeed = 1;
        unsigned bone0;
        render::CCulling::mask_t cullingMask;

		XMVECTOR prev = zero_v;

		int prevId = 0;
		utils::Counter<float> timer, timerCreep;

		bool mixingTimeCreep = false;
		bool canMixing = false;
		float aaa = 0.0f;
		float ddd = 0.0f;
		Handle meEntity;

    private:
        void bakeCullingMask();

    public:
        ~CSkeleton();

        void load(std::string name);
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
        void update(float elapsed);
        inline void init() {}		

#if defined(_DEBUG) && defined(RENDER_DEBUG_MESHES)
		void renderDebug3D() const;
		void renderBoneAxis(int bone_id, float scale) const;
#endif

        inline unsigned getBone0() const {return bone0;}
		void addBonesToBuffer();
        void testAndAddBonesToBuffer() {
            if (isSpatiallyGood() && cullingMask.any()){addBonesToBuffer();}
        }

		void stopAnimations();
        inline CalModel* getModel() const {return model;}
        inline void setAnimationSpeed (float speed) {animSpeed = speed;}

        bool disabled() const {return !isSpatiallyGood() || cullingMask.none();}

        inline render::CCulling::mask_t getCullingMask() const {return cullingMask;}

};

}

#endif