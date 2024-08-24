using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace JardConfig
{
    public partial class StatisticsDlg : Form
    {
        public StatisticsDlg()
        {
            InitializeComponent();
        }

        private void AddItem(string title,string value)
        {
            ListViewItem item=listStats.Items.Add(title);
            item.SubItems.Add(value);
        }

        public void SetJardInfo(JardInfo info)
        {
            listStats.Items.Clear();
            AddItem("Nombre de boots",info.statTotBoots.ToString());
            AddItem("Durée de fonctionnement pompe 1 (H)",info.statTotP1.ToString());
            AddItem("Durée de fonctionnement pompe 2 (H)",info.statTotP2.ToString());
            AddItem("Durée d'ensoleillement (H)",info.statTotSun.ToString());
            AddItem("Nombre d'appuis bouton 1",info.statTotBtn1.ToString());
            AddItem("Nombre d'appuis bouton 2",info.statTotBtn2.ToString());
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Close();
        }
    }
}
