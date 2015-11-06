#ifndef ANIMATION_MAX_IMPORTER_H
#define ANIMATION_MAX_IMPORTER_H

#include "mcv_platform.h"
#include "handles/handle.h"
#include "handles/entity.h"
#include "handles/importXML.h"

#define ANIMATION_PATH "data/animmax/"

#include "animation/animation_max.h"

using namespace utils;

namespace animation{

	
	class AnimationMaxImporter : private XMLParser{
	
	private:
		bool		success = false;
		int			max = 0;
		bool		play;
		bool		loop;
		std::string name;
		CMaxAnim::MaxAnim type_;

	private:
		void onStartElement(const std::string &elem, utils::MKeyValue &atts);
		void onEndElement(const std::string &elem){}

	public:
		AnimationMaxImporter(){}
		virtual ~AnimationMaxImporter(){};

		bool getPlay(){ return (success) ? play : false; }
		bool getLoop(){ return (success) ? loop : false; }
		int getMax(){ return (success) ? max : -1; }

		bool loadXMl(const char *path);
		bool loadAnim(const char *path);

	};

}

#endif