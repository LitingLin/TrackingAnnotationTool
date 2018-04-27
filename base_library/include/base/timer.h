#pragma once
#include <cstdint>

typedef void *HANDLE;
namespace Base
{
	class Timer
	{
	public:
		enum class TimerState { INACTIVE, ACTIVE, PAUSE };
		Timer();
		Timer(const Timer &) = delete;
		~Timer();
		void activate(uint32_t period_ms, uint64_t delay_100ns = 0);
		void inactivate();
		void pause();
		void reactivate();
		TimerState getTimerState();
		HANDLE getHandle() const;
	private:
		uint32_t _period_ms;
		uint64_t _delay;
		uint64_t _pause_time;
		uint64_t _activated_time;
		HANDLE _handle;
		TimerState _state;
	};
}