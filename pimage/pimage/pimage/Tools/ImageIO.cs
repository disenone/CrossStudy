using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace pimage.Tools
{

    public interface ImageIO
    {
        CImageByte LoadImageFromEmbeddedResource(string filename);

        byte[] ToPng(CImageByte img);
    }

}
