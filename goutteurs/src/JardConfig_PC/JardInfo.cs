using System;
using System.Collections.Generic;
using System.Text;

namespace JardConfig
{
    public class JardInfo
    {
        public int batt_dxV;
        public int sun_dxV;
        public int temp_dg;
        public int hum_pc;

        public JardPumpInfo pmp1=new JardPumpInfo();
        public JardPumpInfo pmp2=new JardPumpInfo();

        public bool batOK;
        public bool sunOK;
        public bool veille;

        public DateTime date;

        public int version;
        public int serial;
        public int soft;

        public int tmrComm;
        public int tmrVeille;

        public int statTotBoots;
        public int statTotP1;
        public int statTotP2;
        public int statTotSun;
        public int statTotBtn1;
        public int statTotBtn2;


        public float VBatt
        {
            get
            {
                return (float)batt_dxV/(float)10;
            }
        }
        
        public float VSun
        {
            get
            {
                return (float)sun_dxV/(float)10;
            }
        }
    }
}
