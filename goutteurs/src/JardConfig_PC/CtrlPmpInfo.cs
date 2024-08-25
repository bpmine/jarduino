using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace JardConfig
{
    public partial class CtrlPmpInfo : UserControl
    {
        public delegate void OnSetHandler(CtrlPmpInfo ctrl);
        public OnSetHandler OnSet;

        public CtrlPmpInfo()
        {
            InitializeComponent();
        }

        public void SetTitle(string title)
        {
            grpPmp.Text=title;
        }

        private void UpdateHMIState()
        {
            if (checkAuto.Checked==false)
            {
                checkedListDays.Enabled=false;
                numHour.Enabled=false;
                numMin.Enabled=false;
                numDuration.Enabled=false;
            }
            else
            { 
                checkedListDays.Enabled=true;
                numHour.Enabled=true;
                numMin.Enabled=true;
                numDuration.Enabled=true;
            }

            if (checkRemote.Checked==false)
            {
                checkCmdRemote.Checked=false;
            }
            else
            {
                checkCmdRemote.Checked=true;
            }
        }

        public void SetPmpInfo(JardPumpInfo pmpInfo)
        {
            checkForced.Checked=pmpInfo.forced;
            checkEnabled.Checked=pmpInfo.enabled;
            checkAuto.Checked=pmpInfo.auto;
            checkRemote.Checked=pmpInfo.remote;
            checkCmdRemote.Checked=pmpInfo.cmd_remote;

            numHour.Value=pmpInfo.hour;
            numMin.Value=pmpInfo.minute;
            numDuration.Value=pmpInfo.duration;

            for (int i=0;i<checkedListDays.Items.Count;i++)
            {
                if ((pmpInfo.days&(1<<i))==(1<<i))
                    checkedListDays.SetItemChecked(i,true);
                else
                    checkedListDays.SetItemChecked(i,false);
            }

            numTimer.Value=pmpInfo.timer;

            UpdateHMIState();
        }

        public JardPumpInfo GetPmpInfo()
        {
            JardPumpInfo pmpInfo=new JardPumpInfo();
            pmpInfo.forced=checkForced.Checked;
            pmpInfo.enabled=checkEnabled.Checked;
            pmpInfo.auto=checkAuto.Checked;
            pmpInfo.remote=checkRemote.Checked;
            pmpInfo.cmd_remote=checkCmdRemote.Checked;

            pmpInfo.hour=(int)numHour.Value;
            pmpInfo.minute=(int)numMin.Value;
            pmpInfo.duration=(int)numDuration.Value;

            pmpInfo.days=0;
            for (int i=0;i<checkedListDays.Items.Count;i++)
            {
                if (checkedListDays.GetItemChecked(i))
                    pmpInfo.days|=(1<<i);                
            }

            pmpInfo.timer=(int)numTimer.Value;

            return pmpInfo;
        }

        private void groupBox1_Enter(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (OnSet!=null)
                OnSet(this);
        }

        private void checkAuto_CheckedChanged(object sender, EventArgs e)
        {
            if (checkAuto.Checked==true)
            {
                checkedListDays.Enabled=true;
                numHour.Enabled=true;
                numMin.Enabled=true;
                numDuration.Enabled=true;
            }
            else
            {
                checkedListDays.Enabled=false;
                numHour.Enabled=false;
                numMin.Enabled=false;
                numDuration.Enabled=false;
            }
        }
    }
}
