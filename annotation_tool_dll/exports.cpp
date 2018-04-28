#include "exports.h"

#include <decoder.h>

void* jpegDecompressorInit(const unsigned char *src, unsigned long size)
{
	try {
		return new JPEGDecompressor(src, size);
	}
	catch (...)
	{
		return nullptr;
	}
}

unsigned jpegDecompressorGetWidth(void* handle)
{
	JPEGDecompressor *jpeg_decompressor = (JPEGDecompressor*)handle;
	return jpeg_decompressor->getWidth();
}

unsigned jpegDecompressorGetHeight(void* handle)
{
	JPEGDecompressor *jpeg_decompressor = (JPEGDecompressor*)handle;
	return jpeg_decompressor->getHeight();
}

unsigned jpegDecompressorGetSize(void* handle)
{
	JPEGDecompressor *jpeg_decompressor = (JPEGDecompressor*)handle;
	return jpeg_decompressor->getSize();
}

int jpegDecompress(void* handle, unsigned char* buf)
{
	JPEGDecompressor *jpeg_decompressor = (JPEGDecompressor*)handle;
	try {
		jpeg_decompressor->process(buf);
		return 0;
	}
	catch (...)
	{
		return 1;
	}
}

void jpegDecompressDestroy(void* handle)
{
	delete static_cast<JPEGDecompressor*>(handle);
}
