#include <base/d2d_window.h>

#pragma comment(lib, "D2d1.lib")

namespace Base
{
	//
	// Initialize members.
	//
	D2DWindow::D2DWindow() :
		m_hwnd(NULL)
	{
	}

	//
	// Creates the application window and initializes
	// device-independent resources.
	//
	HRESULT D2DWindow::Initialize()
	{
		HRESULT hr;

		// Initialize device-indpendent resources, such
		// as the Direct2D factory.
		hr = CreateDeviceIndependentResources();
		if (SUCCEEDED(hr))
		{
			// Register the window class.
			WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = D2DWindow::WndProc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = sizeof(LONG_PTR);
			wcex.hInstance = GetModuleHandle(nullptr);
			wcex.hbrBackground = NULL;
			wcex.lpszMenuName = NULL;
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.lpszClassName = L"D2DDemoApp";

			RegisterClassEx(&wcex);

			// Create the application window.
			//
			// Because the CreateWindow function takes its size in pixels, we
			// obtain the system DPI and use it to scale the window size.
			FLOAT dpiX, dpiY;
			m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

			m_hwnd = CreateWindow(
				L"D2DDemoApp",
				L"Direct2D Demo Application",
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
				static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
				NULL,
				NULL,
				GetModuleHandle(nullptr),
				this
			);
			hr = m_hwnd ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				ShowWindow(m_hwnd, SW_SHOWNORMAL);

				UpdateWindow(m_hwnd);
			}
		}

		return hr;
	}


	//
	//  This method creates resources which are bound to a particular
	//  Direct3D device. It's all centralized here, in case the resources
	//  need to be recreated in case of Direct3D device loss (eg. display
	//  change, remoting, removal of video card, etc).
	//
	HRESULT D2DWindow::CreateDeviceResources()
	{
		HRESULT hr = S_OK;

		if (!m_pRenderTarget)
		{

			RECT rc;
			GetClientRect(m_hwnd, &rc);

			D2D1_SIZE_U size = D2D1::SizeU(
				rc.right - rc.left,
				rc.bottom - rc.top
			);

			_width = size.width;
			_height = size.height;
			_dpix = 96;
			_dpiy = 96;
			_imageBuffer.resize(_width * _height * 4);

			// Create a Direct2D render target.
			hr = m_pD2DFactory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(m_hwnd, size),
				&m_pRenderTarget
			);
		}

		return hr;
	}


	//
	//  Discard device-specific resources which need to be recreated
	//  when a Direct3D device is lost
	//
	void D2DWindow::DiscardDeviceResources()
	{
		m_pRenderTarget.Release();
	}

	//
	// The main window message loop.
	//
	void D2DWindow::RunMessageLoop()
	{
		MSG msg;

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void D2DWindow::setImage(const unsigned char* buffer, const uint32_t width, const uint32_t height, const double dpix, const double dpiy)
	{
		{
			std::lock_guard<std::mutex> lock_guard(_lock);
			_imageBuffer.resize(width * height * 4);
			_width = width;
			_height = height;
			_dpix = dpix;
			_dpiy = dpiy;
			memcpy(_imageBuffer.data(), buffer, width * height * 4);
		}
		PostMessage(m_hwnd, WM_PAINT, 0, 0);
	}

	void D2DWindow::exit()
	{
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
	}

	HRESULT D2DWindow::CreateDeviceIndependentResources()
	{
		return D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	}


	//
	//  Called whenever the application needs to display the client
	//  window. This method writes "Hello, World"
	//
	//  Note that this function will not render anything if the window
	//  is occluded (e.g. when the screen is locked).
	//  Also, this function will automatically discard device-specific
	//  resources if the Direct3D device disappears during function
	//  invocation, and will recreate the resources the next time it's
	//  invoked.
	//
	HRESULT D2DWindow::OnRender()
	{
		HRESULT hr;

		hr = CreateDeviceResources();

		if (SUCCEEDED(hr) && !(m_pRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
		{
			m_pRenderTarget->BeginDraw();

			m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

			m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

			CComPtr<ID2D1Bitmap> bitmap;
			D2D1_BITMAP_PROPERTIES bitmap_properties;
			bitmap_properties.dpiX = (FLOAT)_dpix;
			bitmap_properties.dpiY = (FLOAT)_dpiy;
			bitmap_properties.pixelFormat = { DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_IGNORE };
			{
				std::lock_guard<std::mutex> lock_guard(_lock);
				hr = m_pRenderTarget->CreateBitmap(D2D1_SIZE_U{ _width, _height }, _imageBuffer.data(), _width * 4, bitmap_properties, &bitmap);
			}

			if (SUCCEEDED(hr))
				m_pRenderTarget->DrawBitmap(bitmap);

			hr = m_pRenderTarget->EndDraw();

			if (hr == D2DERR_RECREATE_TARGET)
			{
				hr = S_OK;
				DiscardDeviceResources();
			}
		}

		return hr;
	}

	//
	//  If the application receives a WM_SIZE message, this method
	//  resizes the render target appropriately.
	//
	void D2DWindow::OnResize(UINT width, UINT height)
	{
		if (m_pRenderTarget)
		{
			D2D1_SIZE_U size;
			size.width = width;
			size.height = height;

			// Note: This method can fail, but it's okay to ignore the
			// error here -- it will be repeated on the next call to
			// EndDraw.
			m_pRenderTarget->Resize(size);
		}
	}


	//
	// The window message handler.
	//
	LRESULT CALLBACK D2DWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = 0;

		if (message == WM_CREATE)
		{
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
			D2DWindow *pDemoApp = (D2DWindow *)pcs->lpCreateParams;

			::SetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA,
				LONG_PTR(pDemoApp)
			);

			result = 1;
		}
		else
		{
			D2DWindow *pDemoApp = reinterpret_cast<D2DWindow *>(static_cast<LONG_PTR>(
				::GetWindowLongPtrW(
					hwnd,
					GWLP_USERDATA
				)));

			bool wasHandled = false;

			if (pDemoApp)
			{
				switch (message)
				{
				case WM_SIZE:
				{
					UINT width = LOWORD(lParam);
					UINT height = HIWORD(lParam);
					pDemoApp->OnResize(width, height);
				}
				wasHandled = true;
				result = 0;
				break;

				case WM_PAINT:
				case WM_DISPLAYCHANGE:
				{
					PAINTSTRUCT ps;
					BeginPaint(hwnd, &ps);

					pDemoApp->OnRender();
					EndPaint(hwnd, &ps);
				}
				wasHandled = true;
				result = 0;
				break;

				case WM_DESTROY:
				{
					PostQuitMessage(0);
				}
				wasHandled = true;
				result = 1;
				break;
				}
			}

			if (!wasHandled)
			{
				result = DefWindowProc(hwnd, message, wParam, lParam);
			}
		}

		return result;
	}
}