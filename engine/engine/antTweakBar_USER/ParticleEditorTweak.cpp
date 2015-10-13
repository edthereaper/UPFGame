#include "mcv_platform.h"
#include "antTW.h"
#include "app.h"

#include <sstream>
#include "utils/data_provider.h"

#include "gameElements/props.h"
using namespace gameElements;

#include "render/mesh/component.h"
#include "render/mesh/mesh.h"
#include "render/components.h"
using namespace DirectX;

#include "animation\components.h"
using namespace animation;

#include "PhysX_USER/pxcomponents.h"
#include "PhysX_USER/CollisionMesh.h"
#include "render/render_utils.h"

using namespace component;

#include "Particles/ParticlesManager.h"

using namespace particles;

using namespace render;

#define TIME_LAPSED_CREATE_PARTICLES 1.0;

namespace antTw_user {

#define MAX_LIGHTS 100

#ifdef _PARTICLES

	Handle emitter_h;
	Entity* particle;
	Entity* emitter;

	TwBar *barParticle1 = nullptr;
	TwBar *barPhysx = nullptr;
	TwBar *fileParticlesManagerBar = nullptr;
	TwBar *particlesHierarchy = nullptr;

	int x = 15;
	int y = 15;
	int heightBar = 100;
	int currentParticles;

	std::string file_name = "";
	std::string file_particules_name = "";
	std::ofstream myfileParticle;
	std::vector<std::string> files;
	std::vector<std::string> emitterFiles;

	Handle element_h_cam_particle;
	Entity* element_cam_particle;

	/*struct ParticlesBarData{
	TwBar* bar;
	std::string name;
	int index;
	bool open;
	};
	std::vector<ParticlesBarData> data;*/


	void setHandleParticleCam(CCamera* cam){
		Handle h(cam);
		element_h_cam_particle = h;
		element_cam_particle = element_h_cam_particle.getOwner();
	}


	static void TW_CALL read_enable_particles(void *value, void *clientData)
	{
		*static_cast<int *>(value) =
			static_cast<CEmitter *>(clientData)->getCurrentEditIndex();
	}

	static void TW_CALL update_enable_particles(const void *value, void *clientData)
	{

		int index = *static_cast<const int *>(value);
		CEmitter * emitt = static_cast<CEmitter *>(clientData);
		particle = emitt->get(index);

		CParticleSystem *newParticles = particle->get<CParticleSystem>();
		file_particules_name = newParticles->getNameParticles();
		static_cast<CEmitter *>(clientData)->setCurrentEditIndex(index);

		if (barParticle1 != nullptr){
			TwDeleteBar(barParticle1);
			TwDeleteBar(barPhysx);
			barParticle1 = nullptr;
			barPhysx = nullptr;
		}
		AntTWManager::createParticleEditorTweak();
	}

	static void TW_CALL read_emitter_position(void *value, void *clientData)
	{
		CEmitter *emitter = static_cast<CEmitter *>(clientData);
		if (emitter->getEmitterData().valid)
			*static_cast< XMVECTOR *>(value) = emitter->getEmittersPosition();
	}

	static void TW_CALL update_emitter_position(const void *value, void *clientData)
	{
		CEmitter *emitter = static_cast<CEmitter *>(clientData);
		if (emitter->getValid())
			emitter->setEmittersPosition(*static_cast<const XMVECTOR *>(value));
	}


	//-----------------------------------------------------------------------------------------

	// MAX PARTICLE
	static void TW_CALL read_max_particles(void *value, void *clientData)
	{
		*static_cast< int *>(value) = static_cast<const CParticleSystem *>(clientData)->getMaxParticle();
	}

	static void TW_CALL update_max_particles(const void *value, void *clientData)
	{

		static_cast<CParticleSystem *>(clientData)->setMaxParticleTWEAK(*static_cast<const int *>(value));
	}

	//LIFETIME
	static void TW_CALL read_lifetime_particles(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getLifeTimeRate();
	}

	static void TW_CALL update_lifetime_particles(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setLifeTimeRate(*static_cast<const float *>(value));
	}

	static void TW_CALL read_lifetime_revive(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->canRevive();
	}

	static void TW_CALL update_lifetime_revive(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setRevive(*static_cast<const bool *>(value));
	}

	static void TW_CALL read_lifetime_random(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getRandomRevive();
	}

	static void TW_CALL update_lifetime_random(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setRandomRevive(*static_cast<const bool *>(value));
	}

	//RANGE DISTANCE
	static void TW_CALL read_range_distance(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getRangeDistance();
	}

	static void TW_CALL update_range_distance(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setRangeDistance(*static_cast<const float *>(value));
	}

	//Range Y Enable

	static void TW_CALL read_range_y_enable(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getRangeYEnable();
	}

	static void TW_CALL update_range_y_enable(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setRangeYEnable(*static_cast<const bool *>(value));
	}

	//Type
	static void TW_CALL read_cone(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getCone();
	}

	static void TW_CALL update_cone(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setCone(*static_cast<const bool *>(value));
	}

	static void TW_CALL read_box(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getBox();
	}

	static void TW_CALL update_box(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setBox(*static_cast<const bool *>(value));
	}

	static void TW_CALL read_sphere(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getSphere();
	}

	static void TW_CALL update_sphere(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setSphere(*static_cast<const bool *>(value));
	}

	static void TW_CALL read_semisphere(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getSemisphere();
	}

	static void TW_CALL update_semisphere(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setSemisphere(*static_cast<const bool *>(value));
	}


	static void TW_CALL read_random(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getRandom();
	}

	static void TW_CALL update_random(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setRandom(*static_cast<const bool *>(value));
	}

	static void TW_CALL read_disk(void *value, void *clientData)
	{
		*static_cast<bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getDisk();
	}
	static void TW_CALL update_disk(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setDisk(*static_cast<const bool *>(value));
	}


	static void TW_CALL update_butterfly(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setButterfly(*static_cast<const bool *>(value));
	}

	static void TW_CALL read_butterfly(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getButterfly();
	}

	// gravity
	static void TW_CALL read_gravity(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getGravity();
	}

	static void TW_CALL update_gravity(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setGravity(*static_cast<const float *>(value));
	}

	// timing
	static void TW_CALL read_timing(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getTiming();
	}

	static void TW_CALL update_timing(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setTiming(*static_cast<const float *>(value));
	}

	// number of framer
	static void TW_CALL read_number_of_frames(void *value, void *clientData)
	{
		*static_cast< int *>(value) = static_cast<const CParticleSystem *>(clientData)->getNumberOfFrames();
	}

	static void TW_CALL update_number_of_frames(const void *value, void *clientData)
	{
		int n = *static_cast<const int *>(value);
		static_cast<CParticleSystem *>(clientData)->setNumberOfFrames(n);
	}

	// number of Frames per Row
	static void TW_CALL read_number_of_frames_per_row(void *value, void *clientData)
	{
		*static_cast< int *>(value) = static_cast<const CParticleSystem *>(clientData)->getNumberOfFramePerRow();
	}

	static void TW_CALL update_number_of_frames_per_row(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setNumberOfFramePerRow(*static_cast<const int *>(value));
	}

	//name particle entity 
	static void TW_CALL read_particle_system_name(void *value, void *clientData)
	{
		std::string name = static_cast<const CParticleSystem *>(clientData)->getNameParticles();
		*static_cast<std::string *>(value) = name;
	}

	static void TW_CALL update_particle_system_name(const void *value, void *clientData)
	{
		const std::string *destPtr = static_cast<const std::string *>(value);
		file_particules_name = *destPtr;
		static_cast<CParticleSystem *>(clientData)->setNameParticles(file_particules_name.c_str());

	}

	// is animated
	static void TW_CALL read_is_animated(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getAnimated();
	}

	static void TW_CALL update_is_animated(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setAnimated(*static_cast<const bool *>(value));
	}

	// is random particles system
	static void TW_CALL read_is_random_particles(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getRandomParticles();
	}

	static void TW_CALL update_is_random_particles(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setRandomParticles(*static_cast<const bool *>(value));
	}

	// is random particles system
	static void TW_CALL read_multiple_frames(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getmultipleFrames();
	}

	static void TW_CALL update_muliple_frames(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setmultipleFrames(*static_cast<const bool *>(value));
	}

	// local rotation
	static void TW_CALL read_rotation(void *value, void *clientData)
	{
		*static_cast< XMVECTOR *>(value) = static_cast<const CParticleSystem *>(clientData)->getRotationTWEAK();
	}

	static void TW_CALL update_rotation(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setRotationTWEAK(*static_cast<const XMVECTOR *>(value));
	}


	//rotation Z
	static void TW_CALL read_speed(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getSpeed();
	}

	static void TW_CALL update_speed(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setSpeed(*static_cast<const float *>(value));
	}

	//motion acceleration
	static void TW_CALL read_acceleration_motion(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getMotionAcceleration();
	}

	static void TW_CALL update_acceleration_motion(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setMotionAcceleration(*static_cast<const float *>(value));
	}

	//angle
	static void TW_CALL read_angle(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getAngle();
	}

	static void TW_CALL update_angle(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setAngleTWEAK(*static_cast<const float *>(value));
	}

	//colorRate
	static void TW_CALL read_color_rate(void *value, void *clientData)
	{
		float data = static_cast<const CParticleSystem *>(clientData)->getColorRate();
		*static_cast<float *>(value) = data;
	}

	static void TW_CALL update_color_rate(const void *value, void *clientData)
	{
		float data = *static_cast<const float *>(value);
		static_cast<CParticleSystem *>(clientData)->setColorRate(data);
	}

	// is using begin color
	static void TW_CALL read_use_color(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getHaveAColor();
	}

	static void TW_CALL update_use_color(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setHaveAColor(*static_cast<const bool *>(value));
	}


	// is using multi color
	static void TW_CALL read_use_multicolor(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getHaveMultiColor();
	}

	static void TW_CALL update_use_mulitcolor(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setHaveMultiColor(*static_cast<const bool *>(value));
	}
	// is using ending color
	static void TW_CALL read_use_end_color(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getHaveAFinalColor();
	}

	static void TW_CALL update_use_end_color(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setHaveAFinalColor(*static_cast<const bool *>(value));
	}


	// is Physx Available
	static void TW_CALL read_physx(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getEnablePhysX();
	}

	static void TW_CALL update_physx(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setEnablePhysX(*static_cast<const bool *>(value));
	}

	//is using color ending

	//color begin
	static void TW_CALL read_intime_begin_color(void *value, void *clientData)
	{

		CParticleSystem * ps = static_cast<CParticleSystem *>(clientData);
		Color color = static_cast<const CParticleSystem *>(clientData)->getCurrentBeginColor();
		Color t;
		t.set(XMVectorSet(color.af(), color.bf(), color.gf(), color.rf()));
		*static_cast<Color *>(value) = t;
	}

	static void TW_CALL update_intime_begin_color(const void *value, void *clientData)
	{
		Color col = *static_cast<const Color *>(value);
		Color t;
		t.set(XMVectorSet(col.af(), col.bf(), col.gf(), col.rf()));
		static_cast<CParticleSystem *>(clientData)->setCurrentBeginColor(t);
	}

	// color ending
	static void TW_CALL read_intime_end_color(void *value, void *clientData)
	{
		CParticleSystem * ps = static_cast<CParticleSystem *>(clientData);
		Color color = static_cast<const CParticleSystem *>(clientData)->getCurrentEndingColor();
		Color t;
		t.set(XMVectorSet(color.af(), color.bf(), color.gf(), color.rf()));
		*static_cast<Color *>(value) = t;
	}

	static void TW_CALL update_intime_end_color(const void *value, void *clientData)
	{
		Color col = *static_cast<const Color *>(value);
		Color t;
		t.set(XMVectorSet(col.af(), col.bf(), col.gf(), col.rf()));
		static_cast<CParticleSystem *>(clientData)->setCurrentEndColor(t);
	}



	//texture
	static void TW_CALL read_texture(void *value, void *clientData)
	{
		std::string txt_tmp = static_cast<const CParticleSystem *>(clientData)->getNameFile();

		if (txt_tmp.compare("") != 0){

			for (int idx = 0; idx < files.size(); idx++){
				std::string tmp = files[idx];
				if (tmp.compare(txt_tmp) == 0){
					*static_cast<int *>(value) = idx;
					break;
				}
			}
		}

	}

	static void TW_CALL update_texture(const void *value, void *clientData)
	{
		int index = *static_cast<const int *>(value);
		std::string fil_name_tmp = files[index];
		file_name = fil_name_tmp;
		static_cast<CParticleSystem *>(clientData)->setNameFile(file_name.c_str());
	}

	//Type
	static void TW_CALL read_active_counter(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getActiveLifeCounter();
	}

	static void TW_CALL update_active_counter(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setActiveLifeCounter(*static_cast<const bool *>(value));
	}

	// is each particle rotate
	static void TW_CALL read_is_active_rotation_particle(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getActiveRotation();
	}

	static void TW_CALL update_is_active_rotation_particle(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setActiveRotation(*static_cast<const bool *>(value));
	}

	// rate each particle rotate
	static void TW_CALL read_rate_rotation_particle(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getRotationRateParticle();
	}

	static void TW_CALL update_rate_rotation_particle(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setRotationRateParticle(*static_cast<const float *>(value));
	}

	// particle global scaling
	static void TW_CALL read_scale(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getGlobalScale();
	}

	static void TW_CALL update_scale(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setGlobalScale(*static_cast<const float *>(value));
	}

	// is Physx Available
	static void TW_CALL read_motion(void *value, void *clientData)
	{
		*static_cast< bool *>(value) = static_cast<const CParticleSystem *>(clientData)->getMotionSimulator();
	}

	static void TW_CALL update_motion(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setMotionSimulator(*static_cast<const bool *>(value));
	}


	//----------------------------------------------------------------------------------------------------------//


	// physx gridsize
	static void TW_CALL read_physx_gridsize(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getPhysxGridSize();
	}

	static void TW_CALL update_physx_gridsize(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setPhysXGridSize(*static_cast<const float *>(value));
	}

	// physx restoffset
	static void TW_CALL read_physx_restoffset(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getPhysxRestOffset();
	}

	static void TW_CALL update_physx_restoffset(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setPhysXRestOffset(*static_cast<const float *>(value));
	}

	// physx max motion
	static void TW_CALL read_physx_maxmotion(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getPhysxMaxMotion();
	}

	static void TW_CALL update_physx_maxmotion(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setPhysXMaxMotion(*static_cast<const float *>(value));
	}

	// physx static
	static void TW_CALL read_physx_static(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getPhysxStatic();
	}

	static void TW_CALL update_physx_static(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setPhysXStatic(*static_cast<const float *>(value));
	}

	// physx dynamic
	static void TW_CALL read_physx_dynamic(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getPhysxDynamic();
	}

	static void TW_CALL update_physx_dynamic(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setPhysXDynamic(*static_cast<const float *>(value));
	}

	// physx restution
	static void TW_CALL read_physx_restitution(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getPhysxRestitution();
	}

	static void TW_CALL update_physx_restitution(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setPhysXRestitution(*static_cast<const float *>(value));
	}

	// physx acceleration
	static void TW_CALL read_physx_acceleration(void *value, void *clientData)
	{
		*static_cast< XMVECTOR *>(value) = static_cast<const CParticleSystem *>(clientData)->getPhysXExternalAcceleration();
	}

	static void TW_CALL update_physx_acceleration(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setPhysXExternalAcceleration(*static_cast<const XMVECTOR *>(value));
	}

	// physx mass
	static void TW_CALL read_physx_mass(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getPhysxParticleMass();
	}

	static void TW_CALL update_physx_mass(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setPhysXParticleMass(*static_cast<const float *>(value));
	}

	// physx damping
	static void TW_CALL read_physx_damping(void *value, void *clientData)
	{
		*static_cast< float *>(value) = static_cast<const CParticleSystem *>(clientData)->getPhysXDamping();
	}

	static void TW_CALL update_physx_damping(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setPhysXDamping(*static_cast<const float *>(value));
	}

	// list all file manager
	void AntTWManager::GetFilesInDirectory(std::vector<std::string> &out, const std::string &directory)
	{

		HANDLE dir;
		WIN32_FIND_DATA file_data;

		if ((dir = FindFirstFile((directory + "/*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
			return; /* No files found */

		do {
			const std::string file_name_from_directory = file_data.cFileName;
			const std::string full_file_name = directory + "/" + file_name_from_directory;
			const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

			if (file_name_from_directory[0] == '.')
				continue;

			if (is_directory)
				continue;

			size_t lastindex = file_name_from_directory.find_last_of(".");
			std::string rawname = file_name_from_directory.substr(0, lastindex);

			out.push_back(rawname);
		} while (FindNextFile(dir, &file_data));

		FindClose(dir);
	}

	bool AntTWManager::validNameTexture(std::string name){

		for (auto& texture : files)
			if (!name.compare(texture))
				return true;

		return false;
	}


	//***********************************************************************************************************//
	//***********************************************************************************************************//
	//*************************************			Callback	 ***********************************************//
	//***********************************************************************************************************//
	//***********************************************************************************************************//

	void TW_CALL Callback_save_particles(void *clientData){
		
		/*CEmitter * emitt = emitter->get<CEmitter>();
		CParticleSystem *part = particle->get<CParticleSystem>();
		CParticleSystem::ParticlesEmitter ps_emit = part->getParticlesEmitter();

		std::string oldKey = ps_emit.nameParticles;
		ps_emit.nameParticles = file_particules_name;
		emitt->updateTag(oldKey, file_particules_name);

		if (particlesHierarchy != nullptr){

			TwRemoveVar(particlesHierarchy, "Emitters");

			std::string defineEnum = "";
			TwType emitterType;

			for (int i = emitt->size() - 1; i >= 0; --i){
				CEmitter::EmitterData emitData = emitt->getEmitterData();
				std::string key = emitData.listKeys[i];
				if (i == files.size() - 1) defineEnum += key;
				else defineEnum += key + ",";
			}

			emitterType = TwDefineEnumFromString("EmittersType", defineEnum.c_str());
			TwAddVarCB(particlesHierarchy, "Emitters", emitterType, update_enable_particles, read_enable_particles, emitt, "");
		
		}*/
		
	}


	void TW_CALL Callback_reset_position(void *clientData){

		static_cast<CParticleSystem *>(clientData)->resetposition();
	}

	void TW_CALL Callback_reload(void *clientData){

		static_cast<CParticleSystem *>(clientData)->reload();

	}

	void TW_CALL Callback_remove_particles(void *clientData){

	}

	void TW_CALL Callback_save_XML(void *clientData){
		
		CEmitter* emitt = emitter->get<CEmitter>();
		CEmitter::EmitterData emitterData = emitt->getEmitterData();
		ParticleSystemManager::get().save(emitterData);
	}

	void TW_CALL Callback_create_particles(void *clientData){

		CEmitter* emitt = nullptr;
		emitt = emitter->get<CEmitter>();
		int index = emitt->add();
		AntTWManager::createEmitterEditorTweak(index);

	}

	void TW_CALL Callback_reload_list(void *clientData){

		if (particlesHierarchy != nullptr){

			CEmitter* emitt = emitter->get<CEmitter>();

			TwRemoveVar(particlesHierarchy, "Emitters");
			TwRemoveVar(particlesHierarchy, "Reload List");

			std::string defineEnum = "";
			TwType emitterType;

			for (int i = emitt->size() - 1; i >= 0; --i){
				CEmitter::EmitterData emitData = emitt->getEmitterData();
				std::string key = emitData.listKeys[i];
				if (i == files.size() - 1) defineEnum += key;
				else defineEnum += key + ",";
			}

			emitt->setCurrentEditIndex(0);
			emitterType = TwDefineEnumFromString("EmittersType", defineEnum.c_str());
			TwAddVarCB(particlesHierarchy, "Emitters", emitterType, update_enable_particles, read_enable_particles, emitt, "");
			TwAddButton(particlesHierarchy, "Reload List", Callback_reload_list, NULL, "");
		}
	}

	void TW_CALL Callback_create_emitter_camera(void *clientData)
	{

		Handle h = App::get().getCamera();
		Entity *eCamera(h);
		CTransform *tCamera = eCamera->get<CTransform>();

		

		if (!emitter_h.isValid()){
			emitter_h = getManager<Entity>()->createObj();
			emitter = emitter_h;
		}


		if (!emitter->has<CTransform>()){
			Handle t_h = getManager<CTransform>()->createObj();
			CTransform* t = t_h;
			t->setPosition(tCamera->getPosition());
			t->setRotation(XMQuaternionIdentity());
			emitter->add(t);
		}
		else{
			CTransform* t = emitter->get<CTransform>();
			t->setPosition(tCamera->getPosition());
			t->setRotation(XMQuaternionIdentity());
		}

		CEmitter* emitt = nullptr;
		int idx = 0;

		if (!emitter->has<CEmitter>()){
			Handle emitt_h = getManager<CEmitter>()->createObj();
			emitt = emitt_h;
			emitter->add(emitt);
			idx = emitt->add();
			emitt->setValid(true);
		}
		else{

			emitt = emitter->get<CEmitter>();
			emitt->removeAll();
			idx = emitt->add();
			emitt->setValid(true);
		}

		AntTWManager::createEmitterEditorTweak(idx);
	}

	void TW_CALL Callback_create_emitter(void *clientData)
	{

		if (!emitter_h.isValid()){
			emitter_h = getManager<Entity>()->createObj();
			emitter = emitter_h;
		}


		if (!emitter->has<CTransform>()){
			Handle t_h = getManager<CTransform>()->createObj();
			CTransform* t = t_h;
			t->setPosition(XMVectorSet(0, 1, 0, 0));
			t->setRotation(XMQuaternionIdentity());
			emitter->add(t);
		}
		else{
			CTransform* t = emitter->get<CTransform>();
			t->setPosition(XMVectorSet(0, 1, 0, 0));
			t->setRotation(XMQuaternionIdentity());
		}

		CEmitter* emitt = nullptr;
		int idx = 0;

		if (!emitter->has<CEmitter>()){
			Handle emitt_h = getManager<CEmitter>()->createObj();
			emitt = emitt_h;
			emitter->add(emitt);
			idx = emitt->add();
			emitt->setValid(true);
		}
		else{
			
			emitt = emitter->get<CEmitter>();
			emitt->removeAll();
			idx = emitt->add();
			emitt->setValid(true);
		}

		AntTWManager::createEmitterEditorTweak(idx);
	}

	static void TW_CALL update_loading_particle_system(const void *value, void *clientData)
	{

		bool alreadyCreated = true;
		if (!emitter_h.isValid()){
			emitter_h = getManager<Entity>()->createObj();
			emitter = emitter_h;
			alreadyCreated = false;
		}

		if (!emitter->has<CTransform>()){
			Handle t_h = getManager<CTransform>()->createObj();
			CTransform* t = t_h;
			t->setPosition(XMVectorSet(0, 1, 0, 0));
			t->setRotation(XMQuaternionIdentity());
			emitter->add(t);
		}
		else{
			CTransform* t = emitter->get<CTransform>();
			if (!alreadyCreated){
				t->setPosition(XMVectorSet(0, 1, 0, 0));
				t->setRotation(XMQuaternionIdentity());
			}
		}

		int idx = 0;
		CEmitter* emitt = nullptr;

		if (!emitter->has<CEmitter>()){
			Handle emitt_h = getManager<CEmitter>()->createObj();
			emitt = emitt_h;
			emitter->add(emitt);
		}
		else{

			emitt = emitter->get<CEmitter>();
			emitt->removeAll();
		}


		int index = *static_cast<const int *>(value);
		std::string nameFile = emitterFiles[index];
		EmitterParser::get().load(nameFile);

		CEmitter::EmitterData emitterData = ParticleSystemManager::get().getEmitter(nameFile);
		emitt->setEmitterData(emitterData);

		for (int i = 0; i < emitterData.count; i++){

			std::string key = (std::string)emitterData.listKeys[i];
			emitt->load_editor(nameFile ,key);
		}
		emitt->setValid(true);
		AntTWManager::createEmitterEditorTweak(idx);

	}

	static void TW_CALL update_loading_particle_system_camera(const void *value, void *clientData)
	{
		Handle h = App::get().getCamera();
		Entity *eCamera(h);
		CTransform *tCamera = eCamera->get<CTransform>();

		bool alreadyCreated = true;
		if (!emitter_h.isValid()){
			emitter_h = getManager<Entity>()->createObj();
			emitter = emitter_h;
			alreadyCreated = false;
		}

		if (!emitter->has<CTransform>()){
			Handle t_h = getManager<CTransform>()->createObj();
			CTransform* t = t_h;
			t->setPosition(tCamera->getPosition());
			t->setRotation(XMQuaternionIdentity());
			emitter->add(t);
		}
		else{
			CTransform* t = emitter->get<CTransform>();
			if (!alreadyCreated){
				t->setPosition(tCamera->getPosition());
				t->setRotation(XMQuaternionIdentity());
			}
		}

		int idx = 0;
		CEmitter* emitt = nullptr;

		if (!emitter->has<CEmitter>()){
			Handle emitt_h = getManager<CEmitter>()->createObj();
			emitt = emitt_h;
			emitter->add(emitt);
		}
		else{

			emitt = emitter->get<CEmitter>();
			emitt->removeAll();
		}


		int index = *static_cast<const int *>(value);
		std::string nameFile = emitterFiles[index];
		EmitterParser::get().load(nameFile);

		CEmitter::EmitterData emitterData = ParticleSystemManager::get().getEmitter(nameFile);
		emitt->setEmitterData(emitterData);

		for (int i = 0; i < emitterData.count; i++){

			std::string key = (std::string)emitterData.listKeys[i];
			emitt->load_editor(nameFile, key);
		}
		emitt->setValid(true);
		AntTWManager::createEmitterEditorTweak(idx);

	}

	

	//storage of particle system
	static void TW_CALL read_loading_particle_system(void *value, void *clientData)
	{
		
	}

	

	static void TW_CALL read_BlendCnfg(void *value, void *clientData)
	{
		*static_cast< render::BlendConfig *>(value) = static_cast<const CParticleSystem *>(clientData)->getBlendConfig();
	}

	static void TW_CALL update_BlendCnfg(const void *value, void *clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setBlendConfig(*static_cast<const render::BlendConfig *>(value));
	}

	const char* techniqueNames[] {
		"particles",
			"particles_luminance",
			"particles_value",
	};

	static void TW_CALL read_tech(void *value, void *clientData)
	{
		*static_cast< int *>(value) = 0;

		auto tech = static_cast<const CParticleSystem *>(clientData)->getTechnique();
		auto name = Technique::getManager().getNameOf(tech);
		if (name == "particles_sandbox") { *static_cast< int *>(value) = -1; }
		for (int i = 0; i<ARRAYSIZE(techniqueNames); ++i) {
			if (techniqueNames[i] == name) {
				*static_cast< int *>(value) = i;
				return;
			}
		}
	}

	static void TW_CALL update_tech(const void *value, void *clientData)
	{
		int id = *static_cast<const int *>(value);
		static_cast<CParticleSystem *>(clientData)->setTechnique(
			Technique::getManager().getByName(
			id >= 0 ? techniqueNames[id] : "particles_sandbox"
			));
	}

	static void TW_CALL update_zSort(const void* value, void* clientData)
	{
		static_cast<CParticleSystem *>(clientData)->setZSort(*static_cast<const bool *>(value));
	}

	static void TW_CALL read_zSort(void* value, void* clientData)
	{
		*static_cast<bool *>(value) = static_cast<CParticleSystem *>(clientData)->getZSort();
	}

	//***********************************************************************************************************//
	//***********************************************************************************************************//
	//************************************* Creating Bars Methods ***********************************************//
	//***********************************************************************************************************//
	//***********************************************************************************************************//


	void AntTWManager::createParticleEditorTweak()
	{
		static const TwEnumVal blendConfigs_e[] = {
				{ BLEND_CFG_DEFAULT, "DEFAULT" },
				{ BLEND_CFG_COMBINATIVE, "COMBINATIVE" },
				{ BLEND_CFG_ADDITIVE, "ADDITIVE" },
				{ BLEND_CFG_ADDITIVE_BY_SRC_ALPHA, "ADDITIVE_BY_SRC_ALPHA" },
				{ BLEND_GLASS, "GLASS" },
				{ BLEND_PARTICLES, "PARTICLES" },
		};
		static const TwType blendCnfgType =
			TwDefineEnum("Blend configs", blendConfigs_e, ARRAYSIZE(blendConfigs_e));

		static const TwEnumVal techniques_e[] = {
				{ 0, "Default" },
				{ 1, "Luminance as alpha" },
				{ 2, "Value as alpha" },
				{ -1, "<for testing>" }, //Do not use! use this to try things
		};
		static const TwType techniqueType =
			TwDefineEnum("Particle techniques", techniques_e, ARRAYSIZE(techniques_e));


		CTransform *componentTransform = particle->get<CTransform>();
		CParticleSystem *componentParticle = particle->get<CParticleSystem>();
		
		CEmitter *emitt = emitter->get<CEmitter>();
		

		CEmitter::EmitterData emitterData = emitt->getEmitterData();

		char defineLine[100];
		int width = (int)App::get().getConfigX() - 210;
		int height = (int)App::get().getConfigY();

		sprintf(defineLine, "Particle_Editor position='%i 20' size='200 %i'", width, height - 30);
		const char* defineLineFinal = (const char*)defineLine;

		barParticle1 = TwNewBar("Particle_Editor");
		TwDefine(defineLineFinal);

		TwAddSeparator(barParticle1, "Name", NULL);
		TwAddVarRW(barParticle1, "particleSys name:", TW_TYPE_STDSTRING, &file_particules_name, NULL);

		TwAddSeparator(barParticle1, "name_separated", NULL);

		files.clear();
		GetFilesInDirectory(files, PARTICLES_TEXTURE_PATH);

		int idx = 0;
		int idxTexture = -1;
		std::string defineEnum = "";
		TwType textureType;

		for (auto &texture : files){
			if (idx == files.size() - 1) defineEnum += texture;
			else defineEnum += texture + ",";
		}

		textureType = TwDefineEnumFromString("Texture_Separated", defineEnum.c_str());

		TwAddVarCB(barParticle1, "Texture name:", textureType, update_texture, read_texture, componentParticle, "");
		TwAddVarCB(barParticle1, "Blend config:", blendCnfgType, update_BlendCnfg, read_BlendCnfg, componentParticle, "");
		TwAddVarCB(barParticle1, "Technique:", techniqueType, update_tech, read_tech, componentParticle, "");
		TwAddVarCB(barParticle1, "ZSort: ", TW_TYPE_BOOLCPP, update_zSort, read_zSort, componentParticle, "");

		TwAddSeparator(barParticle1, "Type_Separated", NULL);

		TwAddVarCB(barParticle1, "Cone", TW_TYPE_BOOL8, update_cone, read_cone, componentParticle, "true=1 false=0");
		TwAddVarCB(barParticle1, "Box", TW_TYPE_BOOL8, update_box, read_box, componentParticle, "true=1 false=0");
		TwAddVarCB(barParticle1, "Sphere", TW_TYPE_BOOL8, update_sphere, read_sphere, componentParticle, "true=1 false=0");
		TwAddVarCB(barParticle1, "Semisphere", TW_TYPE_BOOL8, update_semisphere, read_semisphere, componentParticle, "true=1 false=0");
		TwAddVarCB(barParticle1, "Disk", TW_TYPE_BOOL8, update_disk, read_disk, componentParticle, "true=1 false=0");
		TwAddVarCB(barParticle1, "Random", TW_TYPE_BOOL8, update_random, read_random, componentParticle, "true=1 false=0");
		TwAddVarCB(barParticle1, "Butterfly", TW_TYPE_BOOL8, update_butterfly, read_butterfly, componentParticle, "true=1 false=0");

		TwAddSeparator(barParticle1, "General_separator", NULL);

		TwAddVarCB(barParticle1, "max particles", TW_TYPE_INT32, update_max_particles, read_max_particles, componentParticle, "min=1 max=10000 step=1");
		TwAddVarCB(barParticle1, "range", TW_TYPE_FLOAT, update_range_distance, read_range_distance, componentParticle, "min=0.01 step=0.01");
		TwAddVarCB(barParticle1, "AxisY range enable", TW_TYPE_BOOL8, update_range_y_enable, read_range_y_enable, componentParticle, "true=1 false=0");

		TwAddSeparator(barParticle1, "life_separator", NULL);
		TwAddVarCB(barParticle1, "lifetime", TW_TYPE_FLOAT, update_lifetime_particles, read_lifetime_particles, componentParticle, "min=0.01 step=0.01");
		TwAddVarCB(barParticle1, "can revive", TW_TYPE_BOOL8, update_lifetime_revive, read_lifetime_revive, componentParticle, "true=1 false=0");
		TwAddVarCB(barParticle1, "random lifetime", TW_TYPE_BOOL8, update_lifetime_random, read_lifetime_random, componentParticle, "true=1 false=0");

		TwAddSeparator(barParticle1, "gravity_separator", NULL);

		TwAddVarCB(barParticle1, "gravity", TW_TYPE_FLOAT, update_gravity, read_gravity, componentParticle, "min=-100.000 max=100 step=0.001	");

		TwAddSeparator(barParticle1, "velocity_separator", NULL);

		TwAddVarCB(barParticle1, "Start speed", TW_TYPE_FLOAT, update_speed, read_speed, componentParticle, "min=-1000.000 max=1000.000 step=0.001");

		TwAddSeparator(barParticle1, "transform_separator", NULL);

		TwAddVarRW(barParticle1, "Position", TW_TYPE_DIR3F, &componentTransform->refPosition(), "opened=true axisx=-x axisz=-z");
		TwAddVarCB(barParticle1, "Rotation", TW_TYPE_QUAT4F, update_rotation, read_rotation, componentParticle, "opened=true axisx=-x axisz=-z");

		TwAddSeparator(barParticle1, "rotation_separator", NULL);

		TwAddVarCB(barParticle1, "Rotation Effect", TW_TYPE_FLOAT, update_rate_rotation_particle, read_rate_rotation_particle, componentParticle, "min=0 max=360 step=0.01");
		TwAddVarCB(barParticle1, "Scale", TW_TYPE_FLOAT, update_scale, read_scale, componentParticle, "min=0.0001 step=0.01");

		TwAddSeparator(barParticle1, "angle_separator", NULL);

		TwAddVarCB(barParticle1, "Angle", TW_TYPE_FLOAT, update_angle, read_angle, componentParticle, "min=0.0 max=180 step=1.0");

		TwAddSeparator(barParticle1, "Texture_Frames", NULL);

		TwAddVarCB(barParticle1, "Multiple Frames", TW_TYPE_BOOL8, update_muliple_frames, read_multiple_frames, componentParticle, "true=1 false=0");

		TwAddSeparator(barParticle1, "Texture_Frames_edit", NULL);

		TwAddVarCB(barParticle1, "Animated", TW_TYPE_BOOL8, update_is_animated, read_is_animated, componentParticle, "true=1 false=0");
		TwAddVarCB(barParticle1, "Texture random particles", TW_TYPE_BOOL8, update_is_random_particles, read_is_random_particles, componentParticle, "true=1 false=0");
		TwAddVarCB(barParticle1, "Total of frames", TW_TYPE_INT8, update_number_of_frames, read_number_of_frames, componentParticle, "min=1 step=1");
		TwAddVarCB(barParticle1, "Number of frames per row", TW_TYPE_INT8, update_number_of_frames_per_row, read_number_of_frames_per_row, componentParticle, "min=1 step=1");
		TwAddVarCB(barParticle1, "Timing", TW_TYPE_FLOAT, update_timing, read_timing, componentParticle, "min=0.0 max=60.0 step=1.0");

		TwDefine(" Main/CSString label='Character capitalization' help='This example demonstates different use of C-Static sized variables.' ");
		TwAddSeparator(barParticle1, "color_Separated", NULL);

		TwAddVarCB(barParticle1, "use color", TW_TYPE_BOOL8, update_use_color, read_use_color, componentParticle, "true=1 false=0");
		TwAddVarCB(barParticle1, "rainbow effect", TW_TYPE_BOOL8, update_use_mulitcolor, read_use_multicolor, componentParticle, "true=1 false=0");

		TwAddVarCB(barParticle1, "Color", TW_TYPE_COLOR32, update_intime_begin_color, read_intime_begin_color, componentParticle, "opened=true coloralpha=true colororder=rgba colormode=rgb ");

		TwAddSeparator(barParticle1, "color_Separated_end", NULL);

		TwAddVarCB(barParticle1, "use color end ", TW_TYPE_BOOL8, update_use_end_color, read_use_end_color, componentParticle, "true=1 false=0");

		TwAddVarCB(barParticle1, "Color end", TW_TYPE_COLOR32, update_intime_end_color, read_intime_end_color, componentParticle, "opened=true coloralpha=true colororder=rgba colormode=rgb ");

		TwAddVarCB(barParticle1, "color rate speed", TW_TYPE_FLOAT, update_color_rate, read_color_rate, componentParticle, "min=0.0 max=1.0 step=0.01");

		TwAddSeparator(barParticle1, "motion_separator", NULL);

		TwAddVarCB(barParticle1, "motion simulator", TW_TYPE_BOOL8, update_motion, read_motion, componentParticle, "true=1 false=0");
		TwAddVarCB(barParticle1, "motion acceleration", TW_TYPE_FLOAT, update_acceleration_motion, read_acceleration_motion, componentParticle, "min=1 max=1000.000 step=0.001");

		TwAddButton(barParticle1, "reset position", Callback_reset_position, componentParticle, "");

		TwAddButton(barParticle1, "reload", Callback_reload, componentParticle, "");

		//-------------------------------------------------------------------------------------

		char definePhysxLine[100];
		width = width - 210;
		height = height - 510;

		sprintf(definePhysxLine, "bar_physx position='%i %i' size='200 500'", width, height);
		const char* definePhysxLineFinal = (const char*)definePhysxLine;

		barPhysx = TwNewBar("bar_physx");
		TwDefine(definePhysxLineFinal);

		TwAddSeparator(barPhysx, "physx_separator", NULL);

		TwAddVarCB(barPhysx, "use physics", TW_TYPE_BOOL8, update_physx, read_physx, componentParticle, "true=1 false=0");

		TwAddSeparator(barPhysx, "transform_separator", NULL);


		TwAddVarCB(barPhysx, "damping", TW_TYPE_FLOAT, update_physx_damping, read_physx_damping, componentParticle, "min=0.0 max=30.0 step=0.01");
		TwAddVarCB(barPhysx, "mass", TW_TYPE_FLOAT, update_physx_mass, read_physx_mass, componentParticle, "min=0.0 max=10000.0 step=0.1");
		TwAddVarCB(barPhysx, "restitution", TW_TYPE_FLOAT, update_physx_restitution, read_physx_restitution, componentParticle, "min=0.0 max=1.0 step=0.001");
		TwAddVarCB(barPhysx, "dynamic collision", TW_TYPE_FLOAT, update_physx_dynamic, read_physx_dynamic, componentParticle, "min=0.0 max=1.0 step=0.001");
		TwAddVarCB(barPhysx, "static collision", TW_TYPE_FLOAT, update_physx_static, read_physx_static, componentParticle, "min=0.0 max=1.0 step=0.001");
		TwAddVarCB(barPhysx, "max motion", TW_TYPE_FLOAT, update_physx_maxmotion, read_physx_maxmotion, componentParticle, "min=0.0 max=1.0 step=0.001");
		TwAddVarCB(barPhysx, "restoffset", TW_TYPE_FLOAT, update_physx_restoffset, read_physx_restoffset, componentParticle, "min=0.0 max=1.0 step=0.001");
		TwAddVarCB(barPhysx, "gridsize", TW_TYPE_FLOAT, update_physx_gridsize, read_physx_gridsize, componentParticle, "min=0.0 max=1.0 step=0.001");


		TwAddVarCB(barPhysx, "acceleration", TW_TYPE_DIR3F, update_physx_acceleration, read_physx_acceleration, componentParticle, "opened=true axisx=-x axisz=-z");



	}

	void AntTWManager::createBarManagerTweak(){

		ParticleSystemParser::get().loadLibrary();

		fileParticlesManagerBar = TwNewBar("File_Manager");
		TwDefine("File_Manager size='200 300'");


		TwAddSeparator(fileParticlesManagerBar, "new_section", NULL);

		TwAddButton(fileParticlesManagerBar, "New Emitter", Callback_create_emitter, NULL, "");
		TwAddButton(fileParticlesManagerBar, "New Emitter Camera", Callback_create_emitter_camera, NULL, "");

		TwAddSeparator(fileParticlesManagerBar, "Edit_section", NULL);

		TwAddButton(fileParticlesManagerBar, "Save Emitter", Callback_save_particles, NULL, "");
		TwAddButton(fileParticlesManagerBar, "Remove Emitter", Callback_remove_particles, NULL, "");


		TwAddSeparator(fileParticlesManagerBar, nullptr, NULL);

		GetFilesInDirectory(emitterFiles, EMITTERS_PATH);

		std::string defineEnum = "";
		TwType densityType;

		int idx = 0;
		for (auto &ps : emitterFiles){
			if (idx == emitterFiles.size() - 1) defineEnum += ps;
			else defineEnum += ps + ",";
		}

		densityType = TwDefineEnumFromString("files", defineEnum.c_str());
		TwAddVarCB(fileParticlesManagerBar, "OpenFile:", densityType,
			update_loading_particle_system, read_loading_particle_system,
			NULL, "");
		TwAddVarCB(fileParticlesManagerBar, "CamPos-OpenFile:", densityType,
			update_loading_particle_system_camera, read_loading_particle_system,
			NULL, "");
		
		TwAddSeparator(fileParticlesManagerBar, "Particles Storage4", NULL);
		TwAddSeparator(fileParticlesManagerBar, "Particles Storage5", NULL);
		TwAddSeparator(fileParticlesManagerBar, "Particles Storage6", NULL);

		TwAddButton(fileParticlesManagerBar, "SAVE XML", Callback_save_XML, NULL, "");

	}

	void AntTWManager::createEmitterEditorTweak(int idx){

		if (particlesHierarchy == nullptr){

			char defineLine[100];
			int height = (int)App::get().getConfigY();
			sprintf(defineLine, "list position='%i %i' size='200 %i'", x, 320, height - 320);
			const char* defineLineFinal = (const char*)defineLine;

			particlesHierarchy = TwNewBar("list");
			TwDefine(defineLineFinal);
		}

		CEmitter* emitt = emitter->get<CEmitter>();
		CTransform* emittT = emitter->get<CTransform>();

		TwRemoveVar(particlesHierarchy, "Emitters");

		std::string defineEnum = "";
		TwType emitterType;

		for (int i = emitt->size() - 1; i >= 0; --i){
			CEmitter::EmitterData emitData = emitt->getEmitterData();
			std::string key = emitData.listKeys[i];
			if (i == files.size() - 1) defineEnum += key;
			else defineEnum += key + ",";
		}

		emitt->setCurrentEditIndex(0);
		emitterType = TwDefineEnumFromString("EmittersType", defineEnum.c_str());
		TwAddVarRW(particlesHierarchy, "General Name:", TW_TYPE_STDSTRING, &emitt->refName(), NULL);
		TwAddButton(particlesHierarchy, "New Particles ", Callback_create_particles, NULL, "");
		TwAddVarCB(particlesHierarchy, "position", TW_TYPE_DIR3F, update_emitter_position, read_emitter_position, emitt, "opened=true axisx=-x axisz=-z");
		TwAddVarCB(particlesHierarchy, "Emitters", emitterType, update_enable_particles, read_enable_particles, emitt, "");
		
	}


#endif

}
