#pragma once
#include <turbojpeg.h>

#include <stdint.h>

enum class PixelFormat : uint32_t
{
	RGB = 0, BGR, RGBA, BGRA, ABGR, ARGB, GRAY
};

class DLLEXPORT JPEGDecompressor
{
public:
	JPEGDecompressor(const unsigned char *src, unsigned long size);
	~JPEGDecompressor() noexcept;
	void process(unsigned char *dst);
	unsigned getSize() const noexcept;
	unsigned getWidth() const noexcept;
	unsigned getHeight() const noexcept;
	void setFormat(PixelFormat format);
	PixelFormat getFormat() const;
private:
	tjhandle _tjhandle;
	const unsigned char *_src;
	const unsigned long _srcSize;
	unsigned _width;
	unsigned _height;
	TJPF _format;
};
