#ifndef INC_FONT_H_
#define INC_FONT_H_

#include "FW1FontWrapper.h"
#include "render/camera/camera.h"

struct Font {
  IFW1FontWrapper* font;
  IFW1Factory*     FW1Factory;
  unsigned         color;
  float            size;
  Font();
  bool create();
  void destroy();
  float print(float x, float y, const char *text) const;
  float printf(float x, float y, const char *fmt, ... ) const;
  float print3D( XMVECTOR p3d, const char *fmt ) const;
};

#endif
