#include <base/timer.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <base/logging.h>

namespace Base
{
	Timer::Timer() : _period_ms(0), _delay(0), _pause_time(0), _activated_time(0), _state(TimerState::INACTIVE)
	{
		_handle = CreateWaitableTimer(nullptr, FALSE, nullptr);
		CHECK_WIN32API(_handle);
	}

	Timer::~Timer()
	{
		LOG_IF_FAILED_WIN32API(_handle);
	}

	void Timer::activate(uint32_t period, uint64_t delay)
	{
		_period_ms = period;
		_delay = delay;

		LARGE_INTEGER large_integer;
		large_integer.QuadPart = -int64_t(delay);
		CHECK_LE(period, (uint32_t)std::numeric_limits<LONG>::max());
		CHECK_WIN32API(SetWaitableTimer(_handle, &large_integer, (LONG)period, nullptr, nullptr, FALSE));
		_activated_time = GetTickCount64();
		_state = TimerState::ACTIVE;
	}

	void Timer::inactivate()
	{
		if (_state == TimerState::ACTIVE)
			CHECK_WIN32API(CancelWaitableTimer(_handle));
		_state = TimerState::INACTIVE;
	}

	void Timer::pause()
	{
		if (_state == TimerState::ACTIVE)
		{
			_pause_time = GetTickCount64();
			CHECK_WIN32API(CancelWaitableTimer(_handle));
			_state = TimerState::PAUSE;
		}
	}

	void Timer::reactivate()
	{
		if (_state == TimerState::PAUSE)
		{
			uint64_t elapsedTime = uint64_t(_pause_time - _activated_time) * 10000ULL;
			if (!_period_ms)
			{
				if (elapsedTime < _delay)
					activate(0, _delay - elapsedTime);
			}
			else
			{
				if (elapsedTime < _delay)
					activate(_period_ms, _delay - elapsedTime);
				else
					activate(_period_ms, (_period_ms * 10000ULL) - (elapsedTime - _delay) % (_period_ms * 10000ULL));
			}
			_state = TimerState::ACTIVE;
		}
	}

	Timer::TimerState Timer::getTimerState()
	{
		return _state;
	}

	HANDLE Timer::getHandle() const
	{
		return _handle;
	}
}