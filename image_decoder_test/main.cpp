#include <base/d2d_window.h>
#include <base/memory_mapped_io.h>

#include <base/logging.h>
#include <decoder.h>

#include <dshow.h>

int __stdcall wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	int nShowCmd)
{
	(hInstance);
	(hPrevInstance);
	(nShowCmd);
	ENSURE_HR(CoInitialize(nullptr));
	{
		Base::MemoryMappedIO file(lpCmdLine);
		JPEGDecompressor decompressor(file.getPtr(), (uint32_t)file.getSize());
		uint32_t width = decompressor.getWidth(), height = decompressor.getHeight();
		decompressor.setFormat(PixelFormat::BGRA);
		double dpix = 96, dpiy = 96;
		//decoder.getResolution(&width, &height);
		std::vector<unsigned char> image(decompressor.getSize());
		decompressor.process(image.data());

		Base::D2DWindow window;
		window.Initialize();
		window.setImage(image.data(), width, height, dpix, dpiy);
		window.RunMessageLoop();
	}
	CoUninitialize();
	return 0;
}
