#ifndef CAMERA_MANAGER
#define CAMERA_MANAGER

#pragma once

#include "mcv_platform.h"
#include "handles/handle.h"
#include "handles/entity.h"
#include "handles/importXML.h"

#include "utils/XMLParser.h"

using namespace utils;

namespace cinematic{

	class CameraManager : private XMLParser{

	public:

		struct CameraStream{

		public:
			std::string name;
            std::string nextCam;
			int max;
			int level;
			int idx;
			bool play; // false = stop
			bool active;
			float timing;
            float time_lapse_finish;
            bool finish;


			void defaultStream(){
				
				name = "";
                nextCam="";
				max = 0;
				idx = 0;
				level = 0;
				timing = 1.f;
				play = false;
				active = false;
                finish = false;
                time_lapse_finish = 0.0;
			}

			CameraStream(){defaultStream();}
			CameraStream& operator=(CameraStream const& copy)
			{
				name = "";
                nextCam = "";
				max = copy.max;
				idx = copy.idx;
				play = copy.play;
				active = copy.active;
				level = copy.level;
				timing = copy.timing;
                finish = copy.finish;
				return *this;
			}
		};

		typedef std::vector<CameraStream> CameraStreamList;

	private:
		static CameraManager manager;
		CameraStreamList streams;
		bool freeCam;
		bool playerCam;
		bool cannonCam = false;
	private:
		CameraManager(){
			
		#if defined(_OBJECTTOOL) || defined(_LIGHTTOOL) || defined (_PARTICLES) || defined(_CINEMATIC_) 
            freeCam = true; 
            playerCam = false;
			cannonCam = false;
        #else
            freeCam = false; 
            playerCam = true;
			cannonCam = false;
        #endif

        }
		virtual ~CameraManager(){}
		void onStartElement(const std::string &elem, utils::MKeyValue &atts);
		void onEndElement(const std::string &elem){}

	public:
		bool load(const char* name);
		void alreadyPlayed(std::string name);
		int getIndexByCameraName(std::string name);
		
        static inline CameraManager& get() { return manager; }
		inline void setCannonCam(bool cinematic){ cannonCam = cinematic; }
		inline bool isCannonCam() const { return cannonCam; }
		inline void setFreeCam(bool cinematic){ freeCam = cinematic; }
		inline bool isFreeCam() const { return freeCam; }
		inline void setPlayerActive(bool player){ playerCam = player; }
		inline bool isPlayerCam() const { return playerCam; }
        inline CameraStreamList getCamerasCinematic() { return streams; }
		inline int count() { return (int)streams.size(); }
		inline CameraStream getCameraByIndex(int index) { return streams[index]; }
		static void cameraTriggerAction (const char *script);
	};

	
}
#endif