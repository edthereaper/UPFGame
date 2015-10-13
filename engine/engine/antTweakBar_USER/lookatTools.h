#ifndef LOOKAT_TOOLS_H_
#define LOOKAT_TOOLS_H_
#ifdef LOOKAT_TOOL

#include "antTW.h"

namespace antTw_user {

class LookAtTW {
    public:
        struct entryTw_t {
            component::Handle handle;
            animation::CBoneLookAt::entryId_t id;
            LookAtTW* self;
            bool relativeRef = false;
            bool draw = false;
        };

    private:
        TwBar* bar = nullptr;
        TwType boneEnum, validBoneEnum;
        component::Handle entity_h;
        component::Handle camera_h;
        std::string filename;
        component::Color color;
        bool active=true;
        bool muted=false;
        bool valid;

        entryTw_t entryTw[animation::CBoneLookAt::N_LOOKATS];
        XMVECTOR target;

    private:
        void createBoneEnum();
        void createEntry(animation::CBoneLookAt::entryId_t);
    public:
        ~LookAtTW() {if(bar!=nullptr) {TwDeleteBar(bar);}}
        LookAtTW(component::Handle entity, component::Handle camera, std::string filename, component::Color c = 0);
        inline component::Color getColor() const {return color;}
        inline XMVECTOR getTarget() const {return target;}
        void setTarget(XMVECTOR v);
        void resetTarget();
        void save();
        void reload();

        void setActive(bool b);
        void setMuted(bool b);
        inline bool isActive() const {return active;}
        inline bool isMuted() const {return muted;}
        inline component::Handle getEntity() const {return entity_h;}
        inline component::Handle getCamera() const {return camera_h;}
        inline bool isValid() const {return valid;}

        void draw() const;
};

class ArmPointTW {
    public:

    private:
        TwBar* bar = nullptr;
        TwType boneEnum;
        component::Handle entity_h;
        component::Handle camera_h;
        std::string filename;
        component::Color color;
        bool active=true;
        bool muted=false;
        bool valid;

        bool relativeForeRef  = true;
        bool relativeUpperRef = true;
        bool relativeHandRef  = true;

        bool drawUpper = false;
        bool drawFore = false;

        XMVECTOR target;

    private:
        void createBoneEnum();
        void createEntry(animation::CBoneLookAt::entryId_t);
    public:
        ~ArmPointTW() {if(bar!=nullptr) {TwDeleteBar(bar);}}
        ArmPointTW(component::Handle entity, component::Handle camera, std::string filename, component::Color c = 0);
        inline component::Color getColor() const {return color;}
        inline XMVECTOR getTarget() const {return target;}
        void setTarget(XMVECTOR v);
        void resetTarget();
        void save();
        void reload();

        void setActive(bool b);
        void setMuted(bool b);
        inline bool isActive() const {return active;}
        inline bool isMuted() const {return muted;}
        inline component::Handle getEntity() const {return entity_h;}
        inline component::Handle getCamera() const {return camera_h;}
        inline bool isValid() const {return valid;}

        inline bool useRelFore() const {return relativeForeRef;}
        inline bool useRelUpper() const {return relativeUpperRef;}
        inline bool useRelHand() const {return relativeHandRef;}
        
        void draw() const;
};


#ifdef LOOKAT_TOOL
extern LookAtTW* lookAtTw[4];
extern ArmPointTW* armPointTw[4];
#endif

}
#endif
#endif