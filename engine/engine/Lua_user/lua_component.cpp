#include "mcv_platform.h"
#include "lua_component.h"
#include "lua_classes/lua_callback.h"

namespace lua_user{

	void CLua::loadFromProperties(const std::string& elem, utils::MKeyValue &atts){

		pushLuaFiles(atts.getString("namefile", "test"), atts.getString("nametable", "none"), atts.getString("type", "NONE"));		
	}

	//------------------------------------------------------

	void CLua::initialize(){

		if (state == nullptr){
			state = luaL_newstate();
			luaL_openlibs(state);
		}
	}

	void CLua::pushLuaFiles(const std::string filename_, const std::string descriptor_, const std::string type_){

		if (strstr(filename_.c_str(), "data/lua") == nullptr){

			std::stringstream ss;
			ss << "data/lua/" << filename_ << ".lua";
			nameFile = ss.str();
		}
		else{
			nameFile = filename_;
		}

		type = ConnectorClass::getType(type_);

		initialize();


		switch (type)
		{
		case ConnectorClass::LuaType::TEST:
			test();
			break;
		default:
			break;
		}

		conector.loadMethod(state,nameFile);
	}

	//------------------------------------------------------

	// special cases
	/*
		--- stun enemy ---
		stunChannelVolume
		stunChannelFileName
		stunChannelPan
		stunChannelRadius
		...................
	*/

	// get values

	//------------------------------------ data type filter ------------------------------------------

	int CLua::getIntAtTag(const char* method, const char* tag){
		
		if (type == ConnectorClass::LuaType::ENEMY || type == ConnectorClass::LuaType::PLAYER){

			LuaRef get_int = getGlobal(state, method);

			int result = 0;
			if (strcmp(tag, "") == 0)
				result = get_int(true);
			else
			    result = get_int(tag, true);

			if (result == -1)
				fatal("INT NOT LOAD");

			return result;
		}

		return 0;

	}

	float CLua::getFloatAtTag(const char* method, const char* tag){
		if (type == ConnectorClass::LuaType::ENEMY || type == ConnectorClass::LuaType::PLAYER){

			LuaRef get_float = getGlobal(state, method);
			float result = 0.0;
			if (strcmp(tag, "") == 0)
				result = get_float(true);
			else
				result = get_float(tag, true);
				
			if (result == -1.0)
				fatal("FLOAT NOT LOAD");

			return result;
		}

		return 0.0;
	}
	std::string CLua::getStringAtTag(const char* method, const char* tag){

		if (type == ConnectorClass::LuaType::ENEMY || type == ConnectorClass::LuaType::PLAYER){

			LuaRef get_string = getGlobal(state, method);

			const char* result;
			if (strcmp(tag,"") == 0)
				result = get_string(true);
			else
				result = get_string(tag, true);

			if (strcmp(result, "INVALID") == 0)
				fatal("STRING NOT LOAD");

			char copy[50];
			strcpy(copy, result);
			std::string returnResult(copy);

			return returnResult;
		}

		return "INVALID";
	}
}