#ifndef TARI_COMMON_HEADER
#define TARI_COMMON_HEADER

#ifdef DREAMCAST

#define fup	

#elif defined _WIN32
#define fup __declspec(dllexport)

#elif defined __EMSCRIPTEN__

#define fup 

#endif

#endif
