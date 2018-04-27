#include "decoder.h"

#include <base/logging.h>

#define CHECK_TURBOJPEG(exp) \
	if (!(exp)) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::USERDEFINED, -1, __FILE__, __LINE__, __func__, #exp).stream() << tjGetErrorStr()

#define CHECK_OP_TURBOJPEG(exp1, exp2, op, functional_op)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::USERDEFINED, GetLastError(), __FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") " << tjGetErrorStr()

#define CHECK_EQ_TURBOJPEG(exp1, exp2) \
	CHECK_OP_TURBOJPEG(exp1, exp2, ==, std::equal_to<>())

JPEGDecompressor::JPEGDecompressor(const unsigned char *src, unsigned long srcSize)
	: _src(src), _srcSize(srcSize), _format(TJPF_RGB)
{
	_tjhandle = tjInitDecompress();
	CHECK_TURBOJPEG(_tjhandle);
	int width, height;
	try {
		int subsample, colorspace;
		CHECK_EQ_TURBOJPEG(tjDecompressHeader3(_tjhandle, _src, _srcSize, &width, &height, &subsample, &colorspace), 0);
		CHECK_GT(width, 0);
		_width = width;
		CHECK_GT(height, 0);
		_height = height;
	} catch (std::exception &)
	{
		tjDestroy(_tjhandle);
		throw;
	}
}

JPEGDecompressor::~JPEGDecompressor()
{
	tjDestroy(_tjhandle);
}

void JPEGDecompressor::process(unsigned char* dst)
{
	CHECK_EQ_TURBOJPEG(tjDecompress2(_tjhandle, _src, _srcSize, dst, 0, 0, 0, _format, TJFLAG_NOREALLOC), 0);
}

unsigned JPEGDecompressor::getSize() const noexcept
{
	return _width * _height * tjPixelSize[_format];
}

unsigned JPEGDecompressor::getWidth() const noexcept
{
	return _width;
}

unsigned JPEGDecompressor::getHeight() const noexcept
{
	return _height;
}

void JPEGDecompressor::setFormat(PixelFormat format)
{
	switch (format)
	{
	case PixelFormat::RGB:
		_format = TJPF_RGB;
		break;
	case PixelFormat::BGR:
		_format = TJPF_BGR;
		break;
	case PixelFormat::RGBA:
		_format = TJPF_RGBA;
		break;
	case PixelFormat::GRAY:
		_format = TJPF_GRAY;
		break;
	case PixelFormat::BGRA:
		_format = TJPF_BGRA;
		break;
	case PixelFormat::ABGR:
		_format = TJPF_ABGR;
		break;
	case PixelFormat::ARGB:
		_format = TJPF_ARGB;
		break;
	default:
		UNREACHABLE_ERROR;
	}
}

PixelFormat JPEGDecompressor::getFormat() const
{
	switch (_format)
	{
	case TJPF_RGB:
		return PixelFormat::RGB;
		break;
	case TJPF_BGR:
		return PixelFormat::RGB;
		break;
	case TJPF_GRAY:
		return PixelFormat::GRAY;
		break;
	case TJPF_RGBA:
		return PixelFormat::RGBA;
		break;
	case TJPF_BGRA:
		return PixelFormat::BGRA;
		break;
	case TJPF_ABGR:
		return PixelFormat::ABGR;
		break;
	case TJPF_ARGB:
		return PixelFormat::ARGB;
		break;
	default: 
		UNREACHABLE_ERROR;
	}
}

void* jpegDecompressorInit(const unsigned char *src, unsigned long size)
{
	try {
		return new JPEGDecompressor(src, size);
	} catch (...)
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
	} catch (...)
	{
		return 1;
	}
}

void jpegDecompressDestroy(void* handle)
{
	delete static_cast<JPEGDecompressor*>(handle);
}
