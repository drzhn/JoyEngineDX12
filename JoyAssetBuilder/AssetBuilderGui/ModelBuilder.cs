using System;
using System.IO;
using System.Runtime.InteropServices;

namespace JoyAssetBuilder
{
    public class ModelBuilder
    {
        #region Dll

        const string dllPath = @"D:\CppProjects\JoyEngine\JoyAssetBuilder\x64\Debug\JoyDataBuilderLib.dll";

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        static extern unsafe int BuildModel(
            string modelFileName,
            IntPtr* vertexPtr,
            UInt64* vertexSize,
            IntPtr* indexPtr,
            UInt64* indexSize,
            IntPtr* errorMessage);

        static unsafe int BuildModel(string modelFileName,
            out byte[] vertexBuffer,
            out byte[] indexBuffer,
            out string errorMessage)
        {
            IntPtr vertexData = IntPtr.Zero;
            UInt64 vertexDataSize;
            IntPtr indexData = IntPtr.Zero;
            UInt64 indexDataSize;
            IntPtr errorMessagePtr = IntPtr.Zero;

            int result = BuildModel(modelFileName,
                &vertexData, &vertexDataSize,
                &indexData, &indexDataSize,
                &errorMessagePtr);
            if (result == 0)
            {
                vertexBuffer = new byte[vertexDataSize];
                indexBuffer = new byte[indexDataSize];
                Marshal.Copy(vertexData, vertexBuffer, 0, (int)vertexDataSize);
                Marshal.Copy(indexData, indexBuffer, 0, (int)indexDataSize);
                errorMessage = null;
            }
            else
            {
                vertexBuffer = null;
                indexBuffer = null;
                errorMessage = Marshal.PtrToStringAnsi(errorMessagePtr);
            }

            return result;
        }
        #endregion

        public static bool BuildModel(string modelPath, out string resultMessage)
        {
            int result = BuildModel(modelPath, out var vertexBuffer, out var indexBuffer, out var buidlResult);
            if (result != 0)
            {
                resultMessage = Path.GetFileName(modelPath) + ": Error building model\n" + buidlResult +
                                Environment.NewLine;
                return false;
            }

            FileStream fileStream = new FileStream(modelPath + ".data", FileMode.Create);
            fileStream.Write(BitConverter.GetBytes(vertexBuffer.Length), 0, 4);
            fileStream.Write(BitConverter.GetBytes(indexBuffer.Length), 0, 4);
            fileStream.Write(vertexBuffer, 0, vertexBuffer.Length);
            fileStream.Write(indexBuffer, 0, indexBuffer.Length);
            fileStream.Close();
            resultMessage = Path.GetFileName(modelPath) + ": OK" + Environment.NewLine;
            return true;
        }

    }
}