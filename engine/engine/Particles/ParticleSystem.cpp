#include "mcv_platform.h"
#include "ParticleSystem.h"
#include "handles/message.h"
#include "handles/handle.h"
#include "handles/importXML.h"
#include "handles/entity.h"
#include "handles/objectManager.h"
#include "components/transform.h"
#include "render/camera/component.h"
#include "render/mesh/component.h"
#include "render/renderManager.h"
#include "render/mesh/mesh.h"
#include "components/transform.h"
#include "handles/entity.h"

#include "render/shader/vertex_declarations.h"
#include "render/render_utils.h"

#include "app.h"

using namespace render;


using namespace DirectX;
using namespace utils;
using namespace physx;
using namespace component;


namespace particles {

	void CParticleSystem::initType(){
		behavior::ParticlesFSM::initType();
		SUBSCRIBE_MSG_TO_MEMBER(CParticleSystem, gameElements::MsgEmitterParticles,        receive);
		SUBSCRIBE_MSG_TO_MEMBER(CParticleSystem, gameElements::MsgActiveParticles,		   receive);
		SUBSCRIBE_MSG_TO_MEMBER(CParticleSystem, gameElements::MsgActiveParticlesByTime,   receive);
		SUBSCRIBE_MSG_TO_MEMBER(CParticleSystem, gameElements::MsgInactiveParticles,	   receive);
		SUBSCRIBE_MSG_TO_MEMBER(CParticleSystem, gameElements::MsgInactiveParticlesByTime, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CParticleSystem, gameElements::MsgKillParticlesInmidiatly, receive);
		SUBSCRIBE_MSG_TO_MEMBER(CParticleSystem, gameElements::MsgKillParticlesByTime,	   receive);
	}

	void CParticleSystem::load(Mesh* mesh_){

		if (emitter.meshEntity){
			instanceMesh = mesh_;
			Entity* e = Handle(this).getOwner();
			CMesh * cmesh = e->get<CMesh>();
			cmesh->setMesh(instanceMesh);
		}
	}

	void CParticleSystem::loadFromProperties(const std::string& elem, utils::MKeyValue &atts){


#if !defined(_PARTICLES)

		std::string tmp = "";


		if (elem == "particle_system"){


			if (atts.has("max_particles")){
				emitter.maxParticle = atts.getInt("max_particles");
				emitter.backupMaxParticle = emitter.maxParticle;
			}
			if (atts.has("name")){ emitter.nameParticles = atts.getString("name", "test"); }
			emitter.localposition = XMVectorSet(0, 0, 0, 0);
			if (atts.has("local_position")){
				emitter.localposition = atts.getPoint("local_position");
			}
			if (atts.has("life")){ emitter.lifeTimeRate = atts.getFloat("life"); }
			if (atts.has("revive")){ emitter.reviveEnable = (atts.getInt("revive") == 1) ? true : false; }
			if (atts.has("random_lapse")){ emitter.randomParticles = (atts.getInt("random_lapse") == 1) ? true : false; }
			if (atts.has("rot")){ emitter.rotation = atts.getQuatWithoutNorm("rot"); }
			if (atts.has("technique")){ emitter.tech = 
                Technique::getManager().getByName(atts.getString("technique", "particles")); }
			if (atts.has("texture")){ emitter.nameFile = atts.getString("texture", "ghost"); }
			if (atts.has("gravity")){ emitter.gravity = atts.getFloat("gravity"); }
			if (atts.has("is_mesh")){ emitter.meshEntity = (atts.getInt("is_mesh") == 1) ? true : false; }
			if (atts.has("range")){ emitter.rangeDistance = atts.getFloat("range"); }
			if (atts.has("range_y_enable")){ emitter.rangeYEnable = (atts.getInt("range_y_enable") == 1) ? true : false; }
			if (atts.has("angle")){ emitter.angle = atts.getFloat("angle"); }
			if (atts.has("speed")){ emitter.speed = (atts.getFloat("speed")); }
			if (atts.has("rotation_rate")){ emitter.rotationRateParticle = (atts.getFloat("rotation_rate")); }
			if (atts.has("type")){ emitter.type = emitter.getTypeByInt(atts.getInt("type")); }
			if (atts.has("timing")){ emitter.timing = atts.getFloat("timing"); }
			if (atts.has("multiple_frame")){ emitter.multipleFrames = (atts.getInt("multiple_frame") == 1) ? true : false; }
			if (atts.has("random_frame")){ emitter.randomParticles = (atts.getInt("random_frame") == 1) ? true : false; }
			if (atts.has("animated")){ emitter.animated = (atts.getInt("animated") == 1) ? true : false; }
			if (atts.has("active_lifetime_counter")){ emitter.activeLifeCounter = (atts.getInt("active_lifetime_counter") == 1) ? true : false; }
			if (atts.has("active_particle_system")){ emitter.active = (atts.getInt("active_particle_system") == 1) ? true : false; }
			if (atts.has("total_frames")){ emitter.numberOfFrames = atts.getInt("total_frames"); }
			if (atts.has("total_frames_row")){ emitter.numberOfFramePerRow = atts.getInt("total_frames_row"); }
			if (atts.has("haveColorBegin")){ emitter.haveColor = (atts.getInt("haveColorBegin") == 1) ? true : false; }
			if (atts.has("haveColorEnding")){ emitter.haveFinalColor = (atts.getInt("haveColorEnding") == 1) ? true : false; }
			if (atts.has("multicolor")){ emitter.multicolor = (atts.getInt("multicolor") == 1) ? true : false; }
			if (atts.has("colorBegin")){
				
				if (emitter.nameParticles.compare("smoke_v3") == 0){
					emitter.colorBeginGlobal = atts.getQuatWithoutNorm("colorBegin");
				} else {
					emitter.colorBeginGlobal = atts.getQuatWithoutNorm("colorBegin");
				}


			}
			if (atts.has("colorEnd")){
				XMVECTOR colorBase = atts.getQuatWithoutNorm("colorEnd");
				colorBase = colorBase * 255;
				emitter.colorEndingGlobal = colorBase;
			}
			
			if (atts.has("emitting")){ emitter.emitting = (atts.getInt("emitting") == 1) ? true : false; }
			if (atts.has("physx")){ emitter.physXEnable = (atts.getInt("physx") == 1) ? true : false; physXEnable = emitter.physXEnable; }
		
			Entity* e = Handle(this).getOwner();
			CTransform * transform = e->get<CTransform>();
			emitter.position = transform->getPosition();

			/*if (emitter.emitting)
				setup();*/

			emitter.scale = atts.getFloat("scale", emitter.scale);
			emitter.color_rate = atts.getFloat("color_rate", emitter.color_rate);
            emitter.blendConfig = (BlendConfig)atts.getInt("blendConfig", emitter.blendConfig);
            emitter.zSort = atts.getBool("zSort", emitter.zSort);

		}

		//----------------------------- Temp-----------------------//
			
#endif
	}

	void CParticleSystem::setParticlesEmitter(ParticlesEmitter emitter_){
		emitter = emitter_; 
		Entity* e = Handle(this).getOwner();
		CTransform * transform = e->get<CTransform>();
		transform->setPosition(emitter.position);
	}

	void CParticleSystem::setup(){
		vectors = new vectors_t;

		Entity* e;
		e = Handle(this).getOwner();
		
		CTransform * t = e->get<CTransform>();
		CTint *tint = e->get<CTint>();

		UINT32 idx = 0;

		if (emitter.maxParticle > 0){

			vectors->indices.clear();

			vectors->particlesDatas.clear();
			vectors->particlesDatas.resize(emitter.maxParticle);

			vectors->lifeTimes.clear();

			if (physXEnable){
				vectors->physx_indices.clear();
				vectors->physx_positions.clear();
				vectors->physx_velocitys.clear();

				vectors->physx_indices.resize(emitter.maxParticle);
				vectors->physx_positions.resize(emitter.maxParticle);
				vectors->physx_velocitys.resize(emitter.maxParticle);

			}

            int i =0;
			for (auto& p : vectors->particlesDatas) {
                p.index = i++;

				XMVECTOR random = (!emitter.rangeYEnable) ?
                        t->getPosition() + emitter.localposition +
                        utils::rand_vectorXZ(emitter.rangeDistance, -emitter.rangeDistance)
                        :
                        t->getPosition() + emitter.localposition + 
                        utils::rand_vector3(emitter.rangeDistance, -emitter.rangeDistance);

                p.scale = emitter.scale;
                XMStoreFloat3(&p.pos, random);

				p.frame = float(utils::rand_uniform(emitter.numberOfFrames, 0));
				p.setAngle(utils::rand_uniform(M_2_PIf, 0.f));

				//lifetime
				LifeTimeData lifetime;
				lifetime.index = idx;

				if (emitter.randomReviveLapse){
					if (idx < vectors->particlesDatas.size()/2)
						lifetime.setLifeTime(utils::rand_uniform(emitter.lifeTimeRate, emitter.lifeTimeRate / 2));
					else{
						lifetime.setLifeTime(emitter.lifeTimeRate);
						lifetime.setTimer(utils::rand_uniform(emitter.lifeTimeRate, emitter.lifeTimeRate / 2));
					}
				}
				else
					lifetime.setLifeTime(emitter.lifeTimeRate);

				vectors->lifeTimes.push_back(lifetime);

				//indices
				vectors->indices.push_back(idx);

				if (physXEnable){

					vectors->physx_indices[idx] = (PxU32)idx;
					vectors->physx_positions[idx] = toPxVec3(random);
					PxVec3 physx_velocity(toPxVec3(setVelocityParticle() * emitter.speed));

					vectors->physx_velocitys[idx] = physx_velocity;
				}

				idx++;
			}

			if (physXEnable){
				vectors->physx_velocitys;

				physx_particle_data.numParticles = (PxU32)emitter.maxParticle;
				physx_particle_data.indexBuffer =
					PxStrideIterator<const PxU32>(&vectors->physx_indices[0]);
				physx_particle_data.positionBuffer =
					PxStrideIterator<const PxVec3>(&vectors->physx_positions[0]);
				physx_particle_data.velocityBuffer =
					PxStrideIterator<const PxVec3>(&vectors->physx_velocitys[0]);


				bool success = false;

				clear();
				physx_particle_system = Physics.gPhysicsSDK->
											createParticleSystem(emitter.maxParticle);

				physx_particle_system->setParticleReadDataFlag(PxParticleReadDataFlag::eVELOCITY_BUFFER, true);

					//#if PX_SUPPORT_GPU_PHYSX
//					physx_particle_system->setParticleBaseFlag(PxParticleBaseFlag::eGPU, mRunOnGpu);
					//#endif
				Physics.gScene->addActor(*physx_particle_system);
				success = physx_particle_system->createParticles(physx_particle_data);

				if (!success)
				{
					assert("ERROR - creating particles with physx");
				}
				else{
					
					physx_particle_system->setParticleBaseFlag(PxParticleBaseFlag::eCOLLISION_WITH_DYNAMIC_ACTORS, false);
					physx_particle_system->setParticleBaseFlag(PxParticleBaseFlag::eCOLLISION_TWOWAY, true);

					physx_particle_system->setGridSize(3.0f);
					physx_particle_system->setMaxMotionDistance(0.43f);
					physx_particle_system->setRestOffset(0.0143f);
					physx_particle_system->setContactOffset(0.0143f * 2);
					physx_particle_system->setDamping(0.0f);
					physx_particle_system->setRestitution(0.2f);
					physx_particle_system->setDynamicFriction(0.05f);
				}

					//#if PX_SUPPORT_GPU_PHYSX
//					//check gpu flags after adding to scene, cpu fallback might have been used.
//					mRunOnGpu = mRunOnGpu && (physx_particle_system->getParticleBaseFlags() & PxParticleBaseFlag::eGPU);
//					if (mRunOnGpu){
//						dbg("Running on GPU");
//					}
					//#endif
			} else{
                setShapesVelocity();
            }

			setColors();

			instanceData = new Mesh;
            nAlive = emitter.maxParticle;
			bool success = instanceData->create(
				unsigned(emitter.maxParticle), vectors->particlesDatas.data(), 0, nullptr,
				Mesh::POINTS, getVertexDecl<VertexParticleUData>(), 1 /*instance stream*/,
				utils::zero_v, utils::zero_v, true);

			assert(success || !"ERROR - creating particles");
		}
	}

void CParticleSystem::updateParticle(
    ParticleData& p, float elapsed,
    PersonalData& pData, const LifeTimeData& lifeTime)
{
    if (!emitter.randomParticles) {p.frame += elapsed * emitter.timing;}

    if (emitter.rotationRateParticle > 0) {
        p.setAngle(p.getAngle() + elapsed * utils::deg2rad(emitter.rotationRateParticle));
    }
	if (emitter.haveColor && emitter.haveFinalColor) {
        p.colorWeight = pData.getColorWeight(lifeTime, emitter.color_rate, elapsed);
    } else {
        p.colorWeight = 0;
    }

    // No particle varies in scale
    //p.scale = emitter.scale * pData.getScale();
}

XMMATRIX cam_vp = XMMatrixIdentity();

void CParticleSystem::partitionAlive()
{
    auto lifeTimes = vectors->lifeTimes;
    auto it = std::partition(vectors->particlesDatas.begin(),
        vectors->particlesDatas.end(),
        [lifeTimes](const ParticleData& p) {
            return !lifeTimes[p.index].isDead();
        });
    int newNAlive = unsigned(it - vectors->particlesDatas.begin() );
    assert(newNAlive <= emitter.maxParticle);
    nAlive = newNAlive;
}

void CParticleSystem::sortParticlesZ()
{
    auto size = vectors->particlesDatas.size();
    if (size == 0) {return;}
    CCamera* cam = App::get().getCamera().getSon<CCamera>();
    cam_vp = cam->getViewProjection();
    static const auto calculateViewZ = [](const ParticleData& p) {
        XMVECTOR pos = XMLoadFloat3(&p.pos);
        auto z = XMVectorGetZ( XMVector3Transform(pos, cam_vp) );
        //Multiply by a factor and rounding to group particles that are too close
        //With this, and stable_sort, we eliminate z-fighting
        return int(-z*5);
    };

    std::vector<lazy<ParticleData, int, decltype(calculateViewZ)>> lazyKeys;
    lazyKeys.resize(size);
    const auto& lifeTimes = vectors->lifeTimes;
    const auto sortFN = [&lazyKeys, lifeTimes]
        (const ParticleData& a, const ParticleData& b) -> bool {
        auto aL = lifeTimes[a.index].isDead();
        auto bL = lifeTimes[b.index].isDead();
        if (aL && !bL) {
            return true;
        } else if (!aL && bL) {
            return false;
        } else {
            return lazyKeys[a.index].get(a, calculateViewZ)
                < lazyKeys[b.index].get(b, calculateViewZ);
        }
    };

    std::stable_sort(vectors->particlesDatas.begin(), vectors->particlesDatas.end(), sortFN);
}


	void CParticleSystem::physx_particles_update(float elapsed){

		if (vectors != nullptr && vectors->particlesDatas.size() > 0){

			PxParticleReadData	*readdata = physx_particle_system->lockParticleReadData();

			Entity* e = Handle(this).getOwner();
			CTransform * t = e->get<CTransform>();
            
            if (readdata) {
				PxStrideIterator<const PxParticleFlags> flagsIt(readdata->flagsBuffer);
				PxStrideIterator<const PxVec3> positionIt(readdata->positionBuffer);

                nAlive = 0;
				for (unsigned idx = 0; idx < readdata->validParticleRange; ++idx, ++flagsIt, ++positionIt) {
                    //lifetime
					auto &p = vectors->particlesDatas[idx];
					auto &lifeTime = vectors->lifeTimes[idx];
					auto &pData = vectors->personalDatas[idx];
					auto &indexParticle = vectors->indices[idx];

					if (!lifeTime.countTime(elapsed)){
                        p.dead = 0;
                        nAlive++;
						const PxVec3& position = *positionIt;
						p.pos = toXMFLOAT3(position);
					} else if (!deleteSelf && emitter.reviveEnable){
                        nAlive++;
                        p.dead = 0;
						vectors->physx_indices_updates.push_back((PxU32)idx);
						vectors->indicesUpdate.push_back(idx);

						XMVECTOR random = (!emitter.rangeYEnable) ?
                                t->getPosition() + emitter.localposition +
                                utils::rand_vectorXZ(emitter.rangeDistance, -emitter.rangeDistance)*.5f
                                :
                                t->getPosition() + emitter.localposition + 
                                utils::rand_vector3(emitter.rangeDistance, -emitter.rangeDistance)*.5f;
						
						vectors->physx_positions_updates.push_back(toPxVec3(random));

						XMVECTOR speed = setVelocityParticle() * emitter.speed;
						vectors->physx_velocitys_updates.push_back(toPxVec3(speed));

						if (getRandomRevive()) {
							lifeTime.setLifeTime(utils::rand_uniform(emitter.lifeTimeRate, emitter.lifeTimeRate *.5f));
                        } else {
							lifeTime.setLifeTime(emitter.lifeTimeRate);
                        }
					} else {
                        p.dead = ~0;
                    }

                    updateParticle(p, elapsed, pData, lifeTime);
				}
			}
			if (nAlive > 0) {

				if (emitter.zSort) {
					sortParticlesZ();
					instanceData->updateFromCPU(
						vectors->particlesDatas.data(), nAlive * sizeof(ParticleData));
				}
				else {
					//partitionAlive();
					instanceData->updateFromCPU(
						vectors->particlesDatas.data(), emitter.maxParticle * sizeof(ParticleData));
				}
			}
			else if (deleteSelf) {
				e->postMsg(MsgDeleteSelf());
			}
			readdata->unlock();
		}



		if (emitter.reviveEnable){

			if (vectors->physx_indices_updates.size() > 0){

				PxStrideIterator<const PxU32> indexBuff(&vectors->physx_indices_updates[0]);
				PxStrideIterator<const PxVec3> newPositionBuff(&vectors->physx_positions_updates[0]);
				PxStrideIterator<const PxVec3> newVelocityBuff(&vectors->physx_velocitys_updates[0]);

				physx_particle_system->setPositions((PxU32)vectors->physx_indices_updates.size(), indexBuff, newPositionBuff);
				physx_particle_system->setVelocities((PxU32)vectors->physx_indices_updates.size(), indexBuff, newVelocityBuff);
			}

			vectors->physx_indices_updates.clear();
			vectors->physx_positions_updates.clear();
			vectors->physx_velocitys_updates.clear();
			vectors->indicesUpdate.clear();
		}
	}

	void CParticleSystem::update(float elapsed){
        

		

		fsm.update(elapsed);
		valid();
		
		if (isEmitting() && (vectors == nullptr || vectors->isEmpty())) { return; }


		Entity* e;
		e = Handle(this).getOwner();
		CTransform * t = e->get<CTransform>();

		if (physXEnable) {
			physx_particles_update(elapsed);
        } else {
            if (vectors->particlesDatas.size() > 0){

                nAlive = 0;
                for (int idx = 0; idx < vectors->particlesDatas.size(); idx++) {
					//lifetime
					auto &p = vectors->particlesDatas[idx];
					auto &lifeTime = vectors->lifeTimes[idx];
					auto &velocity = vectors->velocitys[idx];
					auto &pData = vectors->personalDatas[idx];
					auto &indexParticle = vectors->indices[idx];

					if (!lifeTime.countTime(elapsed)){
                        nAlive++;
                        p.dead = 0;

						XMVECTOR current = t->getPosition();

						if (emitter.type == ParticlesType::RANDOM)
							polutionSimulator(p,velocity, current, elapsed);
						else if (emitter.type == ParticlesType::BUTTERFLY)
							butterflySimulator(p,velocity, current, elapsed);
                        else {
							current = velocity.currentVelocity * emitter.speed * elapsed;

							p.pos.x += meter2Cm(XMVectorGetX(current));
							p.pos.y += meter2Cm(XMVectorGetY(current));
							p.pos.z += meter2Cm(XMVectorGetZ(current));

						}

						velocity.currentVelocity -= yAxis_v *  meter2Cm(emitter.gravity);

					} else if (!deleteSelf && emitter.reviveEnable){
                        nAlive++;
                        p.dead = 0;
                        lifeTime.reset();
					    vectors->indicesUpdate.push_back(indexParticle);

					    XMVECTOR random = (!emitter.rangeYEnable) ?
                                t->getPosition() + emitter.localposition +
                                utils::rand_vectorXZ(emitter.rangeDistance, -emitter.rangeDistance)*.5f
                                :
                                t->getPosition() + emitter.localposition + 
                                utils::rand_vector3(emitter.rangeDistance, -emitter.rangeDistance)*.5f;
					    XMStoreFloat3(&p.pos, random);
					    velocity.currentVelocity = velocity.startVelocity;

					    lifeTime.setLifeTime(utils::rand_uniform(emitter.lifeTimeRate, emitter.lifeTimeRate / 2));
					} else {
                        p.dead = ~0;
                    }
                    
                    updateParticle(p, elapsed, pData, lifeTime);
				}
                
                
                if (nAlive > 0) {

                    if (emitter.zSort) {
                        sortParticlesZ();
			            instanceData->updateFromCPU(
                            vectors->particlesDatas.data(), nAlive * sizeof(ParticleData));
                    } else {
                        //partitionAlive();
			            instanceData->updateFromCPU(
                            vectors->particlesDatas.data(), emitter.maxParticle * sizeof(ParticleData));
                    }
                } else if(deleteSelf) {
                    e->postMsg(MsgDeleteSelf());
                }
				vectors->indicesUpdate.clear();
			}
		}
	}

	void CParticleSystem::polutionSimulator(ParticleData& p, VelocityData &velocity, XMVECTOR currentPosition, float elapsed){
	
		XMVECTOR rand = rand_vector3(1, -1);
		rand = XMVector3Cross(velocity.currentVelocity, rand);
		XMVECTOR drunkVector1 = XMVector3Normalize(rand);
		rand = XMVector3Cross(velocity.currentVelocity, rand);
		XMVECTOR drunkVector2 = XMVector3Normalize(rand);

		currentPosition = velocity.currentVelocity + (drunkVector1 + drunkVector2) * emitter.speed * elapsed;

		p.pos.x += meter2Cm(XMVectorGetX(currentPosition));
		p.pos.y += meter2Cm(XMVectorGetY(currentPosition));
		p.pos.z += meter2Cm(XMVectorGetZ(currentPosition));

		velocity.currentVelocity = currentPosition;
	}

	void CParticleSystem::butterflySimulator(ParticleData& p, VelocityData &velocity, XMVECTOR currentPosition, float elapsed){

		accumulated += elapsed;

		XMVectorSetX(velocity.currentVelocity, (sin(deg2rad(accumulated)) * emitter.speed + emitter.rangeDistance));
		XMVectorSetZ(velocity.currentVelocity, (cos(deg2rad(accumulated)) * emitter.speed + emitter.rangeDistance));

		currentPosition = velocity.currentVelocity;

		p.pos.x = meter2Cm(XMVectorGetX(currentPosition));
		p.pos.z = meter2Cm(XMVectorGetZ(currentPosition));

		//velocity.currentVelocity = currentPosition;

	}

	void CParticleSystem::render() {


        if(vectors->isEmpty())
			return;
		

		if (isEmitting() && vectors->particlesDatas.size() > 0 && nAlive > 0){

			
			//dbg("%s     %s\n", emitter.owner.c_str(), emitter.nameParticles.c_str());

			Entity* me = Handle(this).getOwner();
			emitter.tech->activate();
			Texture::getManager().getByName(emitter.nameFile.c_str())->activate(0);

			activateBlendConfig(emitter.blendConfig);

			render::setTextureData(emitter.numberOfFrames, emitter.numberOfFramePerRow);
			render::updateTextureData();
			instanceMesh = &render::mesh_textured_quad_xy_centered;
			instanceMesh->renderInstanced(*instanceData, nAlive);
		}
	}

	void CParticleSystem::resetposition(){

		setMotionSimulator(false);

		Entity* me = Handle(this).getOwner();
		CTransform* transform = me->get<CTransform>();
		transform->setPosition(XMVectorSet(0, 0, 0, 0));
	}

	void CParticleSystem::setEnablePhysX(bool enable_) {

		physXEnable = enable_;
		if (getFinish())
			setup();
	};

	void CParticleSystem::setMaxParticleTWEAK(int mParticles){

		if (mParticles >= 0){

			emitter.maxParticle = mParticles;
			vectors->particlesDatas.clear();
			vectors->indices.clear();
			vectors->velocitys.clear();
			if (getFinish()) 
				setup();
		}
	}

	void CParticleSystem::setColorRate(float color_rate){
		emitter.color_rate = color_rate;
	}


	void CParticleSystem::setRotationTWEAK(XMVECTOR rotation_){

		Entity* e = Handle(this).getOwner();
		CTransform * transform = e->get<CTransform>();
		transform->setRotation(rotation_);
		emitter.rotation = rotation_;
		setShapesVelocity();
	}

	void CParticleSystem::setRevive(bool revive_){

		emitter.reviveEnable = revive_;
		if (getFinish()) setup();
	}
	void CParticleSystem::setRandomRevive(bool revive_random_){

		emitter.randomReviveLapse = revive_random_;
		if (getFinish()) setup();
	}

	XMVECTOR CParticleSystem::getRotationTWEAK() const{

		Entity* e = Handle(this).getOwner();
		CTransform * transform = e->get<CTransform>();

		return transform->getRotation();
	}

	void CParticleSystem::setShapesVelocity(){

		VelocityData velocity_;

		if (vectors->velocitys.size() != emitter.maxParticle){
			vectors->velocitys.clear();
			vectors->velocitys.resize(emitter.maxParticle);
		}
		//int idx = 0;
		for (int i = 0; i < emitter.maxParticle; i++) {

			velocity_.startVelocity = setVelocityParticle();
			velocity_.currentVelocity = velocity_.startVelocity;
			vectors->velocitys[i] = velocity_;
			//idx++;
		}
	}

	XMVECTOR CParticleSystem::setVelocityParticle(){

		Entity* e = Handle(this).getOwner();
		CTransform * transform = e->get<CTransform>();

		XMVECTOR startVelocity = XMVectorSet(0, 0, 0, 0);

		float angleX = 0.f;
		float angleZ = 0.f;
		float rAnglex = 0.0f;
		float rAngleZ = 0.0f;

		XMVECTOR xRotQ = XMVectorZero();
		XMVECTOR zRotQ = XMVectorZero();
		XMVECTOR rotQ = XMVectorZero();


		switch (emitter.type)
		{
		case ParticlesType::CONE:

			angleX = emitter.angle;
			angleZ = emitter.angle;

			rAnglex = deg2rad(utils::rand_uniform(angleX, -angleX));
			rAngleZ = deg2rad(utils::rand_uniform(angleZ, -angleZ));

			xRotQ = XMQuaternionRotationAxis(transform->getFront(), rAnglex);
			zRotQ = XMQuaternionRotationAxis(transform->getLeft(), rAngleZ);
			rotQ = XMQuaternionMultiply(xRotQ, zRotQ);
			startVelocity = XMVector3Rotate(transform->getUp(), rotQ);

			break;
		case ParticlesType::BOX:
			startVelocity = transform->getUp() * emitter.speed;


			break;
		case ParticlesType::SPHERE:
			angleX = 180;
			angleZ = 180;

			rAnglex = deg2rad(utils::rand_uniform(angleX, -angleX));
			rAngleZ = deg2rad(utils::rand_uniform(angleZ, -angleZ));

			xRotQ = XMQuaternionRotationAxis(transform->getFront(), rAnglex);
			zRotQ = XMQuaternionRotationAxis(transform->getLeft(), rAngleZ);

			rotQ = XMQuaternionMultiply(xRotQ, zRotQ);

			startVelocity = XMVector3Rotate(transform->getUp(), rotQ);


			break;
		case ParticlesType::SEMISPHERE:

			angleX = 90;
			angleZ = 90;

			rAnglex = deg2rad(utils::rand_uniform(angleX, -angleX));
			rAngleZ = deg2rad(utils::rand_uniform(angleZ, -angleZ));

			xRotQ = XMQuaternionRotationAxis(transform->getFront(), rAnglex);
			zRotQ = XMQuaternionRotationAxis(transform->getLeft(), rAngleZ);

			rotQ = XMQuaternionMultiply(xRotQ, zRotQ);

			startVelocity = XMVector3Rotate(transform->getUp(), rotQ);


			break;
		case ParticlesType::RANDOM:

			angleX = 90;
			angleZ = 90;

			rAnglex = deg2rad(utils::rand_uniform(angleX, -angleX));
			rAngleZ = deg2rad(utils::rand_uniform(angleZ, -angleZ));

			xRotQ = XMQuaternionRotationAxis(transform->getFront(), rAnglex);
			zRotQ = XMQuaternionRotationAxis(transform->getLeft(), rAngleZ);

			rotQ = XMQuaternionMultiply(xRotQ, zRotQ);

			startVelocity = XMVector3Rotate(transform->getUp(), rotQ);

			break;

		case ParticlesType::BUTTERFLY:

			angleX = 90;
			angleZ = 90;

			rAnglex = deg2rad(utils::rand_uniform(angleX, -angleX));
			rAngleZ = deg2rad(utils::rand_uniform(angleZ, -angleZ));

			xRotQ = XMQuaternionRotationAxis(transform->getFront(), rAnglex);
			zRotQ = XMQuaternionRotationAxis(transform->getLeft(), rAngleZ);

			rotQ = XMQuaternionMultiply(xRotQ, zRotQ);

			startVelocity = XMVector3Rotate(transform->getUp(), rotQ);


			break;

		case ParticlesType::DISK:

			angleX = 90;
			angleZ = 90;

			rAnglex = deg2rad(utils::rand_uniform(angleX, -angleX));
			rAngleZ = deg2rad(utils::rand_uniform(angleZ, -angleZ));

			xRotQ = XMQuaternionRotationAxis(transform->getFront(), rAnglex);
			zRotQ = XMQuaternionRotationAxis(transform->getLeft(), rAngleZ);

			rotQ = XMQuaternionMultiply(xRotQ, zRotQ);

			startVelocity = XMVector3Rotate(transform->getUp(), rotQ);
			startVelocity = XMVectorSetY(startVelocity, 0.0);

			break;
		default:
			break;
		}

		return startVelocity;

	}

	void CParticleSystem::setmultipleFrames(bool enable_){

		if (!enable_){
			setNumberOfFrames(1);
			setNumberOfFramePerRow(1);
			emitter.animated = enable_;
			emitter.randomParticles = enable_;
		}
		else{
			setRandomParticles(enable_);
		}

		emitter.multipleFrames = enable_;
	}

	void CParticleSystem::setAnimated(bool animated_){

		if (animated_){

			setNumberOfFrames(16);
			setNumberOfFramePerRow(4);

		}

		emitter.animated = animated_;
		emitter.randomParticles = false;
	}



	void CParticleSystem::setRandomParticles(bool randomParticles_){

		if (randomParticles_){

			setNumberOfFrames(16);
			setNumberOfFramePerRow(4);
		}
		emitter.randomParticles = randomParticles_;
		emitter.animated = false;
	}

	bool CParticleSystem::getAnimated() const{
		return emitter.animated;
	}

	bool CParticleSystem::getRandomParticles() const{
		return emitter.randomParticles;
	}

	CParticleSystem::ParticlesType CParticleSystem::ParticlesEmitter::getTypeByInt(int type_){

		switch (type_)
		{
		case 1: return ParticlesType::CONE; break;
		case 2: return ParticlesType::BOX; break;
		case 3: return ParticlesType::DISK; break;
		case 4: return ParticlesType::EDGE; break;
		case 5: return ParticlesType::SPHERE; break;
		case 6: return ParticlesType::SEMISPHERE; break;
		case 7: return ParticlesType::RANDOM; break;
		case 8: return ParticlesType::BUTTERFLY; break;
		default: return ParticlesType::DEFAULT; break;
		}
	}


	void CParticleSystem::setParticlesEmitterByDefault(){

		setMaxParticle(50);
		sefLocalPosition(zero_v);
		setRangeDistance(1.f);
		setAngle(deg2rad(30));
		setEmitting(true);

		reload();

		setAnimated(false);
		setGravity(0.0);
		setTiming(10);
		setEntityMesh(false);
		setTechnique(Technique::getManager().getByName("particles"));
		setEntityRotationEnable(false);
		setNameFile("ghost");
		setNameParticles("Test");
		setLifeTimeRate(2.5);
		setHaveAColor(false);
		setHaveAFinalColor(false);
		setNumberOfFrames(1);
		setNumberOfFramePerRow(1);
		setColorRate(1.0);
		setSpeed(3.0f);
		setCone(true);
		setFinish(true);
		
}

	//out of system
	void CParticleSystem::enable(bool enable_){

		if (enable_)
			setMaxParticleTWEAK(emitter.backupMaxParticle);
		else
			setMaxParticleTWEAK(0);

		emitter.active = enable_;
	}
	void CParticleSystem::activeParticlesEmitterByTime(float time, float elapsed)
    {
        if (timer.count(elapsed) >= time){
			enable(!emitter.active);
			timer.reset();
		}
	}

	void CParticleSystem::activeParticlesEmitterHeight(float time, float elapsed)
    {
        if (timer.count(elapsed) >= time){
			emitter.lifeTimeRate++;
			timer.reset();
		}
	}

	void CParticleSystem::setColors(){
        if (vectors->personalDatas.size()!= emitter.maxParticle) {
            vectors->personalDatas.resize(emitter.maxParticle);
        }

		for (int i = 0; i < emitter.maxParticle; i++){
			
			auto& pData = vectors->personalDatas[i];
			auto &psData = vectors->particlesDatas[i];

			psData.setColorA(
				emitter.multicolor ?
				ColorV(utils::rand_vector3(0.50f, 0.0f, 1.f)) : emitter.colorBeginGlobal);
			psData.setColorB(emitter.colorEndingGlobal);
            psData.colorWeight = 0;
		}

	}

	void CParticleSystem::setLoadColors(const ColorV& color_){
		setCurrentBeginColor(emitter.colorBeginGlobal);
		setCurrentEndColor(emitter.colorEndingGlobal);
	}

	void CParticleSystem::setCurrentBeginColor(const ColorV& color_){

		if (emitter.haveColor){
			emitter.colorBeginGlobal = color_;
			setColors();
		}
	}
	void CParticleSystem::setCurrentEndColor(const ColorV& color_){

		if (emitter.haveFinalColor){
			emitter.colorEndingGlobal = color_;
			setColors();
		}
	}

	void CParticleSystem::clear(){

		if (physx_particle_system != nullptr){

			Physics.gScene->removeActor(*physx_particle_system);
			physx_particle_system->release();
			physx_particle_system = NULL;
		}
	}

	static PxFilterFlags particles_syster_filter(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		pairFlags = PxPairFlag::eCCD_LINEAR;

		return PxFilterFlags();
	}

	void CParticleSystem::valid(){

		Entity* e;
		e = Handle(this).getOwner();
		CTransform * t = e->get<CTransform>();
#if defined(_PARTICLES)

		if (emitter.emitting && !emitter.success){
			setup();
			emitter.success = true;
		}
#else
		if (isEmitting() && !emitter.success && t->getPosition() != zero_v){
			setup();
			emitter.success = true;
		}
#endif

	}
}