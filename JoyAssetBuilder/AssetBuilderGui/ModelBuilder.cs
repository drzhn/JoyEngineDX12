using System;
using System.IO;
using System.Runtime.InteropServices;

namespace JoyAssetBuilder
{
    public class ModelBuilder
    {
        public static bool BuildModel(string modelPath, string materialsDir, out string resultMessage)
        {
            int result = BuilderFacade.BuildModel(modelPath,  materialsDir, out var buidlResult);
            if (result != 0)
            {
                resultMessage = Path.GetFileName(modelPath) + ": Error building model\n" + buidlResult +
                                Environment.NewLine;
                return false;
            }

            resultMessage = Path.GetFileName(modelPath) + ": OK" + Environment.NewLine;
            return true;
        }
    }
}