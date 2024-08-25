using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EasyModbus;

namespace JardConfig
{
    public partial class MainForm : Form
    {
        private JardClient m_cln=null;

        private JardClient GetJardClient()
        {
            if (m_cln==null)
            {
                List<JardClient> lst=SerialAutoDetect.DetectJarduinoClients();
                if ( (lst!=null) && (lst.Count>0) )
                {
                    m_cln=lst[0];
                    return m_cln;
                }
            }
            else
                return m_cln;

            return null;
        }

        public MainForm()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
        }

        private void button2_Click(object sender, EventArgs e)
        {
        }

        private void button1_Click_1(object sender, EventArgs e)
        {
            JardClient cln=GetJardClient();
            if (cln==null)
                return;

            JardInfo inf=cln.ReadAllFromSlave();
            try 
            {
                dateTimePicker1.Value=inf.date;
            }
            catch (Exception)
            {
                dateTimePicker1.Value=DateTime.Now;
            }

            lblVersion.Text=inf.version.ToString();
            lblSerie.Text=inf.serial.ToString();
            lblSoft.Text=inf.soft.ToString();

            ctrlPmp1.SetPmpInfo(inf.pmp1);
            ctrlPmp2.SetPmpInfo(inf.pmp2);

            ctrlMonitoring.SetJardInfo(inf);
        }

        private void button5_Click(object sender, EventArgs e)
        {
            JardClient cln=GetJardClient();
            if (cln==null)
                return;

            cln.SetDate(dateTimePicker1.Value);
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            ctrlPmp1.SetTitle("POMPE 1");
            ctrlPmp1.OnSet+=Pmp1_Set;

            ctrlPmp2.SetTitle("POMPE 2");
            ctrlPmp2.OnSet+=Pmp2_Set;
        }

        private void Pmp1_Set(CtrlPmpInfo ctrl)
        {
            JardClient cln=GetJardClient();
            if (cln==null)
                return;

            JardPumpInfo pmpInfo=ctrl.GetPmpInfo();            
            cln.SetPump(1,pmpInfo);
        }
        private void Pmp2_Set(CtrlPmpInfo ctrl)
        {
            JardClient cln=GetJardClient();
            if (cln==null)
                return;

            JardPumpInfo pmpInfo=ctrl.GetPmpInfo();            
            cln.SetPump(2,pmpInfo);
        }

        private void btnQuit_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void button1_Click_2(object sender, EventArgs e)
        {
            if (m_cln!=null)
            { 
                StatisticsDlg dlg=new StatisticsDlg();
                dlg.SetJardInfo(m_cln.ReadAllFromSlave());
                dlg.ShowDialog();
            }
        }
    }
}
