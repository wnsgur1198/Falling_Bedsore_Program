using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Threading;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;

namespace Loadcell
{
    public partial class WeightChart : Window
    {
        ObservableCollection<ChartData>[] chartData = new ObservableCollection<ChartData>[4];
        ChartData[] objChartData = new ChartData[4];
        Thread chartThread;
        DateTime dtNow = DateTime.Now;
        private static EventWaitHandle waitHandle = new ManualResetEvent(initialState: true);
        int i1 = 0;    // 차트의 데이터 카운트 위한 변수

        public WeightChart()
        {
            InitializeComponent();
            KeyDown += new KeyEventHandler(Grid_KeyDown);

            // 그래프 사이즈 조정하는 부분
            Weight_Chart.Width = Width + 300;
            Weight_Chart.Height = Height - 50;


            for (int i = 0; i < chartData.Length; i++)
            {
                chartData[i] = new ObservableCollection<ChartData>();
                objChartData[i] = new ChartData() { Name = dtNow, Value = 0.0 };

                chartData[i].Add(objChartData[i]);
                chartData[i].Add(new ChartData() { Name = (dtNow + TimeSpan.FromSeconds(5)), Value = Math.Round(SPRS.massTotal, 2) });

            }


            // UL_Chart-----------       
            xAxisUL.Minimum = chartData[0][0].Name;
            Weight_Chart.DataContext = chartData[0];

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
                    //UL_Chart
                    var data1 = new ChartData() { Name = DateTime.Now, Value = Math.Round(SPRS.massTotal, 1) };
                    chartData[0].Add(data1);

                    if (chartData[0].Count % 100 == 0 && i1 == 0)
                    {
                        xAxisUL.Minimum = chartData[0][i1 + 1].Name;
                        i1++;
                    }
                    if (i1 >= 1)
                    {
                        xAxisUL.Minimum = chartData[0][i1 + 1].Name;
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
    public class WeightChartData : INotifyPropertyChanged
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
