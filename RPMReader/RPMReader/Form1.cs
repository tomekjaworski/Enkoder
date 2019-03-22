using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;

namespace RPMReader
{
    public partial class Form1 : Form
    {
        SerialPort sp;
        string s = "";

        public Form1()
        {
            this.InitializeComponent();
            
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (this.sp != null)
            {
                this.timer1.Stop();

                this.sp.Close();
                this.sp.Dispose();
                this.sp = null;

                return;
            }

            this.sp = new SerialPort(this.textBox1.Text, 115200);
            this.s = "";
            sp.Open();
            sp.Write("+++\n");
            Thread.Sleep(250);
            sp.DiscardInBuffer();


            sp.Write("run\n");

            this.timer1.Start();
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (this.sp == null)
                return;

            sp.Write("+++\n");
            Thread.Sleep(250);
            sp.DiscardInBuffer();

            this.sp.Close();
            this.sp.Dispose();
            this.sp = null;
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            this.s = sp.ReadExisting();
            while (s.Contains("\n"))
            {
                int p = s.IndexOf('\n');
                string line = s.Substring(0, p);
                s = s.Substring(p + 1);

                if (line.Length != 2 + 1 + 4 + 1 + 8)
                    continue;

                p = line.LastIndexOf(':');
                line = "0x" + line.Substring(p + 1);

                int pps = Convert.ToInt32(line, 16);
                float velocity = (float)pps / 20000.0f;

                this.label1.Text = String.Format("{0:N3}RPM", velocity);
                break;
            }

        }
    }
}
