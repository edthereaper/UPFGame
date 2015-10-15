#ifndef PARTICLES_EMITTER_H_
#define PARTICLES_EMITTER_H_

#include "mcv_platform.h"


#include "handles/handle.h"
#include "handles/entity.h"


namespace particles{

	class CEmitter{

	public:

		typedef std::map<std::string, component::Handle> EmitterMap;
		typedef std::pair<std::string, component::Handle> EmitterPair;

		struct EmitterData{
			
			struct key_t {
				component::Handle h;
				unsigned n = ~0;

				key_t() = default;
				key_t(component::Handle h, unsigned n) :
					h(h), n(n) {}

				inline bool operator<(const key_t& b) const {
					return h.getRaw() < b.h.getRaw() ? true :
						(h.getRaw() == b.h.getRaw() ? n < b.n : false);
				}
			};

			unsigned currentIndex = 0;

			bool valid = true;
			std::string name;
			int count = 0;
			int currentEditIndex = 0;
			
#if !defined(_PARTICLES)
			typedef std::vector<key_t> keys_t;
			keys_t keys;
#else
			EmitterMap emitterMap;
#endif
			std::vector<std::string> listKeys;
			XMVECTOR position;
			XMVECTOR rotation;

		};


#ifdef _LIGHTTOOL
		bool exportable = false;
		bool selected = false;
#endif

	private:
		component::Entity *emitterPtr;
		EmitterData* emitterData = nullptr;
        
        inline void createEmitterData() {if (emitterData == nullptr) {emitterData = new EmitterData;}}

	public:
		
		CEmitter(){}
		~CEmitter();

		void setEmitterData(EmitterData emit){
     
            if (emitterData == nullptr) 
				emitterData = new EmitterData(emit);
            else 
			{
				removeAll();
				*emitterData = emit;
			}
        }
		const CEmitter::EmitterData& getEmitterData(){ return *emitterData; }
		void setEmittersPosition(XMVECTOR position);
		XMVECTOR getEmittersPosition() const;
		inline void setName(const std::string& name_){ 
            createEmitterData();
            emitterData->name = name_;
        }
		std::string& refName(){ 
            assert(emitterData != nullptr);
            return emitterData->name;
        }
		std::string getName() const {
            assert(emitterData != nullptr);
            return emitterData->name;
        }
		void setCurrentEditIndex(int curr_){
            assert(emitterData != nullptr);
            emitterData->currentEditIndex = curr_;
        }
		inline int getCurrentEditIndex() const {
            assert(emitterData != nullptr);
            return emitterData->currentEditIndex;
        }

		#if defined(_PARTICLES)
		EmitterMap getEmitterMap(){
            assert(emitterData != nullptr); 
            return emitterData->emitterMap;
        }
		#endif
		inline void setCount(int count_){
            assert(emitterData != nullptr); 
            emitterData->count = count_;
        }
		inline int getCount() const { 
            assert(emitterData != nullptr); 
            return emitterData->count;
        }
		inline void setValid(bool valid_) {
            assert(emitterData != nullptr); 
            emitterData->valid = valid_;
        }

		inline bool getValid() const {
            assert(emitterData != nullptr); 
            return emitterData->valid;
        }

        
#if defined(_PARTICLES)
		int add();
		int add(std::string key, component::Handle handle_);
#endif
		void addKey(std::string key){
            assert(emitterData != nullptr); 
            emitterData->listKeys.push_back(key);
        }

		std::string getKey(int idx) const {
            assert(emitterData != nullptr); 
            return emitterData->listKeys[idx];
        }

		bool particlesExist(std::string key);

		component::Handle get(std::string name);
		component::Handle get(int index);

		void load_editor(std::string nameParticles, std::string key);
		void load(std::string nameParticles, EmitterData::key_t k);
		
		void removeScene(std::string name);
		void removeScene(int index);
		void removeAll();

		void updateTag(std::string name, std::string newName);
		#if defined(_PARTICLES)
		int size(){ return (int)emitterData->emitterMap.size(); }
		#endif

		void init();
		void update(float elapsed);
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);

		EmitterData::key_t getKey(std::string key);

		void drawMarker() const;

#if !defined(_PARTICLES)
		inline utils::range_t<EmitterData::keys_t::iterator> iterateKeys() {
			return utils::range_t<EmitterData::keys_t::iterator>(
				emitterData->keys.begin(), emitterData->keys.end());
		}
#endif

#ifdef _LIGHTTOOL
		void setSelectable();
		inline bool isSelected() const { return selected; }
		inline void setSelected(bool b = true) { selected = b; }
		inline bool getExport() const { return exportable; }
		inline void setExport(bool b) { exportable = b; }
#endif
	};

}

#endif