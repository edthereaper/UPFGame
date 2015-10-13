#ifndef _TRIGGER_FILTER_SIMULATOR_
#define _TRIGGER_FILTER_SIMULATOR_

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>
#include "mcv_platform.h"

namespace physX_user {

class Shape;

struct filter_t {
    public:
        enum id_t {
            NONE                = 0,
            PLAYER              = 1<<0,
            ENEMY               = 1<<1,
            SCENE               = 1<<2,
            PROP                = 1<<3,
            TOOL                = 1<<4,
            LIANA               = 1<<5,
            BULLET              = 1<<6,
            PICKUP              = 1<<7,
            DESTRUCTIBLE        = 1<<8,
            FLARESHOT           = 1<<9,
            CANNONPATH          = 1<<10,
            PLAYERCANNON        = 1<<11,
			KNIFE				= 1<<12,
			BOSS                = 1<<13,
			WEAK_SPOT			= 1<<14,
			SMOKE_PANEL			= 1<<15,
			PARTICLES_SYSTEM	= 1<<16,
			PAINT_SPHERE        = 1<<17,

            TOOLS_SELECTABLE    = 1<<31,
            ALL_IDS             = ~0,
        };
        enum traits_t {
            NOTHING         = 0,
            CCD             = 1<<0,
            TRIGGER         = 1<<1,
            STATIC          = 1<<2,
            CCT             = 1<<3,
			DEFAULT			= 1 << 31, //So entities pass PhysX's default test
            ALL_TRAITS      = ~0,
        };

        friend Shape;
        
        static id_t getByName(std::string name);

        static filter_t getFromStrings(
            std::string is, std::string supress, std::string report);

	    static physx::PxFilterFlags filter(
            physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
		    physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
		    physx::PxPairFlags& pairFlags,
		    const void* constantBlock, physx::PxU32 constantBlockSize);
        static bool filter(filter_t filter0, filter_t filter1);
        static bool testReport(filter_t filter0, filter_t filter1);

        inline filter_t operator|(const filter_t& f) const {
            filter_t ret;
            ret.is = id_t(is | f.is);
            ret.supress = id_t(supress | f.supress);
            ret.report = id_t(report | f.report);
			ret.has = traits_t(has | f.has | DEFAULT);
            return ret;
        }

        inline bool operator==(const filter_t& f) {
            return is ==f.is && supress == f.supress && report == f.report && f.has == f.has;
        }

        inline filter_t& operator|=(const filter_t& f) {return *this = operator|(f);}

        inline filter_t operator&(const filter_t& f) const {
            filter_t ret;
            ret.is = id_t(is & f.is);
            ret.supress = id_t(supress & f.supress);
            ret.report = id_t(report & f.report);
			ret.has = traits_t((has & f.has) | DEFAULT);
            return ret;
        }
        inline filter_t& operator&=(const filter_t& f) {return *this = operator&(f);}

        inline filter_t operator~() const {
            filter_t ret;
            ret.is = id_t(~is);
            ret.supress = id_t(~supress);
            ret.report = id_t(~report);
			ret.has = traits_t((~has) | DEFAULT);
            return ret;
        }

    private:
		traits_t has = DEFAULT;
    public:
        id_t is      = NONE;
        id_t supress = NONE;
        id_t report  = NONE;

    public:
        filter_t(const physx::PxFilterData& px) :
            is(id_t(px.word0)),
            supress(id_t(px.word1)),
            report(id_t(px.word2)),
			has(traits_t(px.word3 | DEFAULT))
            {}
		filter_t(id_t is, id_t supress = NONE, id_t report = NONE, traits_t has = DEFAULT) :
			is(is), supress(supress), report(report), has(has) {
			has = traits_t(has | DEFAULT);
		}
		filter_t(int is, int supress = NONE, int report = NONE, int has = DEFAULT) :
			is(id_t(is)), supress(id_t(supress)), report(id_t(report)), has(traits_t(has)) {
			has = traits_t(has | DEFAULT);
		}
		filter_t() = default;
		operator physx::PxFilterData() const {
			physx::PxFilterData fd;
			fd.word0 = is;
			fd.word1 = supress;
			fd.word2 = report;
			fd.word3 = has | DEFAULT;
			return fd;
		}

        inline bool isTrigger() const {
            return (has & TRIGGER) != 0;
        }
};

}

#endif