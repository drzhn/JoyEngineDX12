using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;

namespace JoyAssetBuilder
{
    public class AssetPanelViewController
    {
        private TreeView m_view;
        private TextBox m_logBox;

        private List<IBuildable> m_assetToBuilds = new List<IBuildable>();
        private IBuildable m_currentSelected = null;

        private readonly string m_dataPath;

        private DatabaseBuilder m_databaseBuilder;

        public AssetPanelViewController(TreeView panel, TextBox box, string dataPath)
        {
            m_view = panel;
            m_logBox = box;
            m_dataPath = dataPath;
            ImageList imageList = new ImageList();
            imageList.Images.Add(AssetType.Folder.ToString(), Properties.Resources.FolderClosed_16x);
            imageList.Images.Add(AssetType.Model.ToString(), Properties.Resources.Model3D_outline_16x);
            imageList.Images.Add(AssetType.Texture.ToString(), Properties.Resources.Image_16x);
            imageList.Images.Add(AssetType.Shader.ToString(), Properties.Resources.MaterialDiffuse_16x);
            imageList.Images.Add(AssetType.Material.ToString(), Properties.Resources.MaterialDiffuse_16x);
            m_view.ImageList = imageList;
            GetDirsAndFiles(dataPath, null);
            m_view.ExpandAll();
            m_view.Nodes[0].EnsureVisible();

            m_databaseBuilder = new DatabaseBuilder(dataPath);
        }

        public void RebuildDatabase()
        {
            //m_databaseBuilder.RebuildDatabase();
        }

        public void ExpandAll()
        {
            m_view.ExpandAll();
        }

        public void CollapseAll()
        {
            m_view.CollapseAll();
        }

        private void GetDirsAndFiles(string path, AssetTreeNode parentNode)
        {
            AssetTreeNode dirItem = new AssetTreeNode(AssetType.Folder, Path.GetFileName(path), m_dataPath);
            //rootItem.SelectedImageKey = "Folder";
            dirItem.ImageKey = "Folder";
            if (parentNode == null)
            {
                m_view.Nodes.Add(dirItem);
            }
            else
            {
                parentNode.Nodes.Add(dirItem);
            }

            foreach (string dir in Directory.GetDirectories(path))
            {
                if (Path.GetFileName(dir)[0] == '.')
                {
                    continue;
                }

                GetDirsAndFiles(dir, dirItem);
            }

            foreach (string file in Directory.GetFiles(path))
            {
                AssetTreeNode fileItem;
                switch (Path.GetExtension(file))
                {
                    case ".obj":
                        fileItem = new AssetTreeNode(AssetType.Model, file, m_dataPath);
                        break;
                    case ".png":
                    case ".jpg":
                    case ".jpeg":
                    case ".hdr":
                    case ".tga":
                        //case ".dds":
                        fileItem = new AssetTreeNode(AssetType.Texture, file, m_dataPath);
                        break;
                    //case ".mtl":
                    //    fileItem = new AssetTreeNode(AssetType.Material, file, m_dataPath);
                    //    break;
                    // for now we build shaders in runtime
                    //case ".shader": 
                    //    fileItem = new AssetTreeNode(AssetType.Shader, file);
                    //    break;
                    default:
                        continue;
                }

                dirItem.Nodes.Add(fileItem);
                m_assetToBuilds.Add(fileItem);
            }
        }

        public void BuildSelection()
        {
            if (m_currentSelected == null) return;
            foreach (string resultMessage in m_currentSelected.Build())
            {
                m_logBox.AppendText(resultMessage);
            }
        }

        public void SetSelection(TreeNode node)
        {
            m_currentSelected = node as IBuildable;
        }

        public void BuildUnbuilded()
        {
            m_assetToBuilds.ForEach(x =>
            {
                if (x.Built) return;
                foreach (string resultMessage in x.Build())
                {
                    m_logBox.AppendText(resultMessage);
                }
            });
        }

        public void BuildAll()
        {
            Stopwatch _stopWatch = Stopwatch.StartNew();
            m_assetToBuilds.ForEach(x =>
            {
                foreach (string resultMessage in x.Build())
                {
                    m_logBox.AppendText(resultMessage);
                }
            });

            _stopWatch.Stop();
            string message = "Elapsed time: " + _stopWatch.Elapsed.Minutes + " minutes, "
                             + _stopWatch.Elapsed.Seconds + " seconds";
            MessageBox.Show(message, "Rebuild finished", MessageBoxButtons.OK,
                MessageBoxIcon.Information);
        }
    }
}