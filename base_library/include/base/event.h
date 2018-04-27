#pragma once

typedef void * HANDLE;

namespace Base {
	class Event
	{
	public:
		Event();
		Event(const Event &) = delete;
		Event(Event &&) noexcept;
		~Event();
		void set();
		void reset();
		void join() const;
		HANDLE getHandle() const;
	private:
		HANDLE _event_handle;
	};
}