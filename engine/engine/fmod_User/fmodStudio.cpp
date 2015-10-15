#include "mcv_platform.h"
#include "fmodStudio.h"

#define DEBUG_AUDIO

using namespace utils;

namespace fmodUser {

FMOD::Studio::System* FmodStudio::system;
std::map<std::string, FMOD::Studio::Bank*> FmodStudio::banks;
std::map<std::string, FMOD::Studio::EventDescription*> FmodStudio::events;

inline void CHECKED(FMOD_RESULT result)
{
    if (result != FMOD_OK) {
        dbg("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    }
}

FMOD::Studio::EventDescription* FmodStudio::loadEvent(const std::string& name, bool loadSampleData)
{
    static const std::string BASE= "event:/";

    auto i = events.find(name);
    if(i != events.end()) {
        return i->second;
    }
    FMOD::Studio::EventDescription* event = nullptr;
    auto eventName = BASE+name;
    CHECKED(system->getEvent(eventName.c_str(), &event));
    events[name] = event;
    if (loadSampleData) {event->loadSampleData();}
    return event;
}

bool FmodStudio::unloadEvent(const std::string& name)
{
    auto i = events.find(name);
    if (i != events.end()) {
        i->second->unloadSampleData();
        events.erase(i);
        return true;
    } else {
        return false;
    }
}

FMOD::Studio::EventInstance* FmodStudio::getEventInstance(FMOD::Studio::EventDescription* ev)
{
    assert(ev != nullptr);
    FMOD::Studio::EventInstance* in;
    CHECKED(ev->createInstance(&in));
    return in;
}

FMOD::Studio::EventInstance* FmodStudio::playEvent(FMOD::Studio::EventInstance* in)
{
    assert(in != nullptr);
    CHECKED(in->start());
    return in;
}

FMOD::Studio::EventInstance* FmodStudio::stopEvent(FMOD::Studio::EventInstance* in)
{
    assert(in != nullptr);
    CHECKED(in->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT));
    return in;
}

FMOD::Studio::Bank* FmodStudio::loadBank(const std::string& name,
    bool loadSampleData, bool nonblocking, bool decompress)
{
    static const std::string BASE_PATH = "data/sounds-studio/";

    auto i = banks.find(name);
    if(i != banks.end()) {
        return i->second;
    }
    unsigned flags = FMOD_STUDIO_LOAD_BANK_NORMAL;
    if (decompress) {flags |= FMOD_STUDIO_LOAD_BANK_DECOMPRESS_SAMPLES;}
    if (nonblocking) {flags |= FMOD_STUDIO_LOAD_BANK_NONBLOCKING;}
    FMOD::Studio::Bank* bank = nullptr;
    CHECKED(system->loadBankFile((BASE_PATH+name+".bank").c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank));
    banks[name] = bank;
    if (loadSampleData) {bank->loadSampleData();}
    return bank;
}

void FmodStudio::init()
{
    CHECKED(FMOD::Studio::System::create(&system));
    unsigned flags = FMOD_STUDIO_INIT_NORMAL;
#if defined(_DEBUG) && defined(DEBUG_AUDIO)
    flags |= FMOD_STUDIO_INIT_LIVEUPDATE;
#endif
    CHECKED(system->initialize(512, flags, FMOD_INIT_NORMAL, 0));
    
    loadBank("Master Bank");
    loadBank("Master Bank.strings");
}

void FmodStudio::update()
{
    CHECKED(system->update());
}

bool FmodStudio::unloadBank(const std::string& name)
{
    auto i = banks.find(name);
    if (i != banks.end()) {
        i->second->unload();
        banks.erase(i);
        return true;
    } else {
        return false;
    }
}

void FmodStudio::shutdown()
{
    for (const auto& bank : banks) {
        bank.second->unload();
    }
    banks.clear();
    CHECKED(system->release());
}

}

