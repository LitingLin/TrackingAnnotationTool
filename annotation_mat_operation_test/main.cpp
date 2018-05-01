#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <operation.h>

TEST_CASE("read")
{
	AnnotationOperator op(L"res.mat", AnnotationOperator::DesiredAccess::read, AnnotationOperator::CreationDisposition::open_always);
	size_t numberOfRecords = op.getNumberOfRecords();
	int id, x, y, w, h;
	bool labeled, occlusion, outOfView;
	std::wstring path;
	for (size_t i = 0; i < numberOfRecords; ++i) {
		CHECK(op.get(i, &id, &labeled, &x, &y, &w, &h, &occlusion, &outOfView, &path));
	}
}

TEST_CASE("write")
{
	{
		AnnotationOperator op(L"res1.mat", AnnotationOperator::DesiredAccess::write, AnnotationOperator::CreationDisposition::create_always);
		CHECK(op.getNumberOfRecords() == 0);
		size_t n = 5;
		op.resize(n);
		int id, x=1, y=2, w=3, h=4;
		bool labeled = true, occlusion = true, outOfView = true;
		std::wstring path = L"0001.jpg";
		for (int i = 0; i < n; ++i) {
			id = i;
			op.update(i, id, labeled, x, y, w, h, occlusion, outOfView,path);
		}
		const size_t numberOfRecords = op.getNumberOfRecords();
		for (size_t i = 0; i < numberOfRecords; ++i) {
			CHECK(op.get(i, &id, &labeled, &x, &y, &w, &h, &occlusion, &outOfView, &path));
		}
	}
}
