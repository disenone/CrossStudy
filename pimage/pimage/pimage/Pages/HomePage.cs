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

            var cimg = new cimage();

            foreach(int i in new int[] {1, 2, 3} )
            {
                byte[] bimgs = DependencyService.Get<ImageIO>().LoadImageFromEmbeddedResource(
                    i.ToString() + ".png");
                cimg.AddImage(bimgs, 1242);
            }

            cimg.testImageBuffer();

            bimgs = DependencyService.Get<ImageIO>().ToPng(bimgs, 1242);

            var stream = new MemoryStream(bimgs);
            stream.Position = 0;
            Debug.WriteLine("stream info: " + stream.Length.ToString());

            Image image = new Image
            {
                Source = ImageSource.FromStream(() => stream),
                VerticalOptions = LayoutOptions.CenterAndExpand
            };

            Debug.WriteLine("Image info: " + image.Width.ToString() + " " + image.Height.ToString());
            Debug.WriteLine("byte info: (" + bimgs[0].ToString() + ", " + 
                bimgs[1].ToString() + ", " + bimgs[2].ToString() + ", " + bimgs[2].ToString() + ")");

            this.Title = "Image Show";
            this.Content = new StackLayout
            {
                Children =
                {
                    image
                }
            };
            
        }

    }
}