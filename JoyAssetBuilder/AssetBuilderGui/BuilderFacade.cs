﻿using JoyAssetBuilder.Properties;
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


        public static unsafe int BuildTexture(string textureFileName,
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
                &errorMessagePtr);
            if (result == 0)
            {
                textureBuffer = new byte[textureDataSize];
                Marshal.Copy(textureData, textureBuffer, 0, (int)textureDataSize);
                width = textureWidth;
                height = textureHeight;
                type = textureType;
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


        [DllImport(m_dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern unsafe int BuildTexture(
            string textureFileName,
            IntPtr* textureDataPtr,
            UInt64* textureDataSize,
            UInt32* textureWidth,
            UInt32* textureHeight,
            UInt32* textureType,
            IntPtr* errorMessage);


        [DllImport(m_dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern unsafe int InitializeBuilder();


        [DllImport(m_dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern unsafe int TerminateBuilder();
    }
}