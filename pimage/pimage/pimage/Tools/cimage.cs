using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Diagnostics;


namespace pimage.Tools
{
    [StructLayout(LayoutKind.Sequential)]
    public struct CImageBuffer
    {
        public CImageBuffer(byte[] bytebuf, int width)
        {
            Length = (uint)bytebuf.Length;
            Width = (uint)width;
            Selfgc = 0;
            unsafe
            {
                fixed (byte* p = &bytebuf[0])
                {
                    Ptr = (IntPtr)p;
                }
            }
        }

        public IntPtr Ptr;

        public uint Length;

        public uint Width;

        public uint Selfgc;
    }

    public class cimage
    {
        [DllImport("libcimage")]
        public extern static int dllInt();

        [DllImport("libcimage")]
        public extern static int testImageBuffer(IntPtr bufs, int len);

        [DllImport("libcimage")]
        public extern static void setDebugLogFunc(DebugLogFunc func);

        public delegate void DebugLogFunc(IntPtr pstr);

        static void DebugLog(IntPtr pstr)
        {
            var log = Marshal.PtrToStringAnsi(pstr);
            Debug.WriteLine(log);
            //Debug.WriteLine("DebugLog");
        }

        public void AddImage(byte[] buf, int width)
        {
            imgs.Add(new CImageBuffer(buf, width));
        }

        public void testImageBuffer()
        {
            CImageBuffer[] bufs = new CImageBuffer[imgs.Count];
            bufs[0] = imgs[0];

            setDebugLogFunc(DebugLog);

            unsafe
            {
                fixed (CImageBuffer* p = &bufs[0])
                {
                    var ret = testImageBuffer((IntPtr)p, imgs.Count);
                    Debug.WriteLine("ret: " + ret.ToString());
                }
            }

        }

        List<CImageBuffer> imgs =  new List<CImageBuffer>();
    }
}
