using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace jardcmd.wsclient
{
    public class jsWifiIo
    {
        public string name {get;set; }
        //public string ip {get;set; }

        public bool cmd {get;set; }
        public bool n1 {get;set; }
        public bool n2 {get;set; }
        public bool n3 {get;set; }
        public bool valid {get;set; }

        public int rssi {get;set; }
        public int pwr {get;set; }

        public string date { get;set;}

        public bool sleep { get;set;}

        public jsWifiIo()
        {
        }
    }
}
