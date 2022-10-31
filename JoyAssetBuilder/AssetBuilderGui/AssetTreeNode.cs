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
        void Build(out string resultMessage);
        bool Built { get; }
    }

    public enum AssetType
    {
        Folder,
        Model,
        Texture,
        Shader
    }

    public class AssetTreeNode : TreeNode, IBuildable
    {
        private AssetType m_type;
        private string m_path;
        private readonly Color okColor = Color.DarkSeaGreen;
        private readonly Color errorColor = Color.IndianRed;

        bool IBuildable.Built => _mBuilt;
        private bool _mBuilt = false;

        public AssetTreeNode(AssetType type, string path)
        {
            m_type = type;
            m_path = path;
            Text = Path.GetFileName(path);
            ImageKey = m_type.ToString();
            SelectedImageKey = m_type.ToString();
            _mBuilt = File.Exists(m_path + ".data");
            BackColor = _mBuilt ? okColor : errorColor;
        }

        private void SetImage()
        {
            ImageKey = m_type.ToString();
            SelectedImageKey = m_type.ToString();
        }

        void IBuildable.Build(out string resultMessage)
        {
            resultMessage = ""; // TODO remove later
            switch (m_type)
            {
                case AssetType.Folder:
                    break;
                case AssetType.Model:
                    _mBuilt = ModelBuilder.BuildModel(m_path, out resultMessage);
                    break;
                case AssetType.Texture:
                    _mBuilt = TextureBuilder.BuildTexture(m_path, out resultMessage);
                    break;
                case AssetType.Shader:
                    _mBuilt = ShaderBuilder.Compile(m_path, out resultMessage);
                    break;
                default:
                    throw new ArgumentOutOfRangeException();
            }

            BackColor = _mBuilt ? okColor : errorColor;
        }
    }
}