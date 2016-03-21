using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Diagnostics;


namespace pimage.Tools
{
    public struct CImageByte
    {
        public CImageByte(byte[] bytebuf, uint width, uint channel)
        {
            Bytes = bytebuf;
            Width = width;
            Channel = channel;
        }

        public byte[] Bytes;
        public uint Width;
        public uint Channel;
        public uint Height
        {
            get
            {
                if(Width == 0 || Channel == 0 || Length == 0)
                {
                    return 0;
                }
                return Length / (Width * Channel);
            }
        }
        public uint Length
        {
            get { return (uint)Bytes.Length; }
        }
        public uint Stride
        {
            get { return Width * Channel; }
        }

    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CImageBuffer
    {
        public CImageBuffer(byte[] bytebuf, uint width, uint channel)
        {
            Length = (uint)bytebuf.Length;
            Width = width;
            Channel = channel;
            Height = Length / Width / Channel;
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

        public uint Height;

        public uint Channel;

        public uint Selfgc;
    }

    public class CImageConverter
    {
        static public CImageBuffer CImageByteToBuffer(CImageByte img)
        {
            return new CImageBuffer(img.Bytes, img.Width, img.Channel);
        }

        static public CImageByte CImageBufferToByte(CImageBuffer img)
        {
            byte[] bytes = new byte[img.Length];
            Marshal.Copy(img.Ptr, bytes, 0, (int)img.Length);
            return new CImageByte(bytes, img.Width, img.Channel);
        }
    }


    public class cimage
    {
        [DllImport("libcimage")]
        public extern static int dllInt();

        [DllImport("libcimage")]
        public extern static void setDebugLogFunc(DebugLogFunc func);

        [DllImport("libcimage")]
        public extern static int runImageMerge(IntPtr bufs, int len, out CImageBuffer ret);

        public delegate void DebugLogFunc(IntPtr pstr);

        static void DebugLog(IntPtr pstr)
        {
            var log = Marshal.PtrToStringAnsi(pstr);
            Debug.WriteLine(log);
            //Debug.WriteLine("DebugLog");
        }

        public void AddImage(CImageByte img)
        {
            imgs.Add(img);

        }

        public CImageBuffer testImageBuffer()
        {
            CImageBuffer[] bufs = new CImageBuffer[imgs.Count];
            for (int i= 0; i < imgs.Count; ++i)
            {
                bufs[i] = CImageConverter.CImageByteToBuffer(imgs[i]);
            }

            Debug.WriteLine(Marshal.SizeOf(bufs[0]).ToString());
            //setDebugLogFunc(DebugLog);

            unsafe
            {
                fixed (CImageBuffer* p = &bufs[0])
                {
                    int state = runImageMerge((IntPtr)p, imgs.Count, out ret);
                    Debug.WriteLine("ret: " + ret.ToString());
                }
            }
            return ret;
        }

        List<CImageByte> imgs =  new List<CImageByte>();
        public CImageBuffer Ret
        { get
            {
                return ret;
            }
        }
        CImageBuffer ret;
    }
}
