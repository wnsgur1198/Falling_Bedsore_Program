using System;
using System.Windows;
using System.IO.Ports;

namespace Loadcell 
{
   partial class xSerialPortDlg : Window
   {
      public xSerialPortDlg()
      {
         InitializeComponent();
      }

      public void InitializeDialogBox(SerialPort port)
      {
         BaudBox.Text = port.BaudRate.ToString();
         DataBitsBox.Text = port.DataBits.ToString();

         foreach (string s in SerialPort.GetPortNames()) PortCombo.Items.Add(s);
         PortCombo.Text = port.PortName;

         foreach (string s in Enum.GetNames(typeof(Parity))) ParityCombo.Items.Add(s);
         ParityCombo.Text = port.Parity.ToString();

         foreach (string s in Enum.GetNames(typeof(StopBits))) StopBitsCombo.Items.Add(s);
         StopBitsCombo.Text = port.StopBits.ToString();

         foreach (string s in Enum.GetNames(typeof(Handshake))) HandShakeCombo.Items.Add(s);
         HandShakeCombo.Text = port.Handshake.ToString();
      }

      void OKBtnClicked(object sender, RoutedEventArgs args)
      {
         DialogResult = true;
      }

      void CancelBtnClicked(object sender, RoutedEventArgs args) {
         DialogResult = false;
      }
   }//class xSerialPortDlg
}//namespace