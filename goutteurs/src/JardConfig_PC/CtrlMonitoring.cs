using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace JardConfig
{
    public partial class CtrlMonitoring : UserControl
    {
        public CtrlMonitoring()
        {
            InitializeComponent();
        }


        private void SetLblColorGreen(Control ctrl,bool flg)
        {
            if (flg)
            { 
                ctrl.BackColor=Color.Green;
                ctrl.ForeColor=Color.White;
            }
            else
            { 
                ctrl.BackColor=Color.Red;
                ctrl.ForeColor=Color.White;
            }
        }

        private void SetLblColorPump(Control ctrl,bool flg)
        {
            if (flg)
            { 
                ctrl.BackColor=Color.Blue;
                ctrl.ForeColor=Color.White;
            }
            else
            { 
                ctrl.BackColor=SystemColors.Control;
                ctrl.ForeColor=Color.Black;
            }
        }

        public void SetJardInfo(JardInfo jardInfo)
        {
            lblBattVoltage.Text=string.Format("{0:0.0} V",jardInfo.VBatt);
            lblSunVoltage.Text=string.Format("{0:0.0} V",jardInfo.VSun);
            lblTemp.Text=jardInfo.temp_dg.ToString()+" °C";
            lblHum.Text=jardInfo.hum_pc.ToString()+" %";
            lblTmr1.Text=jardInfo.pmp1.timer+" min";
            lblTmr2.Text=jardInfo.pmp2.timer+" min";
            lblTmrComm.Text=jardInfo.tmrComm+" s";
            lblTmrVeille.Text=jardInfo.tmrVeille+" s";

            SetLblColorGreen(lblBatt,jardInfo.batOK);
            SetLblColorGreen(lblSun,jardInfo.sunOK);

            SetLblColorGreen(lblVeille,jardInfo.veille);
            SetLblColorPump(lblPmp1,jardInfo.pmp1.led);
            SetLblColorPump(lblPmp2,jardInfo.pmp2.led);
        }
    }
}
