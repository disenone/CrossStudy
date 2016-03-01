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


[assembly: Xamarin.Forms.Dependency(typeof(pimage.Droid.Tools.ImageIODroid))]
namespace pimage.Droid.Tools
{
    public class ImageIODroid : pimage.Tools.ImageIO
    {
        public byte[] LoadImageFromEmbeddedResource(string filename)
        {
            var stream = pimage.Tools.ResourceLoader.GetEmbeddedResourceStream(filename);
            var bitmap = BitmapFactory.DecodeStream(stream);
            var bmpdata = bitmap.LockPixels();
            byte[] bytedata = new byte[bitmap.ByteCount];
            System.Runtime.InteropServices.Marshal.Copy(bmpdata, bytedata, 0, bitmap.ByteCount);

            return bytedata;
        }

        public byte[] ToPng(byte[] rgba, int width)
        {
            int components = 4;
            int height = rgba.Length / components / width;

            Bitmap img = Bitmap.CreateBitmap(width, height, Bitmap.Config.Argb8888);

            var byteBuffer = Java.Nio.ByteBuffer.Wrap(rgba);
            img.CopyPixelsFromBuffer(byteBuffer);

            MemoryStream stream = new MemoryStream();
            img.Compress(Bitmap.CompressFormat.Png, 50, stream);

            return stream.ToArray();
        }
    }
}