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
    public partial class CtrlPmp : UserControl
    {
        Control []panels= { }; 
        private string name;

        public delegate void OnClickPumpFunc(string name,int duration_s);
        public OnClickPumpFunc OnClickPump;

        public CtrlPmp()
        {
            InitializeComponent();
            panels=new Control[] {panel1,panel2,panel3 };
            cmbDuree.SelectedIndex=3;
        }

        private void panelBottom_Paint(object sender, PaintEventArgs e)
        {

        }

        private void SetLevel(Label lbl,bool lvl)
        {
            if (lvl==false)
            {
                lbl.BackColor=SystemColors.Control;
                lbl.ForeColor=Color.Black;
            }
            else 
            { 
                lbl.BackColor=Color.Blue;
                lbl.ForeColor=Color.White;
            }
        }

        public void UpdatePump(WifiIO io)
        {
            if (io==null)
            { 
                name=null;
                lblName.Text="";
                SetLevel(lblBas,false);
                SetLevel(lblMoy,false);
                SetLevel(lblHaut,false);

                lblRSSI.Text="";
                lblBatt.Text="";
                btnPump.BackColor=SystemColors.Control;
                btnPump.ForeColor=Color.Black;
                panel1.BackColor=SystemColors.Info;
            
                foreach (Control c in panels)
                { 
                    c.BackColor=SystemColors.Info;
                    c.ForeColor=Color.Black;
                }                

                return;
            }

            name=io.Name;
            lblName.Text=io.Name;
            SetLevel(lblBas,io.N1);
            SetLevel(lblMoy,io.N2);
            SetLevel(lblHaut,io.N3);

            lblRSSI.Text="RSSI= "+(0xFFFF-io.RSSI).ToString();
            lblBatt.Text="Batt= "+io.PWR.ToString();

            dteTime.Value=io.Date;

            if (io.Cmd==true)            
            {
                btnPump.BackColor=Color.Green;
                btnPump.ForeColor=Color.White;
            }
            else
            {
                btnPump.BackColor=SystemColors.Control;
                btnPump.ForeColor=Color.Black;
            }            
            
            foreach (Control c in panels)
                if (io.Valid==false)
                {
                    btnPump.Enabled=false;
                    cmbDuree.Enabled=false;

                    if (io.Sleep==true)
                    {                        
                        c.BackColor=Color.Black;
                        c.ForeColor=Color.White;
                    }
                    else
                    { 
                        c.BackColor=Color.Red;
                        c.ForeColor=Color.White;
                    }
                }
                else 
                {
                    btnPump.Enabled=true;
                    cmbDuree.Enabled=true;

                    c.BackColor=SystemColors.Info;
                    c.ForeColor=Color.Black;
                }
        }

        private void btnPump_Click(object sender, EventArgs e)
        {
            if (OnClickPump!=null)
            {
                int duree=15;
                if (cmbDuree.Text=="15 s")
                    duree=15;
                else if (cmbDuree.Text=="1 min")
                    duree=60;
                else if (cmbDuree.Text=="2 mins")
                    duree=2*60;
                else if (cmbDuree.Text=="5 mins")
                    duree=5*60;
                else if (cmbDuree.Text=="15 mins")
                    duree=15*60;
                else if (cmbDuree.Text=="30 mins")
                    duree=30*60;

                OnClickPump(name,duree);                
            }
        }
    }
}
