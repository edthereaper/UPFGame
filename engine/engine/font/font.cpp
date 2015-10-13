#include "mcv_platform.h"
#include "font.h"
#include "app.h"
#include "render/camera/component.h"

#pragma comment(lib, "font/FW1FontWrapper.lib") 

using namespace render;

Font::Font()
  : font(nullptr)
  , FW1Factory(nullptr)
  , color(0xffffffff)   // Text color default to white: 0xAaBbGgRr
  , size(30.0)
{}

bool Font::create() {
  assert(Render::getDevice() != nullptr);
  assert(font == nullptr);
  HRESULT hResult = FW1CreateFactory(FW1_VERSION, &FW1Factory);
  hResult = FW1Factory->CreateFontWrapper(Render::getDevice(), L"Lucida Console", &font);
  return !FAILED(hResult);
}

void Font::destroy() {
  if (font)
    font->Release();
  font = nullptr;
  if (FW1Factory)
    FW1Factory->Release();
  FW1Factory = nullptr;
}

float Font::print(float x, float y, const char *text) const {
  if (!font)
    return 0.f;
  assert(font);
  WCHAR utf16[512];
  memset(utf16, 0x80, 512 * 2);
  size_t n = mbstowcs(utf16, text, strlen(text));
  utf16[n] = 0x00;
  font->DrawString(
    Render::getContext(),
    utf16,
    size,
    x, y,
    color,
    FW1_RESTORESTATE// Flags (for example FW1_RESTORESTATE to keep context states unchanged)
    );
  return size;
}

float Font::printf(float x, float y, const char *fmt, ...) const {
  va_list args;
  va_start(args, fmt);

  char buf[ 2048 ];
  int n = vsnprintf( buf, sizeof( buf )-1, fmt, args );

  // Confirm the msg fits in the given buffer
  if( n < 0 )
    buf[ sizeof( buf )-1 ] = 0x00;

  return print( x, y, buf );
}

float Font::print3D(XMVECTOR world_p3d, const char *text) const
{
  CCamera* camera = App::get().getCamera().getSon<CCamera>();
  float x,y;
  if( camera->getScreenCoords( world_p3d, &x, &y ) )
    return print( x, y, text );
  return 0.f;
}
