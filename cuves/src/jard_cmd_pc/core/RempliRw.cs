using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace jardcmd.core
{
    public class RempliRw:Rempli
    {
        public void setName(string name)
        {
            _name=name;
        }
        public void setConsTgt(int cons_tgt)
        {
            _cons_tgt=cons_tgt;
        }
        public void setLimitSrc(int limit_src)
        {
            _limit_src=limit_src;
        }
        public void setOn(bool on)
        {
            _on=on;
        }
    }
}
