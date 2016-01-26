using System;

using UIKit;

using System.Runtime.InteropServices;

namespace IOSApp2
{
    public class MyClass
    {
        [DllImport("__Internal")]
        public extern static string iOSInfo();

        [DllImport("__Internal")]
        public extern static int iOSInt();
    }
    
    public partial class ViewController : UIViewController
    {       
        public ViewController(IntPtr handle) : base(handle)
        {
        }

        public override void ViewDidLoad()
        {
            base.ViewDidLoad();
            // Perform any additional setup after loading the view, typically from a nib.
        }

        public override void DidReceiveMemoryWarning()
        {
            base.DidReceiveMemoryWarning();
            // Release any cached data, images, etc that aren't in use.
        }

        partial void CheckButton_TouchUpInside(UIButton sender)
        {
            string title = MyClass.iOSInfo();
            int ii = MyClass.iOSInt();
            CheckButton.SetTitle(title, UIControlState.Normal);
            CheckButton.Enabled = true;
        }
    }
}