#ifndef ANIMATION_ARM_POINT_H_
#define ANIMATION_ARM_POINT_H_

#include "mcv_platform.h"

#include "cBoneLookAt.h"

namespace animation {

class CArmPoint {
    public:
        typedef CBoneLookAt::entry_t::parameters_t parameters_t;
        enum bone_e {UPPER_ARM, FOREARM};
    private:
        CBoneLookAt lookAt;
        CBoneLookAt::entryId_t armLookAt, forearmLookAt;
        int calBoneId[3]; // UpperArm, Forearm, Hand
        int calBoneTwist[2]; // UpArmTwist, ForeTwist
        XMVECTOR target;
        bool active=true;
    public:
        CArmPoint() {
            calBoneId[0] = -1;
            calBoneId[1] = -1;
            calBoneId[2] = -1;
            calBoneTwist[0] = -1;
            calBoneTwist[1] = -1;
        }

        void setUpperArm(int calBone);
        void setUpperArmTwist(int calBone);
        void setForeArm(int calBone);
        void setForeArmTwist(int calBone);
        void setHand(int calBone);

        inline int  getUpperArm()     {return calBoneId[0];}
        inline int  getUpperArmTwist(){return calBoneTwist[0];}
        inline int  getForeArm()      {return calBoneId[1];}
        inline int  getForeArmTwist() {return calBoneTwist[1];}
        inline int  getHand()         {return calBoneId[2];}

        void setForeArmRef(XMVECTOR v);
        XMVECTOR getForeArmRef() const;
        void setUpperArmRef(XMVECTOR v);
        XMVECTOR getUpperArmRef() const;

        void setBones(
            int armId, int forearmId, int handId,
            XMVECTOR armReference = yAxis_v, XMVECTOR forearmReference = yAxis_v,
            int armTwistId=-1, int foreTwistId=-1);
        void setBones(
            std::string armName, std::string forearmName, std::string handName,
            XMVECTOR armReference = yAxis_v, XMVECTOR forearmReference = yAxis_v,
            std::string armTwistName = "<none>", std::string foreTwistName= "<none>");

        inline void setTarget(const XMVECTOR v) { target = v;}
        inline XMVECTOR getTarget() const {return target;}
        
        void loadFromProperties(std::string elem, utils::MKeyValue atts);
        inline void init() {}

        void setActive(bool b=true);
        void setAllActive(bool b=true);
        inline bool getActive() const {return active;}

        void update(float);

		XMVECTOR getPosHand() const;

        inline void setParams(const parameters_t& params, const bone_e& e) {
            lookAt.getEntry(e==UPPER_ARM?armLookAt:forearmLookAt).setParameters(params);
        }
        inline parameters_t getParams(const bone_e& e) const {
            return lookAt.getEntry(e==UPPER_ARM?armLookAt:forearmLookAt).getParameters();
        }

        inline float getGlobalStrength() const {return lookAt.getGlobalStrength();}
        inline void setGlobalStrength(float s) {return lookAt.setGlobalStrength(s);}
};

}

#endif