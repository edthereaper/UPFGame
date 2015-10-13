#ifndef ANIMATION_PLUGGED_H_
#define ANIMATION_PLUGGED_H_

#include "mcv_platform.h"

#include "utils/itemsByName.h"
#include "skeleton_manager.h"

#include "cal3d/animcallback.h"

namespace animation {

	class AnimationArchetype : private XMLParser {
	public:
		typedef utils::ItemsByName<AnimationArchetype> Manager;
		static inline Manager& getManager() { return manager; }

		enum anim_e { MAIN_CYCLE, ACTION, BLEND_CYCLE };
		enum state_e { LEAVE, OFF, ON };
		enum feature_e { NONE, ARMPOINT, BONELOOKAT, CHAIN };

		struct plugFeature_t {
			feature_e what;
			union{
				struct{
					int id;
					state_e state;
				};
				struct {
					bool checked;
					bool onDelayOut;
				} chain;
			};
		};

		struct plug_t {
			std::string name = "";
			int plugId = -1;
			int calAnimId = -1;
			float delay = 0.1f;
			float delayOut = 0.1f;
			float previousFade = 0.1f;
			float factor = 1.f;
			float weight = 1.f;
			bool unplug = false;
			bool actionLock = false;
			bool repeat = false;
			bool followDummyPos = false;
			bool footControl = false;
			bool creepControl = false;
			bool fakeCycle = false;
			float limitDummyCycle = 100.0f;

			anim_e type = MAIN_CYCLE;
			std::vector<plugFeature_t> features;
		};

	private:
		static Manager manager;
		typedef std::vector<uint32_t> pluggedContainer_t;
		static pluggedContainer_t currentPluggedV;
		static plug_t currentPlug;

		class AnimChainer : public CalAnimationCallback {
		private:
			struct chain_t{
				int plugId = -1;
				int check = -1;
				float anim_time = -100;
				bool triggered = true;
				chain_t() = default;
				chain_t(unsigned plugId) : plugId(plugId) {}
			};
			std::map<unsigned, chain_t> chains;
			AnimationArchetype* archetype;
			void chain(chain_t& c, void* modelUserData);
		public:
			void AnimationUpdate(float anim_time, CalModel* model,
				void* modelUserData, void* animUserData);

			void AnimationComplete(CalModel* model,
				void* modelUserData, void* animUserData);

			inline void prepareChain(unsigned prevPlugId, bool checked, float time) {
				chain_t def;
				auto& c(atOrDefault(chains, prevPlugId, def));
				if (c.plugId >= 0) {
					c.check = checked ? prevPlugId : -1;
					c.anim_time = time;
					c.triggered = false;
				}
			}

			inline void addChain(unsigned prevPlugId, unsigned nextPlugId) {
				chains[prevPlugId] = chain_t(nextPlugId);
			}
			AnimChainer(AnimationArchetype* archetype) : archetype(archetype) {}
		} chainer;
		friend AnimChainer;

	private:
		std::map<uint32_t, plug_t> plugs;
		std::map<unsigned, plug_t> plugsById;
		CoreModel* model = nullptr;

		void onStartElement(const std::string &elem, utils::MKeyValue &atts);
		void onEndElement(const std::string &elem);

	public:
		AnimationArchetype() : chainer(this) {}
		~AnimationArchetype(){
			for (int i = 0; i<model->getCoreAnimationCount(); ++i) {
				model->getCoreAnimation(i)->removeCallback(&chainer);
			}
		}

		bool load(const char* newName);

		plug_t getPlug(uint32_t) const;
		plug_t getPlugById(unsigned plugId) const;

		inline AnimChainer& getChainer() { return chainer; }
	};

	/* Component ANIMATION  PLUGGER
	Manages animation loading from IDs

	XML PROPERTIES
	archetype -> the name of the AnimationArchetype to be loaded
	*/
	class CAnimationPlugger
	{
	private:
		AnimationArchetype* archetype = nullptr;
		int lastPlug = ~0;

		float lastPlugDelayOut = ~0;

		AnimationArchetype::plug_t currentCycle = AnimationArchetype::plug_t();
		AnimationArchetype::plug_t currentAction = AnimationArchetype::plug_t();

		AnimationArchetype::plug_t actualPlug = AnimationArchetype::plug_t();
		AnimationArchetype::plug_t previousPlug = AnimationArchetype::plug_t();

		float elapsedPlug = 0.0f;
		float plugDuration = 0.0f;
		float previousPlugDuration = 0.0f;

	public:
		/* If id is valid, plug said animation. Otherwise, ignore */
		void plug(uint32_t id);
		void plug(const AnimationArchetype::plug_t& plug);

		inline int getLastPlugId() const { return lastPlug; }
		inline AnimationArchetype::plug_t getActualPlug() const { return actualPlug; }
		inline AnimationArchetype::plug_t getPreviousPlug() const { return previousPlug; }
		inline AnimationArchetype::plug_t getBackgroundCycle() const { return currentCycle; }
		inline float getActualPlugDuration() const { return plugDuration / actualPlug.factor; }
		inline float getPreviousPlugDuration() const { return previousPlugDuration / previousPlug.factor; }
		inline float getActualPlugDurationWithDelay() const { return (plugDuration / actualPlug.factor) - actualPlug.delayOut; }
		inline float getAnimationElapsed() const { return elapsedPlug; }
		void setAnimationElapsed(float elapsed){ elapsedPlug += elapsed; }
		inline void resetAnimationElapsed() { elapsedPlug = 0.0f; }

		void loadArchetype(const std::string&);
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
		void update(float elapsed);
		void init();
	};

}

#endif