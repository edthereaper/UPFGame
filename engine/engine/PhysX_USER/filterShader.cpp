#include "mcv_platform.h"
#include "filterShader.h"

using namespace physx;

namespace physX_user {

filter_t::id_t filter_t::getByName(std::string name) {
    if      (name=="player")            {return PLAYER;}
    else if (name=="enemy")             {return ENEMY;}
	else if (name=="knife")             {return KNIFE;}
    else if (name=="scene")             {return SCENE;}
    else if (name=="prop")              {return PROP;}
    else if (name=="tool")              {return TOOL;}
    else if (name=="liana")             {return LIANA;}
    else if (name=="bullet")            {return BULLET;}
    else if (name=="pickup")            {return PICKUP;}
    else if (name=="destructible")      {return DESTRUCTIBLE;}
    else if (name=="flare-shot")        {return FLARESHOT;}
    else if (name=="cannon-path")       {return CANNONPATH;}
    else if (name=="player-cannon")     {return PLAYERCANNON;}
    else if (name=="boss")              {return BOSS;}
    else if (name=="weak-spot")         {return WEAK_SPOT;}
    else if (name=="smoke-panel")       {return SMOKE_PANEL;}
	else if (name=="player_system")	    {return PARTICLES_SYSTEM;}
	else if (name=="paint-sphere")	    {return PAINT_SPHERE;}
    
    else if (name=="tools-selectable")  {return TOOLS_SELECTABLE;}
    else if (name=="all")               {return id_t(~0);}
    else {return NONE;}
}

filter_t filter_t::getFromStrings(
    std::string is, std::string supress, std::string report)
{
    filter_t ret;
    ret.is      = utils::enumFromString<id_t>(is, NONE, getByName);
    ret.supress = utils::enumFromString<id_t>(supress, NONE, getByName);
    ret.report  = utils::enumFromString<id_t>(report, NONE, getByName);
    return ret;
}

bool filter_t::filter( filter_t filter0, filter_t filter1 )
{
    return ((filter0.is & filter1.supress)!=0) || ((filter1.is & filter0.supress)!=0);
}

bool filter_t::testReport(filter_t filter0, filter_t filter1)
{
    return ((filter0.is & filter1.report)!=0) || ((filter1.is & filter0.report)!=0);
}
   
PxFilterFlags filter_t::filter(
    PxFilterObjectAttributes attributes0, PxFilterData filterData0,
    PxFilterObjectAttributes attributes1, PxFilterData filterData1,
    PxPairFlags& pairFlags,
    const void* constantBlock, PxU32 constantBlockSize)
{
    filter_t filter0(filterData0);
    filter_t filter1(filterData1);

    if (filter(filter0, filter1)) {
        if ((filter0.has & STATIC) && (filter1.has & STATIC)) {
            return PxFilterFlag::eKILL;
        } else {
            return PxFilterFlag::eSUPPRESS;
        }
    } else {
        bool ccd = (filter0.has & CCD) || (filter1.has & CCD);
        bool trigger = (filter0.has & TRIGGER) || (filter1.has & TRIGGER);
        if (ccd && !trigger) { pairFlags |= PxPairFlag::eCCD_LINEAR; }
    
        if (testReport(filter0, filter1)) {
            pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
            if (!trigger) {pairFlags |= PxPairFlag::eNOTIFY_TOUCH_PERSISTS;}
            if (ccd && !trigger) {pairFlags |= PxPairFlag::eNOTIFY_TOUCH_CCD;}
        }

        if (trigger) {
            pairFlags |= PxPairFlag::eTRIGGER_DEFAULT;
        } else {
            pairFlags |= PxPairFlag::eCONTACT_DEFAULT;
        }

        return PxFilterFlag::eDEFAULT;
    }
}

}