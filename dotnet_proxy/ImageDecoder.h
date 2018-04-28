#pragma once

#include <decoder.h>

ref class ImageDecoderProxy
{
public:
	ImageDecoderProxy(System::String ^imagePath);

private:
	JPEGDecompressor * _nativeDecompressor;
};

