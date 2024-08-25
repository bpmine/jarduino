using System;
using System.Collections.Generic;
using System.Text;
using System.IO.Ports;

namespace JardConfig
{
    public class SerialAutoDetect
    {
        private static JardClient TestComPort(string sPort)
        {
            int []speeds=new int[] {115200,9600 };
            foreach (int spd in speeds)
            {
                JardClient j=new JardClient(sPort,spd);
                if (j.Test()==true)
                    return j;
            }

            return null;
        }
        public static List<JardClient> DetectJarduinoClients()
        {
            List<JardClient> lst=new List<JardClient>();

            string []names=SerialPort.GetPortNames();
            foreach (string s in names)
            {
                JardClient j=TestComPort(s);
                if (j!=null)
                    lst.Add(j);
            }

            return lst;
        }
    }
}
