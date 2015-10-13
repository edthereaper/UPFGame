#include "mcv_platform.h"
#include "lua_conector.h"
#include "lua_callback.h"

namespace lua_user{

	void ConnectorClass::who_am_i(const std::string& text){
		utils::dbg("\n--->>--->> it's me: %s\n", text.c_str());
	}

	void ConnectorClass::interact(CallbackLua callback){

		CallbackLua callbackend = (CallbackLua)callback;

		if (interactFunc) {
			try{
				(*interactFunc)(this, callback);
			}
			catch (luabridge::LuaException const& e) {
				utils::dbg("LuaException: %s\n", e.what());
				fatal("Error creating a intarct method\n");
			}
		}
	}

	void ConnectorClass::loadScript(lua_State* L, std::string scriptFilename, std::string tableName) {
		
		int report = luaL_dofile(L, scriptFilename.c_str());
		
		if (report == 0) { // script has opened

			LuaRef table = getGlobal(L, tableName.c_str());
			if (table.isTable()) {
				if (table["id"].isString()) {
					identifier = table["id"].cast<std::string>();
				}
				else {
					identifier = "Null";
					fatal("LUA IDENTIFIER NOT FOUND");
				}

				if (table["interact"].isFunction()) {
					interactFunc = std::make_shared<LuaRef>(table["interact"]);
				}
				else 
				{
					interactFunc.reset();
				}
			}
		}
		else {
			report_errors(L, report);
		}
	}

	void ConnectorClass::loadMethod(lua_State* L, std::string scriptFilename){
	
		int report = luaL_dofile(L, scriptFilename.c_str());

		if (report == 0) { // script has opened
			
			lua_pcall(L, 0, 0, 0);
		}
		else{
			report_errors(L, report);
		}
	}

	void ConnectorClass::report_errors(lua_State* L, int report)
	{

			std::stringstream ss;
			ss << "ERR: " << lua_tostring(L, report) << "\n";
			std::string error = ss.str();
			fatal("%s", error.c_str());
			lua_pop(L, 1);
	}

	ConnectorClass::LuaType ConnectorClass::getType(std::string type_){
		if (type_.compare("test") == 0)
			return TEST;
		else if (type_.compare("bixito") == 0)
			return BIXITO;
		else if (type_.compare("audio") == 0)
			return AUDIO_BASIC;
		else if (type_.compare("enemy") == 0)
			return ENEMY;
		else if (type_.compare("player") == 0)
			return PLAYER;
		else if (type_.compare("melee") == 0)
			return MELEE;
		else if (type_.compare("prop") == 0)
			return PROP;

		return NONE;	
	}


}