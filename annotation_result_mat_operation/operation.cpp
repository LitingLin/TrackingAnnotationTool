#include <matio.h>

#include <string>

class AnnotationOperator
{
public:
	AnnotationOperator(const std::wstring &matFilePath);
	int getNumberOfRecords();
	int getRecord(int index, int *id, int *labeled, int *x, int *y, int *w, int *h, int *occlusion, int *out_view);
	int updateRecord(int index, int id, int labeled, int x, int y, int w, int h, int occlusion, int out_view, const char *path);
};

AnnotationOperator::AnnotationOperator(const std::wstring& matFilePath)
{
}
