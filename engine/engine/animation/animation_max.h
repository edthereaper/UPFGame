#ifndef _MAX_ANIMATION_
#define _MAX_ANIMATION_

#include "mcv_platform.h"
#include "utils/data_provider.h"
#include "gameElements/gameMsgs.h"


using namespace DirectX;
using namespace utils;


namespace animation{

    class Transform;
	class CMaxAnim{

	public:

		typedef enum MAXANIMTAG{

			PROP_POS,
			PROP_ROT,
			PROP_POS_ROT
			
		} MaxAnim;

		struct Name {
			char name[32];
		};

		struct HeaderCamera {
			unsigned magic;
			unsigned ntime_keys;
			float    animation_duration;
			unsigned dummy;

			static const unsigned valid_magic = 0x00aaaaa1;
			bool isValid() const {
				if (magic == valid_magic)
					return true;
				return false;
			}
		};

		// A single key
		struct Key {
			XMVECTOR trans;
			XMVECTOR rotation;
		};

	private:

        //-- get information of hexa camera anim
		HeaderCamera      headerCamera;
		std::vector<Key>  keys;
		float             curr_time = 0;
		int				  max = 0;
		bool			  play;
		bool			  finish = false;
		bool			  postfinish = false;
		float			  timing = 1;
		MaxAnim			  type;

		std::string tag1;
		std::string tag2;
		std::string tag3;

		bool load(DataProvider& fdp);
		bool load(const char* name);
		

	public:

		CMaxAnim(){}
		~CMaxAnim(){}

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
		void update(float elapsed);
		void init();
		inline void setPlay(bool play_){ play = play_; }
		inline void setFinish(bool finish_){ finish = finish_; }
		inline void setPostFinish(bool finish_){ postfinish = finish_; }
		void setPivot(XMVECTOR pivot);
		XMVECTOR getPivot();
		void setTiming(float timing_){timing = timing_;}

		bool loadCannon(int tag);
		void enableCannon();

		bool getTransform(float t) const;

	};
}

#endif