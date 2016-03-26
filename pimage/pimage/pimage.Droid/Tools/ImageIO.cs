using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Android.App;
using Android.Content;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using Android.Graphics;
using System.IO;
using System.Reflection;
using pimage.Tools;


[assembly: Xamarin.Forms.Dependency(typeof(pimage.Droid.Tools.ImageIODroid))]
namespace pimage.Droid.Tools
{
    public class ImageIODroid : pimage.Tools.ImageIO
    {
        public CImageByte LoadImageFromEmbeddedResource(string filename)
        {
            var stream = pimage.Tools.ResourceLoader.GetEmbeddedResourceStream(filename);
            var bitmap = BitmapFactory.DecodeStream(stream);
            var bmpdata = bitmap.LockPixels();
            byte[] bytedata = new byte[bitmap.ByteCount];
            System.Runtime.InteropServices.Marshal.Copy(bmpdata, bytedata, 0, bitmap.ByteCount);

            return new CImageByte(bytedata, (uint)bitmap.Width, 4);
        }

        public byte[] ToPng(CImageByte img)
        {
            if (img.Channel != 4)
                return null;

            Bitmap bitmap = Bitmap.CreateBitmap((int)img.Width, (int)img.Height, Bitmap.Config.Argb8888);

            var byteBuffer = Java.Nio.ByteBuffer.Wrap(img.Bytes);
            bitmap.CopyPixelsFromBuffer(byteBuffer);

            MemoryStream stream = new MemoryStream();
            bitmap.Compress(Bitmap.CompressFormat.Png, 50, stream);

            return stream.ToArray();
        }
    }
}