#ifndef GAME_ELEMENTS_PROP_H_
#define GAME_ELEMENTS_PROP_H_

#include "mcv_platform.h"
#include "gameMsgs.h"

#include "handles/entity.h"

#include "render/mesh/mesh.h"
#include "components/Transform.h"

#include "behavior/fsm.h"

namespace gameElements {
    void initPropTypes();
    class TransformableFSMExecutor;
}
namespace behavior { typedef Fsm<gameElements::TransformableFSMExecutor> TransformableFSM; }
namespace gameElements{

class CTransformable;

enum transformType_e {
	TRANSFORMABLE_NONE,
	TRANSFORMABLE_MESH,
	TRANSFORMABLE_MATERIAL,
	TRANSFORMABLE_CREEP,
	TRANSFORMABLE_LIANA,
	TRANSFORMABLE_TINT,
	TRANSFORMABLE_HAMMER,
};

class TransformableFSMExecutor {
	public:
		friend CTransformable;
		friend behavior::TransformableFSM;

		enum states {
			STATE_notTransformed,
			STATE_hit,
			STATE_transforming,
			STATE_changeMeshOut,
			STATE_changeMeshIn,
            STATE_changeCreepOut,
            STATE_changeCreepIn,
			STATE_changeMaterial,
			STATE_changeTint,
			STATE_transformed,
			STATE_breathe,
			STATE_breatheXZ,
			STATE_spring,
            STATE_hammerTransform,
            STATE_hammerTransformed,
		};
        
        static void updateHighlights(float elapsed, bool increase);
        static void updateSpecialHighlights(float elapsed, bool increase);

    private:
        struct updateHighlightsFnData {
            float* prevGlow;
            float* currGlow;
            utils::Counter<float>* glowTimer;
            bool* goingDown;
            float rampUpTime;
            float rampDownTime;
            float growFreq;
            float glowCosDeviation;
            float glowCosMean;

            updateHighlightsFnData (
                float* prevGlow, float* currGlow,
                utils::Counter<float>* glowTimer, bool* goingDown,
                float rampUpTime, float rampDownTime,
                float growFreq, float glowCosDeviation, float glowCosMean) : 
                prevGlow(prevGlow), currGlow(currGlow),
                glowTimer(glowTimer), goingDown(goingDown),
                rampUpTime(rampUpTime), rampDownTime(rampDownTime),
                growFreq(growFreq), glowCosDeviation(glowCosDeviation), glowCosMean(glowCosMean)
            {}
        };
        static updateHighlightsFnData generalHLUpdateData;
        static updateHighlightsFnData specialHLUpdateData;
        static void updateHighlightsFn(float elapsed, bool increase, 
            updateHighlightsFnData& data);

        static const float TRANSFORMED_DIFFUSE_SELFILLUMINATION;
        static const float GLOW_FACTOR;
        static const float TransformableFSMExecutor::GLOW_RAMP_UP_TIME;
        static const float TransformableFSMExecutor::GLOW_RAMP_DOWN_TIME;
        static const float TransformableFSMExecutor::GLOW_FREQ;
        static const float TransformableFSMExecutor::GLOW_COS_DEVIATION;
        static const float TransformableFSMExecutor::GLOW_COS_MEAN;
        
        static const float TransformableFSMExecutor::GLOW_MARKED_RAMP_UP_TIME;
        static const float TransformableFSMExecutor::GLOW_MARKED_RAMP_DOWN_TIME;
        static const float TransformableFSMExecutor::GLOW_MARKED_FREQ;
        static const float TransformableFSMExecutor::GLOW_MARKED_COS_DEVIATION;
        static const float TransformableFSMExecutor::GLOW_MARKED_COS_MEAN;
        
        static const float TransformableFSMExecutor::GLOW_SPECIAL_RAMP_UP_TIME;
        static const float TransformableFSMExecutor::GLOW_SPECIAL_RAMP_DOWN_TIME;
        static const float TransformableFSMExecutor::GLOW_SPECIAL_FREQ;
        static const float TransformableFSMExecutor::GLOW_SPECIAL_COS_DEVIATION;
        static const float TransformableFSMExecutor::GLOW_SPECIAL_COS_MEAN;
        
        static utils::Counter<float> glowTimer;
        static float currGlow;
        static float prevGlow;
        static bool  glowingDown;
        
        static utils::Counter<float> glowTimer_special;
        static float currGlow_special;
        static float prevGlow_special;
        static bool  glowingDown_special;

	private:
		component::Handle meEntity;
		component::Handle xtraMesh1;
		component::Handle xtraMesh2;
		XMVECTOR savedPosition;
        component::Handle instancedMeshNot_h;
        component::Handle instancedMeshYes_h;
		utils::Counter<float> timer;
		XMVECTOR originalRotation = DirectX::XMQuaternionIdentity();
		XMVECTOR originalScale = utils::one_v;
		bool initTransformed = false;
		transformType_e type = TRANSFORMABLE_NONE;
        int hits = 0;
		int damageDone = 0;
        component::Color glowColor = Color(Color::SPRING_GREEN).setAf(0.8f);
        component::Color glowMarkedColor = Color::MAGENTA;
        float glowMax = 0.075f;
        float glowMarkedMax = 0.075f;
        float prevMarkedGlow = 0;
        float currMarkedGlow = 0;
        utils::Counter<float> glowMarkedTimer;
        bool markedGlowingDown = false;
        bool marked = false;
        unsigned instanceIndexNot = ~0;
        unsigned instanceIndex = ~0;

        updateHighlightsFnData markedGlowData = updateHighlightsFnData(
            &prevMarkedGlow, &currMarkedGlow,
            &glowMarkedTimer, &markedGlowingDown,
            GLOW_MARKED_RAMP_UP_TIME, GLOW_MARKED_RAMP_DOWN_TIME,
            GLOW_MARKED_FREQ, GLOW_MARKED_COS_DEVIATION, GLOW_MARKED_COS_MEAN
        );

		//Not the most optimal choice
		std::string resourceName;
		std::string originalresourceName;
        component::Color previousTint;
        component::Color targetTint;
        
        void setupTransforming();
        void setupTransformed();
        void applySelfIllumination(const component::Color& color, float glowMax, bool transformed, bool transforming=false);
        void applyTint(const component::Color& color,bool transformed, bool transforming=false);
        void applyDiffuseAsSelfIllumination(float f, bool transformed, bool transforming=false);
		void revive(const MsgRevive& msg = MsgRevive());

		//states
		behavior::fsmState_t notTransformed(float elapsed);
		behavior::fsmState_t hit(float elapsed);
		behavior::fsmState_t transforming(float elapsed);
        behavior::fsmState_t changeTint(float elapsed);
		behavior::fsmState_t changeMeshOut(float elapsed);
		behavior::fsmState_t changeMeshIn(float elapsed);
		behavior::fsmState_t changeCreepOut(float elapsed);
		behavior::fsmState_t changeCreepIn(float elapsed);
		behavior::fsmState_t changeMaterial(float elapsed);
		behavior::fsmState_t transformed(float elapsed);
		behavior::fsmState_t breathe(float elapsed);
		behavior::fsmState_t breatheXZ(float elapsed);
		behavior::fsmState_t spring(float elapsed);
		behavior::fsmState_t hammerTransform(float elapsed);
		behavior::fsmState_t hammerTransformed(float elapsed);

	public:
        ~TransformableFSMExecutor() {
            if (xtraMesh1.isValid()) {xtraMesh1.destroy();}
            if (xtraMesh2.isValid()) {xtraMesh2.destroy();}
        }

		inline behavior::fsmState_t init() { return reset(); }
		inline behavior::fsmState_t reset() {
			timer.reset();
			return initTransformed ? STATE_transformed : STATE_notTransformed;
		}
		inline void update(float elapsed) {
            markedGlowData.prevGlow = &prevMarkedGlow;
            markedGlowData.currGlow = &currMarkedGlow;
            markedGlowData.glowTimer = &glowMarkedTimer;
            markedGlowData.goingDown = &markedGlowingDown;
            markedGlowData.prevGlow = &prevMarkedGlow;
            markedGlowData.currGlow = &currMarkedGlow;
            markedGlowData.glowTimer = &glowMarkedTimer;
            markedGlowData.goingDown = &markedGlowingDown;
            updateHighlightsFn(elapsed, marked, markedGlowData);
        }
		inline void setTransformed(bool b = true) { initTransformed = b; }
        inline void setMarked(bool b=true) {marked = b;}
        inline bool isMarked() const {return marked;}
};

class CTransformable {
    public:
        friend TransformableFSMExecutor;
	private:
		behavior::TransformableFSM fsm;
		bool inert = false;
        bool selected = false;
		XMVECTOR centerAim = utils::zero_v;
		inline bool isNotTransformed() const {
            const auto& state = fsm.getState();
			return state == TransformableFSMExecutor::STATE_notTransformed
			    || state == TransformableFSMExecutor::STATE_hit
                || state == TransformableFSMExecutor::STATE_transforming;
        }
		inline bool isTransforming() const {
            const auto& state = fsm.getState();
			return state == TransformableFSMExecutor::STATE_changeMaterial
			    || state == TransformableFSMExecutor::STATE_changeMeshOut
			    || state == TransformableFSMExecutor::STATE_changeMeshIn
			    || state == TransformableFSMExecutor::STATE_changeCreepOut
			    || state == TransformableFSMExecutor::STATE_changeCreepIn
			    || state == TransformableFSMExecutor::STATE_changeTint;
		}

	public:
		inline void setResourceName(std::string newTransformProp) {
			fsm.getExecutor().resourceName = newTransformProp;
		}
		inline void setOriginalResourceName(std::string newTransformProp) {
			fsm.getExecutor().originalresourceName = newTransformProp;
		}
		inline std::string getResourceName() const {
			return fsm.getExecutor().resourceName;
		}

		inline transformType_e getType() const { return fsm.getExecutor().type; }
		inline void setType(transformType_e nType) { fsm.getExecutor().type = nType; }
		inline int getHits() const { return fsm.getExecutor().hits; }
        inline void setHits(int hits) {fsm.getExecutor().hits = hits; }

		inline bool isTransformed() const {
            const auto& state = fsm.getState();
			return state != TransformableFSMExecutor::STATE_notTransformed
			    && state != TransformableFSMExecutor::STATE_hit;
        }
		void receive(const MsgShot&);

		void receive(const MsgTransform&) {
			if (inert) {
				fsm.getExecutor().setTransformed();
				fsm.reset();
			}
		}

		void init();

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);

		inline void update(float elapsed) {
			if (!inert) {
                fsm.update(elapsed);
                auto& fsme = fsm.getExecutor();
                if (fsme.type == TRANSFORMABLE_HAMMER) {
                    if (!isTransformed()) {
                        float currGlow = TransformableFSMExecutor::currGlow_special;
                        float prevGlow = TransformableFSMExecutor::prevGlow_special;
                        if (currGlow != prevGlow) {
                            auto ma = currGlow*TransformableFSMExecutor::GLOW_FACTOR;
                            auto mc = Color(fsme.glowColor).setAf(ma*fsme.glowColor.af());
                            auto mm = fsme.glowMax*ma;
                            mc.premultiplyAlpha();
                            fsme.applySelfIllumination(mc, mm, false);
                        }
                    }
                }
            } else {
                auto& fsme = fsm.getExecutor();
                fsme.update(elapsed);
                if (fsme.currMarkedGlow != fsme.prevMarkedGlow) {
                    auto ma = fsme.currMarkedGlow*TransformableFSMExecutor::GLOW_FACTOR;
                    auto mc = Color(fsme.glowMarkedColor).setAf(ma*fsme.glowMarkedColor.af());
                    auto mm = fsme.glowMarkedMax*ma;
                    mc.premultiplyAlpha();
                    fsme.applySelfIllumination(mc, mm, false);
                }
                if (fsme.type == TRANSFORMABLE_HAMMER) {
                    if (!isTransformed() && selected) {
                        fsme.damageDone = false;
                        fsme.notTransformed(elapsed);
                    }
                } 
            }
            if (fsm.getExecutor().type == TRANSFORMABLE_HAMMER) {
                Entity* me = Handle(this).getOwner();
                CAABB* aabb = me->get<CAABB>();
                CTransform* t = me->get<CTransform>();
                setCenterAim(DirectX::operator+(aabb->getCenter(), t->getPosition()));
            } 
		}

		inline void setInert(bool b = true) { inert = b; }
		inline bool getInert() const { return inert; }

		inline void setSelected(bool b = true) { selected = b; }
		inline bool getSelected() const { return selected; }

		inline void removeGlow() {
                auto& fsme = fsm.getExecutor();
                fsme.currMarkedGlow = 0;
                fsme.applySelfIllumination(0, 1, false);
                fsme.applySelfIllumination(0, 1, true);
        }

        void setInstanced(Handle nInstancedMeshNot_h, Handle nInstancedMeshYes_h,
            unsigned untransformatedIndex) {
            auto& fsme(fsm.getExecutor());
            fsme.instancedMeshNot_h = nInstancedMeshNot_h;
            fsme.instancedMeshYes_h = nInstancedMeshYes_h;
            fsme.instanceIndexNot = fsme.instanceIndex = untransformatedIndex;
        }

		void setCenterAim(XMVECTOR aaa) { centerAim = aaa; }
		XMVECTOR getCenterAim() { return centerAim; }
        inline void setMarked(bool b=true) {fsm.getExecutor().setMarked(b);}
        inline bool isMarked() const {return fsm.getExecutor().isMarked();}

        inline void spring() {
            auto& fsme = fsm.getExecutor();
            fsme.timer.reset();
            fsm.changeState(TransformableFSMExecutor::STATE_spring);
        }

		inline void revive(const MsgRevive& msg = MsgRevive()) {
            fsm.getExecutor().revive(msg);
            fsm.reset();
        }
};

class CProp {
	public:
		static const component::EntityListManager::key_t TAG = 0xBAD00001;
    public:
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}
        void update(float elapsed) {}
        inline void init() {}
};

class CTrampoline {
	public:
		static const EntityListManager::key_t TAG = 0xBAD00002;
	private:
		DirectX::XMVECTOR rotQ = DirectX::XMQuaternionIdentity();
		component::Handle playerEntity;
	public:
		CTrampoline() = default;
		CTrampoline(DirectX::XMVECTOR rotQ) : rotQ(rotQ){}
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}
		void update(float elapsed) {}
		inline void init() {}

		inline void setRotation(DirectX::XMVECTOR rot_q) { rotQ = rot_q; }
		inline XMVECTOR getRotation(){ return rotQ; }
		inline XMVECTOR getNormal() {
			component::CTransform t;
			t.setRotation(rotQ);
			return t.getUp();
		}

        inline DirectX::XMVECTOR getRotation() const {return rotQ;}

        void receive(const MsgTransform&);
		void receive(const MsgSetPlayer&);
        void revive(const MsgRevive& msg = MsgRevive());
};

class CCannon {
    public:
        static const EntityListManager::key_t TAG = 0xBAD00003;
    private:
        DirectX::XMVECTOR rotQ = DirectX::XMQuaternionIdentity();
        DirectX::XMVECTOR lookAt = utils::zero_v;
		component::Handle playerEntity;
        float fovV, fovH, impulse;
        bool bossCannon=false;

        void lookAtTarget();
	public:
		CCannon() = default;
		CCannon(DirectX::XMVECTOR rotQ) : rotQ(rotQ){}
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
		void update(float elapsed){}
		inline void init() {}

		inline void setRotation(DirectX::XMVECTOR rot_q) { rotQ = rot_q; }
		inline XMVECTOR getRotation(){ return rotQ; }
        inline XMVECTOR getNormal() {
            component::Transform t;
            t.setRotation(rotQ);
            return t.getUp();
        }

        inline float getFovH() const {return fovH;}
        inline float getFovV() const {return fovV;}
        inline float getImpulse() const {return impulse;}
        inline void setFov(float h, float v) {fovH = h; fovV = v;}
        inline void setImpulse(float i) {impulse = i;}

        void receive(const MsgTransform&);
        void receive(const MsgFlyingMobileEnded&);
		void receive(const MsgSetPlayer&);
        void revive(const MsgRevive& msg = MsgRevive());
};

class CCreep {
	public:
		static const EntityListManager::key_t TAG = 0xBAD00005;
	private:
		component::Handle playerEntity;
		XMVECTOR normal;
		XMFLOAT2 posterOffset;
        float posterSize = 3;
		float height = 0.f, width = 0.f;
    private:
        void generateCreepPlane();
        void generatePosterPlane();
	public:
		CCreep() = default;
		CCreep(XMVECTOR normal, float height, float width) :
			normal(normal), height(height), width(width) {}

        inline float getHeight() const {return height;}
        inline float getWidth() const {return width;}
        inline XMVECTOR getNormal() const {return normal;}
        inline void setup(XMVECTOR n, float w, float h, XMFLOAT2 offset) {
            normal = n; width = w, height = h;
            posterOffset = offset;
        }
        inline void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {
            posterSize = atts.getFloat("posterSize", posterSize);
        }
        inline void update(float elapsed) {}
        inline void init() {generatePosterPlane();}

        inline void setupTransformed() {generateCreepPlane();}

        void receive(const MsgTransform&);
		void receive(const MsgSetPlayer&);
        void revive(const MsgRevive& msg = MsgRevive());
};

}

#endif
