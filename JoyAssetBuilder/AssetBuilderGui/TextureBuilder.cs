using System;
using System.IO;
using System.Runtime.InteropServices;

namespace JoyAssetBuilder
{
    public class TextureBuilder
    {
        public static bool BuildTexture(string texturePath, out string resultMessage)
        {
            int result = BuilderFacade.BuildTexture(texturePath, out var buildResult);
            if (result != 0)
            {
                resultMessage = Path.GetFileName(texturePath) + ": Error building texture\n" + buildResult +
                                Environment.NewLine;
                return false;
            }

            resultMessage = Path.GetFileName(texturePath) + ": OK" + Environment.NewLine;
            return true;
        }
    }
}