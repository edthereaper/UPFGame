#ifndef FMOD_STUDIO_H_
#define FMOD_STUDIO_H_

#include "fmod/inc/fmod_studio.hpp"
#include "fmod/inc/fmod_errors.h"

#include "../app.h"
#include "../gameElements/player/playerMov.h"
using namespace gameElements;

namespace fmodUser {

	class fmodUserClass;

	class FmodStudio {
	public:
		friend fmodUserClass;

		typedef FMOD::Studio::Bank* Bank;
		typedef FMOD::Studio::EventInstance* EventInstance;
		typedef FMOD::Studio::EventDescription* EventDescription;

	private:
		static FMOD::Studio::System* system;
		static std::map<std::string, FMOD::Studio::Bank*> banks;
		static std::map<std::string, FMOD::Studio::EventDescription*> events;
	public:
		static void init();
		static void update();
		static void shutdown();

		static FMOD::Studio::Bank* FmodStudio::loadBank(
			const std::string& name, bool loadSampleData = false, bool decompress = true, bool nonblocking = false);
		static bool unloadBank(const std::string& name);
		static inline FMOD::Studio::Bank* getBank(const std::string& name) {
			return utils::atOrDefault(banks, name, nullptr);
		}

		static FMOD::Studio::EventDescription* FmodStudio::loadEvent(
			const std::string& name, bool loadSampleData = false);
		static bool unloadEvent(const std::string& name);
		static inline FMOD::Studio::EventDescription* getEvent(
			const std::string& name, bool loadSampleData = false) {
			auto ev = utils::atOrDefault(events, name, nullptr);
			if (ev == nullptr) { ev = loadEvent(name, loadSampleData); }
			return ev;
		}
		static FMOD::Studio::EventInstance* getEventInstance(FMOD::Studio::EventDescription* e);
		static inline FMOD::Studio::EventInstance* getEventInstance(const std::string& name) {
			return getEventInstance(getEvent(name, true));
		}
		static FMOD::Studio::EventInstance* playEvent(FMOD::Studio::EventInstance* ei);

		static FMOD::Studio::EventInstance* play3DSingleEvent(FMOD::Studio::EventInstance* ei, XMVECTOR posSource);
		static void updateListener();
		static FMOD::Studio::EventInstance* FmodStudio::play3DAmbientEvent(FMOD::Studio::EventInstance* in, XMVECTOR posSource);
		static FMOD::Studio::EventInstance* FmodStudio::update3DAmbientEvent(FMOD::Studio::EventInstance* in, XMVECTOR posSource);

		static FMOD::Studio::EventInstance* stopEvent(FMOD::Studio::EventInstance* ei);
		static FMOD::Studio::EventInstance* pauseEvent(FMOD::Studio::EventInstance* ei);
		static FMOD::Studio::EventInstance* resumeEvent(FMOD::Studio::EventInstance* ei);
		static inline FMOD::Studio::EventInstance* playEvent(const std::string& name) {
			return playEvent(getEventInstance(name));
		}
		static inline FMOD::Studio::EventInstance* playEvent(FMOD::Studio::EventDescription* e) {
			return playEvent(getEventInstance(e));
		}


	};



}

#endif