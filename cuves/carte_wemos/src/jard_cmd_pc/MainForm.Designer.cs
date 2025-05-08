
namespace jardcmd
{
    partial class MainForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.ctrlPmp1 = new jardcmd.ctrl.CtrlPmp();
            this.ctrlPmp2 = new jardcmd.ctrl.CtrlPmp();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.ctrlPmp3 = new jardcmd.ctrl.CtrlPmp();
            this.ctrlRempli1 = new jardcmd.ctrl.CtrlRempli();
            this.ctrlRempli2 = new jardcmd.ctrl.CtrlRempli();
            this.ctrlRempli3 = new jardcmd.ctrl.CtrlRempli();
            this.backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
            this.statusBar = new System.Windows.Forms.StatusStrip();
            this.ctrlPmp4 = new jardcmd.ctrl.CtrlPmp();
            this.grpOnOff = new System.Windows.Forms.GroupBox();
            this.radioON = new System.Windows.Forms.RadioButton();
            this.radioOFF = new System.Windows.Forms.RadioButton();
            this.btnSleep = new System.Windows.Forms.Button();
            this.grpOnOff.SuspendLayout();
            this.SuspendLayout();
            // 
            // ctrlPmp1
            // 
            this.ctrlPmp1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.ctrlPmp1.Location = new System.Drawing.Point(12, 12);
            this.ctrlPmp1.Name = "ctrlPmp1";
            this.ctrlPmp1.Size = new System.Drawing.Size(450, 98);
            this.ctrlPmp1.TabIndex = 0;
            // 
            // ctrlPmp2
            // 
            this.ctrlPmp2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.ctrlPmp2.Location = new System.Drawing.Point(12, 116);
            this.ctrlPmp2.Name = "ctrlPmp2";
            this.ctrlPmp2.Size = new System.Drawing.Size(450, 98);
            this.ctrlPmp2.TabIndex = 1;
            // 
            // timer1
            // 
            this.timer1.Enabled = true;
            this.timer1.Interval = 1000;
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // ctrlPmp3
            // 
            this.ctrlPmp3.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.ctrlPmp3.Location = new System.Drawing.Point(12, 220);
            this.ctrlPmp3.Name = "ctrlPmp3";
            this.ctrlPmp3.Size = new System.Drawing.Size(450, 98);
            this.ctrlPmp3.TabIndex = 3;
            // 
            // ctrlRempli1
            // 
            this.ctrlRempli1.Location = new System.Drawing.Point(468, 12);
            this.ctrlRempli1.Name = "ctrlRempli1";
            this.ctrlRempli1.Size = new System.Drawing.Size(306, 98);
            this.ctrlRempli1.TabIndex = 4;
            // 
            // ctrlRempli2
            // 
            this.ctrlRempli2.Location = new System.Drawing.Point(468, 116);
            this.ctrlRempli2.Name = "ctrlRempli2";
            this.ctrlRempli2.Size = new System.Drawing.Size(306, 98);
            this.ctrlRempli2.TabIndex = 5;
            // 
            // ctrlRempli3
            // 
            this.ctrlRempli3.Location = new System.Drawing.Point(468, 220);
            this.ctrlRempli3.Name = "ctrlRempli3";
            this.ctrlRempli3.Size = new System.Drawing.Size(306, 98);
            this.ctrlRempli3.TabIndex = 6;
            // 
            // backgroundWorker1
            // 
            this.backgroundWorker1.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorker1_DoWork);
            this.backgroundWorker1.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.backgroundWorker1_RunWorkerCompleted);
            // 
            // statusBar
            // 
            this.statusBar.Location = new System.Drawing.Point(0, 435);
            this.statusBar.Name = "statusBar";
            this.statusBar.Size = new System.Drawing.Size(793, 22);
            this.statusBar.TabIndex = 7;
            this.statusBar.Text = "statusStrip1";
            // 
            // ctrlPmp4
            // 
            this.ctrlPmp4.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.ctrlPmp4.Location = new System.Drawing.Point(12, 324);
            this.ctrlPmp4.Name = "ctrlPmp4";
            this.ctrlPmp4.Size = new System.Drawing.Size(450, 98);
            this.ctrlPmp4.TabIndex = 8;
            // 
            // grpOnOff
            // 
            this.grpOnOff.Controls.Add(this.radioON);
            this.grpOnOff.Controls.Add(this.radioOFF);
            this.grpOnOff.Location = new System.Drawing.Point(481, 342);
            this.grpOnOff.Name = "grpOnOff";
            this.grpOnOff.Size = new System.Drawing.Size(178, 70);
            this.grpOnOff.TabIndex = 9;
            this.grpOnOff.TabStop = false;
            this.grpOnOff.Text = "Marche/Arrêt du système";
            // 
            // radioON
            // 
            this.radioON.AutoSize = true;
            this.radioON.Location = new System.Drawing.Point(75, 29);
            this.radioON.Name = "radioON";
            this.radioON.Size = new System.Drawing.Size(43, 19);
            this.radioON.TabIndex = 1;
            this.radioON.Text = "ON";
            this.radioON.UseVisualStyleBackColor = true;
            this.radioON.CheckedChanged += new System.EventHandler(this.radioON_CheckedChanged);
            // 
            // radioOFF
            // 
            this.radioOFF.AutoSize = true;
            this.radioOFF.Checked = true;
            this.radioOFF.Location = new System.Drawing.Point(23, 29);
            this.radioOFF.Name = "radioOFF";
            this.radioOFF.Size = new System.Drawing.Size(46, 19);
            this.radioOFF.TabIndex = 0;
            this.radioOFF.TabStop = true;
            this.radioOFF.Text = "OFF";
            this.radioOFF.UseVisualStyleBackColor = true;
            // 
            // btnSleep
            // 
            this.btnSleep.Location = new System.Drawing.Point(665, 360);
            this.btnSleep.Name = "btnSleep";
            this.btnSleep.Size = new System.Drawing.Size(59, 41);
            this.btnSleep.TabIndex = 10;
            this.btnSleep.Text = "Zzz...";
            this.btnSleep.UseVisualStyleBackColor = true;
            this.btnSleep.Click += new System.EventHandler(this.btnSleep_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(793, 457);
            this.Controls.Add(this.btnSleep);
            this.Controls.Add(this.grpOnOff);
            this.Controls.Add(this.ctrlPmp4);
            this.Controls.Add(this.statusBar);
            this.Controls.Add(this.ctrlRempli3);
            this.Controls.Add(this.ctrlRempli2);
            this.Controls.Add(this.ctrlRempli1);
            this.Controls.Add(this.ctrlPmp3);
            this.Controls.Add(this.ctrlPmp2);
            this.Controls.Add(this.ctrlPmp1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Supervision Wiio";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.grpOnOff.ResumeLayout(false);
            this.grpOnOff.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private ctrl.CtrlPmp ctrlPmp1;
        private ctrl.CtrlPmp ctrlPmp2;
        private System.Windows.Forms.Timer timer1;
        private ctrl.CtrlPmp ctrlPmp3;
        private ctrl.CtrlRempli ctrlRempli1;
        private ctrl.CtrlRempli ctrlRempli2;
        private ctrl.CtrlRempli ctrlRempli3;
        private System.ComponentModel.BackgroundWorker backgroundWorker1;
        private System.Windows.Forms.StatusStrip statusBar;
        private ctrl.CtrlPmp ctrlPmp4;
        private System.Windows.Forms.GroupBox grpOnOff;
        private System.Windows.Forms.RadioButton radioON;
        private System.Windows.Forms.RadioButton radioOFF;
        private System.Windows.Forms.Button btnSleep;
    }
}

