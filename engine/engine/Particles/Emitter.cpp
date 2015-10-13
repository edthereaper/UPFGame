#include "mcv_platform.h"
#include "components/transform.h"
#include "ParticleSystem.h"
#include "ParticlesManager.h"


using namespace component;
using namespace gameElements;


namespace particles{

	CEmitter::~CEmitter() {
        #if defined _TOOL_LIGHTS && !defined(_PARTICLES)
		    for (int i = 0; i < emitterData->hexKeys.size(); i++) {
			    std::string name = emitterData->hexKeys[i];
			    ParticleUpdaterManager::get().removePS(name);
		    }
		#endif
	}

	void CEmitter::loadFromProperties(const std::string& elem, utils::MKeyValue &atts){

#if !defined (_PARTICLES)

		if (elem == "Emitter"){
            createEmitterData();

			if (atts.has("name")){ emitterData->name = atts.getString("name", "test"); }
			if (atts.has("position")){ emitterData->position = atts.getPoint("position"); }

			int count = 0;
			if (atts.has("count")){ emitterData->count = atts.getInt("count");}
			
			emitterData->rotation = XMQuaternionIdentity();
			emitterData->valid = true;

			std::stringstream ss;

			for (int i = 0; i < emitterData->count; i++){
			
				ss.str("");
				ss << "particles" << i;
				std::string key = ss.str();
				std::string value = atts.getString(key, "test");
				
				emitterData->listKeys.push_back(value);
				emitterData->keys.push_back(
					EmitterData::key_t(
						Handle(this).getOwner(), emitterData->currentIndex++));
			}
#ifdef _LIGHTTOOL
            exportable = atts.getBool("export", exportable);
#endif
		}
#endif
	}


#if defined(_PARTICLES)
	int CEmitter::add(){
		createEmitterData();

		Handle owner_h = Handle(this).getOwner();
		if (!owner_h.isValid())
			assert("Error");


		Entity * e = owner_h;
		CTransform* meT = e->get<CTransform>();
		

		Handle handle = getManager<Entity>()->createObj();
		emitterPtr = handle;

		Handle transform_h = getManager<CTransform>()->createObj();
		CTransform *transform = transform_h;
		transform->setPosition(meT->getPosition());
		transform->setRotation((meT->getRotation() == zero_v) ? XMVectorSet(0.018028f, -0.021893f, -0.014354f, 0.999495f) : meT->getRotation());
		emitterPtr->add(transform);

		Handle cmesh_h = getManager<CMesh>()->createObj();
		CMesh * cmesh = cmesh_h;
		emitterPtr->add(cmesh);

		std::stringstream ss;
		ss.str("");
		ss << "emitter_" << std::to_string(emitterData->emitterMap.size());


		Handle particles_h = getManager<CParticleSystem>()->createObj();
		CParticleSystem *particles = particles_h;
		emitterPtr->add(particles);
		particles->init();
		particles->load(&render::mesh_textured_quad_xy_centered);
		particles->setParticlesEmitterByDefault();
		particles->setNameParticles(ss.str());
		
		emitterData->emitterMap.insert(EmitterPair(ss.str(), handle));
		emitterData->listKeys.push_back(ss.str());

		setCurrentEditIndex((int)emitterData->emitterMap.size() - 1);

		emitterPtr = nullptr;

		return (int)emitterData->emitterMap.size() - 1;
	}

	int CEmitter::add(std::string key,component::Handle handle_){
		emitterData->emitterMap.insert(EmitterPair(key, handle_));
		return (int)emitterData->emitterMap.size() - 1;
	}
#endif

	void CEmitter::load_editor(std::string nameParticles, std::string key){

#if defined(_PARTICLES)

		Handle owner_h = Handle(this).getOwner();
		if (!owner_h.isValid())
			fatal("Error");

		Entity * e = owner_h;

		CTransform* meT = e->get<CTransform>();
		CTransform* ownerT = e->get<CTransform>();

		Handle handle = getManager<Entity>()->createObj();
		emitterPtr = handle;

		Handle transform_h = getManager<CTransform>()->createObj();
		CTransform *transform = transform_h;
		transform->setPosition(ownerT->getPosition());
		transform->setRotation(ownerT->getRotation());
		emitterPtr->add(transform);

		Handle cmesh_h = getManager<CMesh>()->createObj();
		CMesh * cmesh = cmesh_h;
		emitterPtr->add(cmesh);

		if (emitterData != nullptr) {

			CParticleSystem::ParticlesEmitter ps =
				ParticleSystemManager::get().getParticles
				(nameParticles, key);

			Handle particles_h = getManager<CParticleSystem>()->createObj();
			CParticleSystem *particles = particles_h;
			emitterPtr->add(particles);
			particles->init();
			particles->load(&render::mesh_textured_quad_xy_centered);
			particles->setParticlesEmitter(ps);
			particles->setNameParticles(ps.nameParticles);
			particles->setEnablePhysX(ps.physXEnable);

			particles->receive(MsgEmitterParticles(owner_h));

			emitterData->emitterMap.insert(EmitterPair(key, handle));
			setCurrentEditIndex((int)emitterData->emitterMap.size() - 1);

		}

#endif


	}

	void CEmitter::load(std::string nameParticles, EmitterData::key_t k){
		Entity * e = Handle(this).getOwner();
        assert(e!= nullptr);

		CTransform* meT = e->get<CTransform>();
		CTransform* ownerT = e->get<CTransform>();

		emitterPtr = getManager<Entity>()->createObj();
        
        CName* name(getManager<CName>()->createObj());
        name->setName(e->getName()+'.'+nameParticles);
        emitterPtr->add(name);

		Handle transform_h = getManager<CTransform>()->createObj();
		CTransform *transform = transform_h;
		transform->setPosition(ownerT->getPosition());
		transform->setRotation(ownerT->getRotation());
		emitterPtr->add(transform);

		Handle cmesh_h = getManager<CMesh>()->createObj();
		CMesh * cmesh = cmesh_h;
		emitterPtr->add(cmesh);

        if (emitterData != nullptr) {

		    CParticleSystem::ParticlesEmitter ps = 
                ParticleSystemManager::get().getParticles
								(emitterData->name,nameParticles);

		    Handle particles_h = getManager<CParticleSystem>()->createObj();
		    CParticleSystem *particles = particles_h;
		    emitterPtr->add(particles);
		    particles->init();
		    particles->load(&render::mesh_textured_quad_xy_centered);
		    particles->setParticlesEmitter(ps);
		    particles->setNameParticles(ps.nameParticles);

		    particles->receive(MsgEmitterParticles(e));
		    particles->setup();
		    ParticleUpdaterManager::get().insertPS(k, emitterPtr);
		    particles->setMe();

			transform->setPosition(ownerT->getPosition() + particles->getLocalPosition());

#if defined(_LIGHTTOOL)
			emitterData->listKeys.push_back(nameParticles);
			emitterData->keys.push_back(k);
#endif

        }
		

	}

	Handle CEmitter::get(std::string name){

#if defined(_PARTICLES)
		EmitterMap::iterator it = emitterData->emitterMap.find(name);

		if (it != emitterData->emitterMap.end())
			return it->second;
		
		fatal("Particles not found");
#endif
		return Handle();
	}
	Handle CEmitter::get(int index){

#if defined(_PARTICLES)
		index = (((int)emitterData->emitterMap.size()) - 1) - index;

		EmitterMap::iterator i = emitterData->emitterMap.begin();
		CParticleSystem::ParticlesEmitter component;
		Entity* e = nullptr;
		int idx = 0;
		while (i != emitterData->emitterMap.end()){

			if (idx == index){
				return i->second;
				break;
			}
			idx++;
			i++;
		}
		fatal("Particles not found");
#endif
		return Handle();
	}

	CEmitter::EmitterData::key_t CEmitter::getKey(std::string key){
#if defined(_PARTICLES)
		return CEmitter::EmitterData::key_t();
#else
		if (emitterData != nullptr && emitterData->keys.size() > 0){

			for (int i = 0; i < emitterData->listKeys.size(); i++){

				std::string key_ = emitterData->listKeys[i];
				if (key.compare(key_) == 0)
					return emitterData->keys[i];
			}
		}

		return CEmitter::EmitterData::key_t();
#endif
	}


	void CEmitter::removeScene(std::string name){
		
		Handle h = get(name);
		Entity *e = h;
		e->postMsg(MsgDeleteSelf());
	}
	void CEmitter::removeScene(int index){

		Handle h = get(index);
		Entity *e = h;
		e->postMsg(MsgDeleteSelf());
	}
	
	void CEmitter::removeAll(){

#if defined(_PARTICLES)
	
		if (emitterData->emitterMap.size() > 0){

			EmitterMap::iterator i = emitterData->emitterMap.begin();

			while (i != emitterData->emitterMap.end()){

				Handle h = i->second;
				Entity *e = h;
				e->postMsg(MsgDeleteSelf());
				i++;
			}
			
		}
#endif
#if defined(_LIGHTTOOL) && !defined(_PARTICLES) 

	emitterData->listKeys.clear();

	for (auto i : emitterData->keys)
		ParticleUpdaterManager::get().removePS(i);

	emitterData->keys.clear();
#endif
		
	}

	void CEmitter::updateTag(std::string name,std::string newName){

#if defined(_PARTICLES)
		EmitterMap::iterator it = emitterData->emitterMap.find(name);
		if (it != emitterData->emitterMap.end()) {
			std::swap(emitterData->emitterMap[newName], it->second);
			emitterData->emitterMap.erase(it);
		}

		emitterData->listKeys.at(emitterData->currentEditIndex) = newName;
		dbg("");
#endif
	}


	void CEmitter::setEmittersPosition(XMVECTOR position){
	
#if defined (_PARTICLES)
		Handle owner_h = Handle(this).getOwner();
		if (!owner_h.isValid())
			assert("Error");

		emitterPtr = owner_h;
		CTransform * emitterT = emitterPtr->get<CTransform>();


		Entity *particlesPtr = nullptr;
		int idx = 0;
		EmitterMap::iterator i = emitterData->emitterMap.begin();

		while (i != emitterData->emitterMap.end()){

			Handle e = i->second;
			particlesPtr = e;

			CTransform *positionPartciclesPtr = particlesPtr->get<CTransform>();
			positionPartciclesPtr->setPosition(position);

			idx++;
			i++;

			particlesPtr = nullptr;
		}
#endif

#if defined (_PARTICLES)
		emitterT->setPosition(position);
		emitterPtr = nullptr;
#endif

	
	}
	XMVECTOR CEmitter::getEmittersPosition() const{

		Handle owner_h = Handle(this).getOwner();
		if (!owner_h.isValid())
			fatal("Error");

		Entity *e(owner_h);
		if (e->has<CTransform>()){
			CTransform *t = e->get<CTransform>();
			return t->getPosition();
		}
		return zero_v;
		
	}

	void CEmitter::init(){
        createEmitterData();
	
#if defined(_PARTICLES)
		emitterData->name = "sample";
#else
		for (int i = 0; i < emitterData->count; i++){
			std::string value = emitterData->listKeys[i];
			load(value, emitterData->keys[i]);
		}
#endif
	}

	void CEmitter::update(float elapsed){
        if (emitterData == nullptr) {return;}
#if !defined (_PARTICLES)

		if (emitterData->count > 0 && emitterData->keys.size() > 0){

			for (int i = 0; i < emitterData->keys.size(); i++){

				Handle owner_h = Handle(this).getOwner();
				if (!owner_h.isValid())
					fatal("Error");

				Entity *e(owner_h);
				if (e->has<CTransform>()){

					CTransform *t = e->get<CTransform>();
					XMVECTOR pos = t->getPosition();
					XMVECTOR rot = t->getRotation();

					ParticleUpdaterManager::get().updateTransform(
						emitterData->keys[i], pos, rot);
				}
			}
		}
#endif

	}
	void CEmitter::drawMarker() const
	{
		Entity* e(Handle(this).getOwner());
		CTransform* t = e->get<CTransform>();
		auto pos = t->getPosition();

		mesh_icosahedron_wire.activate();

#ifdef _LIGHTTOOL
		if (selected){
			setObjectConstants(XMMatrixAffineTransformation(one_v*.1f, zero_v, one_q, pos), Color::RED);
			mesh_icosahedron_wire.render();
			setObjectConstants(XMMatrixAffineTransformation(one_v*.09f, zero_v, one_q, pos), Color::YELLOW);
			mesh_icosahedron_wire.render();
		}
		else {
			setObjectConstants(XMMatrixAffineTransformation(one_v*.1f, zero_v, one_q, pos), Color::WHITE);
			mesh_icosahedron_wire.render();
			setObjectConstants(XMMatrixAffineTransformation(one_v*.09f, zero_v, one_q, pos), Color::LIGHT_GRAY);
			mesh_icosahedron_wire.render();
		}
#else
		setObjectConstants(XMMatrixAffineTransformation(one_v*.1f, zero_v, one_q, pos), Color::WHITE);
		mesh_icosahedron_wire.render();
		setObjectConstants(XMMatrixAffineTransformation(one_v*.09f, zero_v, one_q, pos), Color::LIGHT_GRAY);
		mesh_icosahedron_wire.render();
#endif
	}

#ifdef _LIGHTTOOL
void CEmitter::setSelectable()
{
	Handle h;
	Entity* e(Handle(this).getOwner());
    h = e->get<CRigidBody>();
    if (h.isValid()) {h.destroy();}
	h = getManager<CRigidBody>()->createObj();
	e->add(h);
	CRigidBody* r(h);
	r->setSphere(0.5f);
	r->setFilters(filter_t::TOOLS_SELECTABLE, filter_t::ALL_IDS);
	r->createRigidBody();
	r->setKinematic(true);
	r->init();
}
#endif

}