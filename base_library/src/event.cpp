#include <base/event.h>
#define NOMINMAX
#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <base/logging.h>
#include <base/sync.h>

namespace Base
{
	Event::Event()
	{
		_event_handle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		ENSURE_WIN32API(_event_handle);
	}

	Event::Event(Event&& e) noexcept
	{
		_event_handle = e._event_handle;
		e._event_handle = nullptr;
	}

	Event::~Event()
	{
		if (_event_handle)
			LOG_IF_FAILED_WIN32API(CloseHandle(_event_handle));
	}

	void Event::set()
	{
		ENSURE_WIN32API(SetEvent(_event_handle));
	}

	void Event::reset()
	{
		ENSURE_WIN32API(ResetEvent(_event_handle));
	}

	void Event::join() const
	{
		waitForObject(_event_handle);
	}

	HANDLE Event::getHandle() const
	{
		return _event_handle;
	}
}