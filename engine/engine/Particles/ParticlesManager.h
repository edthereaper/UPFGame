#ifndef PARTICLE_MANAGER
#define PARTICLE_MANAGER

#include "mcv_platform.h"

#include "ParticleSystem.h"
#include "handles/handle.h"
#include "handles/entity.h"
#include "handles/importXML.h"

#include "utils/XMLParser.h"
#include "Emitter.h"


#define EMITTERS_PATH "data/particles/emitters/"
#define PARTICLES_PATH "data/particles/"
#define PARTICLES_TEXTURE_PATH "data/textures/particles/"

using namespace utils;
using namespace DirectX;

namespace particles{

	typedef std::vector<std::string> ParticlesFileList;

	typedef std::map<std::string, CEmitter::EmitterData> EmitterManagerMap;
	typedef std::pair<std::string, CEmitter::EmitterData> EmitterManagerPair;

	typedef std::pair<std::string, std::string> EmitterParticleId;

	typedef std::pair<EmitterParticleId, CParticleSystem::ParticlesEmitter> ParticlesPair;
	typedef std::map<EmitterParticleId, CParticleSystem::ParticlesEmitter> ParticlesMap;
	
	typedef CEmitter::EmitterData::key_t key_t;
	typedef std::pair<key_t, Handle> ParticlesUpdaterPair;
	typedef std::map<key_t, Handle> ParticlesUpdaterMap;

	
	//----------------------------------------------------------------------------------//
	//----------------------------------------------------------------------------------//
	//-------------------------------PARTICLES PARSER I/O--------------------------------//
	//----------------------------------------------------------------------------------//
	//----------------------------------------------------------------------------------//

	class CEmitter;
	class ParticleSystemParser : private XMLParser{

		private:
			static ParticleSystemParser manager;
		private:
			ParticleSystemParser(){}
			virtual ~ParticleSystemParser(){}
			void onStartElement(const std::string &elem, utils::MKeyValue &atts);
			void onEndElement(const std::string &elem){}

		public:
			bool loadLibrary();
			void add(CEmitter::EmitterData emitter);
			static inline ParticleSystemParser& get() { return manager; }
			void saveComponents();
			void saveComponents(CEmitter::EmitterData emitter){}
	};


	//----------------------------------------------------------------------------------//
	//----------------------------------------------------------------------------------//
	//-------------------------------PARTICLES PARSER I/O--------------------------------//
	//----------------------------------------------------------------------------------//
	//----------------------------------------------------------------------------------//

	class CEmitter;
	class ParticleFileParser : private XMLParser{

	private:
		static ParticleFileParser manager;
		ParticlesFileList files;
	private:
		ParticleFileParser(){}
		virtual ~ParticleFileParser(){}
		void onStartElement(const std::string &elem, utils::MKeyValue &atts);
		void onEndElement(const std::string &elem){}

	public:
		bool loadLibrary();
		ParticlesFileList getFiles(){ return files; }
		std::string getFileByIndex(int index){ return files[index]; }
		int getIndexByFile(std::string str);
		static inline ParticleFileParser& get() { return manager; }
	};


	//----------------------------------------------------------------------------------//
	//----------------------------------------------------------------------------------//
	//-------------------------------EMITTER PARSER I/O--------------------------------//
	//----------------------------------------------------------------------------------//
	//----------------------------------------------------------------------------------//

	class EmitterParser : private XMLParser{

	private:
		static EmitterParser manager;
	private:
		EmitterParser(){}
		virtual ~EmitterParser(){}
		void onStartElement(const std::string &elem, utils::MKeyValue &atts);
		void onEndElement(const std::string &elem){}

	public:
		bool loadLibrary();
		bool load(std::string owner);
		static inline EmitterParser& get() { return manager; }
		void saveComponent(std::string owner);
		void saveComponent(CEmitter::EmitterData ownerh);
	};

	//----------------------------------------------------------------------------------//
	//----------------------------------------------------------------------------------//
	//-------------------------------PARTICLES-EMITTER MANAGER--------------------------//
	//----------------------------------------------------------------------------------//
	//----------------------------------------------------------------------------------//

	class ParticleSystemManager{

	private:
		EmitterManagerMap mapEmitter;
		ParticlesMap mapParicles;

	private:
		ParticleSystemManager(){}
		virtual ~ParticleSystemManager(){}
		static ParticleSystemManager manager;

	public:

		static inline ParticleSystemManager& get() { return manager; }
		EmitterManagerMap getEmitterMap(){ return mapEmitter; }
		ParticlesMap getParticlesMap(){ return mapParicles; }

		void save(CEmitter::EmitterData emitterData);
		void addEmitter(CEmitter::EmitterData emitterData);
		void addParticlesSystem(std::string owner, CParticleSystem::ParticlesEmitter particlesEmitter);
		 CEmitter::EmitterData getEmitter(std::string name);
		 CEmitter::EmitterData getEmitter(int index);
		 bool isEmitterExist(std::string name);
		 bool isEmitterExist(int index);
		CParticleSystem::ParticlesEmitter getParticles(std::string owner, std::string nameParticles);
		CParticleSystem::ParticlesEmitter getParticles(std::string owner, int index);
		bool isParticlesExist(std::string owner, std::string nameParticles);
		bool isParticlesExist(std::string owner, int index);
		void remove(std::string name);
		void removeParticles(std::string name, std::string tagParticles);
		std::vector<std::string> getListPsByOwnerName(std::string owner);
		
	};


	//----------------------------------------------------------------------------------//
	//----------------------------------------------------------------------------------//
	//-------------------------------PARTICLES-EMITTER MANAGER--------------------------//
	//----------------------------------------------------------------------------------//
	//----------------------------------------------------------------------------------//

	class ParticleUpdaterManager{

	private:
		ParticlesUpdaterMap map;
		

	private:
		ParticleUpdaterManager(){}
		virtual ~ParticleUpdaterManager(){}

		static ParticleUpdaterManager manager;

	public:

		static inline ParticleUpdaterManager& get() { return manager; }
		void insertPS(key_t k, component::Handle h);
		component::Handle getPs(key_t k);
		void removePS(key_t k, bool removeHandle = true);
		bool isExist(key_t k);
		void updateTransform(key_t k, XMVECTOR position, XMVECTOR rotation);
		
		void sendMsgKill(key_t k);
		void sendActive(key_t k);
		void sendInactive(key_t k);
		void sendReset(key_t k);

		void sendMsgKillByTimer(key_t k, float time = -0.01);
		void sendMsgActiveByTimer(key_t k, float time = -0.01);
		void sendMsgInactiveByTimer(key_t k, float time = -0.01);
        void setDeleteSelf(key_t k);
		void sendMsgEffect(key_t k, bool enable_, float time = -0.01);
		

		//make magic
		void update(float elpased);
	};

}

#endif