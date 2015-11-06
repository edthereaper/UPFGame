#include "mcv_platform.h"
#include "animation_max_importer.h"
#include "animation_max.h"
#include "utils/data_provider.h"

using namespace component;

namespace animation{

	bool AnimationMaxImporter::loadXMl(const char *path){
		
		char full_name[100];
		sprintf(full_name, "%s%s", ANIMATION_PATH, path);
		return xmlParseFile(full_name);
	}

	void AnimationMaxImporter::onStartElement(const std::string &elem, utils::MKeyValue &atts){

		if (elem == "MaxAnim"){

			std::string name = atts.getString("name", "INVALID");

			std::string maxString = atts.getString("max", "INVALID");
			maxString = maxString.substr(0, maxString.size() - 1);
			animationData_t.max = atoi(maxString.c_str());

			animationData_t.play = atts.getInt("isOn") ? true : false;
			std::string type_ = atts.getString("type");

			if (type_.compare("PROP") == 0)
				animationData_t.type = CMaxAnim::MaxAnim::PROP_POS;
			else if (type_.compare("ROT") == 0)
				animationData_t.type = CMaxAnim::MaxAnim::PROP_ROT;
			else
				animationData_t.type = CMaxAnim::MaxAnim::PROP_POS_ROT;

			success = true;
		}
	}



}