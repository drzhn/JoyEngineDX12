using System;
using System.IO;
using System.Runtime.InteropServices;

namespace JoyAssetBuilder
{
    public class TextureBuilder
    {
        public static bool BuildTexture(string texturePath, out string resultMessage)
        {
            int result = BuilderFacade.BuildTexture(
                texturePath, out var textureBuffer,
                out var width, out var height, out var type, out var buidlResult);
            if (result != 0)
            {
                resultMessage = Path.GetFileName(texturePath) + ": Error building texture\n" + buidlResult +
                                Environment.NewLine;
                return false;
            }

            FileStream fileStream = new FileStream(texturePath + ".data", FileMode.Create);
            fileStream.Write(BitConverter.GetBytes(width), 0, 4);
            fileStream.Write(BitConverter.GetBytes(height), 0, 4);
            fileStream.Write(BitConverter.GetBytes(type), 0, 4);
            fileStream.Write(textureBuffer, 0, textureBuffer.Length);
            fileStream.Close();
            resultMessage = Path.GetFileName(texturePath) + ": OK" + Environment.NewLine;
            return true;
        }
    }
}