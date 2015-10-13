#pragma once
#ifndef __LUA_CONSOLE__
#define __LUA_CONSOLE__

#include "mcv_platform.h"
#include "handles/handle.h"
#include "lua_classes\lua_conector.h"
#include "lua_classes\lua_callback.h"
#include "lua_classes\lua_vector.h"

using namespace luabridge;
using namespace utils;

namespace lua_user{


	class CLua
	{

	private:
		//member lua state
		lua_State* state = nullptr;

		ConnectorClass::LuaType type;
		std::string nameFile;
		std::string descriptor;

		void initialize();

		ConnectorClass conector;

	public:
		CLua(){};
		~CLua(){}
		void init(){}

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
		void update(float elapsed){}
		void pushLuaFiles(const std::string filename_, const std::string descriptor_, const std::string type_);

	//methods register lua_states and methods
	private:
		/**test of make it work lua register*/
		void test();
		/**this method register the lua functions of bixito*/ 
		void loadBixitoLua();

	public:
		lua_State* getState(){ return state; }
		;
		void ExecuteFile();
		
		int getIntAtTag(const char* method, const char* tag = "");
		float getFloatAtTag(const char* method, const char* tag = "");
		std::string getStringAtTag(const char* method, const char* tag = "");
	};
}
#endif