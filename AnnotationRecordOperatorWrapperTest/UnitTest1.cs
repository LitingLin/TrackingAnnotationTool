using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using AnnotationTool;

namespace AnnotationRecordOperatorWrapperTest
{
    [TestClass]
    public class AnnotationRecordOperatorWrapper_UnitTest
    {
        [TestMethod]
        public void TestRead()
        {
            using (AnnotationRecordOperator annotationRecordOperator =
                new AnnotationRecordOperator("res.mat", DesiredAccess.Read, CreationDisposition.OpenAlways))
            {
                UInt64 numberOfRecords = annotationRecordOperator.GetNumberOfRecords();
                for (UInt64 index = 0; index < numberOfRecords; ++index)
                {
                    int id, x, y, w, h;
                    bool isLabeled, occlusion, outOfView;
                    string path;
                    Assert.IsTrue(annotationRecordOperator.Get(index, out id, out isLabeled, out x, out y, out w, out h,
                        out occlusion, out outOfView, out path));
                }
            }

        }

        [TestMethod]
        public void TestWrite()
        {
            using (AnnotationRecordOperator annotationRecordOperator =
                new AnnotationRecordOperator("res1.mat", DesiredAccess.Both, CreationDisposition.CreateAlways))
            {
                ulong n = 5;

                int id_ = 1 , x_ = 1, y_ = 1, w_ = 1, h_ = 1;
                bool isLabeled_ = true, occlusion_ = false, outOfView_ = false;
                string path_ = "0001.jpg";

                annotationRecordOperator.Resize(n);
                for (ulong index = 0; index < n; ++index)
                {
                    annotationRecordOperator.Update(index, id_, isLabeled_, x_, y_, w_, h_, occlusion_, outOfView_, path_);
                }

                ulong numberOfRecords = annotationRecordOperator.GetNumberOfRecords();
                for (ulong index = 0; index < numberOfRecords; ++index)
                {
                    int id, x, y, w, h;
                    bool isLabeled, occlusion, outOfView;
                    string path;
                    Assert.IsTrue(annotationRecordOperator.Get(index, out id, out isLabeled, out x, out y, out w, out h,
                        out occlusion, out outOfView, out path));
                }
            }
        }
    }
}
