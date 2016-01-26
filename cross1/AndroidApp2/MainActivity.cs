using System;
using Android.App;
using Android.Content;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using Android.OS;
using System.Runtime.InteropServices;

namespace AndroidApp2
{
    [Activity(Label = "AndroidApp2", MainLauncher = true, Icon = "@drawable/icon")]
    public class MainActivity : Activity
    {
        int count = 1;

        [DllImport("libDroid.so")]
        public extern static string AndroidInfo();

        protected override void OnCreate(Bundle bundle)
        {
            base.OnCreate(bundle);

            var mystr = AndroidInfo();

            // Set our view from the "main" layout resource
            SetContentView(Resource.Layout.Main);

            // Get our button from the layout resource,
            // and attach an event to it
            Button button = FindViewById<Button>(Resource.Id.MyButton);

            button.Click += delegate { button.Text = mystr; };
        }
    }
}

