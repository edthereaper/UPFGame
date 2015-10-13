#ifndef _CAMERA_ANIMATION_
#define _CAMERA_ANIMATION_

#include "mcv_platform.h"
#include "components/transform.h"
#include "utils/data_provider.h"
#include "Cinematic/camera_manager.h"
#include "gameElements/gameMsgs.h"

//#include "behavior/bt.h"
//
//namespace cinematic{ class CameraAnimBtExecutor; }
//namespace behaviour {
//	typedef Bt<cinematic::CameraAnimBtExecutor> CameraAnimBT;
//}

using namespace DirectX;
using namespace utils;

//using namespace behavior;

namespace cinematic{

	
  //  class CCameraAnim;

  //  class CameraAnimBtExecutor{
  //  
  //      public:
  //      enum nodeId_e {
		//	WAIT = 0xD101, // 
		//	ENABLE = 0xD102, // is cam handle set on cinematic
		//	DISABLE = 0xD103, // is cam handle set on player
		//	PLAY = 0xD104,  // camera cinematic play
		//	STOP = 0xD105, // camera cinematic stop
		//	TRIGGER = 0xD106, // action by trigger
		//};

  //      enum event_e {
		//	E_PLAY,
		//	E_STOP,
		//	E_WAIT,
		//	E_ENABLE,
		//	E_DISABLE,
		//} lastEvent = E_DISABLE;

  //      friend behaviour::CameraAnimBT; 
		//friend CCameraAnim;

  //  };

    class Transform;
	class CCameraAnim{

	public:

		struct Name {
			char name[32];
		};

		struct HeaderCamera {
			unsigned magic;
			unsigned ntime_keys;
			float    animation_duration;
			unsigned dummy;

			static const unsigned valid_magic = 0xfedcba00;
			bool isValid() const {
				if (magic == valid_magic)
					return true;
				return false;
			}
		};

		// A single key
		struct Key {
			XMVECTOR trans;
			XMVECTOR target;
		};

	private:

        //-- get information of hexa camera anim
		HeaderCamera      headerCamera;
		std::vector<Key>  keys;
		std::string		  fileName;
		float             curr_time;
		cinematic::CameraManager::CameraStream stream;
        Counter<float> counterFinish;

		//- load
		bool load(DataProvider& fdp);
		bool finish = false;

        //---- handle player ---//
        Handle playerH;
        
		

	public:

		CCameraAnim(){}
		~CCameraAnim(){}

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts){}
		void update(float elapsed);
		void init();

		bool load(const char *new_name);
		bool getTransform(float t) const;
		
		inline void        setTiming(float timing_) { stream.timing = timing_; }
		inline float       getTiming() const { return stream.timing; }
		inline void        setPlay(bool play_) { stream.play = play_; }
		inline float       getPlay() const { return stream.play; }

		inline void setActive(bool on){
            stream.active = on;
            if (on) CameraManager::get().setPlayerActive(false);
            else    CameraManager::get().setPlayerActive(true);
        }
		inline bool isActice() const { return stream.active; }
		inline void setStream(cinematic::CameraManager::CameraStream stream_){ 
			stream = stream_;
			finish = stream.finish;
            curr_time = 0;
			stream.name = stream_.name; 
			stream.level = stream_.level;
			load(stream.name.c_str()); 
			getTransform(0.f);
			
			#if defined(_CINEMATIC_)
			#else
			if (!finish){
				setPlay(true);
				CameraManager::get().setPlayerActive(false);
			}
			#endif
		};
		inline cinematic::CameraManager::CameraStream getStream() const {return stream; }
        inline void setPlayerHandle(Handle player_){if (player_.isValid()) playerH = player_;}

	};
}

#endif