#include "mcv_platform.h"
#include "ParticlesManager.h"
#include "utils/data_provider.h"

using namespace component;

namespace particles{

	EmitterParser EmitterParser::manager;

	void EmitterParser::saveComponent(CEmitter::EmitterData owner_h){

#if defined(_PARTICLES)

		std::string name = owner_h.name;

		char full_name[100];
		sprintf(full_name, "%s%s.xml", EMITTERS_PATH, name.c_str());

		std::ofstream myfileParticle;

		myfileParticle.open(full_name);

		XMVECTOR position = zero_v;
		XMVECTOR rotation = XMVectorSet(0.018028f, -0.021893f, -0.014354f, 0.999495f);
		
		std::string posx = std::to_string(XMVectorGetX(position));
		std::string posy = std::to_string(XMVectorGetY(position));
		std::string posz = std::to_string(XMVectorGetZ(position));

		std::string rotx = std::to_string(XMVectorGetX(rotation));
		std::string roty = std::to_string(XMVectorGetY(rotation));
		std::string rotz = std::to_string(XMVectorGetZ(rotation));
		std::string rotw = std::to_string(XMVectorGetW(rotation));

		
		CEmitter::EmitterMap map = owner_h.emitterMap;
		std::stringstream ss;
		ss.str("");
		std::string size = std::to_string(map.size());

		int idx = 0;
		for (CEmitter::EmitterMap::iterator i = map.begin(); i != map.end(); ++i){
			
			ss << " particles" << std::to_string(idx)<<"=\"" << i->first << "\"";
			idx++;
		}

		myfileParticle << "<!-- copy and paste int prefab -->\n";
		myfileParticle << "<General>\n";
		myfileParticle << "\t<Emitter";
		myfileParticle << " name=\"" + name + "\"";
		myfileParticle << " position=\"" + posx + " " + posy + " " + posz + "\"";
		myfileParticle << " rotation=\"" + rotx + " " + roty + " " + rotz + " " + rotw + "\"";
		myfileParticle << " count=\"" + size + "\"";
		myfileParticle << ss.str();
		myfileParticle << "/>\n";
		myfileParticle << "</General>";
		myfileParticle.close();

		if (myfileParticle.fail()) {
			MessageBox(NULL, strerror(errno), "ERROR", MB_OK);
		}
		else {

			MessageBox(NULL, full_name, "Emitter Saved", MB_OK);
		}
#endif
	}

	bool EmitterParser::load(std::string owner){
		char full_name[100];
		sprintf(full_name, "%s%s", EMITTERS_PATH, owner.c_str());
		dbg("%s\n", full_name);
		return xmlParseFile(full_name);
	}

	bool EmitterParser::loadLibrary(){
	
		bool success = true;
		auto fileList = utils::DirLister::listDir(EMITTERS_PATH);
		for (auto i : fileList){
			
			if (success)
				success = load(i);
			else break;
		}
		if (success)
			return true;
		fatal("ERROR");
		return false;

	}

	void EmitterParser::onStartElement(const std::string &elem, utils::MKeyValue &atts){

		CEmitter::EmitterData emitt;

		int count = 0;

		if (elem == "Emitter"){

			if (atts.has("name")) { emitt.name = atts.getString("name", "test"); }
			if (atts.has("position")){ emitt.position = atts.getPoint("position", utils::zero_v); }
			if (atts.has("count")){ emitt.count = atts.getInt("count", 0); count = count = emitt.count; }

			for (int i = 0; i < count; i++){

				char particlesName[50];
				sprintf(particlesName, "particles%i", i);
				std::string particlesID = "";
				if (atts.has(particlesName)){ particlesID = atts.getString(particlesName, "test"); }
				emitt.listKeys.push_back(particlesID);		
			}

			ParticleSystemManager::get().addEmitter(emitt);
		}
	}



}