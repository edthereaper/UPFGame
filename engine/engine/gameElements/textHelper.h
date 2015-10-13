#ifndef GAMEELMENTS_CTEXT_H_
#define GAMEELMENTS_CTEXT_H_

#include "mcv_platform.h"
#include "gameMsgs.h"
#include "utils/itemsByName.h"
#include "../render/render_utils.h"
#include "app.h"
#include "handles/entity.h"
#include "handles/handle.h"
#include "handles/prefab.h"
#include "player\playerMov.h"
using namespace component;
using namespace utils;

namespace gameElements {

	class CTextHelper {
	public:
		struct textos_t {
			int trigger = -1;
			bool isForXbox = false;
			std::string sentence = "";
			float sizefont = 20.0f;
			float timeAnim = 2.0f;
			textos_t() = default;
		};

		void loadFromProperties(const std::string& elem, MKeyValue &atts);
		void update(float elapsed);
		inline void init() {}
		static void initType();
		void receive(const MsgPlayerInTuto& msg);
		void receive(const MsgPlayerOutTuto& msg);
		void renderHelper(float elapsed);
	private:
		std::vector<textos_t> texts;
		std::string actualHint = "";
		float actualSize = 20.0f;
		float actualTimeAnim = 2.0f;
		bool hintIn = false;
		bool hintOut = false;
		std::vector<std::string> mS;
		utils::Counter<float> timer, timerLetters;
	};

}

#endif
