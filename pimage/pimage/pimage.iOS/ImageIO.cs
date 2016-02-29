using System;
using System.Collections.Generic;
using System.Text;
using UIKit;
using CoreGraphics;
using pimage.iOS;


[assembly: Xamarin.Forms.Dependency(typeof(ImageIOIOS))]
namespace pimage.iOS
{
    public class ImageIOIOS : Tools.ImageIO
    {
        public byte[] LoadImage(string filename)
        {
            var uimg = UIImage.FromFile(filename);
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

            return bimg;

        }
        
        public byte[] ToPng(byte[] rgba, int width)
        {
            var provider = new CGDataProvider(rgba, 0, rgba.Length);
            int bitsPerComponent = 8;
            int components = 4;
            int height = rgba.Length / components / width;
            int bitsPerPixel = components * bitsPerComponent;
            int bytesPerRow = components * width;   // Tip:  When you create a bitmap graphics context, you’ll get the best performance if you make sure the data and bytesPerRow are 16-byte aligned.
            bool shouldInterpolate = false;
            var colorSpace = CGColorSpace.CreateDeviceRGB();
            var cgImage = new CGImage(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow,
                                      colorSpace, CGImageAlphaInfo.PremultipliedLast, provider,
                                      null, shouldInterpolate, CGColorRenderingIntent.Default);

            var uimg = UIImage.FromImage(cgImage);
            return uimg.AsPNG().ToArray();
        }
    }
}
