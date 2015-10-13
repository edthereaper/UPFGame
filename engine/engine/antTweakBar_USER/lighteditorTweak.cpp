#include "mcv_platform.h"
#ifdef _LIGHTTOOL
#include "antTW.h"
#include "app.h"

#define KEEP_NONEXPORTABLE_LIGHTS

#include <sstream>

#include "handles/prefab.h"

#include "gameElements/props.h"
using namespace gameElements;

#include "render/components.h"
using namespace DirectX;

#include "animation\components.h"
using namespace animation;

#include "PhysX_USER/pxcomponents.h"

using namespace physX_user;

using namespace component;
using namespace DirectX;

using namespace particles;

namespace antTw_user {

Handle ptLight_h;
Handle dirLight_h;

Handle element_h_cam;
Entity* element_cam = nullptr;

Handle light_h;
static Entity* currentEntity_h = nullptr;

int indexFileList = 0;
std::vector<std::string> fileList;

TwBar *bar = nullptr;
TwBar *bar2 = nullptr;

int configX;
int configY;

TwType shadowResEType;

CTransform* currentTransform_h;
CPtLight* editPtLight;
CVolPtLight* editVolLight;
CDirLight* editDirLight;
CMist* editMist;
CEmitter* editEmitter;
CCamera* cam;

CName* name;
std::stringstream particlesFileStream;

std::ofstream myfile;

int numOfLights = 0;

typedef std::vector<Entity*> vector_t;

void setHandleCam(CCamera* cam){
	Handle h(cam);
	element_h_cam = h;
	element_cam = element_h_cam.getOwner();
}

void deselect() {
	//If the previous item exist, we restart his rigidbody so its clickable again
	currentEntity_h = light_h.getOwner();
	if (currentEntity_h != nullptr){
		if (!currentEntity_h->has<CRigidBody>()){
			if (currentEntity_h->has<CDirLight>()){
				CDirLight* l = currentEntity_h->get<CDirLight>();
				l->setSelectable();
				l->setSelected(false);
			}
			if (currentEntity_h->has<CPtLight>()){
				CPtLight* l = currentEntity_h->get<CPtLight>();
				l->setSelectable();
				l->setSelected(false);
			}
			if (currentEntity_h->has<CVolPtLight>()){
				CVolPtLight* l = currentEntity_h->get<CVolPtLight>();
				l->setSelectable();
				l->setSelected(false);
			}
			if (currentEntity_h->has<CMist>()){
				CMist* l = currentEntity_h->get<CMist>();
				l->setSelectable();
				l->setSelected(false);
			}
		}
	}
    currentEntity_h = nullptr;
}

void TW_CALL CallbackDeleteAllLights(void *clientData)
{
    deselect();
    if (bar2 != nullptr) {TwDeleteBar(bar2); bar2 = nullptr;}
    for(Entity* e : *getManager<Entity>()) {
        if (e->has<CDirLight>()) {
            if (((CDirLight*)e->get<CDirLight>())->getExport()) {
                e->postMsg(MsgDeleteSelf());
            }
        } else if (e->has<CPtLight>()) {
            if (((CPtLight*)e->get<CPtLight>())->getExport()) {
                e->postMsg(MsgDeleteSelf());
            }
        } else if (e->has<CVolPtLight>()) {
            if (((CVolPtLight*)e->get<CVolPtLight>())->getExport()) {
                e->postMsg(MsgDeleteSelf());
            }
        }
    }
    component::MessageManager::dispatchPosts();
    AntTWManager::updateLightList();
}

void TW_CALL CallbackDeleteLight(void *clientData)
{
    Entity* e = currentEntity_h;
    deselect();
	if (bar2 != nullptr) {TwDeleteBar(bar2); bar2 = nullptr;}
	if (light_h.isValid() && e != nullptr) {
          e->destroy();
#ifdef KEEP_NONEXPORTABLE_LIGHTS
        if(e->has<CDirLight>()) {
            CDirLight* item = e->get<CDirLight>();
            if (!item->getExport()) {return;}
        }
        if(e->has<CPtLight>()) {
            CPtLight* item = e->get<CPtLight>();
            if (!item->getExport()) {return;}
        }
        if(e->has<CVolPtLight>()) {
            CVolPtLight* item = e->get<CVolPtLight>();
            if (!item->getExport()) {return;}
        }
#endif
        light_h = Handle();
        AntTWManager::updateLightList();
	}
}

void TW_CALL CallbackCreateEmitter(void *clientData)
{
	CTransform* ctransf = element_cam->get<CTransform>();
	Entity* e = PrefabManager::get().prefabricate("tools/emitter");
	CTransform* t(e->get<CTransform>());
	t->setPosition(ctransf->getPosition() + ctransf->getFront());
	CEmitter* i(e->get<CEmitter>());
	i->setSelectable();
	e->init();
	AntTWManager::selectEmitterTweak(i);
}

void TW_CALL CallbackCreateMist(void *clientData)
{
	CTransform* ctransf = element_cam->get<CTransform>();
	Entity* e = PrefabManager::get().prefabricate("tools/mist");
	CTransform* t(e->get<CTransform>());
	t->setPosition(ctransf->getPosition() + ctransf->getFront());
	CMist* l(e->get<CMist>());
	l->setSelectable();
	e->init();
    AntTWManager::selectMistTweak(l);
    AntTWManager::updateLightList();
}

void TW_CALL CallbackCreateVolLight(void *clientData)
{
	CTransform* ctransf = element_cam->get<CTransform>();
	Entity* e = PrefabManager::get().prefabricate("tools/volLight");
	CTransform* t(e->get<CTransform>());
	t->setPosition(ctransf->getPosition() + ctransf->getFront());
    t->lookAt(t->getPosition()-yAxis_v);
	CVolPtLight* l(e->get<CVolPtLight>());
	l->setSelectable();
	e->init();
    AntTWManager::selectVolLightTweak(l);
    AntTWManager::updateLightList();
}

void TW_CALL CallbackCreatePointLight(void *clientData)
{
	CTransform* ctransf = element_cam->get<CTransform>();
	Entity* e = PrefabManager::get().prefabricate("tools/ptLight");
	CTransform* t(e->get<CTransform>());
	t->setPosition(ctransf->getPosition() + ctransf->getFront());
    t->lookAt(t->getPosition()-yAxis_v);
	CPtLight* l(e->get<CPtLight>());
	l->setSelectable();
	e->init();
    AntTWManager::selectPointLightTweak(l);
    AntTWManager::updateLightList();
}

void TW_CALL CallbackCreateSpotOrthoLight(void *clientData)
{	
	CTransform* ctransf = element_cam->get<CTransform>();
	Entity* e = PrefabManager::get().prefabricate("tools/dirOrthoLight");
	CTransform* t(e->get<CTransform>());
	t->setPosition(ctransf->getPosition() + ctransf->getFront());
    t->lookAt(t->getPosition()-yAxis_v);
	CDirLight* l(e->get<CDirLight>());
	l->setSelectable();
	e->init();
    
    AntTWManager::selectDirectionalLightTweak(l);
    AntTWManager::updateLightList();
}

void TW_CALL CallbackCreateSpotLight(void *clientData)
{	
	CTransform* ctransf = element_cam->get<CTransform>();
	Entity* e = PrefabManager::get().prefabricate("tools/dirLight");
	CTransform* t(e->get<CTransform>());
	t->setPosition(ctransf->getPosition() + ctransf->getFront());
    t->lookAt(t->getPosition() - yAxis_v);
	CDirLight* l(e->get<CDirLight>());
	l->setSelectable();
	e->init();
    AntTWManager::selectDirectionalLightTweak(l);
    
    AntTWManager::updateLightList();
}

void savePointLight(CPtLight* item)
{
    if (!item->getExport()) {return;}
	Handle h(item);
	Handle light_h = h;
	Entity* currentEntity_h = light_h.getOwner();
	CTransform* ctransf = currentEntity_h->get<CTransform>();
	CName* cname = currentEntity_h->get<CName>();

	XMVECTOR pos = ctransf->getPosition();
	std::string posx = std::to_string(XMVectorGetX(pos));
	std::string posy = std::to_string(XMVectorGetY(pos));
	std::string posz = std::to_string(XMVectorGetZ(pos));

	std::string radius = std::to_string(item->getRadius());
	std::string decay = std::to_string(item->getDecay());
	XMVECTOR offset = item->getOffset();
	std::string offsetx = std::to_string(XMVectorGetX(offset));
	std::string offsety = std::to_string(XMVectorGetY(offset));
	std::string offsetz = std::to_string(XMVectorGetZ(offset));
	std::string shadowIntensity = std::to_string(item->getShadowIntensity());
    std::string shadowFocus = std::to_string(item->getShadowFocus());

	char hexcolor[10];
	sprintf(hexcolor, "%x", item->getColor());

	std::string name = cname->getName();

	myfile << "\t<Entity init=\"true\">\n";
	myfile << "\t\t<Name name=\"" + name + "\"/>\n";
	myfile << "\t\t<Transform pos=\"" + posx + " " + posy + " " + posz + "\"/>\n";
	if (item->getShadowIntensity() > 0.0f){
	    CCubeShadow* cshadow = currentEntity_h->get<CCubeShadow>();
	    std::string shadow = std::to_string(cshadow->getResolution());
	    CCamera* ccam = currentEntity_h->get<CCamera>();
	    std::string znear = std::to_string(ccam->getZNear());
	    std::string zfar = std::to_string(ccam->getZFar());
		myfile << "\t\t<Camera fov=\"90\" znear=\"" + znear + "\" zfar=\"" + zfar + "\">\n";
		myfile << "\t\t\t<viewport w=\"" + shadow + "\" h=\"" + shadow + "\"/>\n";
		myfile << "\t\t</Camera>\n";
		myfile << "\t\t<CullingCube/>\n";
	    myfile << "\t\t<CubeShadow resolution=\"" + shadow + "\" enable=\""
            << std::hex << unsigned(cshadow->getEnableMask()) << std::dec <<"\" />\n";
	}
    
	myfile << "\t\t<PtLight radius=\""<< radius << "\" "
        << "decay=\"" << item->getDecay() << "\" "
        << "color=\"0x" << std::hex << uint32_t(item->getColor()) << std::dec << "\" "
        << "shadowIntensity=\"" << item->getShadowIntensity() << "\" "
        << "shadowFocus=\"" << item->getShadowFocus() << "\" "
        << "shadowJittering=\"" << item->getShadowJittering() << "\" "
        << "offset=\"" + offsetx + " " + offsety + " " + offsetz + "\" "
        << "specularAmountModifier=\"" << item->getSpecularAmountModifier() << "\" "
        << "specularPowerFactor=\"" << item->getSpecularPowerFactor() << "\" "
        << "intensity=\"" << item->getIntensity() << "\" "
        << "export=\"true\" "
        << "/>\n";

	myfile << "\t\t<CullingAABBSpecial type=\"ptLight\"/>\n";
	myfile << "\t</Entity>\n";
}

void saveDirLight(CDirLight* item)
{
    if (!item->getExport()) {return;}
	Handle h(item);
	Handle light_h = h;
	Entity* currentEntity_h = light_h.getOwner();
	CTransform* ctransf = currentEntity_h->get<CTransform>();
	CCamera* ccam = currentEntity_h->get<CCamera>();
	CName* cname = currentEntity_h->get<CName>();

	XMVECTOR pos = ctransf->getPosition();
	std::string posx = std::to_string(XMVectorGetX(pos));
	std::string posy = std::to_string(XMVectorGetY(pos));
	std::string posz = std::to_string(XMVectorGetZ(pos));
    
    XMVECTOR lookat = ctransf->getLookAt();
    XMVECTOR front = ctransf->getFront();
    //Make sure lookAt is correct
    if (!utils::lineDetection(ctransf, lookat, 0.001f)) {
        ctransf->lookAt(pos+front);
    }
	
	std::string lookatx = std::to_string(XMVectorGetX(lookat));
	std::string lookaty = std::to_string(XMVectorGetY(lookat));
	std::string lookatz = std::to_string(XMVectorGetZ(lookat));
	std::string name = cname->getName();

	myfile << "\t<Entity init=\"true\">\n";
	myfile << "\t\t<Name name=\"" + name + "\"/>\n";
	myfile << "\t\t<Transform pos=\"" + posx + " " + posy + " " + posz + "\" "
        "lookAt=\"" + lookatx + " " + lookaty + " " + lookatz + "\"/>\n";

	std::string znear = std::to_string(ccam->getZNear());
	std::string zfar = std::to_string(ccam->getZFar());

    int res = 512;
	if (item->getShadowIntensity() > 0.0f){
	    CShadow* cshadow = currentEntity_h->get<CShadow>();
        res = cshadow->getResolution();
	    myfile << "\t\t<Shadow resolution=\"" + std::to_string(res) + "\"/>\n";
	}

	if (ccam->isOrthographic()){
		std::string width = std::to_string(ccam->getW());
		std::string height = std::to_string(ccam->getH());
		myfile << "\t\t<Camera orthographic=\"true\" w=\"" + width + "\" h=\"" + height + "\" znear=\"" + znear + "\" zfar=\"" + zfar + "\">\n";
	} else {
		std::string fov = std::to_string(rad2deg(ccam->getFov()));
		myfile << "\t\t<Camera fov=\"" + fov + "\" znear=\"" + znear + "\" zfar=\"" + zfar + "\">\n";
	}
	myfile << "\t\t\t<viewport w=\"" + std::to_string(res) + "\" h=\"" + std::to_string(res) + "\"/>\n";
	myfile << "\t\t</Camera>\n";

	myfile << "\t\t<Culling/>\n";
    myfile << "\t\t<CullingAABBSpecial type=\"dirLight\"/>\n";

	myfile << "\t\t<DirLight spotlight=\""<< (item->isSpotlight()?"yes":"no") << "\" "
        << "spotDecay=\"" << item->getSpotDecay() << "\" "
        << "decay=\"" << item->getDecay() << "\" "
        << "radius=\"" << item->getRadius() << "\" "
        << "color=\"0x" << std::hex << uint32_t(item->getColor()) << std::dec << "\" "
        << "shadowIntensity=\"" << item->getShadowIntensity() << "\" "
        << "shadowFocus=\"" << item->getShadowFocus() << "\" "
        << "shadowJittering=\"" << item->getShadowJittering() << "\" "
        << "specularAmountModifier=\"" << item->getSpecularAmountModifier() << "\" "
        << "specularPowerFactor=\"" << item->getSpecularPowerFactor() << "\" "
        << "intensity=\"" << item->getIntensity() << "\" "
        << "export=\"true\" "
        << "/>\n";
	myfile << "\t</Entity>\n";
}

void saveEmitter(CEmitter* cemitter)
{
    if(!cemitter->getExport()) {return;}
	// TODO:

	Handle h(cemitter);
	Handle emitter_h = h;
	Entity* currentEntity_h = emitter_h.getOwner();

	CTransform* ctransf = currentEntity_h->get<CTransform>();
	CName* cname        = currentEntity_h->get<CName>();
	std::string name = cname->getName();

	XMVECTOR pos = ctransf->getPosition();
	std::string posx = std::to_string(XMVectorGetX(pos));
	std::string posy = std::to_string(XMVectorGetY(pos));
	std::string posz = std::to_string(XMVectorGetZ(pos));

	XMVECTOR rot = ctransf->getRotation();
	std::string rotx = std::to_string(XMVectorGetX(rot));
	std::string roty = std::to_string(XMVectorGetY(rot));
	std::string rotz = std::to_string(XMVectorGetZ(rot));
	std::string rotw = std::to_string(XMVectorGetZ(rot));

	CEmitter::EmitterData data = cemitter->getEmitterData();
	std::vector<std::string> list = ParticleSystemManager::get().getListPsByOwnerName(cemitter->getName());
	std::string nameEmitt = cemitter->getName();

	particlesFileStream.str("");

	int idx = 0;
	for (std::string namePS : list){

		particlesFileStream << " particles" << std::to_string(idx) << "=\"" << namePS << "\"";
		idx++;
	}

	myfile << "\t<Entity init=\"true\">\n";
	myfile << "\t\t<Name name=\"" + name + "\"/>\n";
	myfile << "\t\t<Transform pos=\"" + posx + " " + posy + " " + posz + "\" "
		"rot=\"" + rotx + " " + roty + " " + rotz + " " + rotw +"\"/>\n";
	
	myfile << "\t\t<Emitter";
	myfile << " name=\"" + nameEmitt + "\"";
	myfile << " position=\"" + posx + " " + posy + " " + posz + "\"";
	myfile << " rotation=\"" + rotx + " " + roty + " " + rotz + " " + rotw + "\"";
	myfile << " count=\"" + std::to_string(cemitter->getCount()) + "\"";
    myfile << " export=\"true\" ";
	myfile << particlesFileStream.str();
	myfile << "/>\n";
	myfile << "\t\t</Entity>\n";

}

void saveMist(const CMist* cmist)
{
    if(!cmist->getExport()) {return;}
    MKeyValue entity;
    Entity* e = Handle(cmist).getOwner();
    entity.setBool("init", true);
    entity.writeStartElement(myfile, "Entity", "\t");
    {
        MKeyValue name;
        CName* cname = e->get<CName>();
        name.setString("name", cname->getName());
        name.writeSingle(myfile, "Name", "\t\t");

        MKeyValue transform;
        CTransform* ctransform = e->get<CTransform>();
        transform.setPoint("pos", ctransform->getPosition());
        transform.writeSingle(myfile, "Transform", "\t\t");

        MKeyValue mist;
        mist.setFloat("darkenAlpha", cmist->getDarkenAlpha());
        mist.setFloat("dX", cmist->getDeltaX());
        mist.setFloat("dZ", cmist->getDeltaZ());
        mist.setFloat("w", cmist->getWidth());
        mist.setFloat("h", cmist->getHeight());
        mist.setFloat("l", cmist->getLength());
        mist.setFloat("depthTolerance", cmist->getDepthTolerance());
        mist.setFloat("factor", cmist->getFactor());
        mist.setFloat("intensity", cmist->getIntensity());
        mist.setFloat("layerDecay", cmist->getLayerDecay());
        mist.setFloat("minimun", cmist->getMinimun());
        mist.setFloat("sqSqrt", cmist->getSqSqrt());
        mist.setFloat("unitWorldSize", cmist->getUnitWorldSize());
        mist.setBool("chaotic", cmist->getChaotic());
        mist.setBool("export", cmist->getExport());
        mist.writeStartElement(myfile, "Mist", "\t\t", "\n", "\t\t\t");
        {
            MKeyValue top;
            top.setHex("color", cmist->getColorTop());
            top.writeSingle(myfile, "top", "\t\t\t");

            MKeyValue bottom;
            bottom.setHex("color", cmist->getColorBottom());
            bottom.writeSingle(myfile, "bottom", "\t\t\t");
        }
        mist.writeEndElement(myfile, "Mist", "\t\t");
    }
    entity.writeEndElement(myfile, "Entity", "\t");
}

void saveVolLight(const CVolPtLight* item)
{
    if(!item->getExport()) {return;}
}

void TW_CALL CallbackSaveLights(void *clientData)
{
	myfile.open("data/lights/light.xml");
	myfile << "<scene>\n";
	component::getManager<CPtLight>()->forall<void>(savePointLight);
	component::getManager<CDirLight>()->forall<void>(saveDirLight);
	component::getManager<CVolPtLight>()->forall<void>(saveVolLight);
	component::getManager<CMist>()->forall<void>(saveMist);
	component::getManager<CEmitter>()->forall<void>(saveEmitter);
	myfile << "</scene>\n";
	myfile.close();
    TwDefine((std::string(TwGetBarName(bar))+"/SAVE label='Saved to lights.xml'").c_str());
}

void TW_CALL CallbackDuplicateVolLight(void *clientData)
{
	if (bar2 != nullptr) {TwDeleteBar(bar2); bar2 = nullptr;}
	const bool* duplicateInCamPos = static_cast<const bool *>(clientData);

    Entity* item = Handle(editVolLight).getOwner();
	Entity* newLight = getManager<Entity>()->cloneObj(item);

    CTransform* t(newLight->get<CTransform>());
	if (duplicateInCamPos){
		CTransform* ctransf = element_cam->get<CTransform>();
		t->setPosition(ctransf->getPosition());
	}

    
	CName* name(newLight->get<CName>());
    std::string oldname = item->getName();
	name->setName(name->generate(oldname.substr(0, oldname.size()-2)));
    newLight->init();

	CVolPtLight* volLight(newLight->get<CVolPtLight>());
    volLight->setExport(true);
	volLight->setSelectable();
    volLight->setSelected(true);	

    AntTWManager::selectVolLightTweak(volLight);
	AntTWManager::updateLightList();
}

void TW_CALL CallbackDuplicateMist(void *clientData)
{
	if (bar2 != nullptr) { TwDeleteBar(bar2); bar2 = nullptr; }
	const bool* duplicateInCamPos = static_cast<const bool *>(clientData);

	Entity* item = Handle(editMist).getOwner();
	Entity* e = getManager<Entity>()->cloneObj(item);

	CTransform* t(e->get<CTransform>());
	if (duplicateInCamPos){
		CTransform* ctransf = element_cam->get<CTransform>();
		t->setPosition(ctransf->getPosition());
	}

	CName* name(e->get<CName>());
	std::string oldname = item->getName();
	name->setName(name->generate(oldname.substr(0, oldname.size() - 2)));
	e->init();

	CMist* mist(e->get<CMist>());
	mist->setExport(true);
	mist->setSelectable();
	mist->setSelected(true);

	AntTWManager::selectMistTweak(mist);
	AntTWManager::updateLightList();
}

void TW_CALL CallbackDuplicateEmitter(void *clientData)
{
	if (bar2 != nullptr) {TwDeleteBar(bar2); bar2 = nullptr;}
	const bool* duplicateInCamPos = static_cast<const bool *>(clientData);

    Entity* item = Handle(editEmitter).getOwner();
	Handle h = getManager<Entity>()->cloneObj(item);
	Entity* e = h;

    CTransform* t(e->get<CTransform>());
	if (duplicateInCamPos){
		CTransform* ctransf = element_cam->get<CTransform>();
		t->setPosition(ctransf->getPosition());
	}
    
	CName* name(e->get<CName>());
    std::string oldname = item->getName();
	name->setName(name->generate(oldname.substr(0, oldname.size()-2)));
    e->init();

	CEmitter* em(e->get<CEmitter>());
    em->setExport(true);
	em->setSelectable();
    em->setSelected(true);	

	CEmitter::EmitterData emittData = em->getEmitterData();

	std::stringstream stream;

	for (int i = 0; i < emittData.count; i++){

		stream.clear();
		
		std::string key = (std::string)emittData.listKeys[i];

		CEmitter::EmitterData::key_t keyEmitter;
		keyEmitter.h = h;
		stream << "emitter_" << i;
		auto keyE = em->getKey(stream.str());
		editEmitter->load(key, keyEmitter);
	}
	editEmitter->setValid(true);

    AntTWManager::selectEmitterTweak(em);
}

void TW_CALL CallbackDuplicatePointLight(void *clientData)
{
	if (bar2 != nullptr) {TwDeleteBar(bar2); bar2 = nullptr;}
	const bool* duplicateInCamPos = static_cast<const bool *>(clientData);

    Entity* item = Handle(editPtLight).getOwner();
	Entity* newLight = getManager<Entity>()->cloneObj(item);

    CTransform* t(newLight->get<CTransform>());
	if (duplicateInCamPos){
		CTransform* ctransf = element_cam->get<CTransform>();
		t->setPosition(ctransf->getPosition());
	}
    
	CName* name(newLight->get<CName>());
    std::string oldname = item->getName();
	name->setName(name->generate(oldname.substr(0, oldname.size()-2)));
    newLight->init();

	CPtLight* ptLight(newLight->get<CPtLight>());
    ptLight->setExport(true);
	ptLight->setSelectable();
    ptLight->setSelected(true);	

    AntTWManager::selectPointLightTweak(ptLight);
	AntTWManager::updateLightList();
}

void TW_CALL CallbackDuplicateDirLight(void *clientData)
{
	if (bar2 != nullptr) {TwDeleteBar(bar2); bar2 = nullptr;}
	const bool* duplicateInCamPos = static_cast<const bool *>(clientData);

    Entity* item = Handle(editDirLight).getOwner();
	Entity* newLight = getManager<Entity>()->cloneObj(item);

	CTransform* t(newLight->get<CTransform>());
	if (duplicateInCamPos){
		CTransform* ctransf = element_cam->get<CTransform>();
		t->setPosition(ctransf->getPosition() + ctransf->getFront());
		t->lookAt(ctransf->getPosition() + ctransf->getFront() + (currentTransform_h->getLookAt() - currentTransform_h->getPosition()));
	}

	CName* name(newLight->get<CName>());
    std::string oldname = item->getName();
	name->setName(name->generate(oldname.substr(0, oldname.size()-2)));
    
	CDirLight* dirLight(newLight->get<CDirLight>());
    dirLight->setExport(true);
	dirLight->setSelectable();
    dirLight->setSelected(true);
    AntTWManager::selectDirectionalLightTweak(dirLight);

    newLight->init();
    
    AntTWManager::updateLightList();
}

void TW_CALL CallbackMoveToCam(void *clientData)
{		
	CTransform* ctransf = element_cam->get<CTransform>();
	currentTransform_h->setPosition(ctransf->getPosition() + ctransf->getFront() * 3);
	currentTransform_h->lookAt(currentTransform_h->getPosition() + currentTransform_h->getFront());
}

void TW_CALL CallbackMoveCam(void *clientData)
{		
	CTransform* ctransf = element_cam->get<CTransform>();
	ctransf->setPosition(currentTransform_h->getPosition() - ctransf->getFront() * 3);
}

void TW_CALL MovePlayer(void* clientData)
{
    auto& app = App::get();
    Entity* player = app.playerModelEntity_h;
    Entity* camera = app.getCamera();
    CTransform* pT = player->get<CTransform>();
    CTransform* cT = camera->get<CTransform>();
    pT->setPosition(cT->getFront()*6 + cT->getPosition()-(1.5f)*yAxis_v);
}

static void TW_CALL readlevelLE(void *value, void *clientData)
{
	*static_cast<int *>(value) = App::get().gamelvl;
}

static void TW_CALL updatelevelLE(const void *value, void *clientData)
{
	int level = *static_cast<const int *>(value);

	if (0 < level && level <= 4){
		App &app = App::get();
		app.changelvl = true;
		app.gamelvl = level;
        AntTWManager::deleteLightBars();
	}
}

void AntTWManager::deleteLightBars()
{
    //if(bar != nullptr) {
    //    TwDeleteBar(bar);
    //    bar = nullptr;
    //}
    if(bar2 != nullptr) {
        TwDeleteBar(bar2);
        bar2 = nullptr;
    }
}

void AntTWManager::createLightEditorTweak(Entity* mainCam)
{
	element_cam = mainCam;
	configX = App::get().getConfigX();
	configY = App::get().getConfigY();
    
    if (bar != nullptr) { TwDeleteBar(bar); bar= nullptr;}
	bar = TwNewBar("LightManager");

	static const TwEnumVal level_e[] = {
			{ 1, "Level 1" },
			{ 2, "Level 2" },
			{ 3, "Level 3" },
			{ 4, "Level 4" },
			{ 5, "Sandbox" },
			{ 0, "<choose level>" },
	};
	static const TwType levelType =
		TwDefineEnum("LevelsLE", level_e, ARRAYSIZE(level_e));

	TwAddVarCB(bar, "Level:", levelType, updatelevelLE, readlevelLE, NULL, "");

	TwAddButton(bar, "New point Light", CallbackCreatePointLight, NULL, "key=CTRL+1");
	TwAddButton(bar, "New directional Light", CallbackCreateSpotLight, NULL, "key=CTRL+2");
	TwAddButton(bar, "New orthographic Light", CallbackCreateSpotOrthoLight, NULL, "key=CTRL+3");
	//TwAddButton(bar, "New volumetric Light", CallbackCreateVolLight, NULL, "key=CTRL+4");
	TwAddButton(bar, "New Mist", CallbackCreateMist, NULL, "key=CTRL+5");
	TwAddButton(bar, "New Particle emitter", CallbackCreateEmitter, NULL, "key=CTRL+6");
	TwAddSeparator(bar, NULL, NULL);
	TwAddButton(bar, "movePlayer", MovePlayer, NULL, "label='Move player model' key=CTRL+END");
	TwAddButton(bar, "SAVE", CallbackSaveLights, NULL, "key=CTRL+s");

	TwDefine(" LightManager label='Item editor' position='0 0' "
        "size='200 250' color='200 200 200' refresh=0.1 ");
    
	TwAddSeparator(bar, NULL, NULL);
	TwAddButton(bar, "Delete item", CallbackDeleteLight, NULL, "key=DEL");
	TwAddButton(bar, "Wipe all lights", CallbackDeleteAllLights, NULL, NULL);
    createLightList();
}

static TwType selectLightEType;
static std::vector<Handle> itemList;
static std::vector<char*> lightNameList;
    
#define NONE_ID 90000


static void TW_CALL updatePtIntensity(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CPtLight* item = e->get<CPtLight>();
    const float val=*static_cast<const float*>(value);
    item->setIntensity(val);
}

static void TW_CALL readPtIntensity(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CPtLight* item = e->get<CPtLight>();
    *static_cast<float*>(value) = item->getIntensity();
}

static void TW_CALL selectLightCB(const void *value, void *clientData)
{
    int val = *static_cast<const int*>(value);
    deselect();
    currentEntity_h = nullptr;
    if (bar2 != nullptr) {TwDeleteBar(bar2); bar2 = nullptr;}
    if (val >= 0 && val < itemList.size()) {
	    Entity* item = itemList[val];
        if (item != nullptr) {
            if (item->has<CDirLight>()) {
                AntTWManager::selectDirectionalLightTweak(item->get<CDirLight>());
            } else if (item->has<CPtLight>()) {
                AntTWManager::selectPointLightTweak(item->get<CPtLight>());
            } else if (item->has<CVolPtLight>()) {
                AntTWManager::selectVolLightTweak(item->get<CVolPtLight>());
            } else if (item->has<CMist>()) {
                AntTWManager::selectMistTweak(item->get<CMist>());
			} else if (item->has<CEmitter>()) {
				AntTWManager::selectEmitterTweak(item->get<CEmitter>());
			}
        }
    }
}
static void TW_CALL getLightCB(void *value, void *clientData)
{
    *static_cast<int*>(value) = NONE_ID;
    if (currentEntity_h != nullptr) {
        Handle h(currentEntity_h);
        for (auto i=0; i<itemList.size(); ++i) {
            if (itemList[i] == h) {
                *static_cast<int*>(value) = i;
                return;
            }
        }
    }
    TwDefine((std::string(TwGetBarName(bar)) + " valueswidth=fit ").c_str());
}

void AntTWManager::updateLightList()
{
    TwRemoveVar(bar, "select");
    createLightList();
}
void AntTWManager::createLightList()
{
    TwDefine((std::string(TwGetBarName(bar))+"/SAVE label='SAVE'").c_str());
    itemList.clear();
    for (auto a : lightNameList) {delete a;}
    lightNameList.clear();
    
    auto addToList = [&](Handle h) {itemList.push_back(h.getOwner());};
    getManager<CDirLight>()->forall<void>(addToList);
    getManager<CPtLight>()->forall<void>(addToList);
    getManager<CVolPtLight>()->forall<void>(addToList);
	getManager<CMist>()->forall<void>(addToList);
	getManager<CEmitter>()->forall<void>(addToList);

    auto nLights = itemList.size();
    std::vector<TwEnumVal> vals;
    vals.resize(nLights);
    for (auto i=0; i<nLights; i++) {
        vals[i].Value = i;
        auto str = ((Entity*)(itemList[i]))->getName();
        char* name = new char[str.size()+1];
        strcpy(name, str.c_str());
        lightNameList.push_back(name);
        vals[i].Label = name;
    }
    TwEnumVal def;
    def.Label = "NONE";
    def.Value = NONE_ID;
    vals.push_back(def);
    selectLightEType = TwDefineEnum("lights", vals.data(), unsigned(vals.size()));
    TwAddVarCB(bar, "select", selectLightEType, selectLightCB, getLightCB, NULL,
        "label='Edit' keyincr=CTRL+RIGHT keydecr=CTRL+LEFT ");

    TwDefine((std::string(TwGetBarName(bar)) + " valueswidth=fit ").c_str());
}

static void TW_CALL updatePtColor(const void *value, void *clientData)
{
	Color col = *static_cast<const Color *>(value);
	Color t(col.af(), col.bf(), col.gf(), col.rf());
	static_cast<CPtLight *>(clientData)->setColor(t);
}

static void TW_CALL readPtColor(void *value, void *clientData)
{
	Color c = static_cast<const CPtLight *>(clientData)->getColor();
	Color t;
	t.set(XMVectorSet(c.af(), c.bf(), c.gf(), c.rf()));
	*static_cast<Color *>(value) = t;
}

static void TW_CALL readPtShadowIntensity(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<const CPtLight *>(clientData)->getShadowIntensity();
}
static void TW_CALL updatePtShadowIntensity(const void *value, void *clientData)
{
    auto l = static_cast<CPtLight *>(clientData);
    auto intensity = *static_cast<const float *>(value);
	l->setShadowIntensity(intensity);
    if (intensity > 0) {
        Entity* e = Handle(l).getOwner();
        if (!e->has<CCubeShadow>()) {
            CCubeShadow* shadow = getManager<CCubeShadow>()->createObj();
            e->add(shadow);
            shadow->init();
        }
        if (!e->has<CCullingCube>()) {
            CCullingCube* culling = getManager<CCullingCube>()->createObj();
            e->add(culling);
            culling->init();
        }
        if (!e->has<CCamera>()) {
            CCamera* camera = getManager<CCamera>()->createObj();
            e->add(camera);
            cam = camera;
            CCubeShadow* shadow = e->get<CCubeShadow>();
            float res = float(shadow->getResolution());
            camera->setViewport(0.f,0.f,res,res);
            camera->init();
        }
    }
}
static void TW_CALL readCubeShadow(void *value, void *clientData)
{
	CCubeShadow* shadow = static_cast<Entity *>(clientData)->get<CCubeShadow>();
    auto barName = TwGetBarName(bar2);
    if (shadow != nullptr) {
        std::stringstream ss;
        ss << " \"" << barName << "\"/\"Shadow quality\" readonly=false ";
        TwDefine(ss.str().c_str());
	    *static_cast<int *>(value) = shadow->getResolution();
    } else {
        std::stringstream ss;
        ss << " \"" << barName << "\"/\"Shadow quality\" readonly=true ";
        TwDefine(ss.str().c_str());
	    *static_cast<int *>(value) = 0;
    }
}

static void TW_CALL updateCubeShadow(const void *value, void *clientData)
{
    auto res = *static_cast<const int *>(value);
    if (res > 0) {
	    CCubeShadow* shadow = static_cast<Entity *>(clientData)->get<CCubeShadow>();
        shadow->setResolution(int(res));
        CCamera* camera = Handle(shadow).getBrother<CCamera>();
        camera->setViewport(0.f,0.f,float(res),float(res));
    }
}

static void makeFaceEnableRO(bool b)
{
    std::string val(b?"true":"false");
    std::stringstream ss;
    ss << " \"" << TwGetBarName(bar2) << "\"/\"+X\" readonly=" << val << " ";
    ss << " \"" << TwGetBarName(bar2) << "\"/\"-X\" readonly=" << val << " ";
    ss << " \"" << TwGetBarName(bar2) << "\"/\"+Y\" readonly=" << val << " ";
    ss << " \"" << TwGetBarName(bar2) << "\"/\"-Y\" readonly=" << val << " ";
    ss << " \"" << TwGetBarName(bar2) << "\"/\"+Z\" readonly=" << val << " ";
    ss << " \"" << TwGetBarName(bar2) << "\"/\"-Z\" readonly=" << val << " ";
}

static void updatePtEnable(const void *value, void *clientData, uint8_t i)
{
    bool val = !(*static_cast<const bool*>(value));
	CCubeShadow* shadow = static_cast<Entity *>(clientData)->get<CCubeShadow>();
    if (shadow != nullptr) {
        auto mask = shadow->getEnableMask();
        shadow->setEnableMask( val? (mask|(1<<i)) : (mask&~(1<<i)));
    }
}
static void readPtEnable(void *value, void *clientData, uint8_t i)
{
	CCubeShadow* shadow = static_cast<Entity *>(clientData)->get<CCubeShadow>();
    makeFaceEnableRO(shadow == nullptr);
    *static_cast<bool *>(value) = !((shadow == nullptr) || ((shadow->getEnableMask() & (1<<i)) != 0));
}

static void TW_CALL updatePtEnableXPos(const void *value, void *clientData) {
    updatePtEnable(value, clientData, 0);
}
static void TW_CALL readPtEnableXPos(void *value, void *clientData) {
    readPtEnable(value, clientData, 0);
}
static void TW_CALL updatePtEnableXNeg(const void *value, void *clientData) {
    updatePtEnable(value, clientData, 1);
}
static void TW_CALL readPtEnableXNeg(void *value, void *clientData) {
    readPtEnable(value, clientData, 1);
}
static void TW_CALL updatePtEnableYPos(const void *value, void *clientData) {
    updatePtEnable(value, clientData, 2);
}
static void TW_CALL readPtEnableYPos(void *value, void *clientData) {
    readPtEnable(value, clientData, 2);
}
static void TW_CALL updatePtEnableYNeg(const void *value, void *clientData) {
    updatePtEnable(value, clientData, 3);
}
static void TW_CALL readPtEnableYNeg(void *value, void *clientData) {
    readPtEnable(value, clientData, 3);
}
static void TW_CALL updatePtEnableZPos(const void *value, void *clientData) {
    updatePtEnable(value, clientData, 5);
}
static void TW_CALL readPtEnableZPos(void *value, void *clientData) {
    readPtEnable(value, clientData, 5);
}
static void TW_CALL updatePtEnableZNeg(const void *value, void *clientData) {
    updatePtEnable(value, clientData, 4);
}
static void TW_CALL readPtEnableZNeg(void *value, void *clientData) {
    readPtEnable(value, clientData, 4);
}

static void TW_CALL updatePtZNear(const void *value, void *clientData)
{
	cam->setPerspective(cam->getFov(), *static_cast<const float *>(value), cam->getZFar());
}
static void TW_CALL updatePtDist(const void *value, void *clientData)
{
    auto radius = *static_cast<const float *>(value);
    if (radius - cam->getZNear() < 0.001) {return;}
	static_cast<CPtLight *>(clientData)->setRadius(radius);
    if (cam != nullptr) {
	    cam->setPerspective(cam->getFov(), cam->getZNear(), radius);
    }
}

static void TW_CALL readPtDist(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<const CPtLight *>(clientData)->getRadius();
}

static std::string s2 = "a STL string";

static void TW_CALL readPtLightName(void *value, void *clientData)
{
	std::string *destPtr = static_cast<std::string *>(value);
	TwCopyStdStringToLibrary(*destPtr, s2);
}

static void TW_CALL updatePtLightName(const void *value, void *clientData)
{
	const std::string *srcPtr = static_cast<const std::string *>(value);
	s2 = *srcPtr;
	CName* name = static_cast<CName *>(clientData);
    name->setName(s2);
    CCubeShadow* shadow = Handle(name).getBrother<CCubeShadow>();
    if (shadow != nullptr) {shadow->resetShadow();}

	AntTWManager::updateLightList();

    if (bar2 != nullptr) {
        std::stringstream ss;
        ss << TwGetBarName(bar2) << " label='Edit - " << s2 << "' "; 
        TwDefine(ss.str().c_str());
    }
}

static void TW_CALL readZNear(void *value, void *clientData)
{
    auto barName = TwGetBarName(bar2);
    //const CCamera* cam = static_cast<const CCamera *>(clientData);
    if (cam != nullptr) {
        std::stringstream ss;
        ss << " \"" << barName << "\"/\"ZNear\" readonly=false ";
        TwDefine(ss.str().c_str());
	    *static_cast<float *>(value) = cam->getZNear();
    } else {
        std::stringstream ss;
        ss << " \"" << barName << "\"/\"ZNear\" readonly=true ";
        TwDefine(ss.str().c_str());
	    *static_cast<float *>(value) = 0;
    }
}

static void TW_CALL updatePtMuted(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CPtLight* item = e->get<CPtLight>();
    const bool val=*static_cast<const bool*>(value);
    item->setEnabled(!val);
}

static void TW_CALL readPtMuted(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CPtLight* item = e->get<CPtLight>();
    *static_cast<bool*>(value) = (item != nullptr) ? !item->isEnabled() : true;
}

static void TW_CALL updateVolMuted(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CVolPtLight* item = e->get<CVolPtLight>();
    const bool val=*static_cast<const bool*>(value);
    item->setEnabled(!val);
}
static void TW_CALL readVolMuted(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CVolPtLight* item = e->get<CVolPtLight>();
    *static_cast<bool*>(value) = !item->isEnabled();
}

static void TW_CALL readPtExport(void *value, void *clientData)
{
    Handle h = Handle::fromRaw(clientData);
    Entity* e = h;
    CPtLight* item = e->get<CPtLight>();
    *static_cast<bool*>(value) = (item != nullptr) ? item->getExport() : false;
}


static void TW_CALL updatePtExport(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CPtLight* item = e->get<CPtLight>();
    const bool val=*static_cast<const bool*>(value);
    item->setExport(val);
}

static void TW_CALL updateDirExport(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    const bool val=*static_cast<const bool*>(value);
    item->setExport(val);
}

static void TW_CALL updateVolExport(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CVolPtLight* item = e->get<CVolPtLight>();
    const bool val=*static_cast<const bool*>(value);
    item->setExport(val);
}

static void TW_CALL readVolExport(void *value, void *clientData)
{
    Handle h = Handle::fromRaw(clientData);
    Entity* e = h;
    CVolPtLight* item = e->get<CVolPtLight>();
    *static_cast<bool*>(value) = item->getExport();
}

static void TW_CALL updatePtDecay(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CPtLight* item = e->get<CPtLight>();
	if (item != nullptr){
		const float val = *static_cast<const float*>(value);
		item->setDecay(val);
	}
}

static void TW_CALL readPtDecay(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CPtLight* item = e->get<CPtLight>();
	if (item != nullptr)
		*static_cast<float*>(value) = (item == nullptr) ? false : item->getDecay();
}
static void TW_CALL updatePtShadowFocus(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);	
	CPtLight* item = e->get<CPtLight>();
	if (item != nullptr){
		const float val = *static_cast<const float*>(value);
		item->setShadowFocus(val);
	}
}

static void TW_CALL readPtShadowFocus(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
	CPtLight* item = e->get<CPtLight>();
	if (item != nullptr)
		*static_cast<float*>(value) = (item == nullptr) ? false : item->getShadowFocus();
	
}

static void TW_CALL updatePtShadowJittering(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CPtLight* item = e->get<CPtLight>();
	if (item != nullptr){
		const float val = *static_cast<const float*>(value);
		item->setShadowJittering(val);
	}
}

static void TW_CALL readPtShadowJittering(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
	CPtLight* item = e->get<CPtLight>();
	if (item != nullptr)
	 *static_cast<float*>(value) = item->getShadowJittering();
}


static void TW_CALL updatePtSpecAmount(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
	CPtLight* item = e->get<CPtLight>();
		if (item != nullptr){
		const float val = *static_cast<const float*>(value);
		item->setSpecularAmountModifier(val);
	}
}
static void TW_CALL readPtSpecAmount(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CPtLight* item = e->get<CPtLight>();
	if (item != nullptr)
		*static_cast<float*>(value) = item->getSpecularAmountModifier();
}
static void TW_CALL updatePtSpecFactor(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CPtLight* item = e->get<CPtLight>();
	if (item != nullptr){
		const float val = *static_cast<const float*>(value);
		item->setSpecularPowerFactor(val);
	}
}
static void TW_CALL readPtSpecFactor(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CPtLight* item = e->get<CPtLight>();
	if (item != nullptr)
		*static_cast<float*>(value) = item->getSpecularPowerFactor();
}

static void TW_CALL updateDirSpecAmount(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    const float val=*static_cast<const float*>(value);
    item->setSpecularAmountModifier(val);
}
static void TW_CALL readDirSpecAmount(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    *static_cast<float*>(value) = item->getSpecularAmountModifier();
}
static void TW_CALL updateDirSpecFactor(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    const float val=*static_cast<const float*>(value);
    item->setSpecularPowerFactor(val);
}
static void TW_CALL readDirSpecFactor(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    *static_cast<float*>(value) = item->getSpecularPowerFactor();
}

static void TW_CALL update_loading_particle_system(const void *value, void *clientData)
{

	int idx = 0;
	editEmitter->removeAll();
	
	int index = *static_cast<const int *>(value);
	std::string nameFileExt = fileList[index];
	int lastindex = (int)nameFileExt.find_last_of(".");
	std::string nameFile = nameFileExt.substr(0, lastindex);
	if (!ParticleSystemManager::get().isEmitterExist(nameFile))
		EmitterParser::get().load(nameFileExt);

	CEmitter::EmitterData emitterData = ParticleSystemManager::get().getEmitter(nameFile);
	editEmitter->setEmitterData(emitterData);


	Handle h(editEmitter);
	Handle owner_h = h.getOwner();

	std::stringstream stream;

	for (int i = 0; i < emitterData.count; i++){

		stream.clear();

		std::string key = (std::string)emitterData.listKeys[i];

		CEmitter::EmitterData::key_t keyEmitter;
		keyEmitter.h = owner_h;
		stream << "emitter_" << i;
		auto keyE = editEmitter->getKey(stream.str());
		editEmitter->load(key, keyEmitter);
	}
	editEmitter->setValid(true);
	indexFileList = index;

}



//storage of particle system
static void TW_CALL read_loading_particle_system(void *value, void *clientData)
{
	*static_cast<int *>(value) = indexFileList;
}

void AntTWManager::selectPointLightTweak(CPtLight* item)
{
    TwDefine((std::string(TwGetBarName(bar))+"/SAVE label='SAVE'").c_str());
    deselect();

	Handle h(item);
	light_h = h;
    Handle e_h = light_h.getOwner();
    void* hraw = e_h.getRawAsVoidPtr();
	currentEntity_h = e_h;
	currentTransform_h = currentEntity_h->get<CTransform>();
	editPtLight = currentEntity_h->get<CPtLight>();
	cam = currentEntity_h->get<CCamera>();
	//Delete the rigidbody in order to edit position
	currentEntity_h->get<CRigidBody>().destroy();
	item->setSelected(true);

	name = currentEntity_h->get<CName>();
	s2 = name->getName();

    if (bar2 != nullptr) {TwDeleteBar(bar2); bar2 = nullptr;}
    std::stringstream ss;
    ss << "Edit - " << s2;
    std::string barName = ss.str().c_str();
	bar2 = TwNewBar("editItem");

	TwAddButton(bar2, "Find Light", CallbackMoveCam, NULL, "key=HOME");
    TwAddButton(bar2, "Move item to camera", CallbackMoveToCam, NULL, "key=END");
	TwAddButton(bar2, "In item position.", CallbackDuplicatePointLight, (void *)(false),
        "key=CTRL+ALT+c group='Duplicate'");
	TwAddButton(bar2, "In camera position.", CallbackDuplicatePointLight, (void *)(true),
        "key=CTRL+c group='Duplicate'");


	TwAddVarCB(bar2, "Name:", TW_TYPE_STDSTRING, updatePtLightName, readPtLightName, name, "");
	TwAddVarCB(bar2, "Exportable", TW_TYPE_BOOLCPP, updatePtExport, readPtExport, hraw, NULL);
	TwAddVarCB(bar2, "MUTE", TW_TYPE_BOOLCPP, updatePtMuted, readPtMuted, hraw, "key=SPACE");

	TwAddVarRW(bar2, "Position", TW_TYPE_DIR3F, &currentTransform_h->refPosition(),
        "opened=true axisx=-x axisz=-z arrowcolor=\"255 0 0\" ");

	TwAddVarCB(bar2, "Decay", TW_TYPE_FLOAT, updatePtDecay, readPtDecay, hraw,
        "min=0 max=1 step=0.05 group='Sphere'");
	TwAddVarCB(bar2, "Radius", TW_TYPE_FLOAT, updatePtDist, readPtDist, editPtLight,
        "min=0.25 step=0.1 group='Sphere'");

	TwAddVarCB(bar2, "Shadow Intensity", TW_TYPE_FLOAT, updatePtShadowIntensity, readPtShadowIntensity,
        editPtLight,"min=0 max=1 step=0.05 group='Shadow'");
    TwAddVarCB(bar2, "ZNear", TW_TYPE_FLOAT, updatePtZNear, readZNear, cam,
        "min=0.0005 step=0.001 group='Shadow'");
	shadowResEType = TwDefineEnum("shadowResEType", NULL, 0);
    TwAddVarCB(bar2, "Shadow quality", shadowResEType, updateCubeShadow, readCubeShadow, currentEntity_h,
        " enum='0 {NONE}, 256 {0.25K}, 512 {0.5K (Default)}, 1024 {1K}, 2048 {2K}, 4096 {4K}' group='Shadow'");	
    TwAddVarCB(bar2, "Shadow focus", TW_TYPE_FLOAT, updatePtShadowFocus, readPtShadowFocus, hraw,
        "min=0 step=0.05 group='Shadow'");
    TwAddVarCB(bar2, "Jittering", TW_TYPE_FLOAT, updatePtShadowJittering, readPtShadowJittering, hraw,
        "min=0 max=1 step=0.05 group='Shadow'");
    
	TwAddVarCB(bar2, "SpecAmount", TW_TYPE_FLOAT, updatePtSpecAmount, readPtSpecAmount, hraw,
        "min=-1 max=1 step=0.05 label='Add amount' group='Specular modifier'");
    TwAddVarCB(bar2, "SpecFactor", TW_TYPE_FLOAT, updatePtSpecFactor, readPtSpecFactor, hraw,
        "min=0 max=5 step=0.01 label='Thinness factor' group='Specular modifier' ");

    TwAddVarCB(bar2, "+X", TW_TYPE_BOOLCPP, updatePtEnableXPos, readPtEnableXPos, currentEntity_h, " group=enableFaces ");
    TwAddVarCB(bar2, "-X", TW_TYPE_BOOLCPP, updatePtEnableXNeg, readPtEnableXNeg, currentEntity_h, " group=enableFaces ");
    TwAddVarCB(bar2, "+Z", TW_TYPE_BOOLCPP, updatePtEnableZPos, readPtEnableZPos, currentEntity_h, " group=enableFaces ");
    TwAddVarCB(bar2, "-Z", TW_TYPE_BOOLCPP, updatePtEnableZNeg, readPtEnableZNeg, currentEntity_h, " group=enableFaces ");
    TwAddVarCB(bar2, "+Y", TW_TYPE_BOOLCPP, updatePtEnableYPos, readPtEnableYPos, currentEntity_h, " group=enableFaces ");
    TwAddVarCB(bar2, "-Y", TW_TYPE_BOOLCPP, updatePtEnableYNeg, readPtEnableYNeg, currentEntity_h, " group=enableFaces ");
	TwAddVarCB(bar2, "Color", TW_TYPE_COLOR32, updatePtColor, readPtColor, editPtLight, "opened=true colororder=rgba colormode=hls ");
	TwAddVarCB(bar2, "Intensity", TW_TYPE_FLOAT, updatePtIntensity, readPtIntensity, hraw,
        "min=0 step=0.01");

	int placeXbar = configX;
	int placeYbar = 0;
	int sizeXbar = 200;
	int sizeYbar = configY;
	std::string str = " \"" + std::string(TwGetBarName(bar2)) + "\" "
        " label='" + barName + "' " +
        " color='20 200 200' " +
        " position='" + std::to_string(placeXbar) + " " + std::to_string(placeYbar) + "' " +
	    " size='" + std::to_string(sizeXbar) + " " + std::to_string(sizeYbar) + "' \n" +
        " \"" + std::string(TwGetBarName(bar2)) + "\"/\"enableFaces\" opened=false label='Shadow faces.'" ;
	TwDefine(str.c_str());
}

static void TW_CALL updateOrthoHeight(const void *value, void *clientData)
{
	static_cast<CCamera *>(clientData)->setOrthographic(cam->getW(), *static_cast<const float *>(value), cam->getZNear(), cam->getZFar());
}

static void TW_CALL readOrthoHeight(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<const CCamera *>(clientData)->getH();
}

static void TW_CALL readDirShadowIntensity(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<const CDirLight *>(clientData)->getShadowIntensity();
}

static void TW_CALL updateDirShadowIntensity(const void *value, void *clientData)
{
    auto l = static_cast<CDirLight *>(clientData);
    auto intensity = *static_cast<const float *>(value);
	l->setShadowIntensity(intensity);
    if (intensity > 0) {
        Entity* e = Handle(l).getOwner();
        if (!e->has<CShadow>()) {
            CShadow* shadow = getManager<CShadow>()->createObj();
            e->add(shadow);
            CCamera* camera = e->get<CCamera>();
            float res = float(shadow->getResolution());
            camera->setViewport(0.f,0.f,res,res);
            shadow->init();
        }
    }
}

static void TW_CALL updateOrthoWidth(const void *value, void *clientData)
{
	static_cast<CCamera *>(clientData)->setOrthographic(*static_cast<const float *>(value), cam->getH(), cam->getZNear(), cam->getZFar());
}

static void TW_CALL readOrthoWidth(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<const CCamera *>(clientData)->getW();
}

static void TW_CALL updateOrthoDist(const void *value, void *clientData)
{
    float zFar = *static_cast<const float *>(value);
    auto cam = static_cast<CCamera *>(clientData);
    CDirLight* item = Handle(cam).getBrother<CDirLight>();
    if (zFar - cam->getZNear() < 0.001) {return;}
    item->setRadius(item->getRadius()/cam->getZFar()*zFar);
	cam->setOrthographic(cam->getW(), cam->getH(), cam->getZNear(), zFar);
}

static void TW_CALL updateOrthoZNear(const void *value, void *clientData)
{
    float zNear = *static_cast<const float *>(value);
    auto cam = static_cast<CCamera *>(clientData);
    if (cam->getZFar() - zNear < 0.001) {return;}
	cam->setOrthographic(cam->getW(), cam->getH(), zNear, cam->getZFar());
}
static void TW_CALL readOrthoDist(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<const CCamera *>(clientData)->getZFar();
}

static void TW_CALL updatePerspZNear(const void *value, void *clientData)
{
    float zNear = *static_cast<const float *>(value);
    auto cam = static_cast<CCamera *>(clientData);
    if (cam->getZFar() - zNear < 0.001) {return;}
	cam->setPerspective(cam->getFov(), zNear, cam->getZFar());
}

static void TW_CALL updatePerspDist(const void *value, void *clientData)
{
    float zFar = *static_cast<const float *>(value);
    auto cam = static_cast<CCamera *>(clientData);
    CDirLight* item = Handle(cam).getBrother<CDirLight>();
    if (zFar - cam->getZNear() < 0.001) {return;}
    item->setRadius(item->getRadius()/cam->getZFar()*zFar);
	cam->setPerspective(cam->getFov(), cam->getZNear(), zFar);
}

static void TW_CALL readPerspDist(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<const CCamera *>(clientData)->getZFar();
}

static void TW_CALL updatePerspFov(const void *value, void *clientData)
{
	static_cast<CCamera *>(clientData)->setPerspective(
        deg2rad(*static_cast<const float *>(value)), cam->getZNear(), cam->getZFar());
}

static void TW_CALL readPerspFov(void *value, void *clientData)
{
	*static_cast<float *>(value) = rad2deg(static_cast<const CCamera *>(clientData)->getFov());
}

static void TW_CALL updateLockLA(const void *value, void *clientData)
{
    static_cast<CDirLight*>(clientData)->setLookAtLocked(*static_cast<const bool *>(value));
}
static void TW_CALL readLockLA(void *value, void *clientData)
{
    bool lock = static_cast<const CDirLight*>(clientData)->getLookAtLocked();
    std::stringstream ss;
    std::string barName = TwGetBarName(bar2);
	*static_cast<bool *>(value) = lock;
	if (lock) {
        ss << " \"" << barName << "\"/\"LookAt\" readonly=true " << std::endl;
        ss << " \"" << barName << "\"/\"LookAt\" arrowcolor=\"127 127 127\" ";
        TwDefine(ss.str().c_str());
    } else {
        ss << " \"" << barName << "\"/\"LookAt\" readonly=false " << std::endl;
        ss << " \"" << barName << "\"/\"LookAt\" arrowcolor=\"255 255 0\" ";
        TwDefine(ss.str().c_str());
    }
}

static void TW_CALL updateLookAt(const void *value, void *clientData)
{
    auto t = static_cast<CTransform *>(clientData);
    if (Handle(t).isValid()) {
	    t->lookAt(*static_cast<const XMVECTOR *>(value));
    }
}

static void TW_CALL readLookAt(void *value, void *clientData)
{
    auto t = static_cast<const CTransform *>(clientData);
    if (Handle(t).isValid()) {
	    *static_cast<XMVECTOR *>(value) = t->getLookAt();
    }
}

static void TW_CALL CallbackAlignXPos(void *clientData)
{
    CTransform* transform(static_cast<CTransform*>(clientData));
	transform->lookAt(transform->getPosition() + xAxis_v);
}
static void TW_CALL CallbackAlignXNeg(void *clientData)
{
    CTransform* transform(static_cast<CTransform*>(clientData));
	transform->lookAt(transform->getPosition() - xAxis_v);
}
static void TW_CALL CallbackAlignYPos(void *clientData)
{
    CTransform* transform(static_cast<CTransform*>(clientData));
	transform->lookAt(transform->getPosition() + yAxis_v);
}
static void TW_CALL CallbackAlignYNeg(void *clientData)
{
    CTransform* transform(static_cast<CTransform*>(clientData));
	transform->lookAt(transform->getPosition() - yAxis_v);
}
static void TW_CALL CallbackAlignZPos(void *clientData)
{
    CTransform* transform(static_cast<CTransform*>(clientData));
	transform->lookAt(transform->getPosition() + zAxis_v);
}
static void TW_CALL CallbackAlignZNeg(void *clientData)
{
    CTransform* transform(static_cast<CTransform*>(clientData));
	transform->lookAt(transform->getPosition() - zAxis_v);
}

static void TW_CALL updateFront(const void *value, void *clientData)
{
    CTransform* camT = App::get().getCamera().getSon<CTransform>();
    XMVECTOR front = XMVector3Rotate(*static_cast<const XMVECTOR *>(value), camT->getRotation());
    CTransform* transform(static_cast<CTransform*>(clientData));
	transform->lookAt(transform->getPosition() + front);
}

static void TW_CALL readFront(void *value, void *clientData)
{
    CTransform* camT = App::get().getCamera().getSon<CTransform>();
    *static_cast<XMVECTOR *>(value) = XMVector3Rotate(
        static_cast<const CTransform *>(clientData)->getFront(),
        XMQuaternionInverse(camT->getRotation()));
}


static void TW_CALL updatePos(const void *value, void *clientData)
{
    auto transform = static_cast<CTransform *>(clientData);
    auto pos = *static_cast<const XMVECTOR *>(value);

    Handle h(transform);
    if (h.hasBrother<CDirLight>() && ((CDirLight*)h.getBrother<CDirLight>())->getLookAtLocked()) {
        auto lookAtDiff = transform->getLookAt() - transform->getPosition();
	    transform->setPosition(pos);
	    transform->lookAt(pos + lookAtDiff);
    } else {
	    transform->setPosition(pos);
	    transform->lookAt(transform->getLookAt());
    }
}

static void TW_CALL readPos(void *value, void *clientData)
{
	*static_cast<XMVECTOR *>(value) = static_cast<const CTransform *>(clientData)->getPosition();
}

static void TW_CALL updateShadow(const void *value, void *clientData)
{
    auto res = *static_cast<const int *>(value);
    if (res > 0) {
	    CShadow* shadow = static_cast<Entity *>(clientData)->get<CShadow>();
        shadow->setResolution(res);
        CCamera* camera = Handle(shadow).getBrother<CCamera>();
        camera->setViewport(0.f,0.f,float(res),float(res));
    }
}

static void TW_CALL readShadow(void *value, void *clientData)
{
	CShadow* shadow = static_cast<Entity *>(clientData)->get<CShadow>();
    auto barName = TwGetBarName(bar2);
    if (shadow != nullptr) {
        std::stringstream ss;
        ss << " \"" << barName << "\"/\"Shadow quality\" readonly=false ";
        TwDefine(ss.str().c_str());
	    *static_cast<int *>(value) = shadow->getResolution();
    } else {
        std::stringstream ss;
        ss << " \"" << barName << "\"/\"Shadow quality\" readonly=true ";
        TwDefine(ss.str().c_str());
	    *static_cast<int *>(value) = 0;
    }
}

static void TW_CALL updateDirColor(const void *value, void *clientData)
{
	Color col = *static_cast<const Color *>(value);
	Color t(XMVectorSet(col.af(), col.bf(), col.gf(), col.rf()));
	static_cast<CDirLight *>(clientData)->setColor(t);
}

static void TW_CALL readDirColor(void *value, void *clientData)
{
	Color c = static_cast<const CDirLight *>(clientData)->getColor();
	Color t;
	t.set(XMVectorSet(c.af(), c.bf(), c.gf(), c.rf()));
	*static_cast<Color *>(value) = t;
}

std::string s3 = "a STL string";

static void TW_CALL readLightName(void *value, void *clientData)
{		
	std::string *destPtr = static_cast<std::string *>(value);
	TwCopyStdStringToLibrary(*destPtr, s3);
}

static void TW_CALL updateDirRadius(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    const float val=*static_cast<const float*>(value);
    item->setRadius(val);
}

static void TW_CALL readDirRadius(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    *static_cast<float*>(value) = item->getRadius();
}

static void TW_CALL updateDirDecay(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    const float val=*static_cast<const float*>(value);
    item->setDecay(val);
}

static void TW_CALL readDirDecay(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    *static_cast<float*>(value) = item->getDecay();
}

static void TW_CALL updateDirSpotDecay(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    const float val=*static_cast<const float*>(value);
    item->setSpotDecay(val);
}

static void TW_CALL readDirSpotDecay(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    *static_cast<float*>(value) = item->getSpotDecay();
}

static void TW_CALL updateDirShadowFocus(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    const float val=*static_cast<const float*>(value);
    item->setShadowFocus(val);
}

static void TW_CALL readDirShadowFocus(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    *static_cast<float*>(value) = item->getShadowFocus();
}

static void TW_CALL updateDirMuted(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    const bool val=*static_cast<const bool*>(value);
    item->setEnabled(!val);
}

static void TW_CALL readDirMuted(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    *static_cast<bool*>(value) = !item->isEnabled();
}

static void TW_CALL readDirExport(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    *static_cast<bool*>(value) = item->getExport();
}

static void TW_CALL updateDirIsSpotlight(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    const bool val=*static_cast<const bool*>(value);
    item->setSpotlight(val);
}

static void TW_CALL readDirIsSpotlight(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    *static_cast<bool*>(value) = item->isSpotlight();
}

static void TW_CALL updateDirShadowJittering(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    const float val=*static_cast<const float*>(value);
    item->setShadowJittering(val);
}

static void TW_CALL readDirShadowJittering(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    *static_cast<float*>(value) = item->getShadowJittering();
}

static void TW_CALL updateDirIntensity(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    const float val=*static_cast<const float*>(value);
    item->setIntensity(val);
}

static void TW_CALL readDirIntensity(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CDirLight* item = e->get<CDirLight>();
    *static_cast<float*>(value) = item->getIntensity();
}

static void TW_CALL updateLightName(const void *value, void *clientData)
{ 
    const std::string *srcPtr = static_cast<const std::string *>(value);
    s3 = *srcPtr;
	CName* name = static_cast<CName *>(clientData);
    name->setName(s3);
    CCubeShadow* shadow = Handle(name).getBrother<CCubeShadow>();
    if (shadow != nullptr) {shadow->resetShadow();}

    AntTWManager::updateLightList();

    if (bar2 != nullptr) {
        std::stringstream ss;
        ss << TwGetBarName(bar2) << " label='Edit - " << s3 << "' "; 
        TwDefine(ss.str().c_str());
    }
}



void AntTWManager::selectDirectionalLightTweak(CDirLight* item)
{
    TwDefine((std::string(TwGetBarName(bar))+"/SAVE label='SAVE'").c_str());
    deselect();

	Handle h(item);
	light_h = h;
    Handle e_h = light_h.getOwner();
	currentEntity_h = e_h;
	currentTransform_h = currentEntity_h->get<CTransform>();
	editDirLight = currentEntity_h->get<CDirLight>();
	cam = currentEntity_h->get<CCamera>();

	name = currentEntity_h->get<CName>();
	s3 = name->getName();

    if (bar2 != nullptr) {TwDeleteBar(bar2); bar2=nullptr;}
    std::stringstream ss;
    ss << "Edit - " << s3;
    std::string barName = ss.str().c_str();
	bar2 = TwNewBar("editItem");
    
    void* hraw = e_h.getRawAsVoidPtr();

	//Delete the rigidbody in order to edit position
	currentEntity_h->get<CRigidBody>().destroy();
	item->setSelected(true);
    
    TwAddButton(bar2, "Find item", CallbackMoveCam, NULL, "key=HOME");
    TwAddButton(bar2, "Move item to camera", CallbackMoveToCam, NULL, "key=END");
	
	TwAddButton(bar2, "In item position.", CallbackDuplicateDirLight, (void *)(false),
        "key=CTRL+ALT+c group='Duplicate'");
	TwAddButton(bar2, "In camera position.", CallbackDuplicateDirLight, (void *)(true),
        "key=CTRL+c group='Duplicate'");

	TwAddVarCB(bar2, "Name:", TW_TYPE_STDSTRING, updateLightName, readLightName, name, "");
	TwAddVarCB(bar2, "Exportable", TW_TYPE_BOOLCPP, updateDirExport, readDirExport, hraw, NULL);
	TwAddVarCB(bar2, "MUTE", TW_TYPE_BOOLCPP, updateDirMuted, readDirMuted, hraw, "key=SPACE");

	TwAddVarCB(bar2, "Position", TW_TYPE_DIR3F, updatePos, readPos, currentTransform_h,
        "opened=true axisx=-x axisz=-z arrowcolor=\"255 0 0\" group='Transform'");
	TwAddVarCB(bar2, "LookAt", TW_TYPE_DIR3F, updateLookAt, readLookAt, currentTransform_h,
        "opened=true axisx=-x axisz=-z arrowcolor=\"255 255 0\" group='Transform'");
    TwAddVarCB(bar2, "Lock LookAt", TW_TYPE_BOOLCPP, updateLockLA, readLockLA, editDirLight, "");
	TwAddVarCB(bar2, "Front", TW_TYPE_DIR3F, updateFront, readFront, currentTransform_h,
        "opened=true axisx=-x axisz=-z arrowcolor=\"0 255 0\" group='Transform'");

    TwAddButton(bar2, "+X", CallbackAlignXPos, currentTransform_h, " group=align ");
    TwAddButton(bar2, "-X", CallbackAlignXNeg, currentTransform_h, " group=align ");
    TwAddButton(bar2, "+Z", CallbackAlignZPos, currentTransform_h, " group=align ");
    TwAddButton(bar2, "-Z", CallbackAlignZNeg, currentTransform_h, " group=align ");
    TwAddButton(bar2, "+Y", CallbackAlignYPos, currentTransform_h, " group=align ");
    TwAddButton(bar2, "-Y", CallbackAlignYNeg, currentTransform_h, " group=align ");
    
	if (cam->isOrthographic()){
		TwAddVarCB(bar2, "Width", TW_TYPE_FLOAT, updateOrthoWidth, readOrthoWidth, cam,
            "min=0.1 step=0.1 group='Ortographic'");
		TwAddVarCB(bar2, "Height", TW_TYPE_FLOAT, updateOrthoHeight, readOrthoHeight, cam,
            "min=0.1 step=0.1 group='Ortographic'");
		TwAddVarCB(bar2, "ZNear", TW_TYPE_FLOAT, updateOrthoZNear, readZNear, cam,
            "min=0.0005 step=0.001 group='Ortographic'");
		TwAddVarCB(bar2, "Distance", TW_TYPE_FLOAT, updateOrthoDist, readOrthoDist, cam,
            "min=0.4 step=0.1 group='Ortographic'");
	} else {
		TwAddVarCB(bar2, "FOV", TW_TYPE_FLOAT, updatePerspFov, readPerspFov, cam,
            "min=1 max=179 step=1 group='Perspective'");
		TwAddVarCB(bar2, "ZNear", TW_TYPE_FLOAT, updatePerspZNear, readZNear, cam,
            "min=0.0005 step=0.001 group='Perspective'");
		TwAddVarCB(bar2, "Distance", TW_TYPE_FLOAT, updatePerspDist, readPerspDist, cam,
            "min=0.4 step=0.1 group='Perspective'");
	}

	TwAddVarCB(bar2, "Radius (from item)", TW_TYPE_FLOAT, updateDirRadius, readDirRadius, hraw,
        "min=1 step=0.05 group='Snowcone'");
    TwAddVarCB(bar2, "Decay", TW_TYPE_FLOAT, updateDirDecay, readDirDecay, hraw,
        "min=0 max=1 step=0.05 group='Snowcone'");

	TwAddVarCB(bar2, "Color", TW_TYPE_COLOR32, updateDirColor, readDirColor, editDirLight,
        "opened=true colororder=rgba colormode=hls ");
	TwAddVarCB(bar2, "Intensity", TW_TYPE_FLOAT, updateDirIntensity, readDirIntensity, hraw,
        "min=0 step=0.01");
	TwAddVarCB(bar2, "SpecAmount", TW_TYPE_FLOAT, updateDirSpecAmount, readDirSpecAmount, hraw,
        "min=-1 max=1 step=0.05 label='Add amount' group='Specular modifier'");
    TwAddVarCB(bar2, "SpecFactor", TW_TYPE_FLOAT, updateDirSpecFactor, readDirSpecFactor, hraw,
        "min=0 max=5 step=0.01 label='Thinness factor' group='Specular modifier' ");

	TwAddVarCB(bar2, "Shadow Intensity", TW_TYPE_FLOAT, updateDirShadowIntensity, readDirShadowIntensity,
        editDirLight, "min=0 max=1 step=0.05 group='Shadow'");
	shadowResEType = TwDefineEnum("shadowResEType", NULL, 0);
	TwAddVarCB(bar2, "Shadow quality", shadowResEType, updateShadow, readShadow, currentEntity_h,
        " enum='0 {NONE}, 256 {0.25K}, 512 {0.5K (Default)}, 1024 {1K}, 2048 {2K}, 4096 {4K}' "
        " group='Shadow'");
    TwAddVarCB(bar2, "Shadow focus", TW_TYPE_FLOAT, updateDirShadowFocus, readDirShadowFocus, hraw,
        "min=0 step=0.05 group='Shadow'");
    TwAddVarCB(bar2, "Jittering", TW_TYPE_FLOAT, updateDirShadowJittering, readDirShadowJittering, hraw,
        "min=0 max=1 step=0.05 group='Shadow'");

	TwAddVarCB(bar2, "Spotlight?", TW_TYPE_BOOL16, updateDirIsSpotlight, readDirIsSpotlight, hraw,
        "true=Yes false=No group='SpotLight'");
	TwAddButton(bar2, "infospotdecay", NULL, NULL, " label='Only works with spotlight' ");
	TwAddVarCB(bar2, "Spot Decay", TW_TYPE_FLOAT, updateDirSpotDecay, readDirSpotDecay, hraw,
        "min=0 max=1 step=0.05 group='SpotLight'");

	int placeXbar = configX;
	int placeYbar = 0;
	int sizeXbar = 200;
	int sizeYbar = configY;
	std::string str = " \"" + std::string(TwGetBarName(bar2)) + "\"" +
        " label='" + barName + "' " +
        (!cam->isOrthographic() ? " color='200 20 200' " : " color='50 100 200' ") +
        " position='" + std::to_string(placeXbar) + " " + std::to_string(placeYbar) + "' " +
        " size='" + std::to_string(sizeXbar) + " " + std::to_string(sizeYbar) + "' " +
        "\n" +
        " \"" + std::string(TwGetBarName(bar2)) + "\"/align opened=false label='Align to axes'" ;
	const char * c = str.c_str();
	TwDefine(c);
}


void AntTWManager::cleanupLightTool()
{
    if (bar  != nullptr) {TwDeleteBar(bar);  bar  = nullptr;}
    if (bar2 != nullptr) {TwDeleteBar(bar2); bar2 = nullptr;}
}

static void TW_CALL readName(void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    TwCopyStdStringToLibrary(*static_cast<std::string *>(value),e->getName());
}
  
static void TW_CALL updateName(const void *value, void *clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CName* name = e->get<CName>();
    name->setName(*static_cast<const std::string *>(value));
}

static void TW_CALL readVolColor(void *value, void *clientData) 
{                                                               
    Entity* e = Handle::fromRaw(clientData);                    
    CVolPtLight* volLight = e->get<CVolPtLight>();                  
    assert(volLight != nullptr);                                
    *static_cast<Color*>(value) = volLight->getColor().abgr();          
}
  
static void TW_CALL updateVolColor(const void *value, void *clientData) 
{                                                                       
    Entity* e = Handle::fromRaw(clientData);                            
    CVolPtLight* volLight = e->get<CVolPtLight>();                          
    assert(volLight != nullptr);                                        
    volLight->setColor(static_cast<const Color*>(value)->abgr());               
}
 
#define DECLARE_READ_VOL(Att, type) \
static void TW_CALL readVol##Att(void *value, void *clientData) \
{                                                               \
    Entity* e = Handle::fromRaw(clientData);                    \
    CVolPtLight* volLight = e->get<CVolPtLight>();                  \
    assert(volLight != nullptr);                                \
    *static_cast<type*>(value) = volLight->get##Att();          \
}
  
#define DECLARE_UPDATE_VOL(Att, type) \
static void TW_CALL updateVol##Att(const void *value, void *clientData) \
{                                                                       \
    Entity* e = Handle::fromRaw(clientData);                            \
    CVolPtLight* volLight = e->get<CVolPtLight>();                          \
    assert(volLight != nullptr);                                        \
    volLight->set##Att(*static_cast<const type*>(value));               \
}
DECLARE_READ_VOL(Density, float)
DECLARE_UPDATE_VOL(Density, float)
DECLARE_READ_VOL(Weight, float)
DECLARE_UPDATE_VOL(Weight, float)
DECLARE_READ_VOL(RayDecay, float)
DECLARE_UPDATE_VOL(RayDecay, float)
DECLARE_READ_VOL(Decay, float)
DECLARE_UPDATE_VOL(Decay, float)
DECLARE_READ_VOL(OccludedAddend, float)
DECLARE_UPDATE_VOL(OccludedAddend, float)
DECLARE_READ_VOL(IlluminatedAddend, float)
DECLARE_UPDATE_VOL(IlluminatedAddend, float)
DECLARE_READ_VOL(Radius, float)
DECLARE_UPDATE_VOL(Radius, float)
DECLARE_READ_VOL(NormalShadeMin, float)
DECLARE_UPDATE_VOL(NormalShadeMin, float)
DECLARE_READ_VOL(MaxSamples, unsigned)
DECLARE_UPDATE_VOL(MaxSamples, unsigned)

void AntTWManager::selectVolLightTweak(CVolPtLight* item)
{
    TwDefine((std::string(TwGetBarName(bar))+"/SAVE label='SAVE'").c_str());
    deselect();

	Handle h(item);
	light_h = h;
    Handle e_h = light_h.getOwner();
    void* hraw = e_h.getRawAsVoidPtr();
	currentEntity_h = e_h;
	currentTransform_h = currentEntity_h->get<CTransform>();
	editVolLight = currentEntity_h->get<CVolPtLight>();

	//Delete the rigidbody in order to edit position
	currentEntity_h->get<CRigidBody>().destroy();
	item->setSelected(true);

	name = currentEntity_h->get<CName>();

    if (bar2 != nullptr) {TwDeleteBar(bar2); bar2 = nullptr;}
    std::stringstream ss;
    ss << "Edit - " << name->getName();
    std::string barName = ss.str().c_str();
	bar2 = TwNewBar("editItem");


    auto ePtr = e_h.getRawAsVoidPtr();

	TwAddButton(bar2, "Find Light", CallbackMoveCam, NULL, "key=HOME");
    TwAddButton(bar2, "Move item to camera", CallbackMoveToCam, NULL, "key=END");
		TwAddButton(bar2, "In item position.", CallbackDuplicateVolLight, (void *)(false),
            "key=CTRL+ALT+c group='Duplicate'");
		TwAddButton(bar2, "In camera position.", CallbackDuplicateVolLight, (void *)(true),
            "key=CTRL+c group='Duplicate'");

	TwAddSeparator(bar2, NULL, NULL);
	TwAddVarCB(bar2, "Name:", TW_TYPE_STDSTRING, updateName, readName, ePtr, NULL);
	TwAddVarCB(bar2, "Exportable", TW_TYPE_BOOLCPP, updateVolExport, readVolExport, hraw, NULL);
	TwAddVarCB(bar2, "MUTE", TW_TYPE_BOOLCPP, updateVolMuted, readVolMuted, hraw, "key=SPACE");

	TwAddSeparator(bar2, NULL, NULL);
	TwAddVarRW(bar2, "Position", TW_TYPE_DIR3F, &currentTransform_h->refPosition(),
        "opened=true axisx=-x axisz=-z arrowcolor=\"255 0 0\" ");

	TwAddSeparator(bar2, NULL, NULL);
	TwAddVarCB(bar2, "Color", TW_TYPE_COLOR32, updateVolColor, readVolColor, ePtr,
        "opened=true coloralpha=true colororder=rgba colormode=hls group='Light'");
	TwAddVarCB(bar2, "Weight", TW_TYPE_FLOAT, updateVolWeight, readVolWeight, ePtr,
        "min=-2 max=2 step=0.01 group='Light'");
	TwAddVarCB(bar2, "NormalShadeMin", TW_TYPE_FLOAT, updateVolNormalShadeMin, readVolNormalShadeMin, ePtr,
        "label='Normal shade compensation.' min=-1 max=1 step=0.01 group='Light'");
	TwAddVarCB(bar2, "Radius", TW_TYPE_FLOAT, updateVolRadius, readVolRadius, ePtr,
        "min=0.003 step=0.1 group='Sphere'");
	TwAddVarCB(bar2, "Decay", TW_TYPE_FLOAT, updateVolDecay, readVolDecay, ePtr,
        "min=0 max=1 step=0.01 group='Sphere'");
	TwAddVarCB(bar2, "RayDecay", TW_TYPE_FLOAT, updateVolRayDecay, readVolRayDecay, ePtr,
        "label='Ray decay' min=0 step=0.001 group='Ray'");
	TwAddVarCB(bar2, "Density", TW_TYPE_FLOAT, updateVolDensity, readVolDensity, ePtr,
        "min=0.003 step=0.001 group='Ray'");
	TwAddVarCB(bar2, "MaxSamples", TW_TYPE_UINT32, updateVolMaxSamples, readVolMaxSamples, ePtr,
        "label='Max samples' min=20 max=500 step=5 group='Ray'");
	TwAddVarCB(bar2, "Occlusion", TW_TYPE_FLOAT, updateVolOccludedAddend, readVolOccludedAddend, ePtr,
        "min=-5 max=5 step=0.01 group='Ray'");
	TwAddVarCB(bar2, "Lightness", TW_TYPE_FLOAT, updateVolIlluminatedAddend, readVolIlluminatedAddend, ePtr,
        "min=-5 max=5 step=0.01 group='Ray'");

	int placeXbar = configX;
	int placeYbar = 0;
	int sizeXbar = 200;
	int sizeYbar = configY;
	std::string str = " \"" + std::string(TwGetBarName(bar2)) + "\" "
        " label='" + barName + "' " +
        " color='200 100 20' " +
        " position='" + std::to_string(placeXbar) + " " + std::to_string(placeYbar) + "' " +
	    " size='" + std::to_string(sizeXbar) + " " + std::to_string(sizeYbar) + "' \n" +
        " \"" + std::string(TwGetBarName(bar2)) + "\"/\"Light\" opened=true \n" +
        " \"" + std::string(TwGetBarName(bar2)) + "\"/\"Sphere\" opened=true \n" +
        " \"" + std::string(TwGetBarName(bar2)) + "\"/\"Ray\" opened=true \n"
        ;
	TwDefine(str.c_str());
}

#define DECLARE_READ_MIST(Att, type)                             \
static void TW_CALL readMist##Att(void *value, void *clientData) \
{                                                                \
    Entity* e = Handle::fromRaw(clientData);                     \
    CMist* m = e->get<CMist>();                                  \
    assert(m != nullptr);                                        \
    *static_cast<type*>(value) = m->get##Att();                  \
}
  
#define DECLARE_UPDATE_MIST(Att, type)                                   \
static void TW_CALL updateMist##Att(const void *value, void *clientData) \
{                                                                        \
    Entity* e = Handle::fromRaw(clientData);                             \
    CMist* m = e->get<CMist>();                                          \
    assert(m != nullptr);                                                \
    m->set##Att(*static_cast<const type*>(value));                       \
}

#define DECLARE_READ_MIST_COLOR(Att)                             \
static void TW_CALL readMist##Att(void *value, void *clientData) \
{                                                                \
    Entity* e = Handle::fromRaw(clientData);                     \
    CMist* m = e->get<CMist>();                                  \
    assert(m != nullptr);                                        \
    *static_cast<Color*>(value) = m->get##Att().abgr();           \
}
 
#define DECLARE_UPDATE_MIST_COLOR(Att)                                   \
static void TW_CALL updateMist##Att(const void *value, void *clientData) \
{                                                                        \
    Entity* e = Handle::fromRaw(clientData);                             \
    CMist* m = e->get<CMist>();                                          \
    assert(m != nullptr);                                                \
    m->set##Att(static_cast<const Color*>(value)->abgr());               \
}

DECLARE_READ_MIST_COLOR(ColorTop)
DECLARE_READ_MIST_COLOR(ColorBottom)
DECLARE_READ_MIST(LayerDecay, float)
DECLARE_READ_MIST(DeltaX, float)
DECLARE_READ_MIST(DeltaZ, float)
DECLARE_READ_MIST(Height, float)
DECLARE_READ_MIST(Width, float)
DECLARE_READ_MIST(Length, float)
DECLARE_READ_MIST(Factor, float)
DECLARE_READ_MIST(Minimun, float)
DECLARE_READ_MIST(DarkenAlpha, float)
DECLARE_READ_MIST(Chaotic, bool)
DECLARE_READ_MIST(UnitWorldSize, float)
DECLARE_READ_MIST(Intensity, float)
DECLARE_READ_MIST(Export, bool)
DECLARE_READ_MIST(Muted, bool)
DECLARE_READ_MIST(SqSqrt, float)
DECLARE_READ_MIST(DepthTolerance, float)
DECLARE_READ_MIST(Spin, float)

DECLARE_UPDATE_MIST_COLOR(ColorTop)
DECLARE_UPDATE_MIST_COLOR(ColorBottom)
DECLARE_UPDATE_MIST(LayerDecay, float)
DECLARE_UPDATE_MIST(DeltaX, float)
DECLARE_UPDATE_MIST(DeltaZ, float)
DECLARE_UPDATE_MIST(Height, float)
DECLARE_UPDATE_MIST(Width, float)
DECLARE_UPDATE_MIST(Length, float)
DECLARE_UPDATE_MIST(Factor, float)
DECLARE_UPDATE_MIST(Minimun, float)
DECLARE_UPDATE_MIST(DarkenAlpha, float)
DECLARE_UPDATE_MIST(Chaotic, bool)
DECLARE_UPDATE_MIST(UnitWorldSize, float)
DECLARE_UPDATE_MIST(Intensity, float)
DECLARE_UPDATE_MIST(Export, bool)
DECLARE_UPDATE_MIST(Muted, bool)
DECLARE_UPDATE_MIST(SqSqrt, float)
DECLARE_UPDATE_MIST(DepthTolerance, float)
DECLARE_UPDATE_MIST(Spin, float)

void TW_CALL mistCopyBottomToTop(void* clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CMist* m = e->get<CMist>();
    m->setColorTop(m->getColorBottom());
}

void TW_CALL mistCopyTopToBottom(void* clientData)
{
    Entity* e = Handle::fromRaw(clientData);
    CMist* m = e->get<CMist>();
    m->setColorBottom(m->getColorTop());
}

void AntTWManager::selectMistTweak(CMist* mist)
{
    TwDefine((std::string(TwGetBarName(bar))+"/SAVE label='SAVE'").c_str());
    deselect();

	Handle h(mist);
	light_h = h;
    Handle e_h = h.getOwner();
    void* hraw = e_h.getRawAsVoidPtr();
	currentEntity_h = e_h;
	currentTransform_h = currentEntity_h->get<CTransform>();
	editMist = mist;

	//Delete the rigidbody in order to edit position
	currentEntity_h->get<CRigidBody>().destroy();
	mist->setSelected(true);

	name = currentEntity_h->get<CName>();

    if (bar2 != nullptr) {TwDeleteBar(bar2); bar2 = nullptr;}
    std::stringstream ss;
    ss << "Edit - " << name->getName();
    std::string barName = ss.str().c_str();
	bar2 = TwNewBar("editItem");

    auto ePtr = e_h.getRawAsVoidPtr();

	TwAddButton(bar2, "Find mist", CallbackMoveCam, NULL, "key=HOME");
    TwAddButton(bar2, "Move mist to camera", CallbackMoveToCam, NULL, "key=END");
		TwAddButton(bar2, "In mist position.", CallbackDuplicateMist, (void *)(false),
            "key=CTRL+ALT+c group='Duplicate'");
		TwAddButton(bar2, "In camera position.", CallbackDuplicateMist, (void *)(true),
            "key=CTRL+c group='Duplicate'");

	TwAddSeparator(bar2, NULL, NULL);
	TwAddVarCB(bar2, "Name:", TW_TYPE_STDSTRING, updateName, readName, ePtr, NULL);
	TwAddVarCB(bar2, "Exportable", TW_TYPE_BOOLCPP, updateMistExport, readMistExport, hraw, NULL);
	TwAddVarCB(bar2, "MUTE", TW_TYPE_BOOLCPP, updateMistMuted, readMistMuted, hraw, "key=SPACE");

	TwAddSeparator(bar2, NULL, NULL);
	TwAddVarRW(bar2, "Position", TW_TYPE_DIR3F, &currentTransform_h->refPosition(),
        "opened=true axisx=-x axisz=-z arrowcolor=\"255 0 0\" group='Dimensions'");
	TwAddVarCB(bar2, "Width", TW_TYPE_FLOAT, updateMistWidth, readMistWidth, ePtr,
        "min=0.1 step=0.01 group='Dimensions'");
	TwAddVarCB(bar2, "Length", TW_TYPE_FLOAT, updateMistLength, readMistLength, ePtr,
        "min=0.1 step=0.01 group='Dimensions'");
	TwAddVarCB(bar2, "Height", TW_TYPE_FLOAT, updateMistHeight, readMistHeight, ePtr,
        "min=0.001 step=0.001 group='Dimensions'");
	TwAddVarCB(bar2, "Texture size", TW_TYPE_FLOAT, updateMistUnitWorldSize, readMistUnitWorldSize, ePtr,
        "min=0.01 step=0.01 group='Dimensions'");

	TwAddSeparator(bar2, NULL, NULL);
	TwAddVarCB(bar2, "Top", TW_TYPE_COLOR32, updateMistColorTop, readMistColorTop, ePtr,
        "opened=true coloralpha=false colororder=rgba colormode=hls group='Color'");
    TwAddButton(bar2, "Top := Bottom", mistCopyBottomToTop, ePtr, "group='Color'");
	TwAddVarCB(bar2, "Bottom", TW_TYPE_COLOR32, updateMistColorBottom, readMistColorBottom, ePtr,
        "opened=true coloralpha=false colororder=rgba colormode=hls group='Color'");
    TwAddButton(bar2, "Botom := Top", mistCopyTopToBottom, ePtr, "group='Color'");
	TwAddVarCB(bar2, "Layer decay", TW_TYPE_FLOAT, updateMistLayerDecay, readMistLayerDecay, ePtr,
        "min=-1 max=1 step=0.001 group='Color'");
	TwAddVarCB(bar2, "Darken alpha", TW_TYPE_FLOAT, updateMistDarkenAlpha, readMistDarkenAlpha, ePtr,
        "min=0 max=5 step=0.001 group='Color'");
    
    TwAddVarCB(bar2, "Dispersion", TW_TYPE_FLOAT, updateMistSqSqrt, readMistSqSqrt, ePtr,
        "min=-1 max=1 step=0.001 group='Appearance'");
    TwAddButton(bar2, "Formula", nullptr, nullptr,
        "label='mist= (tex*f + min)*a' group='Appearance'");
	TwAddVarCB(bar2, "Factor", TW_TYPE_FLOAT, updateMistFactor, readMistFactor, ePtr,
        "min=0 max=1 step=0.001 group='Appearance'");
	TwAddVarCB(bar2, "Minimun", TW_TYPE_FLOAT, updateMistMinimun, readMistMinimun, ePtr,
        "min=0 max=1 step=0.001 group='Appearance'");
	TwAddVarCB(bar2, "Alpha", TW_TYPE_FLOAT, updateMistIntensity, readMistIntensity, ePtr,
        "min=0 max=1 step=0.001 group='Appearance'");
	TwAddVarCB(bar2, "Depth tolerance", TW_TYPE_FLOAT, updateMistDepthTolerance, readMistDepthTolerance, ePtr,
        "min=0 step=1 group='Appearance'");

	TwAddVarCB(bar2, "Chaotic", TW_TYPE_BOOLCPP, updateMistChaotic, readMistChaotic, ePtr,
        "group='Movement'");
	TwAddVarCB(bar2, "DeltaX", TW_TYPE_FLOAT, updateMistDeltaX, readMistDeltaX, ePtr,
        "step=0.01 group='Movement'");
	TwAddVarCB(bar2, "DeltaZ", TW_TYPE_FLOAT, updateMistDeltaZ, readMistDeltaZ, ePtr,
        "step=0.01 group='Movement'");
	TwAddVarCB(bar2, "Rotation", TW_TYPE_FLOAT, updateMistSpin, readMistSpin, ePtr,
        "step=0.01 group='Movement'");

	int placeXbar = configX;
	int placeYbar = 0;
	int sizeXbar = 200;
	int sizeYbar = configY;
	std::string str = " \"" + std::string(TwGetBarName(bar2)) + "\" "
        " label='" + barName + "' " +
        " color='20 200 60' " +
        " position='" + std::to_string(placeXbar) + " " + std::to_string(placeYbar) + "' " +
	    " size='" + std::to_string(sizeXbar) + " " + std::to_string(sizeYbar) + "' \n" +
        " \"" + std::string(TwGetBarName(bar2)) + "\"/\"Dimensions\" opened=true \n" +
        " \"" + std::string(TwGetBarName(bar2)) + "\"/\"Movement\" opened=true \n" +
        " \"" + std::string(TwGetBarName(bar2)) + "\"/\"Appearance\" opened=true \n" +
        " \"" + std::string(TwGetBarName(bar2)) + "\"/\"Color\" opened=true \n"
        ;
	TwDefine(str.c_str());
}


#define DECLARE_READ_EMITTER(Att, type)                          \
static void TW_CALL readEmitter##Att(void *value, void *clientData) \
{                                                                \
    Entity* e = Handle::fromRaw(clientData);                     \
    CEmitter* m = e->get<CEmitter>();                                  \
    assert(m != nullptr);                                        \
    *static_cast<type*>(value) = m->get##Att();                  \
}

#define DECLARE_UPDATE_EMITTER(Att, type)                                \
static void TW_CALL updateEmitter##Att(const void *value, void *clientData) \
{                                                                        \
    Entity* e = Handle::fromRaw(clientData);                             \
    CEmitter* m = e->get<CEmitter>();                                          \
    assert(m != nullptr);                                                \
    m->set##Att(*static_cast<const type*>(value));                       \
}

DECLARE_READ_EMITTER(Export, bool)
DECLARE_UPDATE_EMITTER(Export, bool)

void AntTWManager::selectEmitterTweak(CEmitter* emitter)
{
	TwDefine((std::string(TwGetBarName(bar)) + "/SAVE label='SAVE'").c_str());
	deselect();

	Handle h(emitter);
	light_h = h;
	Handle e_h = h.getOwner();
	void* hraw = e_h.getRawAsVoidPtr();
	currentEntity_h = e_h;
	currentTransform_h = currentEntity_h->get<CTransform>();
	editEmitter = emitter;

	//Delete the rigidbody in order to edit position
	currentEntity_h->get<CRigidBody>().destroy();
	emitter->setSelected(true);

	name = currentEntity_h->get<CName>();

	if (bar2 != nullptr) { TwDeleteBar(bar2); bar2 = nullptr; }
	std::stringstream ss;
	ss << "Edit - " << name->getName();
	std::string barName = ss.str().c_str();
	bar2 = TwNewBar("editItem");

	auto ePtr = e_h.getRawAsVoidPtr();

	TwAddButton(bar2, "Find emitter", CallbackMoveCam, NULL, "key=HOME");
	TwAddButton(bar2, "Move emitter to camera", CallbackMoveToCam, NULL, "key=END");
	TwAddButton(bar2, "In emitter position.", CallbackDuplicateEmitter, (void *)(false),
		"key=CTRL+ALT+c group='Duplicate'");
	TwAddButton(bar2, "In camera position.", CallbackDuplicateEmitter, (void *)(true),
		"key=CTRL+c group='Duplicate'");

	TwAddSeparator(bar2, NULL, NULL);
	TwAddVarCB(bar2, "Name:", TW_TYPE_STDSTRING, updateName, readName, ePtr, NULL);
	TwAddVarCB(bar2, "Exportable", TW_TYPE_BOOLCPP, updateEmitterExport, readEmitterExport, hraw, NULL);

	TwAddSeparator(bar2, NULL, NULL);
	TwAddVarRW(bar2, "Position", TW_TYPE_DIR3F, &currentTransform_h->refPosition(),
		"opened=true axisx=-x axisz=-z arrowcolor=\"255 0 0\" ");

	fileList.clear();
	fileList = DirLister::listDir(EMITTERS_PATH);

	std::string defineEnum = "";
	TwType densityType;

	int idx = 0;
	for (auto &ps : fileList){
		if (idx == fileList.size() - 1) defineEnum += ps;
		else defineEnum += ps + ",";
	}

	densityType = TwDefineEnumFromString("files", defineEnum.c_str());
	TwAddVarCB(bar2, "Emitter:", densityType,
		update_loading_particle_system, read_loading_particle_system,
		NULL, "");


	int placeXbar = configX;
	int placeYbar = 0;
	int sizeXbar = 200;
	int sizeYbar = configY;
	std::string str = " \"" + std::string(TwGetBarName(bar2)) + "\" "
		" label='" + barName + "' " +
		" color='127 255 127' " +
		" position='" + std::to_string(placeXbar) + " " + std::to_string(placeYbar) + "' " +
		" size='" + std::to_string(sizeXbar) + " " + std::to_string(sizeYbar) + "' \n"
		;
	TwDefine(str.c_str());
}

}

#endif