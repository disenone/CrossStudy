using System;
using System.Collections.Generic;
using System.Text;
using UIKit;
using CoreGraphics;
using pimage.Tools;

[assembly: Xamarin.Forms.Dependency(typeof(pimage.iOS.Tools.ImageIOIOS))]
namespace pimage.iOS.Tools
{
    public class ImageIOIOS : pimage.Tools.ImageIO
    {
        public CImageByte LoadImageFromEmbeddedResource(string filename)
        {
            
            var uimg = UIImage.LoadFromData(Foundation.NSData.FromStream(
                pimage.Tools.ResourceLoader.GetEmbeddedResourceStream(filename)
                ));
            var cimg = uimg.CGImage;

            //Console.WriteLine("bytesperrow: " + cimg.BytesPerRow.ToString());
            //Console.WriteLine("size: " + cimg.Width.ToString() + cimg.Height.ToString());

            nint bitsPerComponent = 8;
            nint bytesPerRow = cimg.Width * 4;       // rgba
            var colorSpace = CGColorSpace.CreateDeviceRGB();
            byte[] bimg = new byte[bytesPerRow * cimg.Height];
            var context = new CGBitmapContext(bimg, cimg.Width, cimg.Height, bitsPerComponent,
                bytesPerRow, colorSpace, CGImageAlphaInfo.PremultipliedLast);
            context.DrawImage(new CGRect(0, 0, cimg.Width, cimg.Height), cimg);

            return new CImageByte(bimg, (uint)cimg.Width, 4);

        }
        
        public byte[] ToPng(CImageByte img)
        {
            var provider = new CGDataProvider(img.Bytes, 0, (int)img.Length);
            int bitsPerComponent = 8;
            int components = (int)img.Channel;
            int bitsPerPixel = components * bitsPerComponent;
            int bytesPerRow = (int)img.Stride;

            // Tip:  When you create a bitmap graphics context, 
            // you’ll get the best performance if you make sure the data and bytesPerRow are 16-byte aligned.

            bool shouldInterpolate = false;
            var colorSpace = CGColorSpace.CreateDeviceRGB();
            var cgImage = new CGImage(
                (int)img.Width, (int)img.Height, bitsPerComponent, bitsPerPixel, bytesPerRow,
                 colorSpace, CGImageAlphaInfo.PremultipliedLast, provider,
                 null, shouldInterpolate, CGColorRenderingIntent.Default);

            var uimg = UIImage.FromImage(cgImage);

            Console.WriteLine("topng info: " + uimg.Size.ToString());

            return uimg.AsPNG().ToArray();
        }

        public void openGallery()
        {

        }
    }
}
