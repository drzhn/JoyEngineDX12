namespace JoyAssetBuilder
{
    partial class MainWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainWindow));
            this.collapseAll = new System.Windows.Forms.Button();
            this.expandAll = new System.Windows.Forms.Button();
            this.rebuildAllButton = new System.Windows.Forms.Button();
            this.buildSelectionButton = new System.Windows.Forms.Button();
            this.assetTreeView = new System.Windows.Forms.TreeView();
            this.buildUnbuilded = new System.Windows.Forms.Button();
            this.StatusText = new System.Windows.Forms.TextBox();
            this.propertiesPanel = new System.Windows.Forms.PropertyGrid();
            this.RebuildDatabase = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // collapseAll
            // 
            this.collapseAll.Image = global::JoyAssetBuilder.Properties.Resources.CollapseAll_16x;
            this.collapseAll.Location = new System.Drawing.Point(62, 18);
            this.collapseAll.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.collapseAll.Name = "collapseAll";
            this.collapseAll.Size = new System.Drawing.Size(34, 35);
            this.collapseAll.TabIndex = 2;
            this.collapseAll.UseVisualStyleBackColor = true;
            this.collapseAll.Click += new System.EventHandler(this.collapseAll_Click);
            // 
            // expandAll
            // 
            this.expandAll.BackgroundImage = global::JoyAssetBuilder.Properties.Resources.ExpandAll_16x;
            this.expandAll.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.expandAll.Location = new System.Drawing.Point(18, 18);
            this.expandAll.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.expandAll.Name = "expandAll";
            this.expandAll.Size = new System.Drawing.Size(34, 35);
            this.expandAll.TabIndex = 1;
            this.expandAll.UseVisualStyleBackColor = true;
            this.expandAll.Click += new System.EventHandler(this.expandAll_Click);
            // 
            // rebuildAllButton
            // 
            this.rebuildAllButton.Image = global::JoyAssetBuilder.Properties.Resources.BuildSolution_16x;
            this.rebuildAllButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.rebuildAllButton.Location = new System.Drawing.Point(105, 18);
            this.rebuildAllButton.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.rebuildAllButton.Name = "rebuildAllButton";
            this.rebuildAllButton.Size = new System.Drawing.Size(118, 35);
            this.rebuildAllButton.TabIndex = 3;
            this.rebuildAllButton.Text = "Rebuild All";
            this.rebuildAllButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.rebuildAllButton.UseVisualStyleBackColor = true;
            this.rebuildAllButton.Click += new System.EventHandler(this.rebuildAllButton_Click);
            // 
            // buildSelectionButton
            // 
            this.buildSelectionButton.Image = ((System.Drawing.Image)(resources.GetObject("buildSelectionButton.Image")));
            this.buildSelectionButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.buildSelectionButton.Location = new System.Drawing.Point(396, 18);
            this.buildSelectionButton.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.buildSelectionButton.Name = "buildSelectionButton";
            this.buildSelectionButton.Size = new System.Drawing.Size(152, 35);
            this.buildSelectionButton.TabIndex = 4;
            this.buildSelectionButton.Text = "Build Selection";
            this.buildSelectionButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.buildSelectionButton.UseVisualStyleBackColor = true;
            this.buildSelectionButton.Click += new System.EventHandler(this.buildSelectionButton_Click);
            // 
            // assetTreeView
            // 
            this.assetTreeView.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.assetTreeView.Location = new System.Drawing.Point(18, 63);
            this.assetTreeView.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.assetTreeView.Name = "assetTreeView";
            this.assetTreeView.Size = new System.Drawing.Size(595, 1121);
            this.assetTreeView.TabIndex = 5;
            this.assetTreeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.assetTreeView_AfterSelect);
            // 
            // buildUnbuilded
            // 
            this.buildUnbuilded.Image = global::JoyAssetBuilder.Properties.Resources.BuildSelection_16x;
            this.buildUnbuilded.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.buildUnbuilded.Location = new System.Drawing.Point(232, 18);
            this.buildUnbuilded.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.buildUnbuilded.Name = "buildUnbuilded";
            this.buildUnbuilded.Size = new System.Drawing.Size(154, 35);
            this.buildUnbuilded.TabIndex = 6;
            this.buildUnbuilded.Text = "Build Unbuilded";
            this.buildUnbuilded.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.buildUnbuilded.UseVisualStyleBackColor = true;
            this.buildUnbuilded.Click += new System.EventHandler(this.buildUnbuilded_Click);
            // 
            // StatusText
            // 
            this.StatusText.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.StatusText.Location = new System.Drawing.Point(624, 458);
            this.StatusText.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.StatusText.Multiline = true;
            this.StatusText.Name = "StatusText";
            this.StatusText.ReadOnly = true;
            this.StatusText.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.StatusText.Size = new System.Drawing.Size(816, 726);
            this.StatusText.TabIndex = 7;
            // 
            // propertiesPanel
            // 
            this.propertiesPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.propertiesPanel.HelpVisible = false;
            this.propertiesPanel.Location = new System.Drawing.Point(624, 63);
            this.propertiesPanel.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.propertiesPanel.Name = "propertiesPanel";
            this.propertiesPanel.Size = new System.Drawing.Size(818, 386);
            this.propertiesPanel.TabIndex = 8;
            this.propertiesPanel.ToolbarVisible = false;
            // 
            // RebuildDatabase
            // 
            this.RebuildDatabase.Location = new System.Drawing.Point(624, 17);
            this.RebuildDatabase.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.RebuildDatabase.Name = "RebuildDatabase";
            this.RebuildDatabase.Size = new System.Drawing.Size(243, 35);
            this.RebuildDatabase.TabIndex = 9;
            this.RebuildDatabase.Text = "Rebuild Database";
            this.RebuildDatabase.UseVisualStyleBackColor = true;
            this.RebuildDatabase.Click += new System.EventHandler(this.button1_Click);
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1460, 1205);
            this.Controls.Add(this.RebuildDatabase);
            this.Controls.Add(this.propertiesPanel);
            this.Controls.Add(this.StatusText);
            this.Controls.Add(this.buildUnbuilded);
            this.Controls.Add(this.assetTreeView);
            this.Controls.Add(this.buildSelectionButton);
            this.Controls.Add(this.rebuildAllButton);
            this.Controls.Add(this.collapseAll);
            this.Controls.Add(this.expandAll);
            this.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.Name = "MainWindow";
            this.Text = "Joy Asset Builder";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Button expandAll;
        private System.Windows.Forms.Button collapseAll;
        private System.Windows.Forms.Button rebuildAllButton;
        private System.Windows.Forms.Button buildSelectionButton;
        private System.Windows.Forms.TreeView assetTreeView;
        private System.Windows.Forms.Button buildUnbuilded;
        private System.Windows.Forms.TextBox StatusText;
        private System.Windows.Forms.PropertyGrid propertiesPanel;
        private System.Windows.Forms.Button RebuildDatabase;
    }
}

