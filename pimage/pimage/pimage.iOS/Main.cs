using System;
using System.Collections.Generic;
using System.Linq;

using Foundation;
using UIKit;
using System.Drawing;
using CoreGraphics;
using pimage.iOS;

[assembly: Xamarin.Forms.Dependency(typeof(ImageIOIOS))]
namespace pimage.iOS
{
    public class Application
    {
        // This is the main entry point of the application.
        static void Main(string[] args)
        {
            // if you want to use a different Application Delegate class from "AppDelegate"
            // you can specify it here.
            UIApplication.Main(args, null, "AppDelegate");

            CGContext g = UIGraphics.GetCurrentContext();
            
        }

        void open_image()
        {
            var uimg = UIImage.FromFile("photo.JPG");
            var cimg = uimg.CGImage;
        }
    }
    
    public class ImageIOIOS: ImageIO
    {
        public void loadImage(string filename)
        {
            var uimg = UIImage.FromFile(filename);
            var cimg = uimg.CGImage;
            Console.WriteLine("bytesperrow: " + cimg.BytesPerRow.ToString());
            Console.WriteLine("size: " + cimg.Width.ToString() + cimg.Height.ToString());
            byte[] bimg = new byte[cimg.BytesPerRow * cimg.Height];
            //CGBitmapContext context = new CGBitmapContext(rgba, 1, 1, 8, 4, colorSpace, CGImageAlphaInfo.PremultipliedLast);
        }
    }
}
