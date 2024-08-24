
namespace jardcmd.ctrl
{
    partial class CtrlRempli
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
            this.lblName = new System.Windows.Forms.Label();
            this.src = new System.Windows.Forms.TrackBar();
            this.tgt = new System.Windows.Forms.TrackBar();
            this.checkRemplir = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.src)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.tgt)).BeginInit();
            this.SuspendLayout();
            // 
            // lblName
            // 
            this.lblName.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.lblName.Dock = System.Windows.Forms.DockStyle.Top;
            this.lblName.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point);
            this.lblName.Location = new System.Drawing.Point(0, 0);
            this.lblName.Name = "lblName";
            this.lblName.Size = new System.Drawing.Size(306, 23);
            this.lblName.TabIndex = 0;
            this.lblName.Text = "Nom";
            this.lblName.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // src
            // 
            this.src.Dock = System.Windows.Forms.DockStyle.Left;
            this.src.Location = new System.Drawing.Point(0, 23);
            this.src.Maximum = 3;
            this.src.Minimum = 1;
            this.src.Name = "src";
            this.src.Orientation = System.Windows.Forms.Orientation.Vertical;
            this.src.Size = new System.Drawing.Size(45, 64);
            this.src.TabIndex = 1;
            this.src.TickStyle = System.Windows.Forms.TickStyle.Both;
            this.src.Value = 1;
            this.src.ValueChanged += new System.EventHandler(this.src_ValueChanged);
            // 
            // tgt
            // 
            this.tgt.Dock = System.Windows.Forms.DockStyle.Right;
            this.tgt.Location = new System.Drawing.Point(261, 23);
            this.tgt.Maximum = 3;
            this.tgt.Minimum = 1;
            this.tgt.Name = "tgt";
            this.tgt.Orientation = System.Windows.Forms.Orientation.Vertical;
            this.tgt.Size = new System.Drawing.Size(45, 64);
            this.tgt.TabIndex = 2;
            this.tgt.TickStyle = System.Windows.Forms.TickStyle.Both;
            this.tgt.Value = 1;
            this.tgt.ValueChanged += new System.EventHandler(this.tgt_ValueChanged);
            // 
            // checkRemplir
            // 
            this.checkRemplir.AutoSize = true;
            this.checkRemplir.Location = new System.Drawing.Point(109, 44);
            this.checkRemplir.Name = "checkRemplir";
            this.checkRemplir.Size = new System.Drawing.Size(67, 19);
            this.checkRemplir.TabIndex = 3;
            this.checkRemplir.Text = "Remplir";
            this.checkRemplir.UseVisualStyleBackColor = true;
            this.checkRemplir.CheckedChanged += new System.EventHandler(this.checkRemplir_CheckedChanged);
            // 
            // CtrlRempli
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.checkRemplir);
            this.Controls.Add(this.tgt);
            this.Controls.Add(this.src);
            this.Controls.Add(this.lblName);
            this.Name = "CtrlRempli";
            this.Size = new System.Drawing.Size(306, 87);
            ((System.ComponentModel.ISupportInitialize)(this.src)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.tgt)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblName;
        private System.Windows.Forms.TrackBar src;
        private System.Windows.Forms.TrackBar tgt;
        private System.Windows.Forms.CheckBox checkRemplir;
    }
}
