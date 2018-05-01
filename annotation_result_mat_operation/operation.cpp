#include "operation.h"

#include <mat.h>

#include <base/utils.h>
#include <base/logging.h>

static bool isValid(const mxArray *pa);
static bool isValid(const mxArray *pa, size_t index);

AnnotationOperator::AnnotationOperator(const std::wstring& matFilePath, DesiredAccess desiredAccess,
	CreationDisposition creationDisposition) : _pendingUpdates(false)
{
	std::string matFilePath_ascii = Base::UTF16ToASCII(matFilePath);
	if (desiredAccess == DesiredAccess::read && creationDisposition == CreationDisposition::open_always)
		_matFile = matOpen(matFilePath_ascii.c_str(), "r");
	else if ((desiredAccess == DesiredAccess::write || desiredAccess == DesiredAccess::both) && creationDisposition == CreationDisposition::open_always)
		_matFile = matOpen(matFilePath_ascii.c_str(), "u");
	else if ((desiredAccess == DesiredAccess::write || desiredAccess == DesiredAccess::both) && creationDisposition == CreationDisposition::create_always)
		_matFile = matOpen(matFilePath_ascii.c_str(), "w7");
	else {
		NOT_EXPECT_EXCEPTION;
	}

	CHECK(_matFile);
	if (creationDisposition == CreationDisposition::open_always)
		_variable = matGetVariable(_matFile, "res");
	else
		_variable = nullptr;
}

AnnotationOperator::~AnnotationOperator() noexcept(false)
{
	if (_pendingUpdates) {
		const matError error = matPutVariable(_matFile, "res", (mxArray*)_variable);
		if (error) {
			if (_variable) mxDestroyArray((mxArray*)_variable);
			if (_matFile) matClose(_matFile);
			if (std::uncaught_exception()) {
				LOG_IF_NOT_EQ(error, 0);
			}
			else {
				CHECK_EQ(error, 0);
			}
		}
	}


	if (_variable)
		mxDestroyArray((mxArray*)_variable);
	const matError error = matClose(_matFile);
	if (std::uncaught_exception()) {
		LOG_IF_NOT_EQ(error, 0);
	}
	else {
		CHECK_EQ(error, 0);
	}
}

size_t AnnotationOperator::getNumberOfRecords() const
{
	if (_variable)
		return mxGetNumberOfElements((mxArray*)_variable);
	else
		return 0;
}

bool AnnotationOperator::get(size_t index, int *id, bool *labeled, int *x, int *y, int *w, int *h, bool *occlusion, bool *outOfView, std::wstring *path) const
{
	if (!_variable)
		return false;

	mxArray *pa = (mxArray*)_variable;

	if (!isValid(pa)) return false;

	if (!isValid(pa, index)) return false;

	const mxArray *pid = mxGetField(pa, index, "id");
	double id_ = mxGetScalar(pid);
	*id = (int)id_;
	const mxArray *plabeled = mxGetField(pa, index, "labeled");
	mxLogical *plabeleds_ = mxGetLogicals(plabeled);
	*labeled = plabeleds_[0];
	const mxArray *pbbox = mxGetField(pa, index, "bbox");
	double *pbbox_ = mxGetPr(pbbox);
	*x = pbbox_[0];
	*y = pbbox_[1];
	*w = pbbox_[2];
	*h = pbbox_[3];
	const mxArray *pocclusion = mxGetField(pa, index, "occlusion");
	mxLogical *pOcclusion_ = mxGetLogicals(pocclusion);
	*occlusion = pOcclusion_[0];
	const mxArray *pOutOfView = mxGetField(pa, index, "out_view");
	mxLogical *pOutOfView_ = mxGetLogicals(pOutOfView);
	*outOfView = pOutOfView_[0];
	const mxArray *pPath = mxGetField(pa, index, "path");
	const size_t pathSize = mxGetElementSize(pPath) * mxGetNumberOfElements(pPath);
	path->resize(pathSize / sizeof(wchar_t));
	memcpy((void*)path->data(), mxGetData(pPath), pathSize);

	return true;
}

void AnnotationOperator::update(size_t index, int id, bool labeled, int x, int y, int w, int h, bool occlusion,
	bool outOfView, const std::wstring& path)
{
	mxArray *pa = (mxArray*)_variable;
	CHECK(pa);
	CHECK(isValid(pa));

	mxArray *pid = mxCreateDoubleScalar(id);
	mxArray *plabeled = mxCreateLogicalScalar(labeled);
	mxArray *pBBox = mxCreateDoubleMatrix(1, 4, mxREAL);
	double *pBBox_ = mxGetPr(pBBox);
	pBBox_[0] = x;
	pBBox_[1] = y;
	pBBox_[2] = w;
	pBBox_[3] = h;
	mxArray *pOcclusion = mxCreateLogicalScalar(occlusion);
	mxArray *pOutOfView = mxCreateLogicalScalar(outOfView);
	size_t pathSize[2] = { 1, path.size() };
	mxArray* pPath = mxCreateCharArray(2, pathSize);
	ENSURE_EQ(mxGetElementSize(pPath), sizeof(wchar_t));
	void *pPath_ = mxGetData(pPath);
	memcpy(pPath_, path.c_str(), mxGetElementSize(pPath) * mxGetNumberOfElements(pPath));
	mxSetField(pa, index, "id", pid);
	mxSetField(pa, index, "labeled", plabeled);
	mxSetField(pa, index, "bbox", pBBox);
	mxSetField(pa, index, "occlusion", pOcclusion);
	mxSetField(pa, index, "out_view", pOutOfView);
	mxSetField(pa, index, "path", pPath);

	_pendingUpdates = true;
}

void AnnotationOperator::resize(size_t n)
{
	if (_variable) {
		mxDestroyArray((mxArray*)_variable);
		_variable = nullptr;
	}
	size_t size[2] = { 1, n };
	const char* names[] = { "id", "labeled", "bbox", "occlusion", "out_view", "path" };
	mxArray *res = mxCreateStructArray(2, size, sizeof(names) / sizeof(*names), names);
	CHECK(res);
	_variable = res;
	_pendingUpdates = true;
}

extern "C" {
	void* createAnnotationOperator(BSTR matFilePath, int desiredAccess, int creationDisposition)
	{
		try
		{
			return new AnnotationOperator(matFilePath,
				(AnnotationOperator::DesiredAccess)desiredAccess, (AnnotationOperator::CreationDisposition)creationDisposition);
		}
		catch (...)
		{
			return nullptr;
		}
	}

	BOOL getAnnotationNumberOfRecords(void* handle, uint64_t* numberOfRecords)
	{
		try {
			AnnotationOperator *annotationOperator = (AnnotationOperator*)handle;
			*numberOfRecords = annotationOperator->getNumberOfRecords();
			return TRUE;
		}
		catch (...)
		{
			return FALSE;
		}
	}

	BOOL getAnnotationRecord(void *handle, uint64_t index, int* id, BOOL* labeled, int* x, int* y, int* w, int* h, BOOL* occlusion,
		BOOL* outOfView, BSTR* path)
	{
		try {
			AnnotationOperator *annotationOperator = (AnnotationOperator*)handle;
			bool labeled_, occlusion_, outOfView_;
			std::wstring path_;
			if (!annotationOperator->get(index, id, &labeled_, x, y, w, h, &occlusion_, &outOfView_, &path_))
				return FALSE;

			*path = SysAllocStringLen(path_.c_str(), path_.size());
			*labeled = labeled_;
			*occlusion = occlusion_;
			*outOfView = outOfView_;

			return TRUE;
		}
		catch (...)
		{
			return FALSE;
		}
	}

	BOOL updateAnnotationRecord(void* handle, uint64_t index, int id, BOOL labeled, int x, int y, int w, int h,
		BOOL occlusion, BOOL outOfView, BSTR path)
	{
		try {
			AnnotationOperator *annotationOperator = (AnnotationOperator*)handle;
			annotationOperator->update(index, id, labeled, x, y, w, h, occlusion, outOfView, path);

			return TRUE;
		}
		catch (...)
		{
			return FALSE;
		}
	}

	BOOL resizeAnnotationRecord(void* handle, uint64_t size)
	{
		try {
			AnnotationOperator *annotationOperator = (AnnotationOperator*)handle;
			annotationOperator->resize(size);

			return TRUE;
		}
		catch (...)
		{
			return FALSE;
		}
	}

	BOOL destroyAnnotationOperator(void* handle)
	{
		try {
			AnnotationOperator *annotationOperator = (AnnotationOperator*)handle;
			delete annotationOperator;

			return TRUE;
		}
		catch (...)
		{
			return FALSE;
		}
	}

}

bool isValid(const mxArray *pa)
{
	return mxGetClassID(pa) == mxSTRUCT_CLASS;
}

bool isValid(const mxArray *pa, size_t index)
{
	const mxArray *pid = mxGetField(pa, index, "id");
	if (!pid) return false;
	if (!mxIsScalar(pid)) return false;
	if (!mxIsDouble(pid)) return false;

	const mxArray *plabeled = mxGetField(pa, index, "labeled");
	if (!plabeled) return false;
	if (!mxIsScalar(plabeled)) return false;
	if (!mxIsLogical(plabeled)) return false;

	const mxArray *pbbox = mxGetField(pa, index, "bbox");
	if (!pbbox) return false;
	if (mxGetNumberOfElements(pbbox) != 4) return false;
	if (!mxIsDouble(pbbox)) return false;

	const mxArray *pocclusion = mxGetField(pa, index, "occlusion");
	if (!pocclusion) return false;
	if (!mxIsScalar(pocclusion)) return false;
	if (!mxIsLogical(pocclusion)) return false;

	const mxArray *pOutOfView = mxGetField(pa, index, "out_view");
	if (!pOutOfView) return false;
	if (!mxIsScalar(pOutOfView)) return false;
	if (!mxIsLogical(pOutOfView)) return false;

	const mxArray *pPath = mxGetField(pa, index, "path");
	if (!pPath) return false;
	if (!mxIsChar(pPath)) return false;

	return true;
}
