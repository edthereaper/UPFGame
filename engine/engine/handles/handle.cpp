#include "mcv_platform.h"

#include "handle.h"

#include "objectManager.h"
using namespace utils;

namespace component {

bool Handle::cleanup = false;

void Handle::update(float elapsed)
{
    assert(type != INVALID_TYPE);
    HandleManager::mRegister.get(type)->updateObj(*this, elapsed);
}

void Handle::init()
{
    assert(type != INVALID_TYPE);
    HandleManager::mRegister.get(type)->initObj(*this);
}

// -----------------------------------------
bool Handle::destroy() {
	HandleManager* hm = HandleManager::mRegister.get(type);
    if (hm != nullptr) {
        return hm->destroyObj(*this);
    } else {
        return false;
    }
}
// -----------------------------------------
Handle Handle::clone() const {
	HandleManager* hm = HandleManager::mRegister.get(type);
    if (hm != nullptr) {
        return hm->cloneObj(*this);
    } else {
        return Handle();
    }
}
// -----------------------------------------
bool Handle::isValid() const {
    static const auto INVALID_HANDLE_RAW = Handle().raw;
    if (raw == INVALID_HANDLE_RAW) {return false;}
	HandleManager* hm = HandleManager::mRegister.get(type);
    return hm != nullptr && hm->isValid(*this);
}

Handle Handle::getOwner() const {
	HandleManager* hm = HandleManager::mRegister.get(type);
    if (hm != nullptr) {
        return hm->getOwner(*this);
    } else {
        return Handle();
    }
}

bool Handle::setOwner(Handle owner) {
	HandleManager* hm = HandleManager::mRegister.get(type);
    return hm != nullptr && hm->setOwner(*this, owner);
}

void Handle::loadFromProperties(const std::string& elem, MKeyValue &atts) {
	HandleManager* hm = HandleManager::mRegister.get(type);
    if (hm != nullptr) {
        hm->loadFromProperties(*this, elem, atts);
    }
}

const char* Handle::getTypeName() const {
  HandleManager* hm = HandleManager::mRegister.get(type);
  if (hm && isValid()) {return hm->getTypeName();}
  else {return "<invalid>";}
}

}