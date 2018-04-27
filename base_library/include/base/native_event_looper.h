#pragma once

#include <cstdint>
#include <vector>
#include <map>

#include "event.h"

namespace Base
{
	typedef void *HANDLE;

	class NativeWaitableObject
	{
	public:
		virtual ~NativeWaitableObject() = default;
		virtual void getNumberOfWaitableObjects(uint32_t *numberOfWaitableObject) = 0;
		virtual void getWaitableObjects(HANDLE *nativeWaitableObjects) = 0;
		virtual void callback(uint32_t index) = 0;
	};

	class NativeEventLooper
	{
	public:
		NativeEventLooper();
		void registerWaitableObject(NativeWaitableObject *waitable_object);
		void updateWaitableObject(NativeWaitableObject *waitable_object);
		void eraseWaitableObject(NativeWaitableObject *waitable_object);
		void runLooper();
		void cancel();
	private:
		Event _event;
		std::vector<HANDLE> _nativeWaitableHandles;
		std::vector<NativeWaitableObject *> _objects;
		std::map<NativeWaitableObject*, uint32_t> _objectNativeHandleCountMapper;
	};
}