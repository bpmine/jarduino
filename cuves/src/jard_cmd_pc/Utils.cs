using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Reflection;
using System.Windows.Forms;
using System.Diagnostics;
using System.Security.Principal;
using System.Drawing;
using System.Globalization;
using System.Text.RegularExpressions;

namespace jardcmd
{
    public class Utils
    {        

        public static void ExportListViewToCSV(ListView l, string sFileCSV)
        {
            try
            {
                System.IO.StreamWriter file = new System.IO.StreamWriter(sFileCSV, false, Encoding.UTF8);

                string sLine = "";
                foreach (ColumnHeader h in l.Columns)
                {
                    sLine += "\"" + h.Text + "\";";
                }
                sLine += "\n";
                file.WriteLine(sLine);

                foreach (ListViewItem i in l.Items)
                {
                    sLine = "";
                    foreach (ListViewItem.ListViewSubItem si in i.SubItems)
                    {
                        sLine += "\"" + si.Text + "\";";
                    }
                    file.WriteLine(sLine);
                }

                file.Close();
            }
            catch (Exception ex)
            {
                Utils.AffMessageErreur("Impossible d'écrire le fichier " + sFileCSV + "\n" + ex.Message);
            }
        }

        public static void ShowToolTip(Control ctrl, string sTxt)
        {
            ShowToolTip(ctrl, sTxt,2000);
        }

        public static void ShowToolTip(Control ctrl, string sTxt,int iDuration_ms)
        {
            ToolTip toolTip1 = new ToolTip();
            Point pt = ctrl.PointToClient(Cursor.Position);
            toolTip1.Show(sTxt, ctrl, pt.X, pt.Y, iDuration_ms); 
        }

        public static void ToClipBoard(Control c,string sTxt)
        {
            if (ToClipBoard(sTxt)==true)
                Utils.ShowToolTip(c, sTxt, 2000);
        }
        public static bool ToClipBoard(string sTxt)
        {
            if (sTxt == null)
                return false;

            Clipboard.SetDataObject(sTxt);
            string sRead = Clipboard.GetText();
            return (sRead == sTxt);
        }

        public static string Dte2Iso(DateTime dte)
        {
            return dte.ToString("yyyy-MM-ddTHH:mm:ssZ");
        }

        public static DateTime Iso2Dte(string sDate)
        {
            Regex rg=new Regex("([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2}):([0-9]{2})");

            if (sDate == null)
                return new DateTime(0);

            Match m=rg.Match(sDate);
            if ( (m!=null) && (m.Success==true) )
            {
                int year=int.Parse(m.Groups[1].Value);
                int month=int.Parse(m.Groups[2].Value);
                int day=int.Parse(m.Groups[3].Value);

                int hour=int.Parse(m.Groups[4].Value);
                int min=int.Parse(m.Groups[5].Value);
                int sec=int.Parse(m.Groups[6].Value);

                DateTime dte=new DateTime(year,month,day,hour,min,sec);
                return dte;                
            }
            else
                return new DateTime(0);
        }

        public static string DteToShortDateStr(DateTime dte)
        {
            return dte.ToString("dd/MM/yyyy");
        }
        public static string DteToShortTimeStr(DateTime dte)
        {
            return dte.ToString("HH:mm:ss");
        }
        public static string DteToDateTimeStr(DateTime dte)
        {
            return dte.ToString("dd/MM/yyyy HH:mm:ss");
        }
        public static string DteToDateTimeStr(DateTime ? dte)
        {
            if (dte.HasValue != false)
                return dte.Value.ToString("dd/MM/yyyy HH:mm:ss");
            else
                return "";
        }
        public static string TimeT_ToDateTimeStr(long lTime_s)
        {
            return DteToDateTimeStr(TimeT_ToDateTime(lTime_s));
        }
        public static DateTime TimeT_ToDateTime(long lTime_s)
        {
            DateTime d = new DateTime(1970, 1, 1,0,0,0,0, DateTimeKind.Utc);
            d=d.AddSeconds(lTime_s);
            return d;
        }
        public static string TimeT_ToShortTimeStr(long lTime_s)
        {
            return DteToShortTimeStr(TimeT_ToDateTime(lTime_s));
        }

        public static void VoirPDF(byte[] buffer)
        {
            if ( (buffer==null) || (buffer.Length == 0) )
            {
                AffMessageWarning("Fichier vide!");
                return;
            }

            try
            {
                string s = System.IO.Path.GetTempFileName();
                s=Path.ChangeExtension(s, "pdf");
                File.WriteAllBytes(s, buffer);
                VoirPDF(s);
            }
            catch (Exception ex)
            {
                AffMessageErreur("Impossible de telecharger le fichier : "+ex.Message);
            }
        }

        public static void OuvreShellSurFichier(string sFullPath)
        {
            System.Diagnostics.Process.Start("explorer.exe", "/n, /select, " + sFullPath);
        }

        public static void ExecTonlProg(string sExe, bool v = false)
        {
            string sFullPath = GetExeDirectory() + "\\" + sExe;
            ExecExternalProg(sFullPath, v);
        }

        public static void ExecExternalProg(string sFullPath,bool v=false)
        {
            try
            {
                System.Diagnostics.Process.Start(sFullPath);
            }
            catch (Exception ex)
            {
                if (v == true)
                    Utils.AffMessageErreur(ex.Message);
            }
        }

        public static void GoToUrl(string sUrl)
        {
            System.Diagnostics.Process.Start(sUrl);
        }

        public static Process VoirPDF(string sFileName)
        {
                try
                {
                    Process Proc = new Process();
                    Proc.StartInfo = new ProcessStartInfo(sFileName);
                    Proc.Start();
                    return Proc;
                }
                catch (Exception ex)
                {
                    Utils.AffMessageErreur("Impossible d'ouvrir le fichier PDF : "+ex.Message);
                    return null;
                }
        }

        public static string GetExeDirectory()
        {
            return Path.GetDirectoryName(Application.ExecutablePath);
        }

        public static string GetShortFileName(string sFileName)
        {
            return Path.GetFileName(sFileName);
        }

        public static string GetNameWithoutExt(string sFileName)
        {
            return Path.GetFileNameWithoutExtension(sFileName);
        }

        public static string GetExePath()
        {
            return Application.ExecutablePath;
        }

        public static DialogResult AffMessageYesNo(string sMessage)
        {
            return MessageBox.Show(sMessage, "Question", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
        }

        public static DialogResult AffMessageYesNoCancel(string sMessage)
        {
            return MessageBox.Show(sMessage, "Question", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
        }

        public static void AffMessageErreur(string sMessage)
        {
            MessageBox.Show(sMessage, "Erreur", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        public static void AffMessageInfo(string sMessage)
        {
            MessageBox.Show(sMessage, "Info", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        public static void AffMessageWarning(string sMessage)
        {
            MessageBox.Show(sMessage, "Info", MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }

        public static string GetWinLogin()
        {
            string sName = WindowsIdentity.GetCurrent().Name;
            int iPos = sName.LastIndexOf("\\");
            if (iPos != -1)
            {
                sName = sName.Remove(0, iPos + 1);
            }
            return sName;
        }

        public static string GetAssemblyVersion(bool flgWithBuild)
        {
            Assembly assem = Assembly.ReflectionOnlyLoadFrom(GetExePath());
            //Assembly assem = Assembly.GetExecutingAssembly(); /// DLL en cours...

            string sVersion = assem.GetName().Version.Major.ToString();
            sVersion += "."+assem.GetName().Version.Minor.ToString();
            if (flgWithBuild)
            {
                sVersion += "." + assem.GetName().Version.Build.ToString();
            }

            return sVersion;
        }

        public static DateTime GetAssemblyDate()
        {
            Assembly assem = Assembly.ReflectionOnlyLoadFrom(GetExePath());
            return GetAssemblyLinkTime(assem);
        }

        public static string GetAssemblyBuildNumber(string sAssembly)
        {
            Assembly assem = Assembly.ReflectionOnlyLoadFrom(GetExeDirectory() + "\\" + sAssembly);

            string sVersion =assem.GetName().Version.Build.ToString(); 
            return sVersion;
        }

        public static void ClearClipBoard()
        {
            Clipboard.Clear();
        }

        public static DateTime DateTimeFromTimeT(int iTime_s)
        {
            DateTime epoch = new DateTime(1970, 1, 1, 0, 0, 0);
            TimeSpan spn = new TimeSpan(0, 0, iTime_s);
            DateTime dte = epoch+spn;
            return dte;
        }

        public static DateTime GetStartOfDay(DateTime dte)
        {
            DateTime ret=new DateTime(dte.Year,dte.Month,dte.Day,0,0,0);
            return ret;
        }

        public static DateTime GetEndOfDay(DateTime dte)
        {
            DateTime ret = new DateTime(dte.Year, dte.Month, dte.Day, 23, 59, 59);
            return ret;
        }

        /// <summary>
        /// Ouvre un répertoire sur le fichier donné
        /// </summary>
        /// <param name="sTargetFile"></param>
        /// <returns></returns>
        public static void AutoOpenFolder(string sTargetFile)
        {
            if (Utils.AffMessageYesNo("Voulez-vous ouvrir le répertoire ou se trouve le fichier?") == DialogResult.Yes)
            {
                Utils.OuvreShellSurFichier(sTargetFile);
            }
        }

        public static void SendMailByDefaultClient(string sTo, string sSubject, string sMsg)
        {
            string strMailTo = "mailto:" + sTo + "?subject=" + sSubject + "&body=" + sMsg;
            System.Diagnostics.Process myProcess = new System.Diagnostics.Process();
            myProcess.StartInfo.UseShellExecute = true;
            myProcess.StartInfo.RedirectStandardOutput = false;
            myProcess.StartInfo.FileName = strMailTo;
            myProcess.StartInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Normal;
            myProcess.Start();
        }

        public static DateTime GetAssemblyLinkTime(Assembly assembly, TimeZoneInfo target = null)
        {
            var filePath = assembly.Location;
            const int c_PeHeaderOffset = 60;
            const int c_LinkerTimestampOffset = 8;

            var buffer = new byte[2048];

            using (var stream = new FileStream(filePath, FileMode.Open, FileAccess.Read))
                stream.Read(buffer, 0, 2048);

            var offset = BitConverter.ToInt32(buffer, c_PeHeaderOffset);
            var secondsSince1970 = BitConverter.ToInt32(buffer, offset + c_LinkerTimestampOffset);
            var epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);

            var linkTimeUtc = epoch.AddSeconds(secondsSince1970);

            var tz = target ?? TimeZoneInfo.Local;
            var localTime = TimeZoneInfo.ConvertTimeFromUtc(linkTimeUtc, tz);

            return localTime;
        }
    }
}
