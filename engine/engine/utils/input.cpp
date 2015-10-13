#include "mcv_platform.h"
#include "input.h"
#include "app.h"

namespace utils {

HWND Mouse::hWnd;

bool Mouse::capturing = false;
signed short Mouse::wheel;

keyState Mouse::lmb;
keyState Mouse::mmb;
keyState Mouse::rmb;

POINT Mouse::center;
POINT Mouse::delta;

void Mouse::setSysMouse(int sx, int sy)
{
     if (capturing) {
         delta.x = sx - center.x;
         delta.y = sy - center.y;
         POINT sc = center;
         ::ClientToScreen(hWnd, &sc);
         if( delta.x != 0 || delta.y != 0 ) {
             ::SetCursorPos(sc.x, sc.y);
         }
         auto& app = App::get();
         if( sc.x < 0 || sc.y < 0 || sc.x > app.getConfigX() ||sc.y > app.getConfigY()) {
             clearDelta();
         }
     }
 }

}