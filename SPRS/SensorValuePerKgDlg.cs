using System.Windows;

namespace Loadcell
{
    partial class SensorValuePerKgDlg: Window

    {
      public SensorValuePerKgDlg()
      {
         InitializeComponent();
      }

      void OKBtnClicked(object sender, RoutedEventArgs args)
      {
         DialogResult = true;
      }

      void CancelBtnClicked(object sender, RoutedEventArgs args) {
         DialogResult = false;
      }
    }
}
