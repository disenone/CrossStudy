using System;
using System.Collections.Generic;
using System.Linq;
using Foundation;
using UIKit;
using CoreGraphics;

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
    

}
