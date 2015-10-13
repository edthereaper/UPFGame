#include "mcv_platform.h"
#include "lua_component.h"

namespace lua_user{

	void CLua::test()
	{
		getGlobalNamespace(state)
			.beginClass<Vec>("vec")
			.addConstructor<void(*)(void)>()
			.addConstructor<void(*)(const float, const float, const float, const float)>()
			.endClass()
			.beginClass<ConnectorClass>("connector")
			.addConstructor<void(*)(void)>()
			.addProperty("identifier", &ConnectorClass::getIdentifier, &ConnectorClass::setIdentifier)
			.addFunction("whoami", &ConnectorClass::who_am_i)
			.endClass()
			.deriveClass<CallbackLua, ConnectorClass>("callback")
			.addFunction("test", &CallbackLua::test)
			.addFunction("printlua", &CallbackLua::printLua)
			.addFunction("printvector", &CallbackLua::printVector)
			.endClass();

		//get the handle
		CallbackLua callback;
		callback.setHandle(Handle(this));

		ConnectorClass bixito;
		bixito.loadScript(state, nameFile, descriptor);
		bixito.interact(callback);
	}

	void CLua::loadBixitoLua()
	{
		getGlobalNamespace(state)
			.beginClass<Vec>("vec")
				.addConstructor<void(*)(void)>()
				.addConstructor<void(*)(const float, const float, const float, const float)>()
				.addFunction("distance", &Vec::distance)
			.endClass()
			.beginClass<ConnectorClass>("connector")
				.addConstructor<void(*)(void)>()
				.addProperty("identifier", &ConnectorClass::getIdentifier, &ConnectorClass::setIdentifier)
				.addFunction("whoami", &ConnectorClass::who_am_i)
			.endClass()
			.deriveClass<CallbackLua, ConnectorClass>("callback")
			.endClass()
			.deriveClass<BixitoLua, CallbackLua>("bichito")
				.addConstructor<void(*)(void)>()
				.addFunction("getposition",&BixitoLua::getPosition)
			.endClass();

		//get the handle
		BixitoLua bixito;
		bixito.setHandle(Handle(this));

		ConnectorClass conector;
		conector.loadScript(state, nameFile, descriptor);
		conector.interact(bixito);
	}
}