using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace jardcmd.core
{
    public class WifiIoRw:WifiIO
    {
        public void SetCmd(bool cmd)
        {
            this._Cmd=cmd;
        }
        public void SetN1(bool state)
        {
            this._N1=state;
        }
        public void SetN2(bool state)
        {
            this._N2=state;
        }
        public void SetN3(bool state)
        {
            this._N3=state;
        }

        public void SetName(string name)
        {
            this._Name=name;
        }

        public void SetIP(string ip)
        {
            this._IP=ip;
        }
        public void SetDate(string date)
        {
            this._date=Utils.Iso2Dte(date);
        }

        public void SetRSSI(int rssi)
        {
            this._RSSI=rssi;
        }
        public void SetPWR(int pwr)
        {
            this._PWR=pwr;
        }
        public void SetValid(bool valid)
        {
            this._Valid=valid;
        }
        public void SetSleep(bool sleep)
        {
            this._sleep=sleep;
        }
    }
}
