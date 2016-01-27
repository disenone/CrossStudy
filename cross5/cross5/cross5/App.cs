using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

using Xamarin.Forms;

namespace cross5
{
//     public class MyClass
//     {
//         [DllImport("libSharedLibrary1.so")]
//         public extern static string AndroidInfo();
// 
//         [DllImport("libSharedLibrary1.so")]
//         public extern static int AndroidInt();
//     }
    
    public class MyClassiOS
    {
        [DllImport("__Internal")]
        public extern static string iOSInfo();

        [DllImport("__Internal")]
        public extern static int iOSInt();
    }

    public class App : Application
    {
        public App()
        {
            //var tt = MyClass.AndroidInfo();
            var it = MyClassiOS.iOSInt().ToString();
            // The root page of your application
            MainPage = new ContentPage
            {
                Content = new StackLayout
                {
                    VerticalOptions = LayoutOptions.Center,
                    Children = {
                        new Label {
                            XAlign = TextAlignment.Center,
                            //Text = "Welcome to Xamarin Forms!"
                            Text = it
                        }
                    }
                }
            };
        }

        protected override void OnStart()
        {
            // Handle when your app starts
        }

        protected override void OnSleep()
        {
            // Handle when your app sleeps
        }

        protected override void OnResume()
        {
            // Handle when your app resumes
        }
    }
}
