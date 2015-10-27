#ifndef LEVEL_LEVEL_H_
#define LEVEL_LEVEL_H_

#include "spatialIndex.h"

#include "handles/message.h"
#include "handles/handle.h"
#include "handles/entity.h"
#include "components/transform.h"
using namespace component;

#include "gameElements/gameMsgs.h"

#include "logic/trigger.h"

#include "fmod_User/fmodStudio.h"

//#define MUSIC_DISABLED

namespace level {

struct MsgSetLevel {
    DECLARE_MSG_ID();
    component::Handle levelEntity;
};

class CCheckPoint : public logic::Trigger_AABB<CCheckPoint>
{
    public:
        static void initType();
    private:
        component::Handle levelEntity_h;
        bool spawn;
		int order;
    public:
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts) {}

        inline void onStay(float) {/*Do nothing*/}
        inline void onAbsent(float) {/*Do nothing*/}
        inline void onExit(float) {/*Do nothing*/}

        void onEnter(float);

        inline void setLevel(component::Handle levelEntity) {levelEntity_h = levelEntity;}
        inline void setSpawn(bool b=true) {spawn=b;}
        inline void setCurrent(bool b=true) {spawn=b;}
        inline bool isSpawn() const {return spawn;}
		inline void setOrder(int b) { order = b; }
		inline int getOrder() { return order; }

        inline void receive(const gameElements::MsgSetPlayer& msg) {setPlayer(msg.playerEntity);}
        inline void receive(const MsgSetLevel& msg) {setLevel(msg.levelEntity);}

        inline XMVECTOR getPosition() {
            return ((component::CTransform*)Handle(this).getBrother<component::CTransform>())->getPosition();
        }

};

class CLevelData : public SpatiallyIndexed {
    public:
        static component::Handle currentLevel;
        static const float DEFAULT_ZFAR;

        struct DataReader : public utils::XMLParser {
            private:
                CLevelData* lvl;
            public:
                DataReader(CLevelData* lvl) : lvl(lvl) {}
                void onStartElement(const std::string& elem, utils::MKeyValue &atts);
        };
    private:
        component::Handle spawnCheckPoint;
        component::Handle currentCheckPoint;

        fmodUser::FmodStudio::EventInstance song = nullptr;

        //Not the most efficient thing out there...
        //but there would be only one (or a handful, tops) and it's move-friendly
        //also it might be barely used so won't take much space
        typedef std::pair<std::string, component::Handle> tag_t;
        typedef std::vector<tag_t> tagged_t;

        tagged_t tagged;
        bool bossLevel = false;
        bool highZFar = false;
        float zFar = DEFAULT_ZFAR;

        float skyboxBright;
        float skyboxBlend;

    public:
        void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
        inline void update(float elapsed) {}
        inline void init() {}

        void setSpawnCheckPoint(component::Handle checkPoint_h);
        void setCurrentCheckPoint(component::Handle checkPoint_h);
        component::Handle getSpawnCheckPoint() const {return spawnCheckPoint;}
        component::Handle getCurrentCheckPoint() const  {return currentCheckPoint;}
        
        inline std::vector<component::Handle> getTaggedEntity(const std::string& tag) const {
            std::vector<component::Handle> ret;
            for (const auto& p : tagged) {
                if (p.first == tag) {ret.push_back(p.second);}
            }
            return ret;
        }
        inline void setTaggedEntity(const std::string& tag, component::Handle h) {
            tagged.push_back(tag_t(tag, h));
        }
        
        inline void replaceTaggedEntity(
            component::Handle prev, component::Handle post) {
            for (auto& p : tagged) {
                if (p.second == prev) {p.second = post;}
            }
        }

        inline bool isBossLevel() const {return bossLevel;}
        inline bool isHighZFarLevel() const {return zFar != DEFAULT_ZFAR;}
        
        inline float getZFar() const {return zFar;}

        inline void playSong() const {
#ifndef MUSIC_DISABLED
            if (song != nullptr) {
                fmodUser::FmodStudio::playEvent(song);
            }
#endif
        }
        inline void stopSong() const {
#ifndef MUSIC_DISABLED
            if (song != nullptr) {
                fmodUser::FmodStudio::stopEvent(song);
            }
#endif
        }
		inline void pauseSong() const {
#ifndef MUSIC_DISABLED
			if (song != nullptr) {
				fmodUser::FmodStudio::pauseEvent(song);
			}
#endif
		}
		inline void resumeSong() const {
#ifndef MUSIC_DISABLED
			if (song != nullptr) {
				fmodUser::FmodStudio::resumeEvent(song);
			}
#endif
		}

        inline float getSkyboxBright() const { return skyboxBright;}
        inline float getSkyboxBlend() const { return skyboxBlend;}
};

}

#endif