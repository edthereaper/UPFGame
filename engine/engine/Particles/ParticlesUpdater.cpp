#include "mcv_platform.h"
#include "ParticlesManager.h"

#include "gameElements/gameMsgs.h"




using namespace component;

namespace particles{

	ParticleUpdaterManager ParticleUpdaterManager::manager;

	void ParticleUpdaterManager::insertPS(key_t k, Handle h){
	
		map.insert(ParticlesUpdaterPair(k, h));
	
	}
	void ParticleUpdaterManager::removePS(key_t k, bool removeHandle){

			Handle h = getPs(k);
			if (h.isValid()){
				Entity *e(h);
				e->sendMsg(MsgDeleteSelf());
			}
	}


	bool ParticleUpdaterManager::isExist(key_t k){
		
		ParticlesUpdaterMap::iterator it = map.find(k);
		if (it != map.end())
			return true;

		return false;
	}

	Handle ParticleUpdaterManager::getPs(key_t k){
        assert(k.h.isValid());
		ParticlesUpdaterMap::iterator it = map.find(k);
        assert(it != map.end());
		return it != map.end() ? (Handle)it->second : Handle();

	}

	void ParticleUpdaterManager::updateTransform(key_t k, XMVECTOR position, XMVECTOR rotation){
	
		if (isExist(k)){
			Handle h = getPs(k);

			Entity* e(h);
			
			if (e != nullptr && e->has<CTransform>() && e->has<CParticleSystem>()){
				
				CTransform *t = e->get<CTransform>();
				CParticleSystem *ps = e->get<CParticleSystem>();

				t->setPosition(position + ps->getLocalPosition());
				t->setRotation(ps->getLocalRotation());
			}
		}
	}

	void ParticleUpdaterManager::sendMsgKill(key_t k){
		Handle h = getPs(k);
		Entity *e(h);
		CParticleSystem *ps = e->get<CParticleSystem>();
        if(ps != nullptr) {
		    ps->receive(gameElements::MsgKillParticlesInmidiatly());
        }
	}

    void ParticleUpdaterManager::setDeleteSelf(key_t k){
		Handle h = getPs(k);
		Entity *e(h);
		CParticleSystem *ps = e->get<CParticleSystem>();
        if(ps != nullptr) {
		    ps->setDeleteSelf();
        }
	}

	void ParticleUpdaterManager::sendMsgKillByTimer(key_t k, float timer){
		Handle h = getPs(k);
		Entity *e(h);
		CParticleSystem *ps = e->get<CParticleSystem>();
		
        if(ps != nullptr) {
		    ps->receive(gameElements::MsgKillParticlesByTime((timer <= -0.00001) ?
                ps->getLifeTimeRate() : timer));
        }
	}

	void ParticleUpdaterManager::sendMsgActiveByTimer(key_t k, float time){
		Handle h = getPs(k);
		Entity *e(h);
		CParticleSystem *ps = e->get<CParticleSystem>();
        if(ps != nullptr) {
		    ps->receive(gameElements::MsgActiveParticlesByTime((time <= -0.00001) ?
                ps->getLifeTimeRate() : time));
        }
	}

	void ParticleUpdaterManager::sendMsgInactiveByTimer(key_t k, float time){
		Handle h = getPs(k);
		Entity *e(h);
		CParticleSystem *ps = e->get<CParticleSystem>();
        if(ps != nullptr) {
		    ps->receive(gameElements::MsgInactiveParticlesByTime((time <= -0.00001) ?
                ps->getLifeTimeRate() : time));
        }
	}
	

	void ParticleUpdaterManager::sendActive(key_t k){
		Handle h = getPs(k);
		Entity *e(h);
		CParticleSystem *ps = e->get<CParticleSystem>();
        if(ps != nullptr) {
		    ps->receive(gameElements::MsgActiveParticles());
        }
	}

	void ParticleUpdaterManager::sendInactive(key_t k){
		Handle h = getPs(k);
		Entity *e(h);
		CParticleSystem *ps = e->get<CParticleSystem>();
        if(ps != nullptr) {
		    ps->receive(gameElements::MsgInactiveParticles());
        }
	}

	void ParticleUpdaterManager::sendReset(key_t k){
		Handle h = getPs(k);
		Entity *e(h);
		CParticleSystem *ps = e->get<CParticleSystem>();
        if(ps != nullptr) {
		    ps->receive(gameElements::MsgResetParticles());
        }
	}

	//make magic
	void ParticleUpdaterManager::update(float elpased) {
		if (map.size() > 0){

			ParticlesUpdaterMap::iterator i = map.begin();
			int idx = 0;
            std::vector<ParticlesUpdaterMap::key_type> removeKeys;
            for (auto& i : map) {
				const auto& handle = i.second;
				if (handle.isValid()){
					Entity *e(handle);
					CParticleSystem *ps = e->get<CParticleSystem>();
                    if (ps != nullptr) {
					    ps->update(elpased);
                    } else {
					    removeKeys.push_back(i.first);
                    }
				} else {
					removeKeys.push_back(i.first);
				}
			}
            for (const auto& k : removeKeys) {
                map.erase(k);
            }
		}

	}
}