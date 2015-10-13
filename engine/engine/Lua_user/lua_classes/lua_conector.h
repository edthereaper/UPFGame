#ifndef __LUA_CONNECTOR__
#define __LUA_CONNECTOR__

#include "mcv_platform.h"

using namespace component;
using namespace luabridge;

namespace lua_user{



	class CallbackLua;
	class ConnectorClass{

	public:
		typedef enum LuaType{

			NONE,
			AUDIO_BASIC,
			AUDIO_ANIMATION,
			ENEMY,
			MELEE,
			PROP,

			//test
			CHECKPOINT,
			BIXITO,
			BIXITO_TALK,
			TEST,
			PLAYER,
			CAMERA,

		}LuaType;

	protected:
		std::string identifier;
		std::shared_ptr<LuaRef> interactFunc;

	public:
		ConnectorClass() : interactFunc(nullptr){}
		virtual ~ConnectorClass(){ interactFunc = nullptr; }
	
		void setIdentifier(const std::string& s_){ identifier = s_; }
		std::string getIdentifier() const { return identifier; }

		void who_am_i(const std::string& text);
		void loadScript(lua_State* L,std::string scriptFilename,
						std::string tableName);
		void loadMethod(lua_State* L, std::string scriptFilename);
		void interact(CallbackLua callback);
		static LuaType getType(std::string type_);
		void report_errors(lua_State* L, int report);

	};

	

}

#endif