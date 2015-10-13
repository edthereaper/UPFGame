#include "mcv_platform.h"
#include "ParticlesManager.h"


using namespace component;

namespace particles{

	ParticleSystemParser ParticleSystemParser::manager;

	void ParticleSystemParser::saveComponents(){

		std::ofstream myfileParticle;

		char full_name[100];
		sprintf(full_name, "%s%s.xml", PARTICLES_PATH, "particles_system");

		myfileParticle.open(full_name);
		myfileParticle << "<!--Colocar el XML de cada sistema de particulas en su prefab correspondiente.\n";
		myfileParticle << "Colocar emitted en (1), en caso de que se quiere que este activado desde el inicio -->\n";
		myfileParticle << "<general>\n";

		std::vector<std::string> list;

		ParticlesMap map = ParticleSystemManager::get().getParticlesMap();

		for (ParticlesMap::iterator i = map.begin(); i != map.end(); ++i)
		{
			EmitterParticleId key = i->first;
			CParticleSystem::ParticlesEmitter emitter = i->second;


			std::string name = key.second;
			std::string owner = key.first;
			XMVECTOR local_position = zero_v;

			//position
			std::string posx = std::to_string(XMVectorGetX(emitter.localposition));
			std::string posy = std::to_string(XMVectorGetY(emitter.localposition));
			std::string posz = std::to_string(XMVectorGetZ(emitter.localposition));

			//rotation

			if (emitter.rotation == XMVectorZero() || XMVectorGetX(emitter.rotation) == -431602080.000000){
				emitter.rotation = XMVectorSet(0.018028f, -0.021893f, -0.014354f, 0.999495f);
			}

			std::string rotx = std::to_string(XMVectorGetX(emitter.rotation));
			std::string roty = std::to_string(XMVectorGetY(emitter.rotation));
			std::string rotz = std::to_string(XMVectorGetZ(emitter.rotation));
			std::string rotw = std::to_string(XMVectorGetW(emitter.rotation));

			//max particles	   std::to_string
			std::string max = std::to_string(emitter.maxParticle);

			//life time
			std::string life = std::to_string(emitter.lifeTimeRate);

			//can revive
			std::string canRevive = std::to_string((emitter.reviveEnable == true) ? 1 : 0);

			// random Time
			std::string randomTime = std::to_string((emitter.randomReviveLapse == true) ? 1 : 0);

			//technique
			std::string technique = Technique::getManager().getNameOf(emitter.tech);

			//file texture
			std::string texture = emitter.nameFile;

			//file texture
			std::string angle = std::to_string(emitter.angle);

			//range
			std::string gravity = std::to_string(emitter.gravity);

			//range
			std::string range = std::to_string(emitter.rangeDistance);

			//type
			std::string type = std::to_string(emitter.type);

			//timing
			std::string time = std::to_string(emitter.timing);

			//multiple frame
			std::string mFrame = std::to_string((emitter.multipleFrames == true) ? 1 : 0);

			//multiple frame
			std::string randomFrame = std::to_string((emitter.randomParticles == true) ? 1 : 0);

			//is animated
			std::string animated = std::to_string((emitter.animated == true) ? 1 : 0);

			//number of frame
			std::string total_frames = std::to_string(emitter.numberOfFrames);

			//number of frames per row
			std::string total_frames_row = std::to_string(emitter.numberOfFramePerRow);

			//speed
			std::string speed = std::to_string(emitter.speed);

			//is Mesh
			std::string isMesh = std::to_string((emitter.meshEntity == true) ? 1 : 0);

			//have end color
			std::string multicolor = std::to_string((emitter.multicolor == true) ? 1 : 0);

			//have begin color
			std::string haveBeginColor = std::to_string((emitter.haveColor == true) ? 1 : 0);
			
			//have end color
			std::string haveEndColor = std::to_string((emitter.haveFinalColor == true) ? 1 : 0);

			//begin color
			float colorR_begin = emitter.colorBeginGlobal.rf();
			float colorG_begin = emitter.colorBeginGlobal.gf();
			float colorB_begin = emitter.colorBeginGlobal.bf();
			float colorA_begin = emitter.colorBeginGlobal.af();

			std::string RColorBegin = std::to_string(colorR_begin);
			std::string GColorBegin = std::to_string(colorG_begin);
			std::string BColorBegin = std::to_string(colorB_begin);
			std::string AColorBegin = std::to_string(colorA_begin);

			//end color
			float colorR_end = emitter.colorEndingGlobal.rf();
			float colorG_end = emitter.colorEndingGlobal.gf();
			float colorB_end = emitter.colorEndingGlobal.bf();
			float colorA_end = emitter.colorEndingGlobal.af();

			std::string RColorEnding = std::to_string(colorR_end);
			std::string GColorEnding = std::to_string(colorG_end);
			std::string BColorEnding = std::to_string(colorB_end);
			std::string AColorEnding = std::to_string(colorA_end);

			//rotation rate
			std::string colorrate = std::to_string(emitter.color_rate);

			//rotation rate
			emitter.rotationRateParticle;
			std::string rotationRate = std::to_string(emitter.rotationRateParticle);

			//is animated
			std::string active = std::to_string((emitter.active == true) ? 1 : 0);
			std::string active_life_counter = std::to_string((emitter.activeLifeCounter == true) ? 1 : 0);
			std::string emitting = std::to_string((emitter.emitting == true) ? 1 : 0);
			std::string physx = std::to_string((emitter.physXEnable == true) ? 1 : 0);
			std::string rangeY = std::to_string((emitter.rangeYEnable == true) ? 1 : 0);
			
			

			myfileParticle << "<!-- Particles System: " + name + " Owner: "+ owner +"-->\n" +
				"\t<particle_system local_position=\"" + posx + " " + posy + " " + posz +
				"\" rot=\"" + rotx + " " + roty + " " + rotz + " " + rotw +
				"\" max_particles=\"" + max +
				"\" life=\"" + life +
				"\" revive=\"" + canRevive +
				"\" random_lapse=\"" + randomTime +
				"\" technique=\"" + technique +
				"\" texture=\"" + texture +
				"\" gravity=\"" + gravity +
				"\" range=\"" + range +
				"\" range_y_enable=\"" + rangeY +
				"\" speed=\"" + speed +
				"\" type=\"" + type +
				"\" timing=\"" + time +
				"\" multiple_frame=\"" + mFrame +
				"\" random_frame=\"" + randomFrame +
				"\" animated=\"" + animated +
				"\" total_frames=\"" + total_frames +
				"\" total_frames_row=\"" + total_frames_row +
				"\" name=\"" + name +
				"\" owner=\"" + owner +
				"\" is_mesh=\"" + isMesh +
				"\" angle=\"" + angle +
				"\" rotation_rate=\"" + rotationRate +
				"\" color_rate=\"" + colorrate +
				"\" haveColorBegin=\"" + haveBeginColor +
				"\" multicolor=\"" + multicolor +
				"\" colorBegin=\"" + RColorBegin + " " + GColorBegin + " " + BColorBegin + " " + AColorBegin +
				"\" haveColorEnding=\"" + haveEndColor +
				"\" colorEnd=\"" + RColorEnding + " " + GColorEnding + " " + BColorEnding + " " + AColorEnding +
				"\" active_lifetime_counter=\"" + active_life_counter +
				"\" active_particle_system=\"" + active +
				"\" emitting=\"" + emitting +
				"\" physx=\"" + physx +
				"\" blendConfig=\"" + std::to_string(emitter.blendConfig) +
				"\" scale=\"" + std::to_string(emitter.scale) +
				"\" zSort=\"" + std::to_string(emitter.zSort) +
				"\"/>\n";
		}

		myfileParticle << "</General>\n";
		myfileParticle.close();

		if (myfileParticle.fail()) MessageBox(NULL, strerror(errno), "ERROR", MB_OK);
		else					   MessageBox(NULL, full_name, "File Saved", MB_OK);
		


	}

	bool ParticleSystemParser::loadLibrary(){
		char full_name[100];
		sprintf(full_name, "%s%s", PARTICLES_PATH, "particles_system.xml");
		return xmlParseFile(full_name);
	}

	void ParticleSystemParser::onStartElement(const std::string &elem, utils::MKeyValue &atts){

		CParticleSystem::ParticlesEmitter e;

		if (elem == "particle_system"){

			if (atts.has("max_particles")){
				e.maxParticle = atts.getInt("max_particles");
				e.backupMaxParticle = e.maxParticle;
			}
			if (atts.has("name")){ e.nameParticles = atts.getString("name", "test"); }
			if (atts.has("owner")){ e.owner = atts.getString("owner", "testOwner"); }
			if (atts.has("life")){ e.lifeTimeRate = atts.getFloat("life"); }
			if (atts.has("revive")){ e.reviveEnable = (atts.getInt("revive") == 1) ? true : false; }
			if (atts.has("random_lapse")){ e.randomReviveLapse = (atts.getInt("random_lapse") == 1) ? true : false; }
			if (atts.has("rot")){e.rotation = atts.getQuatWithoutNorm("rot");}
			if (atts.has("technique")){e.tech = Technique::getManager().getByName(atts.getString("technique", "particles"));}
			if (atts.has("texture")){e.nameFile = atts.getString("texture", "ghost");}
			if (atts.has("gravity")){ e.gravity = atts.getFloat("gravity"); }
			if (atts.has("is_mesh")){ e.meshEntity = (atts.getInt("is_mesh") == 1) ? true : false; }
			if (atts.has("range_y_enable")){ e.rangeYEnable = (atts.getInt("range_y_enable") == 1) ? true : false; }
			if (atts.has("range")){ e.rangeDistance = atts.getFloat("range"); }
			if (atts.has("angle")){ e.angle = atts.getFloat("angle"); }
			if (atts.has("speed")){ e.speed = (atts.getFloat("speed")); }
			if (atts.has("rotation_rate")){ e.rotationRateParticle = (atts.getFloat("rotation_rate")); }
			if (atts.has("type")){ e.type = e.getTypeByInt(atts.getInt("type")); }
			if (atts.has("timing")){ e.timing = atts.getFloat("timing"); }
			if (atts.has("multiple_frame")){ e.multipleFrames = (atts.getInt("multiple_frame") == 1) ? true : false; }
			if (atts.has("random_frame")){ e.randomParticles = (atts.getInt("random_frame") == 1) ? true : false; }
			if (atts.has("animated")){ e.animated = (atts.getInt("animated") == 1) ? true : false; }
			if (atts.has("active_lifetime_counter")){ e.activeLifeCounter = (atts.getInt("active_lifetime_counter") == 1) ? true : false; }
			if (atts.has("active_particle_system")){ e.active = (atts.getInt("active_particle_system") == 1) ? true : false; }
			if (atts.has("total_frames")){ e.numberOfFrames = atts.getInt("total_frames"); }
			if (atts.has("total_frames_row")){ e.numberOfFramePerRow = atts.getInt("total_frames_row"); }
			if (atts.has("haveColorBegin")){ e.haveColor = (atts.getInt("haveColorBegin") == 1) ? true : false; }
			if (atts.has("haveColorEnding")){ e.haveFinalColor = (atts.getInt("haveColorEnding") == 1) ? true : false; }
			if (atts.has("multicolor")){ e.multicolor = (atts.getInt("multicolor") == 1) ? true : false; }
			if (atts.has("colorBegin")){
				XMVECTOR colorBase = atts.getQuatWithoutNorm("colorBegin");
				e.colorBeginGlobal = colorBase;
			}
			if (atts.has("colorEnd")){
				XMVECTOR colorBase = atts.getQuatWithoutNorm("colorEnd");
				e.colorEndingGlobal = colorBase;
			}
			e.color_rate = atts.getFloat("color_rate", e.color_rate);
			e.scale = atts.getFloat("scale", e.scale);
			
			if (atts.has("emitting")){ e.emitting = (atts.getInt("emitting") == 1) ? true : false; }
			if (atts.has("physx")){
				e.physXEnable = (atts.getInt("physx") == 1) ? true : false; 
			}
			if (atts.has("local_position")){ e.localposition = atts.getPoint("local_position"); }
	        
			e.blendConfig = (BlendConfig)atts.getInt("blendConfig", e.blendConfig);
	        e.zSort = atts.getBool("zSort", e.zSort);
			
			if (e.nameParticles.compare("") != 0 && e.owner.compare("") != 0){
				std::string key(e.owner);
				ParticleSystemManager::get().addParticlesSystem(key,e);
			}
		}
	}

}