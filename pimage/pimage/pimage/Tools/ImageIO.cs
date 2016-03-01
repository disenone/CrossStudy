using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace pimage.Tools
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
        byte[] LoadImageFromEmbeddedResource(string filename);

        byte[] ToPng(byte[] rgba, int width);
    }

}
