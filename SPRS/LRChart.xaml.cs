using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Threading;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;

namespace Loadcell
{
    public partial class LRChart : Window
    {
        ObservableCollection<ChartData> chartData = null;
        ChartData objChartData = null;
        Thread chartThread;
        DateTime dtNow = DateTime.Now;
        int i1 = 0;
        private static EventWaitHandle waitHandle = new ManualResetEvent(initialState: true);

        public LRChart()
        {
            InitializeComponent();
            KeyDown += new KeyEventHandler(Grid_KeyDown);


            // 그래프 사이즈 조정하는 부분
            LR_Chart.Width = Width + 300;
            LR_Chart.Height = Height - 50;


            chartData = new ObservableCollection<ChartData>();
            objChartData = new ChartData() { Name = dtNow, Value = 0.0 };

            chartData.Add(objChartData);
            chartData.Add(new ChartData() { Name = (dtNow + TimeSpan.FromSeconds(5)), Value = Math.Round(SPRS.massTotal, 2) });

      
            xAxis.Minimum = chartData[0].Name;
            LR_Chart.DataContext = chartData;

            // thread---------------------------
            chartThread = new Thread(new ThreadStart(StartChartDataSimulation));
            chartThread.Start();
        }

        private void Grid_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
            {
                chartThread.Abort();
                Close();
            }
        }

        public void StartChartDataSimulation()
        {           
            while (true)
            {
                waitHandle.WaitOne(Timeout.Infinite);

                Dispatcher.Invoke(new Action(() =>
                {
                    var data1 = new ChartData() { Name = DateTime.Now, Value = Math.Round(SPRS.COP.X, 2) };
                    chartData.Add(data1);

                    if (chartData.Count % 100 == 0 && i1 == 0)
                    {
                        xAxis.Minimum = chartData[i1 + 1].Name;
                        i1++;
                    }
                    if (i1 >= 1)
                    {
                        xAxis.Minimum = chartData[i1 + 1].Name;
                        i1++;
                    }

                }));

                Thread.Sleep(50);
            }
        }

        private void Button_pause_Click(object sender, RoutedEventArgs e)
        {
            waitHandle.Reset();
        }

        private void Button_restart_Click(object sender, RoutedEventArgs e)
        {
            waitHandle.Set();
        }
    }


    //------------------------------------------------
    public class LRChartData : INotifyPropertyChanged
    {
        DateTime _Name;
        double _Value;

        #region properties

        public DateTime Name
        {
            get
            {
                return _Name;
            }
            set
            {
                _Name = value;
                OnPropertyChanged("Name");
            }
        }

        public double Value
        {
            get
            {
                return _Value;
            }
            set
            {
                _Value = value;
                OnPropertyChanged("Value");
            }
        }
        #endregion

        public event PropertyChangedEventHandler PropertyChanged;

        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }
}
