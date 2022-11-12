using JoyAssetBuilder.Properties;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows.Forms;

namespace JoyAssetBuilder
{
    public static class BuilderFacade
    {
        private const string m_dllPath = @"AssetBuilderLib.dll";

        public static void Initialize()
        {
            if (!File.Exists(m_dllPath))
            {
                MessageBox.Show("Cannot find");
            }

            InitializeBuilder();
        }

        public static void Terminate()
        {
            TerminateBuilder();
        }

        public static unsafe int BuildModel(string modelFileName, string materialsDir, out string errorMessage)
        {
            IntPtr errorMessagePtr = IntPtr.Zero;

            int result = BuildModel(modelFileName, materialsDir, &errorMessagePtr);
            if (result == 0)
            {
                errorMessage = null;
            }
            else
            {
                errorMessage = Marshal.PtrToStringAnsi(errorMessagePtr);
            }

            return result;
        }

        [DllImport(m_dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern unsafe int BuildModel(string modelFileName, string materialsDir, IntPtr* errorMessage);


        public static unsafe int BuildTexture(string textureFileName, out string errorMessage)
        {
            IntPtr errorMessagePtr = IntPtr.Zero;

            int result = BuildTexture(textureFileName, &errorMessagePtr);
            if (result == 0)
            {
                errorMessage = null;
            }
            else
            {
                errorMessage = Marshal.PtrToStringAnsi(errorMessagePtr);
            }

            return result;
        }


        [DllImport(m_dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern unsafe int BuildTexture(string textureFileName, IntPtr* errorMessage);


        [DllImport(m_dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern unsafe int InitializeBuilder();


        [DllImport(m_dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern unsafe int TerminateBuilder();
    }
}