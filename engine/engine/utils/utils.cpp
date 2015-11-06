#include "mcv_platform.h"

using namespace DirectX;

namespace utils {

// -----------------------------------------
bool isKeyPressed(int key) {
  return ( ::GetAsyncKeyState(key) & 0x8000 ) != 0;
}

static std::ofstream log = std::ofstream("log.txt");

//#define MUTE_DBG

bool fatal(const char* fmt, ...) {
#if !defined(MUTE_DBG)
  va_list ap;
  va_start(ap, fmt);
  char buf[1024*4];
  size_t n = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
  va_end(ap);
  log << "FATAL: " << buf << std::flush;
  ::OutputDebugString(buf);
#endif
  assert(!"FATAL ERROR");
  exit(EXIT_FAILURE);
  return false;
}

bool dbg(const char* fmt, ...) {
#if !defined(MUTE_DBG)
  va_list ap;
  va_start(ap, fmt);
  char buf[1024*4];
  size_t n = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
  ::OutputDebugString(buf);
  log << buf << std::flush;
  va_end(ap);
#endif
return false;
}

bool dbg_release(const char* fmt, ...) {

	va_list ap;
	va_start(ap, fmt);
	char buf[1024 * 4];
	size_t n = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
	::OutputDebugString(buf);
	va_end(ap);
	return false;
}

}
