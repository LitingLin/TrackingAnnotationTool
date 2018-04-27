#pragma once

typedef void * HANDLE;

namespace Base
{
	void waitForObject(const HANDLE waitableObject);
	bool waitForObject_timeout(const HANDLE waitableObject, unsigned timeout);

	void waitForMultipleObjects(const HANDLE* waitableObjects, unsigned numberOfObjects, unsigned *signaledObjectIndex);
	bool waitForMultipleObjects_timeout(const HANDLE* waitableObjects, unsigned numberOfObjects, unsigned timeout, unsigned *signaledObjectIndex);
	void waitAllObjects(const HANDLE* waitableObjects, unsigned numberOfObjects);
	bool waitAllObjects_timeout(const HANDLE* waitableObjects, unsigned numberOfObjects, unsigned timeout);
}