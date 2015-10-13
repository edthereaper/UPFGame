#ifndef ANIMATION_UTILS_CONVERT_CMF
#define ANIMATION_UTILS_CONVERT_CMF

#include "skeleton_manager.h"

#include "mcv_platform.h"

#include "render/mesh/mesh.h"
#include "render/shader/vertex_declarations.h"
using namespace render;

#include "utils/data_saver.h"
using namespace utils;

#include "cal3d/cal3d.h"

namespace utils {

bool convertCMF(CalCoreMesh* core_mesh, const char* outfile);

}

#endif