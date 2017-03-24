#ifndef TARI_COMMON_HEADER
#define TARI_COMMON_HEADER

#ifdef DREAMCAST

#define fp	{}

#elif defined _WIN32
#define fup __declspec(dllexport)

#endif

#endif