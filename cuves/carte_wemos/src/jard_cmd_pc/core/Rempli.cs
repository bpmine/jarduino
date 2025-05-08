using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace jardcmd.core
{
    public class Rempli
    {
        protected string _name;
        protected int _cons_tgt;
        protected int _limit_src;
        protected bool _on;

        public string name 
        {
            get
            { 
                return _name;
            }
        }
        public int cons_tgt 
        {
            get
            { 
                return _cons_tgt;
            }
        }

        public int limit_src 
        {
            get
            { 
                return _limit_src;
            }
        }

        public bool on 
        {
            get
            { 
                return _on;
            }
        }

        public Rempli()
        {
        }
    }
}
