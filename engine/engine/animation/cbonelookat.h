#ifndef ANIMATION_LOOKAT
#define ANIMATION_LOOKAT

#include "mcv_platform.h"
#include "skeleton_manager.h"

#include "components/transform.h"

namespace animation {

class CBoneLookAt {
    public:
        static const unsigned N_LOOKATS = 4;
        typedef uint8_t entryId_t;
        struct entry_t {
            private:
                struct correction_t {
                    bool rotate;
                    bool recovering;
                    XMVECTOR rotQ = DirectX::XMQuaternionIdentity();

                    correction_t(bool recovering, bool rotate) : rotate(rotate), recovering(recovering) {}
                    correction_t(XMVECTOR rotQ, bool recovering, bool rotate) :
                        rotate(rotate), rotQ(rotQ), recovering(recovering) {}
                };
                static void applyCorrection(CalBone* bone, CalBone* bone2, bool dual, correction_t data);

            public:
                friend CBoneLookAt;
            private:
                bool used = false;

                utils::Counter<float> ipolAccum;
                DirectX::XMVECTOR prevCorrection;
                float prevAngle=0;
                float prevIpol=0;
                bool recovering = false;

                correction_t calculateCorrection(float factor, XMVECTOR desiredDir,
                    float maxAngle, float elapsed);
                correction_t recovery();
                XMVECTOR getDir(XMVECTOR target, CalBone* bone1, CalBone* bone2, bool dual) const;

                DirectX::XMVECTOR target = utils::zero_v;
                DirectX::XMVECTOR iPolTarget = utils::zero_v;
                float iPolTargetTime = 1;

                DirectX::XMVECTOR referenceDir  = yAxis_v;
            public:
                struct parameters_t {
                    float ipolTime = 0.f;
                    float strength = 1.f;
                    float maxAngleXZ = M_4PIf;
                    float maxAngleY = M_4PIf;
                    float threshold = M_4PIf;
                    float fadeIn = 0;
                    float fadeOut = 0;
                } params;
                enum {INACTIVE, ACTIVE, FADEOUT} state = ACTIVE;
                int calBone=-1;
                int calSecondBone=-1;

            public:
                void apply(const CalModel* model, float factor, float elapsed);
                inline void setMaxAngle(float maxAngle) {
                    params.maxAngleXZ = maxAngle;
                    params.maxAngleY = maxAngle;
                }
                inline void setParameters(const parameters_t& nParams) {params = nParams;}
                inline parameters_t getParameters()const {return params;}
                void setActive(bool active);
                inline bool isActive() const {return  state != INACTIVE;}
                inline bool isInactive() const {return  state != ACTIVE;}

                XMVECTOR currentTarget(float elapsed = 0);

                void setTarget(const XMVECTOR& nTarget, bool ipol = false);
                
                inline void changeTarget(const XMVECTOR& nTarget) {
                    target = nTarget;
                }

                inline XMVECTOR getTarget() const {return target;}
                inline XMVECTOR getIpolTarget() const {return target;}

                inline XMVECTOR getRef() const {return referenceDir;}
                inline void setRef(XMVECTOR v) {referenceDir = v;}

            public:
                friend CBoneLookAt;
        };
        float globalStrength = 1.f;
        bool active = false;

    private:
        entry_t entries[N_LOOKATS];

    public:
        CBoneLookAt()=default;
        ~CBoneLookAt();

        inline void init() {}
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
        
        //Must be a component
        void update(float elapsed);
        //Doesn't need to be a component
        void update(CalModel*  model, float elapsed);
        
        inline entry_t& getEntry(entryId_t i) {
            assert(i<N_LOOKATS);
            return entries[i];
        }
        inline const entry_t& getEntry(entryId_t i) const {return entries[i];}
        inline void freeEntry(entryId_t i) {entries[i].used = false;}
        
        inline void setGlobalStrength(float strength) {globalStrength = strength;}
        inline float getGlobalStrength() const {return globalStrength;}
        
        //Must be a component
        entryId_t newEntry(const std::string& calBone, XMVECTOR reference = utils::zAxis_v,
            const std::string& calSecondBone = "<none>");
        //Doesn't need to be a component
        entryId_t newEntry(int calBone, XMVECTOR referenceXZ = utils::zAxis_v,
            int calSecondBone=-1);

        //Set the target for all entries
        inline void setTarget(XMVECTOR target, bool ipol = false) {
            for(auto& entry : entries) {
                entry.setTarget(target, ipol);
            }
        }

        //Set the target for all entries
        inline void changeTarget(XMVECTOR target) {
            for(auto& entry : entries) {
                entry.changeTarget(target);
            }
        }

        //Set the parameters for all entries
        inline void setParams(entry_t::parameters_t params) {
            for(auto& entry : entries) {entry.params = params;}
        }
        inline void setActive(bool b=true) {active=b;}
        inline void setAllActive(bool b=true) {
            for(auto& e : entries) {e.setActive(b);}
        }
};

}

#endif