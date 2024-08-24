using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace jardcmd.wsclient
{
    public class WsClientJardin: WsClient
    { 
        public jsStatus GetStates()
        {            
            string sRet = ExecGET(m_sURL + "/states");
            //sRet=sRet.Replace("true","1");
            //sRet=sRet.Replace("false","0");
            jsStatus lRet = JsonConvert.DeserializeObject<jsStatus>(sRet, jser);
            return lRet;
        }

        public jsSyst GetSystInfo()
        {            
            string sRet = ExecGET(m_sURL + "/wiio");
            jsSyst lRet = JsonConvert.DeserializeObject<jsSyst>(sRet, jser);
            return lRet;
        }

        public jsResult SetSystOn(bool newOn)
        {
            string sRet;
            if (newOn==true)
                sRet=ExecGET(m_sURL + "/wiio/do/on");
            else
                sRet=ExecGET(m_sURL + "/wiio/do/off");

            jsResult ret = JsonConvert.DeserializeObject<jsResult>(sRet, jser);
            return ret;
        }

        public jsResult SetPumpCmd(string name,bool flgOn,int duree=0)
        {            
            if (duree<0)
            {
                duree=0;
                flgOn=false;
            }

            if (duree>30*60)
                duree=30*60;

            string sRet;
            string sCmd=flgOn==true?"on":"off";
            if (flgOn==true)
                sRet = ExecGET(m_sURL + "/wiio/modules/"+name+"/do/"+sCmd+"?duration="+duree.ToString());
            else
                sRet = ExecGET(m_sURL + "/wiio/modules/"+name+"/do/"+sCmd);

            jsResult ret = JsonConvert.DeserializeObject<jsResult>(sRet, jser);
            return ret;
        }
        
        public jsResult DoSystSleep()
        {
            string sRet;
            sRet=ExecGET(m_sURL + "/wiio/do/sleep");
            jsResult ret = JsonConvert.DeserializeObject<jsResult>(sRet, jser);
            return ret;
        }


        public void SetRemplissageOn(string name,bool newOn)
        {
            if (newOn==true)
                ExecGET(m_sURL + "/wiio/remplis/"+name+"/do/on");
            else
                ExecGET(m_sURL + "/wiio/remplis/"+name+"/do/off");
        }
        public void SetSrcLimit(string name,int newVal)
        {
            Dictionary<string,string> par=new Dictionary<string,string>();
            par.Add("src",newVal.ToString());
            
            ExecGET(m_sURL + "/wiio/remplis/"+name+"/do/setcons",par);
        }
        public void SetTgtConsigne(string name,int newVal)
        {
            Dictionary<string,string> par=new Dictionary<string,string>();
            par.Add("dst",newVal.ToString());
            
            ExecGET(m_sURL + "/wiio/remplis/"+name+"/do/setcons",par);
        }
    }
}
