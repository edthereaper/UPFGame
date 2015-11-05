#include "mcv_platform.h"
#include "animation/animation_max.h"
#include "handles/handle.h"
#include "handles/entity.h"
#include "handles/importXML.h"

#define ANIMATION_PATH "data/animmax/"

using namespace utils;

namespace animation{

	
	class AnimationMaxImporter : private XMLParser{

	public:
		friend CMaxAnim;
		CMaxAnim::DataAnim_t animationData_t;
		bool success = false;

	private:
		void onStartElement(const std::string &elem, utils::MKeyValue &atts);
		void onEndElement(const std::string &elem){}

	public:
		AnimationMaxImporter(){}
		AnimarionMaxImporter(CMaxAnim::DataAnim_t &anim){ animationData_t = anim; }
		virtual ~AnimationMaxImporter(){}
		bool loadXMl(const char *path);
		bool loadAnim(const char *path);
		CMaxAnim::DataAnim_t getAnimationData(){ return animationData_t; }

	};

}