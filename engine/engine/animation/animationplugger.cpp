#include "mcv_platform.h"
#include "animationplugger.h"

#include "cskeleton.h"

#include "cArmPoint.h"
#include "cbonelookat.h"

#include "handles/entity.h"
using namespace component;

#include <sstream>

using namespace utils;

namespace animation {

	AnimationArchetype::Manager AnimationArchetype::manager;
	AnimationArchetype::pluggedContainer_t AnimationArchetype::currentPluggedV;
	AnimationArchetype::plug_t AnimationArchetype::currentPlug;

	CalAnimationCycle* findCycle(CalCoreModel* coreModel, CalMixer* mixer, int animId)
	{
		auto coreAnimation = coreModel->getCoreAnimation(animId);
		for (auto& anim : mixer->getAnimationCycle()) {
			if (anim->getCoreAnimation() == coreAnimation) {
				return anim;
			}
		}
		return nullptr;
	}

	CalAnimationAction* findAction(CalCoreModel* coreModel, CalMixer* mixer, int animId)
	{
		auto coreAnimation = coreModel->getCoreAnimation(animId);
		for (auto& anim : mixer->getAnimationActionList()) {
			if (anim->getCoreAnimation() == coreAnimation) {
				return anim;
			}
		}
		return nullptr;
	}

	void CAnimationPlugger::init()
	{
		Handle this_h(this);
		CSkeleton* skeleton = this_h.getBrother<CSkeleton>();
		assert(skeleton != nullptr);
		skeleton->getModel()->setUserData(this_h.getRawAsVoidPtr());
	};

	void plugCycle(CalModel* model, const AnimationArchetype::plug_t& plug)
	{
		model->getMixer()->blendCycle(plug.calAnimId, plug.weight, plug.delay);
		if (plug.factor != 1.0) {
			CalAnimationCycle* anim =
				findCycle(model->getCoreModel(), model->getMixer(), plug.calAnimId);
			if (anim != nullptr) {
				anim->setAsync(0, 0);
				anim->setTimeFactor(plug.factor);
				anim->setUserData((void*)plug.plugId);
			}
		}
	}

	float actiondelayOut = 0.05f;
	bool fromAction = false;
	bool lastPlugisCycle = true;

	void CAnimationPlugger::plug(const AnimationArchetype::plug_t& plug)
	{
		CSkeleton* skeleton = Handle(this).getBrother<CSkeleton>();
		assert(skeleton != nullptr);
		CalModel* model = skeleton->getModel();

		switch (plug.type) {
		case AnimationArchetype::MAIN_CYCLE:
			if (!plug.unplug) {
				//dbg("cycle: %s\n", plug.name.c_str());
				if (lastPlugisCycle){
					previousPlug = currentCycle;
					//dbg("new previousPlug cycle: %s\n", previousPlug.name.c_str());
				}
				else{
					previousPlug = currentAction;
					//dbg("new previousPlug action: %s\n", previousPlug.name.c_str());
				}
				previousPlugDuration = plugDuration;
				lastPlugisCycle = true;

				model->getMixer()->clearCycle(currentCycle.calAnimId, currentCycle.delayOut);
				model->getMixer()->blendCycle(plug.calAnimId, plug.weight, plug.delay);

				CalAnimationCycle* anim =
					findCycle(model->getCoreModel(), model->getMixer(), plug.calAnimId);
				plugDuration = anim->getCoreAnimation()->getDuration();
				elapsedPlug = 0.0f;
				if (anim != nullptr) {
					anim->setAsync(0, 0);
					anim->setTimeFactor(plug.factor);
					anim->setUserData((void*)plug.plugId);
				}
				currentCycle = plug;
				lastPlug = plug.plugId;
				actualPlug = plug;
			}
			break;
		case AnimationArchetype::BLEND_CYCLE:
			if (!plug.unplug) {
				plugCycle(model, plug);
				fromAction = false;
				lastPlug = plug.plugId;
				actualPlug = plug;
			}
			else {
				model->getMixer()->clearCycle(plug.calAnimId, plug.delayOut);
			}
			break;
		case AnimationArchetype::ACTION:
			if (!plug.unplug) {
				if (plug.plugId>0 && !plug.repeat) {
					CalAnimationAction* anim =
						findAction(model->getCoreModel(), model->getMixer(), plug.calAnimId);
					if (anim != nullptr && (unsigned)anim->getUserData() == plug.plugId) {
						break;
					}
				}
				//dbg("action: %s\n", plug.name.c_str());
				if (!plug.repeat){
					if (lastPlugisCycle){
						previousPlug = currentCycle;
						//dbg("new previousPlug cycle: %s\n", previousPlug.name.c_str());
					}
					else{
						previousPlug = currentAction;
						//dbg("new previousPlug action: %s\n", previousPlug.name.c_str());
					}
					previousPlugDuration = plugDuration;
					lastPlugisCycle = false;
				}
				model->getMixer()->executeAction(plug.calAnimId, plug.delay, plug.delayOut, plug.weight, plug.actionLock);
				CalAnimationAction* anim = findAction(model->getCoreModel(), model->getMixer(), plug.calAnimId);
				if (!plug.repeat){
					plugDuration = anim->getCoreAnimation()->getDuration();
					elapsedPlug = 0.0f;
					fromAction = true;
					actiondelayOut = plug.delayOut;
					lastPlug = plug.plugId;
					actualPlug = plug;
					if (anim != nullptr) {
						anim->setTimeFactor(plug.factor);
						anim->setUserData((void*)plug.plugId);
					}
				}
			}
			else {
				//dbg("unplug action: %s\n", plug.name.c_str());
				model->getMixer()->removeAction(plug.calAnimId, plug.delayOut);
			}
			break;
		default: break;
		}

		for (const auto& feature : plug.features) {
			switch (feature.what) {
			case AnimationArchetype::CHAIN: {
				assert(plug.type == AnimationArchetype::ACTION);
				auto anim = findAction(model->getCoreModel(), model->getMixer(), plug.calAnimId);
				anim->setUserData((void*)plug.plugId);
				archetype->getChainer().prepareChain(plug.plugId, feature.chain.checked,
					feature.chain.onDelayOut ? anim->getCoreAnimation()->getDuration() - plug.delayOut : -1.f
					);
			} break;
			case AnimationArchetype::ARMPOINT: {
				CArmPoint* ik = Handle(this).getBrother<CArmPoint>();
				if (ik != nullptr) {
					switch (feature.state) {
					case AnimationArchetype::ON:
						ik->setActive(true);
						break;
					case AnimationArchetype::OFF:
						ik->setActive(false);
						break;
					}
				}
			}
											   break;
			case AnimationArchetype::BONELOOKAT: {
				CBoneLookAt* ik = Handle(this).getBrother<CBoneLookAt>();
				if (ik != nullptr) {
					if (feature.id < 0) {
						switch (feature.state) {
						case AnimationArchetype::ON:
							ik->setActive(true);
							break;
						case AnimationArchetype::OFF:
							ik->setActive(false);
							break;
						}
					}
					else if (feature.id < CBoneLookAt::N_LOOKATS) {
						auto& entry = ik->getEntry(feature.id);
						switch (feature.state) {
						case AnimationArchetype::ON:
							entry.setActive(true);
							break;
						case AnimationArchetype::OFF:
							entry.setActive(false);
							break;
						}
					}
				}
			} break;
			default: break;
			}
		}
	}

	void CAnimationPlugger::plug(uint32_t id)
	{
		if (archetype != nullptr) {
			const AnimationArchetype::plug_t& p(archetype->getPlug(id));
			if (p.calAnimId >= 0) {
				plug(p);
			}
		}
	}

	void CAnimationPlugger::loadArchetype(const std::string& name)
	{
		archetype = AnimationArchetype::getManager().getByName(name.c_str());
	}

	void CAnimationPlugger::loadFromProperties(const std::string& elem, MKeyValue &atts)
	{
		if (atts.has("archetype")) { loadArchetype(atts["archetype"]); }
	}

	AnimationArchetype::plug_t AnimationArchetype::getPlug(uint32_t id) const
	{
		return atOrDefault(plugs, id, plug_t());
	}

	AnimationArchetype::plug_t AnimationArchetype::getPlugById(unsigned plugId) const
	{
		return atOrDefault(plugsById, plugId, plug_t());
	}

	void AnimationArchetype::onStartElement(const std::string &elem, MKeyValue &atts)
	{
		if (elem == "archetype") {
			model = CoreModel::getManager().getByName(atts["model"].c_str());
		}
		else if (elem == "plug") {
			assert(model != nullptr);
			currentPlug = plug_t();
			//#ifdef _ANITOOL
			currentPlug.name = atts.getString("anim", currentPlug.name);
			//#endif
			currentPlug.plugId = atts.getInt("id", currentPlug.plugId);
			currentPlug.delay = atts.getFloat("delay", currentPlug.delay);
			currentPlug.delayOut = atts.getFloat("delayOut", currentPlug.delayOut);
			currentPlug.previousFade = atts.getFloat("previousFade", currentPlug.previousFade);
			currentPlug.factor = atts.getFloat("factor", currentPlug.factor);
			currentPlug.weight = atts.getFloat("weight", currentPlug.weight);
			currentPlug.actionLock = atts.getBool("lock", currentPlug.actionLock);
			currentPlug.repeat = atts.getBool("repeat", currentPlug.repeat);
			currentPlug.followDummyPos = atts.getBool("dummy", currentPlug.followDummyPos);
			currentPlug.footControl = atts.getBool("footControl", currentPlug.footControl);
			currentPlug.creepControl = atts.getBool("creepControl", currentPlug.creepControl);
			currentPlug.fakeCycle = atts.getBool("fakeCycle", currentPlug.fakeCycle);
			currentPlug.unplug = atts.getBool("unplug", false);
			currentPlug.limitDummyCycle = atts.getFloat("limitDummyCycle", currentPlug.limitDummyCycle);
			currentPlug.calAnimId = model->getCoreAnimationId(atts["anim"].c_str());

			std::string typeStr = atts["type"];
			if (typeStr == "action") {
				currentPlug.type = ACTION;
			}
			else if (typeStr == "cycle") {
				currentPlug.type = MAIN_CYCLE;
			}
			else if (typeStr == "blend") {
				currentPlug.type = BLEND_CYCLE;
			}

		}
		else if (elem == "on") {
			currentPluggedV.push_back(atts.getHex("key"));
		}
		else if (elem == "set") {
			plugFeature_t feature;
			std::string whatStr = atts["feature"];
			feature.what =
				whatStr == "armPoint" ? ARMPOINT :
				whatStr == "boneLookAt" ? BONELOOKAT :
				NONE;
			feature.id = atts.getInt("id", -1);
			if (atts.has("state")) {
				feature.state = atts.getBool("state", false) ? ON : OFF;
			}
			currentPlug.features.push_back(feature);
		}
		else if (elem == "chain") {
			if (currentPlug.plugId >= 0) {
				int chainId = atts.getInt("plug", -1);
				if (chainId >= 0) {
					plug_t nextPlug = getPlugById(chainId);
					if (nextPlug.calAnimId >= 0) {
						assert(currentPlug.calAnimId >= 0);
						auto* coreAnim = model->getCoreAnimation(currentPlug.calAnimId);
						// add the chainer if it wasn't already
						bool found = false;
						for (auto callBack : coreAnim->getCallbackList()) {
							if (callBack.callback == &chainer) { found = true; }
						}
						if (!found) { coreAnim->registerCallback(&chainer, 0); }

						//setup the new chain
						plugFeature_t feature;
						feature.what = CHAIN;
						feature.chain.checked = atts.getBool("checked", true);
						feature.chain.onDelayOut = atts.getBool("onDelayOut", false);
						currentPlug.features.push_back(feature);

						chainer.addChain(currentPlug.plugId, chainId);
					}
				}
			}
		}
	}

	void AnimationArchetype::onEndElement(const std::string &elem)
	{
		if (elem == "plug") {
			if (currentPlug.plugId >= 0) {
				plugsById[currentPlug.plugId] = currentPlug;
			}
			for (auto plugged : currentPluggedV) {
				plugs[plugged] = currentPlug;
			}
			currentPluggedV.clear();
			currentPlug = plug_t();
		}
	}

	bool AnimationArchetype::load(const char* name)
	{
		std::stringstream ss;
		ss << "data/animationPlugs/" << name << ".xml";
		return xmlParseFile(ss.str());
	}

	void CAnimationPlugger::update(float elapsed)
	{
		CSkeleton* skeleton = Handle(this).getBrother<CSkeleton>();
	}

	void AnimationArchetype::AnimChainer::chain(AnimationArchetype::AnimChainer::chain_t& c, void* modelUserData)
	{
		Handle plugger_h(Handle::fromRaw(modelUserData));
		assert(plugger_h.isValid());
		CAnimationPlugger* plugger(plugger_h);
		auto p(archetype->getPlugById(c.plugId));
		if (p.calAnimId >= 0 && (c.check < 0 || plugger->getLastPlugId() == c.check)) {
			plugger->plug(p);
		}
		c.triggered = true;
	}


	void AnimationArchetype::AnimChainer::AnimationUpdate(
		float animTime, CalModel* model, void* modelUserData, void* animUserData)
	{
		unsigned plugId = (unsigned)animUserData;
		chain_t def;
		chain_t& c = atOrDefault(chains, plugId, def);
		if (c.plugId >= 0) {
			if (!c.triggered) {
				float chainTime = c.anim_time;
				if (chainTime >= 0 && chainTime <= animTime) {
					chain(c, modelUserData);
				}
			}
		}
	}

	void AnimationArchetype::AnimChainer::AnimationComplete(
		CalModel* model, void* modelUserData, void* animUserData)
	{
		unsigned plugId = (unsigned)animUserData;
		chain_t def;
		chain_t& c = atOrDefault(chains, plugId, def);
		if (c.plugId >= 0) {
			if (!c.triggered) {
				float chainTime = c.anim_time;
				if (chainTime == -1) {
					chain(c, modelUserData);
				}
			}
		}
	}

}

