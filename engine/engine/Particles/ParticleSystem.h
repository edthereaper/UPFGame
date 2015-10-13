#ifndef _PARTICLE_SYSTEM_
#define _PARTICLE_SYSTEM_

#include "mcv_platform.h"
#include "handles/handle.h"
#include "render/mesh/component.h"
#include "render/mesh/mesh.h"
#include "render/render_utils.h"
#include "PhysX_USER/PhysicsManager.h"
#include "PhysX_USER/pxcomponents.h"
#include "gameElements/gameMsgs.h"


using namespace render;
using namespace DirectX;
using namespace component;
using namespace physX_user;

#include "behavior/fsm.h"

namespace particles {
	void  initType();
	class ParticlesFSMExecutor;
}
namespace behavior { typedef Fsm<particles::ParticlesFSMExecutor> ParticlesFSM; }

namespace particles {

	class CParticleSystem;
	class ParticlesFSMExecutor{
	
		public:
			friend CParticleSystem;
			friend behavior::ParticlesFSM;

			enum states {
				STATE_create	   , // particles created
				STATE_active	   , // particles active in scene and emitting
				STATE_reset       , // particles to shutdown states and create explosion
				STATE_inactive	   , // particles inactive but its inside scene 
				STATE_kill         , // particles ready to die
				STATE_wait_active  , // particles waiting to be active
				STATE_wait_inactive, // particles waiting to be inactive
				STATE_wait_kill    , // particles waiting to be kill
				STATE_wait_reset, // particles waiting to be kill
				STATE_done			 // particles done
			};

		private:
			//states
			behavior::fsmState_t create(float elapsed);
			behavior::fsmState_t active(float elapsed);
			behavior::fsmState_t inactive(float elapsed);
			behavior::fsmState_t reset(float elapsed);
			behavior::fsmState_t wait_active(float elapsed);
			behavior::fsmState_t wait_inactive(float elapsed);
			behavior::fsmState_t wait_kill(float elapsed);
			behavior::fsmState_t wait_reset(float elapsed);
			behavior::fsmState_t kill(float elapsed);
			behavior::fsmState_t done(float elapsed){ return STATE_done; }

		public:

			inline behavior::fsmState_t init() { return STATE_create; }
			void update(float elapsed){}

			utils::Counter<float> timerActive;
			utils::Counter<float> timerInactive;
			utils::Counter<float> timerKiller;
			utils::Counter<float> timerReset;

			Handle attatchEntity;
			Handle meEntity;

			float time_to_active = .0f;
			float time_to_inactive = .0f;
			float time_to_kill = .0f;
			float time_to_reset = .0f;

			inline void setTimeToActive(float active){ time_to_active = active; }
			inline void setTimeToInactive(float inactive){ time_to_inactive = inactive; }
			inline void setTimeToKill(float kill)	{ time_to_kill = kill; }
			inline void setTimeToReset(float reset_)	{ time_to_reset = reset_; }


			

	};
	
	//-----------------------------------------------------------------------------//
	//-----------------------------------------------------------------------------//
	//-----------------------------------------------------------------------------//
	

	class CParticleSystem{


	public:
		friend ParticlesFSMExecutor;
	private:
		behavior::ParticlesFSM fsm;

	public:

		enum ParticlesType{
			CONE = 1,
			BOX = 2,
			DISK = 3,
			EDGE = 4,
			SPHERE = 5,
			SEMISPHERE = 6,
			RANDOM = 7,
			BUTTERFLY = 8,
			DEFAULT = CONE
		};

	public:

		struct ParticlesEmitter{
		
			bool  finishLoading;
			bool  isStride;
			bool rangeYEnable;
			float rangeDistance;
			float gravity = 0.0f;
			float timing = 10;
			int   numberOfFrames = 1;
			int   numberOfFramePerRow = 1;
			
			bool  meshEntity = false;
			float angle = 30;
			float speed = 1;
			bool  entityRotationEnable;
			bool  animated = false;
			bool  randomParticles = false;
			bool  multipleFrames = false;
			bool  haveColor = false;
			bool  haveFinalColor = false;
			bool  multicolor = false;
			bool  dissappear = false;
		    render::BlendConfig blendConfig = BLEND_PARTICLES;

			XMVECTOR rotation = XMVectorZero();
			XMVECTOR position = XMVectorZero();
			ParticlesType type;
			ColorV colorBeginGlobal;
			ColorV colorEndingGlobal;
			float color_rate = 1;

            float   scale = 1;
			float   lifeTimeRate = 0.0;
			bool    reviveEnable = true;
			bool    randomReviveLapse = true;

			std::string nameMesh;
			std::string nameFile;
			std::string owner;
			Technique* tech;
			std::string nameParticles;
			int			maxParticle;
			int			backupMaxParticle;

			bool active;
			bool activeLifeCounter;

			bool activeRotation;
			float rotationRateParticle = 0;		

			ParticlesType getTypeByInt(int type_);
			bool motionSimulator = false;
			bool leftMotion = true;
			bool emitting = false;
			bool physXEnable = false;
			float accelerationMotion;
	
			bool effectImplosion = false;
			bool motionVortex = false;
			XMVECTOR localposition;

            bool zSort = false;
			bool valid = true;
			bool success = false;
			bool isOn = false;

			ParticlesEmitter(){ valid = false; }

			bool killWhenFinishColorLoader = false;

		};
	

	private:

		typedef VertexParticleUData ParticleData;
		struct LifeTimeData {

			float lifeTime=0;
			uint32_t index=0;
			utils::Counter<float> timer;
			

			inline float getRate() const {return (timer.get() / lifeTime);}

			inline void setLifeTime(float lifeTime_){ lifeTime = lifeTime_; }
			inline void setTimer(float timer_set){ timer.set(timer_set); }
			inline float getLifeTime(){ return lifeTime; }
			inline bool isDead() const {return timer >= lifeTime;}
			inline bool countTime(float elapsed) {
                return timer.count(elapsed) >= lifeTime;
            }
            void reset() {timer.reset();}
		};

		struct VelocityData{
			XMVECTOR currentVelocity;
			XMVECTOR startVelocity;
		};

		struct PersonalData{
			private:
                float scale = 1;

			public:
				inline float getColorWeight(const LifeTimeData& life,
                    float colorRate, float elapsed) {
                    return life.isDead() ? 0 : inRange(0.f,life.getRate()/colorRate, 1.f);
                }
                inline float getScale() const {return scale;}
		};

	private:

		Mesh* instanceData = nullptr;
		Mesh* instanceMesh = nullptr;
		std::string nameMesh;
		std::string nameFile;
		std::string nameTechnique;
		std::string nameParticles;

        struct vectors_t {

            std::vector<UINT32>		  indices;
		    std::vector<VelocityData> velocitys;
		    std::vector<ParticleData> particlesDatas;
		    std::vector<LifeTimeData> lifeTimes;

		    //---------------------------physx---------------------------
		    std::vector<PersonalData> personalDatas;

		    std::vector<UINT32>		  indicesUpdate;

		    std::vector<PxU32>			physx_indices;
		    std::vector<PxVec3>			physx_positions;
		    std::vector<PxVec3>			physx_velocitys;
		    std::vector<PxU32>			physx_indices_updates;
		    std::vector<PxVec3>			physx_positions_updates;
		    std::vector<PxVec3>			physx_velocitys_updates;

			bool isEmpty(){ return (indices.size() == 0); }
			void clear(){
				indices.clear();
				velocitys.clear();
				particlesDatas.clear();
				lifeTimes.clear();
				personalDatas.clear();
				indicesUpdate.clear();
				physx_indices.clear();
				physx_positions.clear();
				physx_velocitys.clear();
				physx_indices_updates.clear();
				physx_positions_updates.clear();
				physx_velocitys_updates.clear();
			}

		};

		vectors_t* vectors = nullptr;
		
        bool physXEnable = false;

		//PxParticleFluid				*physx_particle_system = nullptr;
		PxParticleSystem				*physx_particle_system = nullptr;
		PxParticleCreationData 		 physx_particle_data;
		PxParticleExt::IndexPool	*indexPool;
		//---------------------------physx---------------------------

	private:

#if PX_SUPPORT_GPU_PHYSX
		bool mRunOnGpu = true;
#endif
		bool deleteSelf = false;
		unsigned nAlive = 0;
		float accumulated = 0.0f;

	private:
        void updateParticle(ParticleData& p, float elapsed,
            PersonalData& pData, const LifeTimeData& lifeTime);
        void partitionAlive();
        void sortParticlesZ();
		void setShapesVelocity();
		XMVECTOR setVelocityParticle();
		void setColors();
		void polutionSimulator(ParticleData& p, VelocityData &velocity, XMVECTOR currentPosition);
		void butterflySimulator(ParticleData& p, VelocityData &velocity, XMVECTOR currentPosition, float elapsed);
		ParticlesEmitter emitter;
		utils::Counter<float> timer;


	public:
		~CParticleSystem(){
            SAFE_DELETE(vectors);

			if (instanceMesh!=nullptr)
				instanceMesh = nullptr;

			if (instanceData != nullptr)
				instanceData = nullptr;
			
			clear();
		}

        inline bool willDeleteSelf() const {return deleteSelf;}
        inline void setDeleteSelf(bool b=true) {deleteSelf = b;}

		inline void setLoadColors(const ColorV& color_ = zero_v);
		inline void init(){ initType(); };
		void loadFromProperties(const std::string& elem, utils::MKeyValue &atts);
		void update(float elapseds);
		void physx_particles_update(float elapsed);
		void fixedUpdate(float elapsed){}
		void render();
		void setup();
		void load(Mesh* mesh_);
		static inline float meter2Cm(float m){ return m * .01f; }
		static inline float meter2Mm(float m){ return m * .001f; }
		void valid();
		void clear();


		/************************************************************************/
		/**************************** Setters **********************************/
		/************************************************************************/
		//properties

		XMVECTOR& refLocalPosition(){ return emitter.localposition; }
		void sefLocalPosition(XMVECTOR vect){ emitter.localposition = vect; }
		void setLifeTimeRate(float  lifetime_){ emitter.lifeTimeRate = lifetime_; }
		void setRevive(bool revive_);
		void setRandomRevive(bool revive_random_);
		inline void setRangeDistance(float  range_){ emitter.rangeDistance = range_; }
		inline void setRangeYEnable(bool y_enable){ emitter.rangeYEnable = y_enable; }
		inline void setNameMesh(std::string name){ emitter.nameMesh = name; }
		inline void setNameFile(std::string nfile){ emitter.nameFile = nfile; }
		inline void setTechnique(Technique* technique){ emitter.tech = technique; }
		inline void setMaxParticle(int mParticles){ emitter.maxParticle = mParticles; }
		inline void setGravity(float gravityRate){ if (!physXEnable){ emitter.gravity = gravityRate; } }
		inline void setTiming(float time_){ emitter.timing = time_; }
		inline void setNumberOfFrames(int n_){ emitter.numberOfFrames = n_; }
		inline void setNumberOfFramePerRow(int n_){ emitter.numberOfFramePerRow = n_; }
		inline void setNameParticles(std::string name_){ emitter.nameParticles = name_; }
		inline void setOwner(std::string name_){ emitter.nameParticles = name_; }
		inline void setEntityMesh(bool is_mesh_) { emitter.meshEntity = is_mesh_; }
		inline void setSpeed(float speed_) { emitter.speed = speed_; }
		inline void setAngleTWEAK(float angle_) { emitter.angle = angle_; setShapesVelocity(); }
		inline void setAngle(float angle_) { emitter.angle = angle_; }
		void setmultipleFrames(bool enable_);
		void setAnimated(bool animated_);
		void setEmitting(bool emitting_){ emitter.emitting = emitting_; }
		void setRandomParticles(bool randomParticles_);
		void setMaxParticleTWEAK(int mParticles);
		void setRotationTWEAK(XMVECTOR rotation_);
		inline void setEntityRotationEnable(bool entityRotationEnable_){ emitter.entityRotationEnable = entityRotationEnable_; };
		inline void setActiveLifeCounter(bool active_){ emitter.activeLifeCounter = active_; }
		inline void setActiveRotation(bool active_){ emitter.activeRotation = active_; }
		inline void setRotationRateParticle(float rateRotation){ emitter.rotationRateParticle = rateRotation; }
        inline void setGlobalScale(float s) {emitter.scale = s;}
		inline void setHaveAColor(bool enable){ emitter.haveColor = enable; setColors(); }
		inline void setHaveAFinalColor(bool enable){ emitter.haveFinalColor = enable; setColors();}
		inline void setHaveMultiColor(bool enable){ emitter.multicolor = enable; setColors(); }
		void setColorRate(float color_rate);
		void setCurrentBeginColor(const ColorV& color_);
		void setCurrentEndColor(const ColorV& color_);
		inline void setMotionSimulator(bool active){ emitter.motionSimulator = active; }
		inline void setMotionAcceleration(float accelaration_){ if (emitter.motionSimulator) emitter.accelerationMotion = accelaration_; }
		void resetposition();
		void reload(){ setup(); }
		
        inline void setBlendConfig(render::BlendConfig cnf) {emitter.blendConfig = cnf;}
        inline render::BlendConfig getBlendConfig() const {return emitter.blendConfig;}
        void setZSort(bool b) {emitter.zSort = b;}
		void setKillWhenFinishColorLoader(bool kill_){ emitter.killWhenFinishColorLoader = kill_; }

		//setStyles
		inline void setCone(bool type_){ if (type_) if (emitter.type != ParticlesType::CONE){			  emitter.type = ParticlesType::CONE; emitter.rangeDistance = 0.01f;  setShapesVelocity(); } }
		inline void setBox(bool type_){ if (type_) if (emitter.type != ParticlesType::BOX){				  emitter.type = ParticlesType::BOX; emitter.rangeDistance = 0.01f; setShapesVelocity(); } }
		inline void setSphere(bool type_){ if (type_) if (emitter.type != ParticlesType::SPHERE){		  emitter.type = ParticlesType::SPHERE; emitter.rangeDistance = 0.01f; setShapesVelocity(); } }
		inline void setSemisphere(bool type_){ if (type_) if (emitter.type != ParticlesType::SEMISPHERE){ emitter.type = ParticlesType::SEMISPHERE; emitter.rangeDistance = 0.01f; setShapesVelocity(); } }
		inline void setEdge(bool type_){ if (type_) if (emitter.type != ParticlesType::EDGE){			  emitter.type = ParticlesType::EDGE; emitter.rangeDistance = 0.01f; setShapesVelocity(); } }
		inline void setRandom(bool type_){ if (type_) if (emitter.type != ParticlesType::RANDOM){		  emitter.type = ParticlesType::RANDOM; emitter.rangeDistance = 100.f;  setShapesVelocity(); } }
		inline void setButterfly(bool type_){ if (type_) if (emitter.type != ParticlesType::BUTTERFLY){   emitter.type = ParticlesType::BUTTERFLY; emitter.rangeDistance = 2;  setShapesVelocity(); } }
		inline void setDisk(bool type_){ if (type_) if (emitter.type != ParticlesType::DISK){			  emitter.type = ParticlesType::DISK; emitter.rangeDistance = 0.01f; setShapesVelocity(); } }

		//----------------------------- Physx ----------------------------------//
		void setEnablePhysX(bool enable_);
		inline void setPhysXDamping(float damping_){ if (physXEnable){ physx_particle_system->setDamping(damping_); } }
		inline void setPhysXExternalAcceleration(XMVECTOR acceleration){ if (physXEnable){ physx_particle_system->setExternalAcceleration(toPxVec3(acceleration)); } }
		inline void setPhysXParticleMass(float particle_mass){ if (physXEnable){ physx_particle_system->setParticleMass(particle_mass); } }
		inline void setPhysXRestitution(float restitution_){ if (physXEnable){ physx_particle_system->setRestitution(restitution_); } }
		inline void setPhysXDynamic(float dynamic){ if (physXEnable){ physx_particle_system->setDynamicFriction(dynamic); } }
		inline void setPhysXStatic(float static_){ if (physXEnable){ physx_particle_system->setStaticFriction(static_); } }
		inline void setPhysXMaxMotion(float max_motion){ if (physXEnable){ physx_particle_system->setMaxMotionDistance(max_motion); } }
		inline void setPhysXRestOffset(float offset_){ if (physXEnable){ physx_particle_system->setRestOffset(offset_); } }
		inline void setPhysXGridSize(float size_){ if (physXEnable){ physx_particle_system->setGridSize(size_); } }
		

		/************************************************************************/
		/**************************** getter`s **********************************/
		/************************************************************************/

		inline int getMaxParticle() const { return emitter.maxParticle; }
		inline XMVECTOR getLocalPosition() { return emitter.localposition; }
		inline float getLifeTimeRate() const { return emitter.lifeTimeRate; }
		inline bool canRevive() const { return emitter.reviveEnable; }
		inline bool getRandomRevive() const { return emitter.randomReviveLapse; }
		inline float getRangeDistance() const { return emitter.rangeDistance; }
		inline bool getRangeYEnable() const { return emitter.rangeYEnable; }
		inline float getGravity()const { return emitter.gravity; }
		inline float getTiming() const { return emitter.timing; }
		inline int getNumberOfFrames() const { return emitter.numberOfFrames; }
		inline int getNumberOfFramePerRow() const { return emitter.numberOfFramePerRow; }
		inline std::string getNameMesh() const { return emitter.nameMesh; }
		inline std::string getNameFile() const { return emitter.nameFile; }
		inline std::string getOwner() const { return emitter.owner; }
		inline Technique* getTechnique() const { return emitter.tech; }
		inline std::string getNameParticles() const { return emitter.nameParticles; }
		bool getAnimated() const;
		bool getRandomParticles() const;
		inline bool getEntityMesh() const { return emitter.meshEntity; };
		inline float getSpeed() const { return emitter.speed; }
		inline float getAngle() const { return emitter.angle; }
		inline bool getmultipleFrames() const{ return emitter.multipleFrames; };
		inline bool getMotionSimulator() const { return emitter.motionSimulator; }
		inline float getMotionAcceleration() const{ return emitter.accelerationMotion; }

		//getStyles
		inline bool getCone()		const { return (emitter.type == ParticlesType::CONE); }
		inline bool getBox()		const { return (emitter.type == ParticlesType::BOX); }
		inline bool getSphere()		const { return (emitter.type == ParticlesType::SPHERE); }
		inline bool getSemisphere() const { return (emitter.type == ParticlesType::SEMISPHERE); }
		inline bool getEdge()		const { return (emitter.type == ParticlesType::EDGE); }
		inline bool getRandom()		const { return (emitter.type == ParticlesType::RANDOM); }
		inline bool getButterfly()		const { return (emitter.type == ParticlesType::BUTTERFLY); }
		inline bool getDisk()		const { return (emitter.type == ParticlesType::DISK); }
		inline int getType()		{ return (int)emitter.type; }
		XMVECTOR getRotationTWEAK() const ;
		XMVECTOR getLocalRotation(){ return emitter.rotation; };
		inline bool getEntityRotationEnable(){ return emitter.entityRotationEnable; };
		inline bool getActiveRotation() const { return emitter.activeRotation; }
		inline float getRotationRateParticle() const { return emitter.rotationRateParticle; }
        inline float getGlobalScale() const {return emitter.scale;}
		inline ColorV getCurrentBeginColor() const{ return emitter.colorBeginGlobal; }
		inline ColorV getCurrentEndingColor() const{ return emitter.colorEndingGlobal; }
		inline bool getHaveAColor() const { return emitter.haveColor; }
		inline bool getHaveAFinalColor() const { return emitter.haveFinalColor; }
		inline bool getHaveMultiColor() const { return emitter.multicolor; }
		inline bool isEmitting(){ return emitter.emitting; }

		bool getActiveLifeCounter() const{ return  emitter.activeLifeCounter; };
		bool getActive() const{ return  emitter.active;};
		float getColorRate() const{ return emitter.color_rate; }
        bool getZSort() const {return emitter.zSort;}


		//----------------------------- Physx ----------------------------------//
		inline bool getEnablePhysX() const				     { return physXEnable; }
		inline float getPhysXDamping() const				 { if (physXEnable){ return (float)physx_particle_system->getDamping(); } return 0.0f; }
		inline XMVECTOR getPhysXExternalAcceleration() const { if (physXEnable){ return toXMVECTOR(physx_particle_system->getExternalAcceleration()); } return XMVectorSet(0,0,0,0); }
		inline float getPhysxParticleMass() const		     { if (physXEnable){ return (float)physx_particle_system->getParticleMass(); } return 0.0f; }
		inline float getPhysxRestitution() const			 { if (physXEnable){ return (float)physx_particle_system->getRestitution(); }return 0.0f; }
		inline float getPhysxDynamic() const				{ if (physXEnable){ return (float)physx_particle_system->getDynamicFriction(); } return 0.0f; }
		inline float getPhysxStatic() const				    { if (physXEnable){ return (float)physx_particle_system->getStaticFriction(); } return 0.0f; }
		inline float getPhysxMaxMotion() const				{ if (physXEnable){ return (float)physx_particle_system->getMaxMotionDistance(); } return 0.0f; }
		inline float getPhysxRestOffset() const				{ if (physXEnable){ return (float)physx_particle_system->getRestOffset(); } return 0.0f; }
		inline float getPhysxGridSize() const				{ if (physXEnable){ return (float)physx_particle_system->getGridSize(); } return 0.0f; }
		

		/************************************************************************/
		/**************************** ParticlesEmitters Duplicates ************************/
		/************************************************************************/

		void setParticlesEmitter(ParticlesEmitter emitter_);
		ParticlesEmitter getParticlesEmitter(){ return emitter; }
		void setParticlesEmitterByDefault();

		//out of texture
		public:
			void enable(bool enable_);
			void activeParticlesEmitterByTime(float time, float elapsed);
			void activeParticlesEmitterHeight(float time, float elapsed);
			void clean();
			static void initType();
			inline void setFinish(bool finish){ emitter.finishLoading = finish; }
			inline bool	 getFinish() { return emitter.finishLoading; }

		//**************************************************************************//
			inline void setMe(){ fsm.getExecutor().meEntity = Handle(this).getOwner(); }
			
			inline void setTimeKill(float kill_){ 
				fsm.getExecutor().setTimeToKill(kill_); 
			}
			inline void setTimeInactive(float inactive_){ fsm.getExecutor().setTimeToInactive(inactive_); }
			inline void setTimeActive(float active_){ fsm.getExecutor().setTimeToActive(active_); }

			inline void receive(const gameElements::MsgEmitterParticles& msg) {
				fsm.getExecutor().attatchEntity = msg.handle;
			}

			inline void receive(const gameElements::MsgActiveParticles& msg){
				fsm.changeState(ParticlesFSMExecutor::states::STATE_active);
			}

			inline void receive(const gameElements::MsgInactiveParticles& msg){
				fsm.changeState(ParticlesFSMExecutor::states::STATE_inactive);
			}

			inline void receive(const gameElements::MsgKillParticlesInmidiatly& msg){
				fsm.changeState(ParticlesFSMExecutor::states::STATE_kill);
			}

			inline void receive(const gameElements::MsgActiveParticlesByTime& msg){
				fsm.getExecutor().setTimeToActive(msg.timer);
				fsm.changeState(ParticlesFSMExecutor::states::STATE_wait_active);
			}

			inline void receive(const gameElements::MsgInactiveParticlesByTime& msg){
				fsm.getExecutor().setTimeToInactive(msg.timer);
				fsm.changeState(ParticlesFSMExecutor::states::STATE_wait_inactive);
			}

			inline void receive(const gameElements::MsgKillParticlesByTime& msg){
				fsm.getExecutor().setTimeToKill(msg.timer);
				fsm.changeState(ParticlesFSMExecutor::states::STATE_wait_kill);
			}

			inline void receive(const gameElements::MsgResetParticles& msg){
				fsm.changeState(ParticlesFSMExecutor::states::STATE_reset);
			}

			inline void receive(const gameElements::MsgResetByTimeParticles& msg){
				fsm.getExecutor().setTimeToReset(msg.timer);
				fsm.changeState(ParticlesFSMExecutor::states::STATE_wait_reset);
			}
			
			
			//**************************************************************************//

	};

}

#endif