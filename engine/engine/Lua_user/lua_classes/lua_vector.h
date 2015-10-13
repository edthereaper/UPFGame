#pragma once
#ifndef __LUA_VECTOR__
#define __LUA_VECTOR__

#include "mcv_platform.h"
#include "../lua_component.h"
#include "lua_conector.h"

using namespace luabridge;
using namespace DirectX;
using namespace utils;

namespace lua_user{

	class Vec : public XMFLOAT4
	{
	public:
		inline float length() { XMVECTOR v = XMLoadFloat4(this); return XMVectorGetX(XMVector3Length(v)); }
		inline float distance(Vec v2_) { 
			XMVECTOR v1 = XMLoadFloat4(this);
			XMVECTOR v2 = XMLoadFloat4(&v2_);
			float distance = sqEuclideanDistance(v1, v2);
			return distance;
		}
		Vec() : XMFLOAT4() { XMVectorZero(); }
		Vec(const float _x, const float _y, const float _z, const float _w) { x = _x; y = _y; z = _z; w = _w; }
		Vec(const double _x, const double _y, const double _z, const double _w) { x = (float)_x; y = (float)_y; z = (float)_z; w = (float)_w; }
	}; 

	static XMVECTOR toXMVECTOR(Vec vec){
		return XMVectorSet(vec.x, vec.y, vec.z, vec.w);
	};

	static Vec toVEC(XMVECTOR vec){
		Vec vecLua;
		vecLua.x = XMVectorGetX(vec);
		vecLua.y = XMVectorGetY(vec);
		vecLua.z = XMVectorGetZ(vec);
		vecLua.w = XMVectorGetW(vec);
		return vecLua;
	}
}

#endif