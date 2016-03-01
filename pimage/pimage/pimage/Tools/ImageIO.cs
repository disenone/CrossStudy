using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace pimage.Tools
{

    public interface ImageIO
    {
        byte[] LoadImageFromEmbeddedResource(string filename);

        byte[] ToPng(byte[] rgba, int width);
    }

}
