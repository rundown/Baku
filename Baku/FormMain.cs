using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Diagnostics;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace Baku
{
    public partial class FormMain : Form
    {
        Process process = null;
        List<string> log = new List<string>();

        public FormMain()
        {
            InitializeComponent();

            this.Load += FormMain_Load;
            this.FormClosing += FormMain_FormClosing;

            this.aboutToolStripMenuItem.Click += aboutToolStripMenuItem_Click;
            this.buttonBrowseDest.Click += buttonBrowseDest_Click;
            this.buttonBrowseSource.Click += buttonBrowseSource_Click;
            this.buttonBackup.Click += buttonBackup_Click;
        }

        void FormMain_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (process != null && !process.HasExited)
            {
                if (MessageBox.Show("Backup is in progress.\r\nDo you want to exit?",
                    "Terminate backup?", MessageBoxButtons.YesNo,
                    MessageBoxIcon.Question) == DialogResult.No)
                {
                    e.Cancel = true;
                    return;
                }
                process.Kill();
            }
            try
            {
                if (File.Exists(Application.UserAppDataPath + "\\settings.baku"))
                    File.Delete(Application.UserAppDataPath + "\\settings.baku");
                List<string> settings = new List<string>();
                settings.Add("LastUsedSource: " + this.textBoxSource.Text);
                settings.Add("LastUsedDestination: " + this.textBoxDest.Text);
                File.WriteAllLines(Application.UserAppDataPath + "\\settings.baku", settings.ToArray(), Encoding.UTF8);
            }
            catch
            {
                //nothing
            }
        }

        void FormMain_Load(object sender, EventArgs e)
        {
            List<Extension> exts = new List<Extension>();
            exts.Add(new Extension("doc*", "Microsoft Word Documents"));
            exts.Add(new Extension("xls*", "Microsoft Excel Documents"));
            exts.Add(new Extension("ppt*", "Microsoft Powerpoint Documents"));
            exts.Add(new Extension("pps*", "Microsoft Powerpoint Slideshow Documents"));
            exts.Add(new Extension("pdf", "Adobe PDF Documents"));
            exts.Add(new Extension("rtf", "RichText Documents"));
            exts.Add(new Extension("pm*", "Adobe Pagemaker Documents"));
            exts.Add(new Extension("jpg", "JPEG Image Files"));
            exts.Add(new Extension("gif", "GIF Image Files"));
            exts.Add(new Extension("png", "PNG Image Files"));
            exts.Add(new Extension("txt", "Text Documents"));
            this.checkedListBoxExtensions.Items.AddRange(exts.ToArray());
            try
            {
                if (File.Exists(Application.UserAppDataPath + "\\settings.baku"))
                {
                    string key, value;
                    int index;
                    string[] settings = File.ReadAllLines(Application.UserAppDataPath + "\\settings.baku", Encoding.UTF8);
                    foreach (string setting in settings)
                    {
                        index = setting.IndexOf(':');
                        key = setting.Substring(0, index);
                        value = setting.Substring(index + 1).Trim();
                        switch (key)
                        {
                            case "LastUsedSource":
                                this.textBoxSource.Text = value;
                                break;
                            case "LastUsedDestination":
                                this.textBoxDest.Text = value;
                                break;
                        }
                    }
                }
            }
            catch
            {
                // nothing
            }
        }

        void buttonBackup_Click(object sender, EventArgs e)
        {
            if (this.textBoxSource.Text.Trim().Length == 0 ||
                this.textBoxDest.Text.Trim().Length == 0 ||
                !Directory.Exists(this.textBoxSource.Text) ||
                !Directory.Exists(this.textBoxDest.Text))
            {
                MessageBox.Show("Please fill in valid details", "Details missing",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            if (this.textBoxDest.Text.Contains(this.textBoxSource.Text) ||
                this.textBoxSource.Text.Contains(this.textBoxDest.Text))
            {
                MessageBox.Show("The source and destination directories cannot belong to the same hierarchy",
                    "Detail error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            Process_Start();
        }

        void Process_Start()
        {
            string arguments = string.Format("\"{0}\" \"{1}\"", this.textBoxSource.Text, this.textBoxDest.Text);
            for (int i = 0; i < this.checkedListBoxExtensions.CheckedItems.Count; i++)
            {
                Extension ext = (Extension)this.checkedListBoxExtensions.CheckedItems[i];
                arguments += " " + ext.Ext;
            }
            ProcessStartInfo startInfo = new ProcessStartInfo("baku-cli.exe", arguments);
            startInfo.WindowStyle = ProcessWindowStyle.Hidden;
            startInfo.CreateNoWindow = true;
            startInfo.UseShellExecute = false;
            startInfo.RedirectStandardError = true;
            startInfo.RedirectStandardOutput = true;
            process = new Process();
            process.EnableRaisingEvents = true;
            process.ErrorDataReceived += process_ErrorDataReceived;
            process.OutputDataReceived += process_OutputDataReceived;
            process.Exited += process_Exited;
            process.StartInfo = startInfo;
            try
            {
                process.Start();
                process.BeginErrorReadLine();
                process.BeginOutputReadLine();
                this.buttonBackup.Enabled = false;
                this.buttonBrowseDest.Enabled = false;
                this.buttonBrowseSource.Enabled = false;
                this.checkedListBoxExtensions.Enabled = false;
            }
            catch
            {
                MessageBox.Show("Could not start the baku-cli executable", "Unexecutable Exception",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        void ProcessExitHandler()
        {
            this.buttonBackup.Enabled = true;
            this.buttonBrowseDest.Enabled = true;
            this.buttonBrowseSource.Enabled = true;
            this.checkedListBoxExtensions.Enabled = true;
            log.Add(string.Format("{0}", DateTime.Now));
            File.WriteAllLines("baku.log", log.ToArray(), Encoding.UTF8);
            if (this.process.ExitCode != 0)
            {
                MessageBox.Show("The backup could not be completed due to an error! Please check the logs.",
                    "Backup incomplete", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
            else
            {
                MessageBox.Show("The backup completed successfully!\r\nPlease check the log for complete details.",
                    "Backup success!", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        void process_Exited(object sender, EventArgs e)
        {
            if (this.InvokeRequired)
                this.Invoke(new ProcessExitDelegate(ProcessExitHandler));
            else
                ProcessExitHandler();
        }

        void LogHandler(string logText)
        {
            this.log.Add(logText);
        }

        void process_OutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            if (this.InvokeRequired)
                this.Invoke(new LogDelegate(LogHandler), e.Data);
            else
                LogHandler(e.Data);
        }

        void process_ErrorDataReceived(object sender, DataReceivedEventArgs e)
        {
            if (this.InvokeRequired)
                this.Invoke(new LogDelegate(LogHandler), e.Data);
            else
                LogHandler(e.Data);
        }

        void buttonBrowseSource_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog browser = new FolderBrowserDialog();
            if (browser.ShowDialog(this) == DialogResult.OK)
            {
                this.textBoxSource.Text = browser.SelectedPath;
            }
        }

        void buttonBrowseDest_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog browser = new FolderBrowserDialog();
            if (browser.ShowDialog(this) == DialogResult.OK)
            {
                this.textBoxDest.Text = browser.SelectedPath;
            }
        }

        void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show("Baku v" + Application.ProductVersion + "\r\n\r\n" +
                "No Nonsense Backup Toolkit\r\n<gaurav.joseph@linuxmail.org>\r\n\r\n" +
                "Uses cryptographic libraries from OpenSSL Project <openssl.org>\r\n\r\n" +
                "Uses 256-bit Secure Hashing Algorithm (SHA-256).\r\n\r\n" +
                "Powered by SQLite database from <sqlite.org>", "About",
                MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
    }

    public delegate void ProcessExitDelegate();
    public delegate void LogDelegate(string logText);

    public class Extension
    {
        public string Ext { get; set; }
        public string Description { get; set; }
        public Extension(string extension, string description)
        {
            this.Ext = extension;
            this.Description = description;
        }

        public override string ToString()
        {
            return this.Description;
        }
    }
}
