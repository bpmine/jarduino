using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;
using System.Drawing;
using jardcmd.wsclient;
using jardcmd.core;
using jardcmd.ctrl;

namespace jardcmd
{
    public partial class MainForm : Form
    {
        List<WifiIO> listWifiIO=new List<WifiIO>();
        List<Rempli> listRempli=new List<Rempli>();

        jsSyst m_syst=null;

        WsClientJardin m_cln=new WsClientJardin();

        CtrlPmp []pump;

        public MainForm()
        {
            InitializeComponent();
        }

        private void JsonToWifoIo(WifiIoRw io, jsWifiIo js)
        {
            io.SetCmd(js.cmd);
            io.SetN1(js.n1);
            io.SetN2(js.n2);
            io.SetN3(js.n3);

            io.SetPWR(js.pwr);
            io.SetRSSI(js.rssi);

            io.SetName(js.name);
            //io.SetIP(js.ip);
            io.SetValid(js.valid);
            io.SetDate(js.date);
            io.SetSleep(js.sleep);
        }

        private void JsonToRempli(RempliRw r, jsRempli js)
        {
            r.setName(js.name);
            r.setConsTgt(js.cons_dst);
            r.setLimitSrc(js.cons_src);
            r.setOn(js.on);
        }

        private void button1_Click(object sender, EventArgs e)
        {
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (backgroundWorker1.IsBusy==false)
                backgroundWorker1.RunWorkerAsync();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            pump = new CtrlPmp [] { 
                ctrlPmp1,
                ctrlPmp2,
                ctrlPmp3,
                ctrlPmp4
                };

            foreach (CtrlPmp c in pump)
            {
                c.OnClickPump+=click_pump;
            }

            ctrlRempli1.OnChangeOnOff=change_onoff;
            ctrlRempli2.OnChangeOnOff=change_onoff;
            ctrlRempli3.OnChangeOnOff=change_onoff;

            ctrlRempli1.OnChangeTgtCons=change_tgt_cons;
            ctrlRempli2.OnChangeTgtCons=change_tgt_cons;
            ctrlRempli3.OnChangeTgtCons=change_tgt_cons;

            ctrlRempli1.OnChangeSrcLimit=change_src_limit;
            ctrlRempli2.OnChangeSrcLimit=change_src_limit;
            ctrlRempli3.OnChangeSrcLimit=change_src_limit;
        }

        public void change_onoff(string name,bool newOn)
        {
            m_cln.SetRemplissageOn(name,newOn);
        }

        public void change_tgt_cons(string name,int newVal)
        {
            m_cln.SetTgtConsigne(name,newVal);
        }
        public void change_src_limit(string name,int newVal)
        {
            m_cln.SetSrcLimit(name,newVal);
        }

        private void backgroundWorker1_DoWork(object sender, DoWorkEventArgs e)
        {
            try
            { 
                listWifiIO.Clear();
                listRempli.Clear();
                jsStatus st=m_cln.GetStates();

                if ( st!=null  )
                {
                    if (st.modules!=null)
                    foreach (jsWifiIo m in st.modules)
                    { 
                        WifiIoRw io=new WifiIoRw();
                        JsonToWifoIo(io,m);
                        listWifiIO.Add(io);
                    }

                    if (st.remplissages!=null)
                    foreach (jsRempli r in st.remplissages)
                    { 
                        RempliRw rempli=new RempliRw();
                        JsonToRempli(rempli,r);
                        listRempli.Add(rempli);
                    }
                }

                m_syst=m_cln.GetSystInfo();
                e.Result="";
            }
            catch (Exception ex)
            {
                e.Result=ex.Message;
            }
        }

        private void click_pump(string name,int duree)
        {
            if (name==null)
                return;

            foreach (WifiIO io in listWifiIO)
            {
                if (io.Name==name)
                {
                    try
                    { 
                        m_cln.SetPumpCmd(name,!io.Cmd,duree);
                    }
                    catch (Exception)
                    {

                    }
                    
                    break;
                }
            }            
        }

        private void backgroundWorker1_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            string res=(string)e.Result;
            if (res!=null)
                statusBar.Text=res;
            else
                statusBar.Text="";

            if (pump==null)
                return;

            CtrlRempli []remp = { 
                ctrlRempli1,
                ctrlRempli2,
                ctrlRempli3
                };

            for (int i=0;i<pump.Length;i++)
            {
                if (i<listWifiIO.Count)
                {
                    pump[i].UpdatePump(listWifiIO[i]);
                }
                else
                { 
                    pump[i].UpdatePump(null);
                }
            }

            for (int i=0;i<remp.Length;i++)
            {
                if (i<listRempli.Count)
                {
                    remp[i].setRempli(listRempli[i]);
                }
                else
                { 
                    remp[i].setRempli(null);
                }
            }

            if (m_syst!=null)
            { 
                radioON.Checked=m_syst.on==true?true:false;
                radioOFF.Checked=m_syst.on==true?false:true;
                grpOnOff.BackColor=m_syst.sleep?Color.DarkBlue:SystemColors.Control;
                grpOnOff.ForeColor=m_syst.sleep?Color.White:Color.Black;
            }
        }

        private void radioON_CheckedChanged(object sender, EventArgs e)
        {
            if (radioON.Checked==true)
                m_cln.SetSystOn(true);
            else
                m_cln.SetSystOn(false);
        }

        private void btnSleep_Click(object sender, EventArgs e)
        {
            if (Utils.AffMessageYesNo("La mise en veille rendra le système indisponible pendant environ 1h. Voulez-vous continuer ?")==DialogResult.Yes)
            {
                m_cln.DoSystSleep();
            }
        }
    }
}
