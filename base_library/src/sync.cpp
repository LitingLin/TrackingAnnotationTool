#include <base/sync.h>

#define NOMINMAX
#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <base/logging.h>

namespace Base
{
	void waitForObject(const HANDLE waitableObject)
	{
		unsigned rc = WaitForSingleObject(waitableObject, INFINITE);
		ENSURE_EQ_WIN32API(rc, WAIT_OBJECT_0);
	}

	bool waitForObject_timeout(const HANDLE waitableObject, unsigned timeout)
	{
		unsigned rc = WaitForSingleObject(waitableObject, timeout);
		if (rc == WAIT_TIMEOUT)
			return false;
		ENSURE_EQ_WIN32API(rc, WAIT_OBJECT_0);
		return true;
	}

	void waitForMultipleObjects(const HANDLE* waitableObjects, unsigned numberOfObjects, unsigned *signaledObjectIndex)
	{
		ENSURE_LE(numberOfObjects, unsigned(MAXIMUM_WAIT_OBJECTS));

		unsigned rc = WaitForMultipleObjects(numberOfObjects, waitableObjects, FALSE, INFINITE);
		ENSURE_GE_WIN32API(rc, WAIT_OBJECT_0);
		ENSURE_LT_WIN32API(rc, WAIT_OBJECT_0 + numberOfObjects);
		*signaledObjectIndex = rc - WAIT_OBJECT_0;
	}

	bool waitForMultipleObjects_timeout(const HANDLE* waitableObjects, unsigned numberOfObjects, unsigned timeout, unsigned *signaledObjectIndex)
	{
		ENSURE_LE(numberOfObjects, unsigned(MAXIMUM_WAIT_OBJECTS));

		unsigned rc = WaitForMultipleObjects(numberOfObjects, waitableObjects, FALSE, timeout);
		if (rc == WAIT_TIMEOUT)
			return false;
		ENSURE_GE_WIN32API(rc, WAIT_OBJECT_0);
		ENSURE_LT_WIN32API(rc, WAIT_OBJECT_0 + numberOfObjects);
		*signaledObjectIndex = rc - WAIT_OBJECT_0;
		return true;
	}

	void waitAllObjects(const HANDLE* waitableObjects, unsigned numberOfObjects)
	{
		ENSURE_LE(numberOfObjects, unsigned(MAXIMUM_WAIT_OBJECTS));

		unsigned rc = WaitForMultipleObjects(numberOfObjects, waitableObjects, TRUE, INFINITE);
		ENSURE_GE_WIN32API(rc, WAIT_OBJECT_0);
		ENSURE_LT_WIN32API(rc, WAIT_OBJECT_0 + numberOfObjects);
	}

	bool waitAllObjects_timeout(const HANDLE* waitableObjects, unsigned numberOfObjects, unsigned timeout)
	{
		ENSURE_LE(numberOfObjects, unsigned(MAXIMUM_WAIT_OBJECTS));

		unsigned rc = WaitForMultipleObjects(numberOfObjects, waitableObjects, TRUE, timeout);
		if (rc == WAIT_TIMEOUT)
			return false;
		ENSURE_GE_WIN32API(rc, WAIT_OBJECT_0);
		ENSURE_LT_WIN32API(rc, WAIT_OBJECT_0 + numberOfObjects);
		return true;
	}
}