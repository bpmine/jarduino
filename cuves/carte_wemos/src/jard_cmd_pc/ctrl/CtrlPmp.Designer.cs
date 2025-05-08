
namespace jardcmd.ctrl
{
    partial class CtrlPmp
    {
        /// <summary> 
        /// Variable nécessaire au concepteur.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Nettoyage des ressources utilisées.
        /// </summary>
        /// <param name="disposing">true si les ressources managées doivent être supprimées ; sinon, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Code généré par le Concepteur de composants

        /// <summary> 
        /// Méthode requise pour la prise en charge du concepteur - ne modifiez pas 
        /// le contenu de cette méthode avec l'éditeur de code.
        /// </summary>
        private void InitializeComponent()
        {
            this.panelTop = new System.Windows.Forms.Panel();
            this.panel3 = new System.Windows.Forms.Panel();
            this.dteTime = new System.Windows.Forms.DateTimePicker();
            this.lblBatt = new System.Windows.Forms.Label();
            this.lblRSSI = new System.Windows.Forms.Label();
            this.lblName = new System.Windows.Forms.Label();
            this.panel2 = new System.Windows.Forms.Panel();
            this.lblBas = new System.Windows.Forms.Label();
            this.lblHaut = new System.Windows.Forms.Label();
            this.lblMoy = new System.Windows.Forms.Label();
            this.panel1 = new System.Windows.Forms.Panel();
            this.cmbDuree = new System.Windows.Forms.ComboBox();
            this.btnPump = new System.Windows.Forms.Button();
            this.panelTop.SuspendLayout();
            this.panel3.SuspendLayout();
            this.panel2.SuspendLayout();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // panelTop
            // 
            this.panelTop.Controls.Add(this.panel3);
            this.panelTop.Controls.Add(this.panel2);
            this.panelTop.Controls.Add(this.panel1);
            this.panelTop.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panelTop.Location = new System.Drawing.Point(0, 0);
            this.panelTop.Name = "panelTop";
            this.panelTop.Size = new System.Drawing.Size(448, 103);
            this.panelTop.TabIndex = 1;
            // 
            // panel3
            // 
            this.panel3.BackColor = System.Drawing.SystemColors.Info;
            this.panel3.Controls.Add(this.dteTime);
            this.panel3.Controls.Add(this.lblBatt);
            this.panel3.Controls.Add(this.lblRSSI);
            this.panel3.Controls.Add(this.lblName);
            this.panel3.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel3.Location = new System.Drawing.Point(0, 0);
            this.panel3.Name = "panel3";
            this.panel3.Size = new System.Drawing.Size(170, 103);
            this.panel3.TabIndex = 5;
            // 
            // dteTime
            // 
            this.dteTime.CustomFormat = "dd/MM/yyyy HH:mm:ss";
            this.dteTime.Dock = System.Windows.Forms.DockStyle.Top;
            this.dteTime.Enabled = false;
            this.dteTime.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.dteTime.Location = new System.Drawing.Point(0, 78);
            this.dteTime.Name = "dteTime";
            this.dteTime.Size = new System.Drawing.Size(170, 23);
            this.dteTime.TabIndex = 5;
            // 
            // lblBatt
            // 
            this.lblBatt.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lblBatt.Dock = System.Windows.Forms.DockStyle.Top;
            this.lblBatt.Location = new System.Drawing.Point(0, 52);
            this.lblBatt.Name = "lblBatt";
            this.lblBatt.Size = new System.Drawing.Size(170, 26);
            this.lblBatt.TabIndex = 4;
            this.lblBatt.Text = "Batt";
            this.lblBatt.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblRSSI
            // 
            this.lblRSSI.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lblRSSI.Dock = System.Windows.Forms.DockStyle.Top;
            this.lblRSSI.Location = new System.Drawing.Point(0, 26);
            this.lblRSSI.Name = "lblRSSI";
            this.lblRSSI.Size = new System.Drawing.Size(170, 26);
            this.lblRSSI.TabIndex = 3;
            this.lblRSSI.Text = "RSSI";
            this.lblRSSI.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblName
            // 
            this.lblName.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lblName.Dock = System.Windows.Forms.DockStyle.Top;
            this.lblName.Location = new System.Drawing.Point(0, 0);
            this.lblName.Name = "lblName";
            this.lblName.Size = new System.Drawing.Size(170, 26);
            this.lblName.TabIndex = 2;
            this.lblName.Text = "Nom";
            this.lblName.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // panel2
            // 
            this.panel2.BackColor = System.Drawing.SystemColors.Info;
            this.panel2.Controls.Add(this.lblBas);
            this.panel2.Controls.Add(this.lblHaut);
            this.panel2.Controls.Add(this.lblMoy);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Right;
            this.panel2.Location = new System.Drawing.Point(170, 0);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(139, 103);
            this.panel2.TabIndex = 4;
            // 
            // lblBas
            // 
            this.lblBas.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lblBas.Location = new System.Drawing.Point(23, 63);
            this.lblBas.Name = "lblBas";
            this.lblBas.Size = new System.Drawing.Size(88, 26);
            this.lblBas.TabIndex = 3;
            this.lblBas.Text = "Bas";
            this.lblBas.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // lblHaut
            // 
            this.lblHaut.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lblHaut.Location = new System.Drawing.Point(23, 9);
            this.lblHaut.Name = "lblHaut";
            this.lblHaut.Size = new System.Drawing.Size(88, 26);
            this.lblHaut.TabIndex = 1;
            this.lblHaut.Text = "Haut";
            this.lblHaut.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // lblMoy
            // 
            this.lblMoy.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lblMoy.Location = new System.Drawing.Point(23, 35);
            this.lblMoy.Name = "lblMoy";
            this.lblMoy.Size = new System.Drawing.Size(88, 26);
            this.lblMoy.TabIndex = 2;
            this.lblMoy.Text = "Millieu";
            this.lblMoy.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // panel1
            // 
            this.panel1.BackColor = System.Drawing.SystemColors.Info;
            this.panel1.Controls.Add(this.cmbDuree);
            this.panel1.Controls.Add(this.btnPump);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Right;
            this.panel1.Location = new System.Drawing.Point(309, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(139, 103);
            this.panel1.TabIndex = 0;
            // 
            // cmbDuree
            // 
            this.cmbDuree.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.Suggest;
            this.cmbDuree.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
            this.cmbDuree.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbDuree.FormattingEnabled = true;
            this.cmbDuree.Items.AddRange(new object[] {
            "15 secs",
            "1 min",
            "2 mins",
            "5 mins",
            "15 mins",
            "30 mins"});
            this.cmbDuree.Location = new System.Drawing.Point(23, 63);
            this.cmbDuree.Name = "cmbDuree";
            this.cmbDuree.Size = new System.Drawing.Size(91, 23);
            this.cmbDuree.TabIndex = 1;
            // 
            // btnPump
            // 
            this.btnPump.Location = new System.Drawing.Point(23, 15);
            this.btnPump.Name = "btnPump";
            this.btnPump.Size = new System.Drawing.Size(91, 42);
            this.btnPump.TabIndex = 0;
            this.btnPump.Text = "Pompe";
            this.btnPump.UseVisualStyleBackColor = true;
            this.btnPump.Click += new System.EventHandler(this.btnPump_Click);
            // 
            // CtrlPmp
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.Controls.Add(this.panelTop);
            this.Name = "CtrlPmp";
            this.Size = new System.Drawing.Size(448, 103);
            this.panelTop.ResumeLayout(false);
            this.panel3.ResumeLayout(false);
            this.panel2.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Panel panelTop;
        private System.Windows.Forms.Label lblBas;
        private System.Windows.Forms.Label lblMoy;
        private System.Windows.Forms.Label lblHaut;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button btnPump;
        private System.Windows.Forms.Panel panel3;
        private System.Windows.Forms.Label lblName;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.Label lblBatt;
        private System.Windows.Forms.Label lblRSSI;
        private System.Windows.Forms.DateTimePicker dteTime;
        private System.Windows.Forms.ComboBox cmbDuree;
    }
}
