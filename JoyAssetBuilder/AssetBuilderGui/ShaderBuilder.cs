using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace JoyAssetBuilder
{
    public class ShaderBuilder
    {
        [Flags]
        private enum ShaderType : uint
        {
            Vertex = 1 << 0,
            Fragment = 1 << 1
        };


        private const string dllPath = @"D:\CppProjects\JoyEngine\JoyAssetBuilder\x64\Release\JoyShaderBuilderLib.dll";


        private struct ShaderDefine
        {
            public uint ShaderUsage;
            public string DefineString;
        }

        private static Dictionary<string, ShaderDefine> defines = new Dictionary<string, ShaderDefine>()
        {
            {
                "JOY_VARIABLES", new ShaderDefine()
                {
                    ShaderUsage = (uint)(ShaderType.Vertex | ShaderType.Fragment),
                    DefineString = Properties.Resources.JOY_VARIABLES
                }
            },
            {
                "GBUFFER_TEXTURES", new ShaderDefine()
                {
                    ShaderUsage = (uint)(ShaderType.Fragment),
                    DefineString = Properties.Resources.GBUFFER_TEXTURES
                }
            }
        };

        private const StringSplitOptions noEmpty = StringSplitOptions.RemoveEmptyEntries;

        private static void ReadInputOrOutput(string body, ref List<string> vars)
        {
            Char[] separators = new[] { ';' };
            foreach (string varStr in body.Split(separators, noEmpty))
            {
                if (string.IsNullOrWhiteSpace(varStr)) continue;
                vars.Add(varStr.Trim(' ', '\r', '\n', '\t'));
            }
        }

        #region Compiler

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        static extern void InitializeCompiler();

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        static extern unsafe int CompileGLSL(string shader, int shdaerSize, ShaderType type, IntPtr* dataPtr,
            UInt64* dataSize, IntPtr* errorMessage);

        static unsafe int CompileGLSL(string shader, ShaderType type, out byte[] buffer, out string errorMessage)
        {
            IntPtr outData = IntPtr.Zero;
            UInt64 len;
            IntPtr errorMessagePtr = IntPtr.Zero;
            int result = CompileGLSL(shader, shader.Length, type, &outData, &len, &errorMessagePtr);
            if (result == 0)
            {
                buffer = new byte[len];
                Marshal.Copy(outData, buffer, 0, (int)len);
                errorMessage = null;
            }
            else
            {
                buffer = null;
                errorMessage = Marshal.PtrToStringAnsi(errorMessagePtr);
            }

            return result;
        }

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        static extern void ReleaseInternalData();

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        static extern void ReleaseCompiler();

        #endregion

        #region Test

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        static extern unsafe void GetString(IntPtr* stringPtr);

        static unsafe void GetString()
        {
            IntPtr outString = IntPtr.Zero;
            GetString(&outString);
            System.Console.WriteLine(Marshal.PtrToStringAnsi(outString));
        }

        #endregion

        public static bool Compile(string shaderPath, out string message)
        {
            string shader = File.ReadAllText(shaderPath);

            const string vertInputAttr = "vert_input";
            const string vertToFragInputAttr = "vert_to_frag";
            const string fragOutputAttr = "frag_output";
            const string vertShaderAttr = "vertex_shader";
            const string fragShaderAttr = "fragment_shader";
            const string definesAttr = "defines";


            List<string> directives = new List<string>();
            List<string> verInputs = new List<string>();
            List<string> vertToFragInputs = new List<string>();
            List<string> fragOutputs = new List<string>();
            List<string> definesList = new List<string>();
            string vertexShader = "";
            string fragmentShader = "";


            for (int i = 0; i < shader.Length; i++)
            {
                if (shader[i] == '#')
                {
                    int posSharp = shader.IndexOf('#', i + 1);
                    int posBracket = shader.IndexOf('[', i + 1);
                    int posDirectiveEnd = -1;
                    if (posSharp == -1 && posBracket == -1)
                    {
                        posDirectiveEnd = shader.Length - 1;
                    }
                    else if (posSharp != -1 && posBracket != -1)
                    {
                        posDirectiveEnd = Math.Min(posSharp, posBracket);
                    }
                    else if (posSharp == -1 && posBracket != -1)
                    {
                        posDirectiveEnd = posBracket;
                    }
                    else if (posSharp != -1 && posBracket == -1)
                    {
                        posDirectiveEnd = posSharp;
                    }

                    directives.Add(shader.Substring(i, posDirectiveEnd - i - 1).Trim('\t', ' ', '\r', '\n'));
                    i = posDirectiveEnd - 1;
                }

                if (shader[i] == '[')
                {
                    int posBracket = shader.IndexOf(']', i + 1);
                    string header = shader.Substring(i + 1, posBracket - i - 1).Trim('\t', ' ', '\r', '\n');
                    int openBrace = shader.IndexOf('{', posBracket);
                    int closeBrace = -1;
                    int count = 1;
                    for (int j = openBrace + 1; j < shader.Length; j++)
                    {
                        if (shader[j] == '}') count--;
                        if (shader[j] == '{') count++;
                        if (count == 0)
                        {
                            closeBrace = j;
                            break;
                        }
                    }

                    string body = shader.Substring(openBrace + 1, closeBrace - openBrace - 1)
                        .Trim('\t', ' ', '\r', '\n');
                    i = closeBrace - 1;


                    if (header == vertInputAttr)
                    {
                        ReadInputOrOutput(body, ref verInputs);
                    }

                    if (header == vertToFragInputAttr)
                    {
                        ReadInputOrOutput(body, ref vertToFragInputs);
                    }

                    if (header == fragOutputAttr)
                    {
                        ReadInputOrOutput(body, ref fragOutputs);
                    }

                    if (header == definesAttr)
                    {
                        ReadInputOrOutput(body, ref definesList);

                        foreach (string define in definesList)
                        {
                            if (!defines.ContainsKey(define))
                            {
                                message = Path.GetFileName(shaderPath) + ": Unknown define " + define + Environment.NewLine;
                                return false;
                            }
                        }
                    }

                    if (header == vertShaderAttr)
                    {
                        vertexShader = body;
                    }

                    if (header == fragShaderAttr)
                    {
                        fragmentShader = body;
                    }
                }
            }

            //Console.Write(directives);
            //Console.Write(pushConstant);
            //foreach (var bindingSet in bindingSets)
            //{
            //    Console.WriteLine(bindingSet);
            //}
            //foreach (var s in verInputs)
            //{
            //    Console.WriteLine(s);
            //}
            //foreach (var s in vertToFragInputs)
            //{
            //    Console.WriteLine(s);
            //}
            //foreach (var s in fragOutputs)
            //{
            //    Console.WriteLine(s);
            //}

            //Console.WriteLine(vertexShader);
            //Console.WriteLine(fragmentShader);


            StringBuilder vertexShaderStr = new StringBuilder();
            foreach (var d in directives)
            {
                vertexShaderStr.AppendFormat("{0}\n", d);
            }

            for (int i = 0; i < verInputs.Count; i++)
            {
                vertexShaderStr.AppendFormat("layout(location = {0}) in {1};\n", i, verInputs[i]);
            }

            vertexShaderStr.Append('\n');
            for (int i = 0; i < vertToFragInputs.Count; i++)
            {
                vertexShaderStr.AppendFormat("layout(location = {0}) out {1};\n", i, vertToFragInputs[i]);
            }

            vertexShaderStr.Append('\n');
            foreach (string define in definesList)
            {
                ShaderDefine d = defines[define];
                if ((d.ShaderUsage & (uint)ShaderType.Vertex) > 0)
                {
                    vertexShaderStr.AppendFormat("{0}\n", d.DefineString);
                }
            }

            vertexShaderStr.Append('\n');
            foreach (var line in vertexShader.Split(
                         new string[] { Environment.NewLine },
                         StringSplitOptions.None
                     ))
            {
                vertexShaderStr.AppendFormat("{0}\n", line);
            }

            Console.WriteLine(vertexShaderStr);


            StringBuilder fragmentShaderStr = new StringBuilder();
            foreach (var d in directives)
            {
                fragmentShaderStr.AppendFormat("{0}\n", d);
            }

            for (int i = 0; i < vertToFragInputs.Count; i++)
            {
                fragmentShaderStr.AppendFormat("layout(location = {0}) in {1};\n", i, vertToFragInputs[i]);
            }

            fragmentShaderStr.Append('\n');

            for (int i = 0; i < fragOutputs.Count; i++)
            {
                fragmentShaderStr.AppendFormat("layout(location = {0}) out {1};\n", i, fragOutputs[i]);
            }

            fragmentShaderStr.Append('\n');
            foreach (string define in definesList)
            {
                ShaderDefine d = defines[define];
                if ((d.ShaderUsage & (uint)ShaderType.Fragment) > 0)
                {
                    fragmentShaderStr.AppendFormat("{0}\n", d.DefineString);
                }
            }

            fragmentShaderStr.Append('\n');
            foreach (var line in fragmentShader.Split(
                         new string[] { Environment.NewLine },
                         StringSplitOptions.None
                     ))
            {
                fragmentShaderStr.AppendFormat("{0}\n", line);
            }

            Console.WriteLine(fragmentShaderStr);

            InitializeCompiler();

            int vResult = CompileGLSL(vertexShaderStr.ToString(), ShaderType.Vertex, out var vertexData,
                out var vErrorMessage);
            ReleaseInternalData();

            int fResult = CompileGLSL(fragmentShaderStr.ToString(), ShaderType.Fragment, out var fragmentData,
                out var fErrorMessage);
            ReleaseInternalData();

            ReleaseCompiler();

            if (vResult != 0)
            {
                message = Path.GetFileName(shaderPath) + ": Error compiling vertex shader\n" + vErrorMessage +
                          Environment.NewLine;
                return false;
            }
            else if (fResult != 0)
            {
                message = Path.GetFileName(shaderPath) + ": Error compiling fragment shader\n" + fErrorMessage +
                          Environment.NewLine;
                return false;
            }
            else
            {
                FileStream fileStream = new FileStream(shaderPath + ".data", FileMode.Create);
                fileStream.Write(BitConverter.GetBytes(vertexData.Length), 0, 4);
                fileStream.Write(BitConverter.GetBytes(fragmentData.Length), 0, 4);
                fileStream.Write(vertexData, 0, vertexData.Length);
                fileStream.Write(fragmentData, 0, fragmentData.Length);
                fileStream.Close();
                message = Path.GetFileName(shaderPath) + ": OK" + Environment.NewLine;
                return true;
            }
        }
    }
}