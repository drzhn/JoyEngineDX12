using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Drawing;

namespace JoyAssetBuilder
{
    interface IBuildable
    {
        IEnumerable<string> Build();
        bool Built { get; }
    }

    public enum AssetType
    {
        Folder,
        Model,
        Texture,
        Shader,
        Material
    }

    public class AssetTreeNode : TreeNode, IBuildable
    {
        private readonly AssetType m_type;
        private readonly string m_path;
        private readonly string m_dataPath;
        private readonly Color okColor = Color.DarkSeaGreen;
        private readonly Color errorColor = Color.IndianRed;

        bool IBuildable.Built => _mBuilt;
        private bool _mBuilt = false;

        public AssetTreeNode(AssetType type, string path, string dataPath)
        {
            m_type = type;
            m_path = path;
            m_dataPath = dataPath;
            Text = Path.GetFileName(path);
            ImageKey = m_type.ToString();
            SelectedImageKey = m_type.ToString();
            if (m_type == AssetType.Material)
            {
                _mBuilt = File.Exists(m_path + ".json");
            }
            else
            {
                _mBuilt = File.Exists(m_path + ".data");
            }

            if (m_type != AssetType.Folder)
            {
                BackColor = _mBuilt ? okColor : errorColor;
            }
        }

        private void SetImage()
        {
            ImageKey = m_type.ToString();
            SelectedImageKey = m_type.ToString();
        }

        public IEnumerable<string> Build()
        {
            string resultMessage;
            switch (m_type)
            {
                case AssetType.Folder:

                    foreach (TreeNode node in this.Nodes)
                    {
                        var buildable = node as IBuildable;
                        if (buildable == null) continue;

                        foreach (string result in buildable.Build())
                        {
                            yield return result;
                        }
                    }
                    break;
                case AssetType.Model:
                    _mBuilt = ModelBuilder.BuildModel(m_path, m_dataPath, out resultMessage);
                    yield return resultMessage;
                    break;
                case AssetType.Texture:
                    _mBuilt = TextureBuilder.BuildTexture(m_path, out  resultMessage);
                    yield return resultMessage;
                    break;
                //case AssetType.Shader:
                //    _mBuilt = ShaderBuilder.Compile(m_path, out resultMessage);
                //    break;
                case AssetType.Material:
                    _mBuilt = MaterialBuilder.BuildMaterial(m_path, out  resultMessage);
                    yield return resultMessage;
                    break;
                default:
                    throw new ArgumentOutOfRangeException();
            }

            if (m_type != AssetType.Folder)
            {
                BackColor = _mBuilt ? okColor : errorColor;
            }
        }
    }
}