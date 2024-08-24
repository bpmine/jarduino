using System;
using System.Collections.Generic;
using System.Text;
using Newtonsoft.Json;
using System.Security.Cryptography.X509Certificates;
using System.IO;
using System.Net;

namespace jardcmd.wsclient
{
    public class WsClient
    {
        protected JsonSerializerSettings jser = new JsonSerializerSettings();

        protected string m_sLogin;
        protected string m_sPass;
        protected string m_sURL;

        protected string Dte2Iso(DateTime dte)
        {
            return dte.ToString("yyyy-MM-ddTHH:mm:ssZ");
        }

        protected string getBody(WebResponse response)
        {
            Stream str = response.GetResponseStream();
            if (str == null)
                return "";

            StreamReader r=new StreamReader(str);
            return r.ReadToEnd();
        }

        protected string ExecGET(string sURL)
        {
            return ExecGET(sURL,null);
        }

        protected string ExecDELETE(string sURL)
        {
            return ExecDELETE(sURL, null);
        }

        protected string GetParamsEncodedURL(Dictionary<string, string> source )
        {
            var builder = new StringBuilder();

            int count = 0;

            foreach (var parameter in source)
            {
                count++;
                builder.AppendFormat("{0}{1}{2}", parameter.Key, "=", Uri.EscapeDataString(parameter.Value));
                if (count != source.Keys.Count)
                    builder.Append("&");
            }

            return builder.ToString();
        }
       
        protected string ExecGET(string sURL,Dictionary<string, string> lParams)
        {
            try
            {
                string uri;
                if (lParams != null)
                {
                    uri = sURL + "?" + GetParamsEncodedURL(lParams);
                }
                else
                {
                    uri = sURL;
                }

                System.Net.ServicePointManager.ServerCertificateValidationCallback +=
                delegate(object sender, System.Security.Cryptography.X509Certificates.X509Certificate certificate,
                        System.Security.Cryptography.X509Certificates.X509Chain chain,
                        System.Net.Security.SslPolicyErrors sslPolicyErrors)
                {
                    return true; // **** Always accept
                };

                HttpWebRequest request = WebRequest.Create(uri) as HttpWebRequest;
                request.Timeout = 300000;
                request.Method = "GET";
                string sToCode = m_sLogin + ":" + m_sPass;
                request.Headers.Set("Authorization", "Basic " + Convert.ToBase64String(Encoding.Default.GetBytes(sToCode)));
                
                //System.Security.Cryptography.X509Certificates.X509Certificate cer=new System.Security.Cryptography.X509Certificates.X509Certificate("C:\\DEV\\BoCSTools\\trunk\\kimsufi.crt");
                //request.ClientCertificates.Add(cer);
                

                WebResponse response = request.GetResponse();
                string s = getBody(response);

                return s;
            }
            catch (Exception ex)
            {
                throw new WsException(ex.Message);
            }
        }

        protected string ExecDELETE(string sURL, Dictionary<string, string> lParams)
        {
            try
            {
                string uri;
                if (lParams != null)
                {
                    uri = sURL + "?" + GetParamsEncodedURL(lParams);
                }
                else
                {
                    uri = sURL;
                }

                System.Net.ServicePointManager.ServerCertificateValidationCallback +=
                delegate(object sender, System.Security.Cryptography.X509Certificates.X509Certificate certificate,
                        System.Security.Cryptography.X509Certificates.X509Chain chain,
                        System.Net.Security.SslPolicyErrors sslPolicyErrors)
                {
                    return true; // **** Always accept
                };

                HttpWebRequest request = WebRequest.Create(uri) as HttpWebRequest;
                request.Timeout = 300000;
                request.Method = "DELETE";
                string sToCode = m_sLogin + ":" + m_sPass;
                request.Headers.Set("Authorization", "Basic " + Convert.ToBase64String(Encoding.Default.GetBytes(sToCode)));

                WebResponse response = request.GetResponse();
                string s = getBody(response);

                return s;
            }
            catch (Exception ex)
            {
                throw new WsException(ex.Message);
            }
        }

        protected string ExecPOST(string sURL, string sDatas)
        {
            return ExecPOST(sURL, sDatas,null);
        }

        protected string ExecPOST(string sURL, string sDatas, Dictionary<string, string> lParams)
        {
            try
            {
                string uri;
                if (lParams != null)
                {
                    uri = sURL + "?" + GetParamsEncodedURL(lParams);
                }
                else
                {
                    uri = sURL;
                }

                System.Net.ServicePointManager.ServerCertificateValidationCallback +=
                delegate(object sender, System.Security.Cryptography.X509Certificates.X509Certificate certificate,
                        System.Security.Cryptography.X509Certificates.X509Chain chain,
                        System.Net.Security.SslPolicyErrors sslPolicyErrors)
                {
                    return true; // **** Always accept
                };

                HttpWebRequest request = WebRequest.Create(uri) as HttpWebRequest;
                request.Method = "POST";
                string sToCode = m_sLogin + ":" + m_sPass;
                request.Headers["Authorization"] = "Basic " + Convert.ToBase64String(Encoding.Default.GetBytes(sToCode));
                request.ContentType = "application/json";
                StreamWriter sw = new StreamWriter(request.GetRequestStream());
                sw.Write(sDatas);
                sw.Close();

                WebResponse response = request.GetResponse();
                string s = getBody(response);

                return s;
            }
            catch (Exception ex)
            {
                throw new WsException(ex.Message);            
            }
        }

        protected string ExecPUT(string sURL, string sDatas, Dictionary<string, string> lParams)
        {
            try
            {
                string uri;
                if (lParams != null)
                {
                    uri = sURL + "?" + GetParamsEncodedURL(lParams);
                }
                else
                {
                    uri = sURL;
                }

                System.Net.ServicePointManager.ServerCertificateValidationCallback +=
                delegate(object sender, System.Security.Cryptography.X509Certificates.X509Certificate certificate,
                        System.Security.Cryptography.X509Certificates.X509Chain chain,
                        System.Net.Security.SslPolicyErrors sslPolicyErrors)
                {
                    return true; // **** Always accept
                };

                HttpWebRequest request = WebRequest.Create(uri) as HttpWebRequest;
                request.Method = "PUT";
                string sToCode = m_sLogin + ":" + m_sPass;
                request.Headers["Authorization"] = "Basic " + Convert.ToBase64String(Encoding.Default.GetBytes(sToCode));
                StreamWriter sw = new StreamWriter(request.GetRequestStream());
                sw.Write(sDatas);
                sw.Close();

                WebResponse response = request.GetResponse();
                string s = getBody(response);

                return s;
            }
            catch (Exception ex)
            {
                throw new WsException(ex.Message);
            }
        }

        protected void InitJsonParser()
        {
            jser.DateFormatHandling = DateFormatHandling.IsoDateFormat;
            jser.DateFormatString = "yyyy-MM-dd'T'HH:mm:ss.fff'Z'";
            jser.NullValueHandling = NullValueHandling.Ignore;
        }


        public WsClient()
        {
            m_sURL = "http://192.168.3.200:5000";
            //m_sURL = "http://192.168.1.110:5000";
            //m_sURL = "http://127.0.0.1:2630";
            m_sLogin = "guest";
            m_sPass = "guest";

            InitJsonParser();
        }

        protected string Serialize(Object o)
        {
            return JsonConvert.SerializeObject(o, jser);
        }
    }    
}
