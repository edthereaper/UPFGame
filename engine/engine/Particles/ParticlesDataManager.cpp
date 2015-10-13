#include "mcv_platform.h"
#include "ParticlesManager.h"

using namespace component;

namespace particles{

	ParticleSystemManager ParticleSystemManager::manager;

	void ParticleSystemManager::save(CEmitter::EmitterData emitterData){

#if defined(_PARTICLES)
		
		CEmitter::EmitterMap::iterator it = emitterData.emitterMap.begin();
		
		while (it != emitterData.emitterMap.end()){

			Handle hParticles = it->second;
			Entity *e(hParticles);
			CParticleSystem *ps = e->get<CParticleSystem>();

			std::string key(emitterData.name);
			CParticleSystem::ParticlesEmitter psEmitt(ps->getParticlesEmitter());
			psEmitt.nameParticles = it->first;
			addParticlesSystem(key, psEmitt);
			it++;
		}

		addEmitter(emitterData);

		EmitterParser::get().saveComponent(emitterData);
		ParticleSystemParser::get().saveComponents();
		dbg("");
#endif

	}

	void ParticleSystemManager::addEmitter(CEmitter::EmitterData emitterData){
	
		std::string key(emitterData.name);
		mapEmitter.insert(EmitterManagerPair(key, emitterData));
	}
	void ParticleSystemManager::addParticlesSystem(std::string owner, CParticleSystem::ParticlesEmitter particlesEmitter){

		std::string key(particlesEmitter.nameParticles);
		EmitterParticleId keyMap(owner, key);
		if (isParticlesExist(owner, key))
			mapParicles[keyMap] = particlesEmitter;
		else
			mapParicles.insert(ParticlesPair(keyMap, particlesEmitter));

	}

	bool ParticleSystemManager::isEmitterExist(std::string name){

		EmitterManagerMap::iterator it = mapEmitter.find(name);
		if (it != mapEmitter.end())
			return true;

	
		return false;
	}

	bool ParticleSystemManager::isEmitterExist(int index){
	
		EmitterManagerMap::iterator i = mapEmitter.begin();
		int idx = 0;
		while (i != mapEmitter.end()){

			if (idx == index){
				return true;
				break;
			}
			idx++;
			i++;
		}

		return false;
	}


	CEmitter::EmitterData ParticleSystemManager::getEmitter(std::string name){

		EmitterManagerMap::iterator it = mapEmitter.find(name);
		if (it != mapEmitter.end())
			return it->second;

		fatal("Particles not found");
		return CEmitter::EmitterData();
	}
	CEmitter::EmitterData ParticleSystemManager::getEmitter(int index){

		EmitterManagerMap::iterator i = mapEmitter.begin();
		int idx = 0;
		while (i != mapEmitter.end()){

			if (idx == index){
				return i->second;
				break;
			}
			idx++;
			i++;
		}
		fatal("Particles not found");
		return CEmitter::EmitterData();
		
	}
	CParticleSystem::ParticlesEmitter ParticleSystemManager::getParticles(std::string owner, std::string nameParticles){
		
		ParticlesMap::iterator it = mapParicles.find(EmitterParticleId(owner, nameParticles));
		if (it != mapParicles.end())
			return it->second;

		fatal("Particles not found");
		return CParticleSystem::ParticlesEmitter();

		
	}
	CParticleSystem::ParticlesEmitter ParticleSystemManager::getParticles(std::string owner, int index){
	
		ParticlesMap::iterator i = mapParicles.begin();
		int idx = 0;
		while (i != mapParicles.end()){

			if (idx == index){
				return i->second;
				break;
			}
			idx++;
			i++;
		}
		fatal("Particles not found");
		return CParticleSystem::ParticlesEmitter();
	}
	void ParticleSystemManager::remove(std::string name){
		mapEmitter.erase(name);
	}
	void ParticleSystemManager::removeParticles(std::string name, std::string tagParticles){
		//mapEmitter.erase(EmitterParticleId(name, tagParticles));
	}	

	std::vector<std::string> ParticleSystemManager::getListPsByOwnerName(std::string owner){
	
		std::vector<std::string> list;
		ParticlesMap::iterator i = mapParicles.begin();
		int idx = 0;
		while (i != mapParicles.end()){
			EmitterParticleId key = i->first;
			std::string owner_tmp = key.first;
			if (owner.compare(owner_tmp) == 0)
				list.push_back(key.second);
			
			i++;
		}
		
		return list;
	}

	bool ParticleSystemManager::isParticlesExist(std::string owner, std::string nameParticles){
		ParticlesMap::iterator it = mapParicles.find(EmitterParticleId(owner, nameParticles));
		if (it != mapParicles.end())
			return true;

		return false;
	}
	bool ParticleSystemManager::isParticlesExist(std::string owner, int index){
		ParticlesMap::iterator i = mapParicles.begin();
		int idx = 0;
		while (i != mapParicles.end()){

			if (idx == index){
				return true;
				break;
			}
			idx++;
			i++;
		}
		return false;
	}

}

