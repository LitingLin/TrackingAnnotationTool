#pragma once

#ifdef _WINDLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif



DLLEXPORT void * jpegDecompressorInit(const unsigned char *src, unsigned long size);
DLLEXPORT unsigned jpegDecompressorGetWidth(void *handle);
DLLEXPORT unsigned jpegDecompressorGetHeight(void *handle);
DLLEXPORT unsigned jpegDecompressorGetSize(void *handle);
DLLEXPORT int jpegDecompress(void *handle, unsigned char *buf);
DLLEXPORT void jpegDecompressDestroy(void *handle);
