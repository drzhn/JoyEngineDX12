using System;
using System.IO;
using System.Runtime.InteropServices;

namespace JoyAssetBuilder
{
    public class ModelBuilder
    {
        public static bool BuildModel(string modelPath, out string resultMessage)
        {
            int result = BuilderFacade.BuildModel(modelPath, out var vertexBuffer, out var indexBuffer,
                out var buidlResult);
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