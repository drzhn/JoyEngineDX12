using JoyAssetBuilder.Properties;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace JoyAssetBuilder
{
    public partial class MainWindow : Form
    {
        private AssetPanelViewController m_panelViewController;
        private readonly string m_dataPath;
        private readonly string m_materialsPath; // we need this path for correct generating models data which contain .mtl files
        private readonly string m_dllPath;

        public MainWindow()
        {
            m_dataPath = Path.Combine(Directory.GetCurrentDirectory(), Resources.DATA_PATH);
            m_dllPath = Path.Combine(Directory.GetCurrentDirectory(), Resources.BUILDER_LIB);
            m_materialsPath = Path.Combine(m_dataPath, Resources.MATERIALS_FOLDER);
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            if (!Directory.Exists(m_dataPath))
            {
                MessageBox.Show("There is no \"JoyData\" folder in the current directory", "Fatal error!", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Close();
            }
            if (!Directory.Exists(m_materialsPath))
            {
                MessageBox.Show("There is no \"JoyData/materials\" folder in the current directory", "Fatal error!", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Close();
            }
            if (!File.Exists(m_dllPath))
            {
                MessageBox.Show("There is no AssetBuilderLib.dll in the current directory", "Fatal error!", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Close();
            }
            this.Text = "Joy Asset Builder: " + m_dataPath;
            BuilderFacade.Initialize();
            m_panelViewController = new AssetPanelViewController(assetTreeView, StatusText, m_dataPath, m_materialsPath);
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            BuilderFacade.Terminate();
        }

        private void expandAll_Click(object sender, EventArgs e)
        {
            m_panelViewController.ExpandAll();
        }

        private void collapseAll_Click(object sender, EventArgs e)
        {
            m_panelViewController.CollapseAll();
        }

        private void rebuildAllButton_Click(object sender, EventArgs e)
        {
            m_panelViewController.BuildAll();
        }

        private void buildUnbuilded_Click(object sender, EventArgs e)
        {
            m_panelViewController.BuildUnbuilded();
        }

        private void buildSelectionButton_Click(object sender, EventArgs e)
        {
            m_panelViewController.BuildSelection();
        }

        private void assetTreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            m_panelViewController.SetSelection(e.Node);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            m_panelViewController.RebuildDatabase();
        }
    }
}
