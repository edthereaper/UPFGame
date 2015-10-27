#include "mcv_platform.h"
#include "textHelper.h"

#define timeIn  1.0f
#define timeOut 1.0f

namespace gameElements {

	void CTextHelper::loadFromProperties(const std::string& elem, utils::MKeyValue &atts)
	{
		if (elem == "textInfo") {
			textos_t currentText;
			if (atts.has("sizefont"))	{ currentText.sizefont	= (float)atts.getInt("sizefont", 20); }
			if (atts.has("xbox"))		{ currentText.isForXbox = atts.getBool("xbox", false); }
			if (atts.has("timeAnim"))	{ currentText.timeAnim	= atts.getFloat("timeAnim", 2.0f); }
			if (atts.has("trigger"))	{ currentText.trigger	= atts.getInt("trigger", -1); }
			if (atts.has("sentence"))	{ currentText.sentence	= atts.getString("sentence", ""); }
			//dbg("loaded: %i\n", currentText.trigger);
			texts.push_back(currentText);
		}
	}

	void CTextHelper::renderHelper(float elapsed){
		if(actualHint == "") return;
		App &app = App::get();
		float imgWidth		= 0;
		float imgHeight		= 0;
		float imgPosX		= 0;
		float imgPosY		= 0;				 
			
		if(hintIn)			animX = 800 * (timeIn - (timer.count(elapsed) / timeIn));
		if(hintOut)			animX = 800 * (timer.count(elapsed)/timeOut);

		float posBocadilloX = 480 + animX;
		app.getImgValues(imgPosX, imgPosY, imgWidth, imgHeight, posBocadilloX, 508, 800, 212);
		drawTexture2D(
            pixelRect(int(imgPosX), int(imgPosY), int(imgWidth), int(imgHeight)),
            pixelRect(app.config.xres, app.config.yres),
			Texture::getManager().getByName("bocadillo_largo"), nullptr, true);

		if(active){
			float tim = timerLetters.count(elapsed);
			if(actualTimeAnim == 0.0f) actualTimeAnim = 0.001f;
			int numToShow = (int)((tim / actualTimeAnim) * actualHint.size());
			if(numToShow >= actualHint.size()) numToShow = unsigned(actualHint.size());
			//dbg("%f %f %i\n", tim, timeToPrint, numToShow);
			std::string auxS = "";
			mS.clear();
			for (int i = 0; i != numToShow; i++){
				if(actualHint[i] == '|'){
					mS.push_back(auxS);
					auxS = "";
				}else{
					auxS += actualHint[i];
				}	
			}
			mS.push_back(auxS);		
			for (int i = 0; i != mS.size(); i++){
				float posSentenceX = 880 - ((actualSize/2) * mS[i].size());
				posSentenceX += animX;
				float posSentenceY;
				if (mS.size() == 1)	posSentenceY = 600;
				if(mS.size() == 2){
					if (i == 0)		posSentenceY = 570;
					if (i == 1)		posSentenceY = 600;
				}
				if(mS.size() == 3){
					if(i == 0)		posSentenceY = 570;
					if(i == 1)		posSentenceY = 600;
					if(i == 2)		posSentenceY = 630;
				}
				app.getImgValues(imgPosX, imgPosY, imgWidth, imgHeight, posSentenceX, posSentenceY, actualSize, actualSize);
				drawText(
                    pixelRect(int(imgPosX), int(imgPosY), int(imgWidth), int(imgHeight)),
                    pixelRect(app.config.xres, app.config.yres), mS[i]);
			}
		}
		if(timer.count(elapsed) > timeIn && hintIn){
			animX = 0;
			hintIn = false;
			active = true;
			timerLetters.reset();
		}
		if(timer.count(elapsed) > timeOut && hintOut){
			mS.clear();
			hintOut = false;
			active = false;
			actualHint = "";
		}
	}

	void CTextHelper::update(float elapsed)
	{
		
	}

	void CTextHelper::initType()
	{
		SUBSCRIBE_MSG_TO_MEMBER(CTextHelper, MsgPlayerInTuto, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CTextHelper, MsgPlayerOutTuto, receive);
	}

	void CTextHelper::receive(const MsgPlayerInTuto &msg)
	{
		App &app = App::get();
		bool found = false;
		for(int i = 0; i != texts.size() && !found; i++){
			if (msg.tutoZone == texts[i].trigger && app.isXboxControllerConnected() == texts[i].isForXbox){
				actualHint = texts[i].sentence;
				actualSize = texts[i].sizefont;
				actualTimeAnim = texts[i].timeAnim;
				found = true;
				hintIn = true;
				hintOut = false;
				timer.reset();		
			}
		}
	}

	void CTextHelper::receive(const MsgPlayerOutTuto &msg)
	{
		hintOut = true;
		hintIn = false;
		timer.reset();
	}
}