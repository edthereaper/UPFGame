#ifndef LOGIC_MANAGER_H_
#define LOGIC_MANAGER_H_

#include "handles/objectManager.h"

#include "trigger.h"
#include "timer.h"

namespace logic {

	class LogicManager {
	private:
		//static class
		//LogicManager() = delete;
	public:
		//DAVID Test code for Lua
		int numagents;
		LogicManager() { numagents = 37; }
		void setNumAgents(int k){ numagents = k; }
		void printAgents() { utils::dbg("numagents is: %i \n", numagents); }

		static void update(float elapsed) {
			component::getManager<CScriptTrigger>()->update(elapsed);
			component::getManager<CTimer>()->update(elapsed);
		}

		static void init() {
			component::getManager<CScriptTrigger>()->initAll();
			component::getManager<CTimer>()->initAll();
		}
	};

}

#endif