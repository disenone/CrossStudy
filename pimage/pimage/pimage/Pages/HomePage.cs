using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Diagnostics;
using Xamarin.Forms;
using pimage.Tools;
using System.Reflection;
using System.Runtime.InteropServices;


namespace pimage.Pages
{
    class HomePage : ContentPage
    {

        public HomePage()
        {
            Command<Type> navigateCommand = 
                new Command<Type>(async (Type pageType) =>
                {
                    Page page = (Page)Activator.CreateInstance(pageType);
                    await this.Navigation.PushAsync(page);
                });

            mergeTest();

            //             var imageInterface = DependencyService.Get<ImageIO>();
            // 
            //             imageInterface.openGallery();
        }

        private void mergeTest()
        {
            var cimg = new cimage();

            foreach (int i in new int[] { 1, 2, 3 })
            {
                var bimgs = DependencyService.Get<ImageIO>().LoadImageFromEmbeddedResource(
                    i.ToString() + ".PNG");
                cimg.AddImage(bimgs);
            }

            cimg.testImageBuffer();

            var ret = DependencyService.Get<ImageIO>().ToPng(CImageConverter.CImageBufferToByte(cimg.Ret));

            Debug.WriteLine("ret info: " + cimg.Ret.Width.ToString() + " " + cimg.Ret.Height.ToString());

            var stream = new MemoryStream(ret);
            stream.Position = 0;
            Debug.WriteLine("stream info: " + stream.Length.ToString());

            Image image = new Image
            {
                Source = ImageSource.FromStream(() => stream),
                VerticalOptions = LayoutOptions.CenterAndExpand,
            };
            Debug.WriteLine("Image info: " + image.Width.ToString() + " " + image.Height.ToString());

            this.Title = "Image Show";
            var layout = new StackLayout
            {
                Children =
                {
                    image
                }
            };

            var scroll = new ScrollView();
            scroll.Content = layout;
            this.Content = scroll;
        }

    }
}