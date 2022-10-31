using System;
using System.IO;
using System.Runtime.InteropServices;

namespace JoyAssetBuilder
{
    public class TextureBuilder
    {
        #region Dll

        const string dllPath = @"D:\CppProjects\JoyEngine\JoyAssetBuilder\x64\Debug\JoyDataBuilderLib.dll";

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        static extern unsafe int BuildTexture(
            string textureFileName,
            IntPtr* textureDataPtr,
            UInt64* textureDataSize,
            UInt32* textureWidth,
            UInt32* textureHeight,
            UInt32* textureType,
            IntPtr* errorMessage);

        static unsafe int BuildTexture(string textureFileName,
            out byte[] textureBuffer,
            out uint width,
            out uint height,
            out uint type,
            out string errorMessage)
        {
            IntPtr textureData = IntPtr.Zero;
            UInt64 textureDataSize;
            UInt32 textureWidth;
            UInt32 textureHeight;
            UInt32 textureType;

            IntPtr errorMessagePtr = IntPtr.Zero;

            int result = BuildTexture(
                textureFileName,
                &textureData, &textureDataSize,
                &textureWidth, &textureHeight,
                &textureType,
                & errorMessagePtr);
            if (result == 0)
            {
                textureBuffer = new byte[textureDataSize];
                Marshal.Copy(textureData, textureBuffer, 0, (int)textureDataSize);
                width = textureWidth;
                height = textureHeight;
                type=textureType;
                errorMessage = null;
            }
            else
            {
                textureBuffer = null;
                width = 0;
                height = 0;
                type = 0;  
                errorMessage = Marshal.PtrToStringAnsi(errorMessagePtr);
            }

            return result;
        }

        #endregion

        public static bool BuildTexture(string texturePath, out string resultMessage)
        {
            int result = BuildTexture(
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