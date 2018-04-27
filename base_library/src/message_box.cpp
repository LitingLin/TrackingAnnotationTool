#include <base/message_box.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Base {
	void showMessageBox_Error(const wchar_t *text, const wchar_t *caption)
	{
		MessageBox(NULL, text, caption, MB_OK | MB_ICONERROR);
	}
}
