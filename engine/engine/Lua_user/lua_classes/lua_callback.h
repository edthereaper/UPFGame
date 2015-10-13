#pragma once
#ifndef __LUA_CALLBACK__
#define __LUA_CALLBACK__

#include "mcv_platform.h"
#include "handles/handle.h"
#include "handles/prefab.h"
#include "handles/entity.h"
#include "components/transform.h"
#include "lua_conector.h"
#include "lua_vector.h"

using namespace component;
using namespace luabridge;
using namespace DirectX;

#include "fmod_User/fmodUser.h"

using namespace fmodUser;

namespace lua_user{

	class CallbackLua : public ConnectorClass
	{
	private:
		Handle handle;

	public:
		CallbackLua(){}
		void setHandle(Handle e_){ handle = e_; }
		Handle getHandle() const { return handle; }
		void printLua(const std::string &txt){ utils::dbg("\n-->LuaConsole: |%s|\n", txt.c_str()); }
		void test(){if (handle.isValid())utils::dbg("Es valido");else fatal("NO ES VALIDO");}
		void printVector(Vec position){utils::dbgXMVECTOR3(toXMVECTOR(position));}
	};

	class BixitoLua : public CallbackLua{

	public:
		BixitoLua(){}
		Vec getPosition();
		Vec getPlayerPosition();

	};
}

#endif