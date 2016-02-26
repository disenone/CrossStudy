using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Xamarin.Forms;
using System.Runtime.InteropServices;


namespace pimage
{
    public class cimage
    {
        [DllImport("libcimage")]
        public extern static string dllInfo();

        [DllImport("libcimage")]
        public extern static int dllInt();
    }

    public interface ImageIO
    {
        void loadImage(string filename);
    }

    public class App : Application
    {
        public App()
        {
            var it = cimage.dllInt().ToString();
            DependencyService.Get<ImageIO>().loadImage("1.png");
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
