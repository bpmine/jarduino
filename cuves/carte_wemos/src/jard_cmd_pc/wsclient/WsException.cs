using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace jardcmd.wsclient
{
    public class WsException: Exception
    {
        public WsException(string msg):base(msg)
        {
        }
    }
}
