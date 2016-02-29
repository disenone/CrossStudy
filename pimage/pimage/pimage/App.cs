using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xamarin.Forms;
using pimage.Tools;
using pimage.Pages;


namespace pimage
{
    public class App : Application
    {
        public App()
        {
            //var it = cimage.dllInt().ToString();
            //var bimgs = DependencyService.Get<ImageIO>().loadImage("1.png");
            MainPage = new NavigationPage(new HomePage());
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
