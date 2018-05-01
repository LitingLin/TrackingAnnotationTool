using System;
using System.Runtime.InteropServices;

namespace AnnotationTool
{
    public enum DesiredAccess : int
    {
        Read = 0, Write,
        Both = Read | Write
    };

    public enum CreationDisposition : int
    {
        OpenAlways = 0,
        CreateAlways
    };

    public class AnnotationRecordOperator : IDisposable
    {
        [DllImport("annotation-record-operator.dll", CharSet = CharSet.Unicode)]
        private static extern IntPtr createAnnotationOperator([MarshalAs(UnmanagedType.BStr)]string matFilePath,
            DesiredAccess desiredAccess, CreationDisposition creationDisposition);

        [DllImport("annotation-record-operator.dll")]
        private static extern bool getAnnotationNumberOfRecords(IntPtr handle, out ulong numberOfRecord);

        [DllImport("annotation-record-operator.dll", CharSet = CharSet.Unicode)]
        private static extern bool getAnnotationRecord(IntPtr handle, ulong index, out int id, out bool isLabeled, 
            out int x, out int y, out int w, out int h, out bool occlusion, out bool outOfView, [MarshalAs(UnmanagedType.BStr)] out string path);

        [DllImport("annotation-record-operator.dll", CharSet = CharSet.Unicode)]
        private static extern bool updateAnnotationRecord(IntPtr handle, ulong index, int id, bool isLabeled,
            int x, int y, int w, int h, bool occlusion, bool outOfView, [MarshalAs(UnmanagedType.BStr)] string path);

        [DllImport("annotation-record-operator.dll")]
        private static extern bool resizeAnnotationRecord(IntPtr handle, ulong size);

        [DllImport("annotation-record-operator.dll")]
        private static extern bool destroyAnnotationOperator(IntPtr handle);


        public AnnotationRecordOperator(string matFilePath, DesiredAccess desiredAccess,
            CreationDisposition creationDisposition)
        {
            _nativeObject = createAnnotationOperator(matFilePath, desiredAccess, creationDisposition);
            if (_nativeObject == IntPtr.Zero)
                throw new ArgumentException();
        }

        public UInt64 GetNumberOfRecords()
        {
            if (!getAnnotationNumberOfRecords(_nativeObject, out var numberOfRecords))
                throw new InvalidOperationException();
            return numberOfRecords;
        }

        public bool Get(UInt64 index, out int id, out bool isLabeled,
            out int x, out int y, out int w, out int h, out bool occlusion, out bool outOfView, out string path)
        {
            return getAnnotationRecord(_nativeObject, index, out id, out isLabeled, out x, out y, out w, out h,
                out occlusion, out outOfView, out path);
        }

        public void Update(ulong index, int id, bool isLabeled,
            int x, int y, int w, int h, bool occlusion, bool outOfView, string path)
        {
            if (!updateAnnotationRecord(_nativeObject, index, id, isLabeled, x, y, w, h, occlusion, outOfView, path))
                throw new InvalidOperationException();
        }

        public void Resize(ulong size)
        {
            if (!resizeAnnotationRecord(_nativeObject, size))
                throw new InvalidOperationException();
        }

        public void Dispose()
        {
            destroyAnnotationOperator(_nativeObject);
        }

        private readonly IntPtr _nativeObject;
    }
}
