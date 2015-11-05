#ifndef _MAX_ANIMATION_
#define _MAX_ANIMATION_

#include "mcv_platform.h"
#include "handles/importXML.h"

using namespace utils;

#include "animation/animation_max_importer.h"

namespace animation{

	class AnimationMaxImporter;

	class CMaxAnim{

	public:

		friend AnimationMaxImporter;

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

		struct DataAnim_t{
			
			std::string name;

			float       curr_time = 0;
			int			max = 0;
			bool		play;
			bool		finish = false;	
			bool		postfinish = false;
			float		timing = 1;
			
			MaxAnim     type;
			
			std::string tag1;
			std::string tag2;
			std::string tag3;

			bool isPiece = false;

		};

	private:

		//-- get information of hexa camera anim
		HeaderCamera      headerCamera;
		std::vector<Key>  keys;

		DataAnim_t anim;
		AnimationMaxImporter *animExporter;

		bool load(DataProvider& fdp);
		bool load(const char* name);


	public:

		CMaxAnim(){}
		~CMaxAnim(){
			if (animExporter != nullptr){
				SAFE_DELETE(animExporter);
			}
		}

		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
		void loadFromLevel(std::string animationName);
		
		void update(float elapsed);
		void updatePiece(float elapsed);
		void updateObject(float elapsed);
		
		void init();

		bool pieceSecure(){ return (isPiece() && animExporter != nullptr); };
		
		void setPiece(bool piece){anim.isPiece = piece; }
		bool isPiece() { return anim.isPiece;}

		inline void setPlay(bool play_){ anim.play = play_; }
		inline void setFinish(bool finish_){ anim.finish = finish_; }
		inline void setPostFinish(bool finish_){ anim.postfinish = finish_; }
		void setPivot(XMVECTOR pivot);
		XMVECTOR getPivot();
		void setTiming(float timing_){ anim.timing = timing_; }

		//spcial case... cannon (wildcard)
		bool loadCannon(int tag);
		void enableCannon();

		bool getTransform(float t) const;

	};

	
}

#endif