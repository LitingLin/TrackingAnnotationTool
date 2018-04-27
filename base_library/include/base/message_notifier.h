#pragma once

#include "event.h"
#include <concurrent_queue.h>

namespace Base
{
	template <typename Type>
	class MessageNotifier
	{
	public:
		MessageNotifier(const MessageNotifier<Type> &) = delete;
		MessageNotifier(const MessageNotifier<Type> &&) noexcept;
		void send(Type message);
		bool try_get(Type *message);
		void get(Type *message);
		HANDLE getEventHandle();
	private:
		void accept(Type message);
		Concurrency::concurrent_queue<Type> _message;
		Event _event;
	};

	template <typename Type>
	MessageNotifier<Type>::MessageNotifier(const MessageNotifier<Type>&& other) noexcept
	{
		_event = std::move(other._event);
		_message = other._message;
		other._message.clear();
	}

	template <typename Type>
	void MessageNotifier<Type>::send(Type message)
	{
		accept(message);
		_event.set();
	}

	template <typename Type>
	bool MessageNotifier<Type>::try_get(Type* message)
	{
		return _message.try_pop(*message);
	}

	template <typename Type>
	void MessageNotifier<Type>::get(Type* message)
	{
		while (true) {
			_event.join();
			if (_message.try_pop(*message))
				break;
		}
	}

	template <typename Type>
	HANDLE MessageNotifier<Type>::getEventHandle()
	{
		return _event.getHandle();
	}

	template <typename Type>
	void MessageNotifier<Type>::accept(Type message)
	{
		_message.push(message);
	}
}