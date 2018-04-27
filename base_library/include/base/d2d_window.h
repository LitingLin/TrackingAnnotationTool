#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <mutex>
#include <memory.h>
#include <wchar.h>
#include <math.h>
#include <atlbase.h>
#include <vector>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

/******************************************************************
*                                                                 *
*  D2DWindow                                                        *
*                                                                 *
******************************************************************/

namespace Base {
	class D2DWindow
	{
	public:
		D2DWindow();

		HRESULT Initialize();

		void RunMessageLoop();
		// must be BGR 32bit, 8bit per channel
		void setImage(const unsigned char *buffer, const uint32_t width, const uint32_t height, const double dpix, const double dpiy);
		void exit();

	private:
		HRESULT CreateDeviceIndependentResources();
		HRESULT CreateDeviceResources();
		void DiscardDeviceResources();

		HRESULT OnRender();

		void OnResize(
			UINT width,
			UINT height
		);

		static LRESULT CALLBACK WndProc(
			HWND hWnd,
			UINT message,
			WPARAM wParam,
			LPARAM lParam
		);

	private:
		HWND m_hwnd;
		CComPtr<ID2D1Factory> m_pD2DFactory;
		CComPtr<ID2D1HwndRenderTarget> m_pRenderTarget;
		CComPtr<ID2D1Bitmap> _bitmap;
		std::vector<unsigned char> _imageBuffer;
		uint32_t _width, _height;
		double _dpix, _dpiy;
		std::mutex _lock;
	};
}
