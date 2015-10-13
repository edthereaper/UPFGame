#include "mcv_platform.h"
#include "antTW.h"
#ifdef _OBJECTTOOL

#include <sstream>

#include "app.h"
#include "gameElements/props.h"
using namespace gameElements;

#include "render/mesh/component.h"
#include "render/mesh/mesh.h"
using namespace DirectX;

#include "animation\components.h"
using namespace animation;

using namespace component;
using namespace animation;
using namespace DirectX;

namespace antTw_user {

	bool elementsSelected[1000] = { false };

	Handle element_h_trampoline;
	Handle element_h_light;
	Entity* element_trampoline;
	Entity* element_light;

	CTransform* ctransform_trampoline;
	CTransform* ctransform_light;

	void TW_CALL CallbackCreateEntity(void *clientData)
	{
		typedef std::vector<std::string> vector_t;
		vector_t a;
		a = DirLister::listDir("data/prefabs/mesh/");
		std::string filename;
		bool elementFound = false;
		for (int k = 0; k != a.size() && !elementFound; k++){
			if (elementsSelected[k]){
				filename = a[k].data();
				filename = filename.substr(0, filename.length() - 4);
				elementFound = true;
				elementsSelected[k] = false;
			}
		}
		if (elementFound){
			//To change mesh of an entity
			element_trampoline->get<CMesh>().destroy();
			CMesh::load(filename, Handle(element_trampoline));
			CMesh* cmesh = element_trampoline->get<CMesh>();
			cmesh->init();
		}
	}

	void AntTWManager::createObjectViewerTweak(component::Handle tramp)
	{
		element_h_trampoline = tramp;
		element_trampoline = element_h_trampoline.getOwner();

		TwBar *bar = TwNewBar("ObjectViewer");
		typedef std::vector<std::string> vector_t;
		vector_t a;
		a = DirLister::listDir("data/prefabs/mesh/");
		std::string filename;
		TwAddButton(bar, "Change Model", CallbackCreateEntity, NULL, "");
		for (int k = 0; k != a.size(); k++){
			filename = a[k].data();
			filename = filename.substr(0, filename.length() - 4);
			TwAddVarRW(bar, filename.c_str(), TW_TYPE_BOOL8, &elementsSelected[k], "");
		}
		TwBar *bar2 = TwNewBar("ObjectTransform");
		ctransform_trampoline = element_trampoline->get<CTransform>();
		TwAddVarRW(bar2, "ObjPosition", TW_TYPE_DIR3F, &ctransform_trampoline->refPosition(), "opened=true");
		TwAddVarRW(bar2, "ObjRotation", TW_TYPE_QUAT4F, &ctransform_trampoline->refRotation(), "opened=true");
		TwAddVarRW(bar2, "ObjScale", TW_TYPE_DIR3F, &ctransform_trampoline->refScale(), "opened=true");

		//Define position and size for the first bar
		int configX = App::get().getConfigX();
		int configY = App::get().getConfigY();
		int placeXbar = 20;
		int placeYbar = 20;
		int sizeXbar = 200;
		int sizeYbar = configY - 40;
		std::string str = " ObjectViewer position='" + std::to_string(placeXbar) + " " + std::to_string(placeYbar) + "' ";
		str += "size='" + std::to_string(sizeXbar) + " " + std::to_string(sizeYbar) + "'";
		const char * c = str.c_str();
		TwDefine(c);

		//Define position and size for the second bar
		int placeXbar2 = configX - 220;
		int placeYbar2 = 20;
		int sizeXbar2 = 200;
		int sizeYbar2 = configY - 40;
		std::string str2 = " ObjectTransform position='" + std::to_string(placeXbar2) + " " + std::to_string(placeYbar2) + "' ";
		str2 += "size='" + std::to_string(sizeXbar2) + " " + std::to_string(sizeYbar2) + "'";
		const char * c2 = str2.c_str();
		TwDefine(c2);
	}

}
#endif