using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using jardcmd.core;

namespace jardcmd.ctrl
{
    public partial class CtrlRempli : UserControl
    {
        string name;

        public delegate void ChangeOnOffEvent(string name,bool flgNewState);
        public delegate void ChangeValue(string name,int newLimit);
        public delegate void ChangeTargetCons(string name,int newCons);

        public ChangeOnOffEvent OnChangeOnOff;
        public ChangeValue OnChangeSrcLimit;
        public ChangeValue OnChangeTgtCons;

        public CtrlRempli()
        {
            InitializeComponent();
        }

        public void setRempli(Rempli r)
        {
            if (r==null)
            {
                lblName.Text="";
                src.Value=1;
                tgt.Value=1;
                return;
            }

            name=r.name;

            lblName.Text=r.name;
            if ((r.limit_src>=src.Minimum) && (r.limit_src<=src.Maximum) )
                src.Value=r.limit_src;
            if ((r.cons_tgt>=tgt.Minimum) && (r.cons_tgt<=tgt.Maximum) )
                tgt.Value=r.cons_tgt;

            checkRemplir.Checked=r.on;
        }

        private void checkRemplir_CheckedChanged(object sender, EventArgs e)
        {
            if ( (OnChangeOnOff!=null) && (name!=null) && (name!="") )
                OnChangeOnOff(name,checkRemplir.Checked);
        }

        private void src_ValueChanged(object sender, EventArgs e)
        {
             if ( (OnChangeSrcLimit!=null) && (name!=null) && (name!="") )
                OnChangeSrcLimit(name,src.Value);
        }


        private void tgt_ValueChanged(object sender, EventArgs e)
        {
             if (OnChangeTgtCons!=null)
                OnChangeTgtCons(name,tgt.Value);
        }
    }
}
