#include <base/native_event_looper.h>
#include <base/sync.h>

#include <base/logging.h>

namespace Base
{
	NativeEventLooper::NativeEventLooper()
	{
		_nativeWaitableHandles.push_back(_event.getHandle());
	}

	void NativeEventLooper::registerWaitableObject(NativeWaitableObject* waitable_object)
	{
		uint32_t numberOfObjects;
		waitable_object->getNumberOfWaitableObjects(&numberOfObjects);
		const uint32_t originalSize = _nativeWaitableHandles.size();
		_nativeWaitableHandles.resize(originalSize + numberOfObjects);
		waitable_object->getWaitableObjects(_nativeWaitableHandles.data() + originalSize);
		_objects.push_back(waitable_object);
		const auto rc = _objectNativeHandleCountMapper.insert(std::make_pair(waitable_object, numberOfObjects));
		CHECK(rc.second);
	}

	void NativeEventLooper::updateWaitableObject(NativeWaitableObject* waitable_object)
	{
		uint32_t nativeObjectIndex = 1;
		for (NativeWaitableObject *object : _objects)
		{
			if (waitable_object == object)
				break;
			else
			{
				const uint32_t index = _objectNativeHandleCountMapper.at(object);
				nativeObjectIndex += index;
			}
		}
		uint32_t numberOfWaitableObjects;
		waitable_object->getNumberOfWaitableObjects(&numberOfWaitableObjects);
		const uint32_t recordedNumberOfWaitableObjects = _objectNativeHandleCountMapper.at(waitable_object);
		if (numberOfWaitableObjects != recordedNumberOfWaitableObjects)
		{
			if (numberOfWaitableObjects > recordedNumberOfWaitableObjects)
			{
				_nativeWaitableHandles.insert(
					_nativeWaitableHandles.begin() + nativeObjectIndex,
					numberOfWaitableObjects - recordedNumberOfWaitableObjects,
					nullptr);
			}
			else
			{
				_nativeWaitableHandles.erase(
					_nativeWaitableHandles.begin() + nativeObjectIndex,
					_nativeWaitableHandles.begin() + nativeObjectIndex + (recordedNumberOfWaitableObjects - numberOfWaitableObjects));
			}
			_objectNativeHandleCountMapper.at(waitable_object) = numberOfWaitableObjects;
		}
		waitable_object->getWaitableObjects(_nativeWaitableHandles.data() + nativeObjectIndex);
	}

	void NativeEventLooper::eraseWaitableObject(NativeWaitableObject* waitable_object)
	{
		uint32_t nativeObjectIndex = 1;
		uint32_t objectIndex = 0;
		for (NativeWaitableObject *object : _objects)
		{
			if (waitable_object == object)
				break;
			else
			{
				const uint32_t index = _objectNativeHandleCountMapper.at(object);
				nativeObjectIndex += index;
				++objectIndex;
			}
		}

		const uint32_t numberOfObject = _objectNativeHandleCountMapper.at(waitable_object);
		_nativeWaitableHandles.erase(_nativeWaitableHandles.begin() + nativeObjectIndex,
			_nativeWaitableHandles.begin() + nativeObjectIndex + numberOfObject);
		_objects.erase(_objects.begin() + objectIndex);
		_objectNativeHandleCountMapper.erase(waitable_object);
	}

	void NativeEventLooper::runLooper()
	{
		while (true)
		{
			unsigned signaledObjectIndex;
			waitForMultipleObjects(_nativeWaitableHandles.data(), _nativeWaitableHandles.size(), &signaledObjectIndex);
			if (signaledObjectIndex == 0)
				return;

			signaledObjectIndex -= 1;
			NativeWaitableObject *signaledObject = nullptr;
			for (NativeWaitableObject *object : _objects)
			{
				const uint32_t numberOfNativeObjects = _objectNativeHandleCountMapper.at(object);

				if (numberOfNativeObjects > signaledObjectIndex) {
					signaledObject = object;
					break;
				}
				else
					signaledObjectIndex -= numberOfNativeObjects;
			}
			signaledObject->callback(signaledObjectIndex);
		}
	}

	void NativeEventLooper::cancel()
	{
		_event.set();
	}
}