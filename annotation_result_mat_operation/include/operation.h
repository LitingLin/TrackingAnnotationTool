#pragma once

#ifdef _WINDLL
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif

#include <cstdint>
#include <string>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <WTypes.h>
#include <OleAuto.h>

class MATFile;

/* 
 * Rely on:
 *  libmat.dll
 *  libmx.dll
 *  libmwfl.dll
 *  libmwfoundation_usm.dll
 *  libmwi18n.dll
 *  libut.dll
 *  hdf5.dll
 *  boost_log-vc140-mt-1_56.dll
 *  boost_system-vc140-mt-1_56.dll
 *  icuuc56.dll
 *  tbb.dll
 *  tbbmalloc.dll
 *  zlib1.dll
 *  libmwresource_core.dll
 *  boost_date_time-vc140-mt-1_56.dll
 *  boost_filesystem-vc140-mt-1_56.dll
 *  boost_thread-vc140-mt-1_56.dll
 *  icuin56.dll
 *  libexpat.dll
 *  boost_regex-vc140-mt-1_56.dll
 *  boost_serialization-vc140-mt-1_56.dll
 *  boost_chrono-vc140-mt-1_56.dll
 *  icudt56.dll
 *  icuio56.dll
 */

class DLLEXPORT AnnotationOperator
{
public:
	enum class DesiredAccess : uint32_t
	{
		read = 0,
		write,
		both = read | write,
	};
	//	                        |                    When the file...
	// This argument:           |             Exists            Does not exist
	// -------------------------+------------------------------------------------------
	// CREATE_ALWAYS            |            Truncates             Creates
	// OPEN_ALWAYS     ===| does this |===>    Opens               Creates
	enum class CreationDisposition : uint32_t
	{
		open_always = 0,
		create_always
	};
	AnnotationOperator(const std::wstring &matFilePath, DesiredAccess desiredAccess, CreationDisposition creationDisposition);
	AnnotationOperator(const AnnotationOperator &) = delete;
	AnnotationOperator(AnnotationOperator &&) = delete;
	~AnnotationOperator() noexcept(false);
	size_t getNumberOfRecords() const;
	bool get(size_t index, int *id, bool *labeled, int *x, int *y, int *w, int *h, bool *occlusion, bool *outOfView, std::wstring *path) const;
	void update(size_t index, int id, bool labeled, int x, int y, int w, int h, bool occlusion, bool outOfView, const std::wstring &path);
	void resize(size_t n);
private:
	MATFile * _matFile;
	void *_variable;
	bool _pendingUpdates;
};

#define ANNOTATION_READ 0
#define ANNOTATION_WRITE 1
#define ANNOTATION_BOTH (ANNOTATION_READ | ANNOTATION_WRITE)
#define ANNOTATION_OPEN_ALWAYS 0
#define ANNOTATION_CREATE_ALWAYS 1

extern "C" {
	DLLEXPORT void *createAnnotationOperator(BSTR matFilePath, int desiredAccess, int creationDisposition);
	DLLEXPORT BOOL getAnnotationNumberOfRecords(void *handle, uint64_t *numberOfRecords);
	DLLEXPORT BOOL getAnnotationRecord(void *handle, uint64_t index, int *id, BOOL *labeled, int *x, int *y, int *w, int *h, BOOL *occlusion, BOOL *outOfView, BSTR*path);
	DLLEXPORT BOOL updateAnnotationRecord(void *handle, uint64_t index, int id, BOOL labeled, int x, int y, int w, int h, BOOL occlusion, BOOL outOfView, BSTR path);
	DLLEXPORT BOOL resizeAnnotationRecord(void *handle, uint64_t size);
	DLLEXPORT BOOL destroyAnnotationOperator(void *handle);
}