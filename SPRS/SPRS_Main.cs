using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using System.Windows.Media;
using System.IO.Ports;
using System.Threading;
using System.Xml;
using System.Diagnostics;
using System.Collections.Generic;
using MySql.Data.MySqlClient;
using DB_Test;

namespace Loadcell
{
    public partial class SPRS : Window
    {
        enum xMode { SETUP, IDLE, TARE_INFO, TARE, UNIT_INFO, UNIT, MEASURE };

        enum xCanvasMode { SETUP, IDLE, MEASURE };

        const double scaleBoardWidth = 0.81;// 0.92; //0.55; //<-- 프로토타입 작은 보드 폭  //  1.0;  //발판 가로폭 : 단위 미터
        const double scaleBoardHeight = 1.89;//2.07; //1.1; //<-- 프로토타입 작은 보드 높이   //1.0; //발판 세로폭 :단위 미터
        const double scaleBoardLeft = -(scaleBoardWidth / 2.0);
        const double scaleBoardRight = scaleBoardWidth / 2.0;
        const double scaleBoardTop = scaleBoardHeight / 2.0;
        const double scaleBoardBottom = -(scaleBoardHeight / 2.0);
        const double minWeight = 1.0;
        const int UPPER_LEFT = 0;
        const int UPPER_RIGHT = 1;
        const int LOWER_LEFT = 2;
        const int LOWER_RIGHT = 3;
        const int labelrow = 20; //6행
        const int labelcol = 15; //4열
        const byte CMD_START = 0xF0;
        const byte CMD_STOP = 0xFF;
        const byte CMD_DATA = 0xFD;

        List<User> users = new List<User>();

        //입력 데이터 16바이트(4x4) 발판의 다음 위치의 순서로 정렬되어 있습니다.
        int[] inputSequence = new int[4] { UPPER_LEFT, LOWER_LEFT, LOWER_RIGHT, UPPER_RIGHT };

        public static Point COP = new Point(0, 0); //Center of Pressure(발판 위의 압력분포의 중심 위치 좌표)
        Point saveCOP = new Point(0, 0);
        Point cursorPos = new Point(0, 0);

        double bedCanvasWidth = 0;  //Screen Canvas Width 단위: 픽셀
        double bedCanvasHeight = 0; //Screen Canvas Height 단위: 픽셀

        Mutex posMutex = new Mutex();
        public static Mutex dataMutex = new Mutex();
        Mutex canvasMutex = new Mutex();
        Mutex cursorMutex = new Mutex();
        Mutex ratioMutex = new Mutex();
        Mutex queueMutex = new Mutex();

        xCanvasMode canvasMode = xCanvasMode.SETUP;
        Thread dataReaderThread = null;
        Thread dataUserThread = null;
        Thread comSeekerThread = null;
        Thread terminatorThread = null;
        Thread dateTimeThread = null;
        Thread databaseThread = null;

        bool running = false;
        bool seeking = false;
        bool newData = false;
        bool tareReady = false;
        bool unitReady = false;
        bool testRun = false;

        xMode currentMode = xMode.SETUP;
        UInt32 samplingCount = 0;

        SerialPort serial = null;

        string serialPortName = "";
        string BaudRate = "";
        string Parity = "";
        string DataBits = "";
        string StopBits = "";
        string Handshake = "";        

        byte[] buffer = new byte[16];
        double[] tare = { 0, 0, 0, 0 };
        double[] unit = { 1, 1, 1, 1 };
        double[] massRatio = { 0, 0, 0, 0 };
        public static double massTotal = 0;

        public static double[] data1 = new double[4]; //data저장

        double samplingMass = 0.0;

        double theta = 0.0;
        double radius = 0.6;
        double radiusInc = 0.05;
        Random rand = null;

        Stopwatch stopWatch = new Stopwatch();
        TimeSpan ts;
        
        string strTotalTimer;
        
        Label[] massLabel = null;

        Label[,] gridLabel = new Label[labelrow, labelcol]; //라벨만들기
        double[,] massdist = new double[labelrow, labelcol]; //더블형 mass

        xByteQueue sendQueue = new xByteQueue(10);

        xMovingAverage[] movingAvr = {new xMovingAverage(4), new xMovingAverage(4),
                                      new xMovingAverage(4), new xMovingAverage(4)};
        [STAThread]
        static void Main(string[] args)
        {
            new Application().Run(new SPRS());
        }

        //---------------------------------------        
        public SPRS()
        {
            InitializeComponent();
            
            massLabel = new Label[4] { UpLeft, UpRight, DnLeft, DnRight };

            Canvas.SetLeft(ToolTipLabel, -100);
            Canvas.SetTop(ToolTipLabel, 0);

            loadConfiguration();

            Show();

            //----

            if (tareReady && unitReady) setMode(xMode.IDLE);
            else setMode(xMode.SETUP);

            serial = getSerialPort();

            if (serial == null)
            {
                testRun = true;
                rand = new Random();
            }

            //sendData(CMD_START);

            running = true;

            // 데이터읽어오는 쓰레드
            dataReaderThread = new Thread(dataReader);
            dataReaderThread.Start(serial);

            // 읽어온 데이터 네 개로 나눠 사용하기 위한 쓰레드
            dataUserThread = new Thread(dataUser);
            dataUserThread.Start();

            // 무게중심 찾는 쓰레드
            comSeekerThread = new Thread(copSeeker);
            comSeekerThread.Start();

            // DB시간측정용 쓰레드
            dateTimeThread = new Thread(timeLogger);
            dateTimeThread.Start();

            // DB 쓰레드
            //databaseThread = new Thread(insertTable);
            //databaseThread.Start();

        }

        //---------------------------------------------------------------------
        private void SPRS_Loaded(object sender, RoutedEventArgs e)
        {
            bedCanvasWidth = bedCanvas.ActualWidth;
            bedCanvasHeight = bedCanvas.ActualHeight;

            setCanvasLabels();
        }

        //------------------------------------
        void loadConfiguration()
        {
            XmlDocument xdoc = new XmlDocument();
            XmlNode node = null;
            XmlNode child = null;

            string[] tareText = { null, null, null, null };
            string[] unitText = { null, null, null, null };
            string[] posStr = { "UpLeft", "UpRight", "DownLeft", "DownRight" };

            try
            {
                xdoc.Load("SPRS_Config.xml");
            }
            catch (Exception e)
            {
                Console.WriteLine("loadConfig " + e.Message);
                return;
            }

            node = xdoc.DocumentElement.SelectSingleNode("SerialPort");
            if (node == null) return;

            node = xdoc.DocumentElement.SelectSingleNode("Loadcells");

            if (node == null) return;

            tareReady = true;
            unitReady = true;

            for (int i = 0; i < 4; i++)
            {
                child = node.SelectSingleNode(string.Format("Loadcell[@pos='{0}']", posStr[i]));
                if (child != null)
                {
                    try
                    {
                        tare[i] = Convert.ToDouble(child.Attributes["tare"].Value);
                        unit[i] = Convert.ToDouble(child.Attributes["unit"].Value);
                    }
                    catch (Exception e)
                    {
                        tare[i] = 0.0;
                        unit[i] = 1.0;
                        tareReady = false;
                        unitReady = false;
                        Console.WriteLine("selectSinglel " + e.Message);
                    }
                }
            }
        }

        //------------------------------------
        void saveConfiguration()
        {
            XmlDocument xdoc = new XmlDocument();
            XmlElement root;
            XmlElement parent;
            XmlElement child;

            string[] posStr = { "UpLeft", "UpRight", "DownLeft", "DownRight" };

            xdoc.AppendChild(root = xdoc.CreateElement("SPRS"));
            root.SetAttribute("desc", "Sleeping Posture Recording Device");
            root.AppendChild(child = xdoc.CreateElement("SerialPort"));
            child.SetAttribute("Port", serialPortName);
            child.SetAttribute("BaudRate", BaudRate);
            child.SetAttribute("Parity", Parity);
            child.SetAttribute("DataBits", DataBits);
            child.SetAttribute("StopBits", StopBits);
            child.SetAttribute("Handshake", Handshake);

            root.AppendChild(parent = xdoc.CreateElement("Loadcells"));

            for (int i = 0; i < 4; i++)
            {
                parent.AppendChild(child = xdoc.CreateElement("Loadcell"));
                child.SetAttribute("pos", posStr[i]);
                child.SetAttribute("tare", tare[i].ToString());
                child.SetAttribute("unit", unit[i].ToString());
            }

            try { xdoc.Save("SPRS_Config.xml"); }
            catch { Console.WriteLine("Configuration File Save Error"); }
        }

        //------------------------------------
        private void btn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            DotButton btn = sender as DotButton;
            if (btn == null) return;
            btn.HollowButton = true;
            ToolTipLabel.Content = "";
        }

        //------------------------------------
        private void btn_MouseLeave(object sender, MouseEventArgs e)
        {
            DotButton btn = sender as DotButton;
            if (btn == null) return;
            btn.HollowButton = true;
            ToolTipLabel.Content = "";
        }

        //------------------------------------
        private void btn_MouseEnter(object sender, MouseEventArgs e)
        {
            DotButton btn = sender as DotButton;
            if (btn == null) return;
            btn.HollowButton = false;
            ToolTipLabel.Content = btn.ButtonText;
            Canvas.SetLeft(ToolTipLabel, btn.ToolTipPos.X);
        }

        //-------------------------------------------
        void setButtons(xMode mode)
        {
            buttonStack.Children.Clear();

            if ((mode == xMode.IDLE) || (mode == xMode.SETUP))
            {
                buttonStack.Children.Add(tareBtn);
                if (tareReady) buttonStack.Children.Add(unitBtn);
                if (unitReady) buttonStack.Children.Add(measureBtn);
                buttonStack.Children.Add(cameraBtn);
                buttonStack.Children.Add(chartBtn);
                buttonStack.Children.Add(weightBtn);
                buttonStack.Children.Add(LRChartBtn);
                buttonStack.Children.Add(closeBtn);
            }
            else { buttonStack.Children.Add(stopBtn); }
        }


        //-------------------------------------------선영
        void setCanvasLabels()
        {
            double width = bedCanvasWidth / labelcol;
            double height = bedCanvasHeight / labelrow;
            for (int i = 0; i < labelrow; i++)
            {
                for (int j = 0; j < labelcol; j++)
                {
                    Label a = new Label();
                    a.Width = width;
                    a.Height = height;
                    a.FontSize = 9.0;
                    Canvas.SetZIndex(a, 10); //맨 앞으로 온다
                    gridLabel[i, j] = a;
                }
            }
        }

        //--------------------------------------------
        void LoadGridLabels()
        {
            for (int i = 0; i < labelrow; i++)
            {
                for (int j = 0; j < labelcol; j++)
                {
                    bedCanvas.Children.Add(gridLabel[i, j]);
                    Canvas.SetLeft(gridLabel[i, j], gridLabel[i, j].Width * j);
                    Canvas.SetTop(gridLabel[i, j], gridLabel[i, j].Height * i);
                }
            }
        }

        //--------------------------------------------
        void setCanvas(xMode mode)
        {
            canvasMutex.WaitOne();

            bedCanvas.Children.Clear();

            if (mode == xMode.SETUP)
            {
                bedCanvas.Background = Brushes.Black;
                canvasMode = xCanvasMode.SETUP;
            }
            else //if ( (mode == xMode.MEASURE) || (mode == xMode.IDLE) )
            {
                if (mode == xMode.MEASURE)
                {
                    bedCanvas.Children.Add(Circle); //내가, 여기는 측정버튼 누르면 원 나오는 곳
                    Canvas.SetLeft(Circle, (bedCanvasWidth * 0.5) - (Circle.Width * 0.5)); //내가, 여기는 측정버튼 누르면 원 나오는 곳
                    Canvas.SetTop(Circle, (bedCanvasHeight * 0.5) - (Circle.Height * 0.5)); //내가, 여기는 측정버튼 누르면 원 나오는 곳

                    bedCanvas.Children.Add(cross1);
                    bedCanvas.Children.Add(cross2);

                    LoadGridLabels();

                    canvasMode = xCanvasMode.MEASURE;
                }
                else
                {
                    double margin = 10;

                    bedCanvas.Children.Add(Center);
                    Canvas.SetLeft(Center, (bedCanvasWidth * 0.5) - (Center.Width * 0.5));
                    Canvas.SetTop(Center, (bedCanvasHeight * 0.5) - (Center.Height * 0.5));
                    Canvas.SetZIndex(Center, 20);

                    bedCanvas.Children.Add(UpLeft);
                    Canvas.SetLeft(UpLeft, 0 + margin);
                    Canvas.SetTop(UpLeft, 0 + margin);

                    bedCanvas.Children.Add(UpRight);
                    Canvas.SetLeft(UpRight, bedCanvasWidth - UpRight.Width - margin);
                    Canvas.SetTop(UpRight, 0 + margin);

                    bedCanvas.Children.Add(DnLeft);
                    Canvas.SetLeft(DnLeft, 0 + margin);
                    Canvas.SetTop(DnLeft, bedCanvasHeight - DnLeft.Height - margin);

                    bedCanvas.Children.Add(DnRight);
                    Canvas.SetLeft(DnRight, bedCanvasWidth - DnRight.Width - margin);
                    Canvas.SetTop(DnRight, bedCanvasHeight - DnRight.Height - margin);

                    canvasMode = xCanvasMode.IDLE;
                }
            }
            canvasMutex.ReleaseMutex();
        }

        //---------------------------------------------
        void setMode(xMode mode)
        {
            //--- set seeking mode
            if ((mode == xMode.IDLE) || (mode == xMode.MEASURE)) seeking = true;
            else seeking = false;

            //--- set canvas contents
            setCanvas(mode);
            setButtons(mode);
            currentMode = mode;
        }

        //--------------------------------------
        void cameraBtn_Click(object sender, MouseButtonEventArgs args)
        {
            Process process = new Process();
            process.StartInfo.FileName = "chrome.exe";
            process.StartInfo.Arguments = "http://192.168.43.224:8091/?action=stream";
            process.StartInfo.WindowStyle = ProcessWindowStyle.Normal;
            process.Start();
        }

        //--------------------------------------
        void chartBtn_Click(object sender, MouseButtonEventArgs args)
        {
            new OnlyCharts().Show();
        }

        //--------------------------------------
        void weightBtn_Click(object sender, MouseButtonEventArgs args)
        {
            new WeightChart().Show();
        }

        //--------------------------------------
        void LRChartBtn_Click(object sender, MouseButtonEventArgs args)
        {
            new LRChart().Show();
        }
        
        //--------------------------------------
        void stopBtn_Click(object sender, MouseButtonEventArgs args)
        {      
            if (currentMode == xMode.TARE) tareReady = true;
            if (currentMode == xMode.UNIT) unitReady = true;

            if (tareReady && unitReady) setMode(xMode.IDLE);
            else setMode(xMode.SETUP);

            stopWatch.Stop();
        }

        //--------------------------------------
        void closeBtn_Click(object sender, MouseButtonEventArgs args)
        {
            ////주의!!
            ////메인 스레드가 아래의 스레드들의 Join함수에서 대기하고 있는 동안 스레드에서 
            ////Invoke를 통하여 호출한 메인스레드 함수의 실행을 할 수 없기 때문에 Deadlock이 발생할 수 있음.
            ////따라서 아래의 명령어들은 별도의 스레드 내에서 이루어져야 함.

            terminatorThread = new Thread(terminator);
            terminatorThread.Start(this);
        }

        //--------------------------------------
        public void tareBtn_Click(object sender, MouseButtonEventArgs args)
        {
            DotButton btn = sender as DotButton;
            if (btn == null) return;
            btn.HollowButton = true;
            ToolTipLabel.Content = "";

            double[] temp = new double[4];
            for (int i = 0; i < 4; i++) temp[i] = tare[i];  //save for cancel event
            stopBtn.Content = (string)btn.ButtonText + " [완료]";
            
            xMode saveMode = currentMode;

            setMode(xMode.TARE_INFO);

            string msg = "1. 침대 위에 침구 이외의 물건을 모두 치우세요.\n\n" +
                         "2. 침대가 수평을 이루고 있는지 확인하세요.\n\n" +
                         "3. 침대의 중량이 모든 센서에 고르게 분포되도록 센서의 높이를 조절하세요.\n\n" +
                         "4. 확인버튼을 눌러서 메시지 창을 닫으면 '자체중량' 평균값의 측정이 시작됩니다.\n\n" +
                         "5. 창이 닫힌 후 네 모서리의 숫자가 일정한 값에 수렴 할 때까지 기다렸다가 왼쪽의 [완료] 버튼을 누르세요.";

            if (MessageBox.Show(msg, "자체중량 지정", MessageBoxButton.OKCancel) == MessageBoxResult.Cancel)
            {
                for (int i = 0; i < 4; i++) tare[i] = temp[i];

                setMode(saveMode);

                return;
            }

            samplingCount = 0;

            setMode(xMode.TARE);
        }

        //--------------------------------------
        void measureBtn_Click(object sender, MouseButtonEventArgs args)
        {
            DotButton btn = sender as DotButton;
            if (btn == null) return;
            btn.HollowButton = true;
            ToolTipLabel.Content = "";

            stopBtn.Content = (string)btn.ButtonText + " [완료]";
            setMode(xMode.MEASURE);

            stopWatch.Start();
            ResetBtn.IsEnabled = true;

            BedsoreTime();
        }

        //--------------------------------------
        void unitBtn_Click(object sender, MouseButtonEventArgs args)
        {
            DotButton btn = sender as DotButton;
            if (btn == null) return;
            btn.HollowButton = true;
            ToolTipLabel.Content = "";

            stopBtn.Content = (string)btn.ButtonText + " [완료]";

            double[] temp = new double[4];

            for (int i = 0; i < 4; i++) temp[i] = unit[i];  //save for cancel event

            xMode saveMode = currentMode;

            setMode(xMode.UNIT_INFO);

            SensorValuePerKgDlg dlg = new SensorValuePerKgDlg();
            string Msg = "1. 침상의 중앙에 질량을 알고 있는 물체를 놓고 물체의 질량 값(Kg)을 아래의 입력창에 입력하세요\n\n" +
                         "2. 'OK' 버튼을 눌러서 창을 닫으면 입력된 질량에 해당하는 센서의 평균값을 측정합니다.\n\n" +
                         "3. 메시지 창이 닫힌 후 네 모서리의 숫자가 일정한 값에 수렴할 때까지 기다렸다가 왼쪽의 [완료] 버튼을 누르세요.";
            dlg.infoMsg.Text = Msg;
            dlg.samplingMass.Focus();
            dlg.ShowDialog();

            if (dlg.DialogResult == false)
            {
                for (int i = 0; i < 4; i++) unit[i] = temp[i];

                setMode(saveMode);

                return;
            }

            try { samplingMass = Convert.ToDouble(dlg.samplingMass.Text) / 4.0; }
            catch (Exception e)
            {
                MessageBox.Show("적절한 double타입의 값이 아닙니다.", "Error");
                Console.WriteLine(e.Message);

                for (int i = 0; i < 4; i++) unit[i] = temp[i];

                setMode(saveMode);

                return;
            }

            if (samplingMass <= 0.0)
            {
                MessageBox.Show("샘플 질량 값은 0보다 커야 합니다.", "Error");

                for (int i = 0; i < 4; i++) unit[i] = temp[i];

                setMode(saveMode);

                return;
            }
            samplingCount = 0;
            setMode(xMode.UNIT);
        }

        //---------------------------------------------------------------------
        void ResetBtn_Click(object sender, RoutedEventArgs args)
        {
            Dispatcher.Invoke(new Action(delegate
            {
                ResetBtn.IsEnabled = true;
                stopWatch.Restart();
                ts = stopWatch.Elapsed;

                strTotalTimer = String.Format("{0:00}:{1:00}:{2:00}.{3:00}", ts.Hours, ts.Minutes, ts.Seconds, ts.Milliseconds / 10);
                ProgressTimer.Text = strTotalTimer;
            }));
        }

        //----------------------------------------
        void getTareMass(double[] data)
        {
            if (currentMode == xMode.TARE_INFO)
            {
                for (int i = 0; i < 4; i++) { massLabel[i].Content = roundUpInt(tare[i] = data[i]).ToString(); }
            }
            else if (currentMode == xMode.TARE)
            {
                samplingCount++;
                if (samplingCount == 0) samplingCount = 1;
                double rate = (samplingCount - 1) / ((double)samplingCount);

                for (int i = 0; i < 4; i++)
                {
                    massLabel[i].Content = roundUpInt(tare[i] = (tare[i] * rate) + (data[i] * (1.0 - rate))).ToString();
                }
            }
            Center.Content = samplingCount.ToString();
        }

        //----------------------------------------
        void getUnitMass(double[] data)
        {
            if (tareReady == false) return;

            for (int i = 0; i < 4; i++) { data[i] -= tare[i]; }

            if (currentMode == xMode.UNIT_INFO)
            {
                for (int i = 0; i < 4; i++)
                {
                    massLabel[i].Content = roundUpInt(unit[i] = data[i]).ToString();
                }
            }
            else if (currentMode == xMode.UNIT)
            {
                samplingCount++;

                if (samplingCount == 0) samplingCount = 1;

                double rate = (samplingCount - 1) / ((double)samplingCount);

                for (int i = 0; i < 4; i++)
                {
                    massLabel[i].Content = roundUpInt(unit[i] = (unit[i] * rate) + (data[i] * (1.0 - rate) / samplingMass)).ToString();
                }
            }
            Center.Content = samplingCount.ToString();
        }

        //----------------------------------------
        void getCenterOfMass(double[] data)
        {
            //단위 질량 당 센서 값이 지정되어 있지 않으면 중량 계산을 할 수 없음.
            if (unitReady == false) return;

            Boolean IfFalling = false; //낙상시 알림 위함이다

            double[] ratio = { 0, 0, 0, 0 };
            double copX = 0;
            double copY = 0;

            double totalMass = 0;

            for (int i = 0; i < 4; i++)
            {
                data[i] = (data[i] - tare[i]) / unit[i]; //로드셀이 인식하는 값들 data 배열에 저장
                totalMass += data[i];
                data1[i] = data[i];//data1(전역변수)에 data 저장, 내가
            }

            if (totalMass < 1.5) //총 합이 1.5보다 작으면
            {
                for (int i = 0; i < 4; i++)
                {
                    data[i] = 0; //0으로 설정하기 
                    data1[i] = data[i];//data1(전역변수)에 data 저장, 내가
                }
                totalMass = 0;

                // 무게가 사라졌을 때 falling
                if (IfFalling)  { MessageBox.Show("Falling!"); }
            }


            // 기본상태(측정모드가 아닌)에서 값 표시--------------------------------------------------------------------
            if (currentMode == xMode.IDLE)
            {
                Dispatcher.Invoke(new Action(delegate
                {
                    for (int i = 0; i < 4; i++) { massLabel[i].Content = data[i].ToString("N2"); }

                    Center.Content = totalMass.ToString("N1");
                }));
            }


            // 무게중심 계산해서 캔버스의 cop위치 바꾸는 부분--------------------------------------------------------------
            if (totalMass >= minWeight)
            {
                IfFalling = true; // falling 판별 위해 일단 환자가 있었다는 것을 알려주기 위해서이다
                                  //--질량 중심 계산-------------------------------
                copX = ((data[0] + data[2]) * scaleBoardLeft + (data[1] + data[3]) * scaleBoardRight) / totalMass;
                copY = ((data[0] + data[1]) * scaleBoardTop + (data[2] + data[3]) * scaleBoardBottom) / totalMass;

                for (int i = 0; i < 4; i++) ratio[i] = data[i] / totalMass;
            }
            else
            {
                copX = 0;
                copY = 0;
                for (int i = 0; i < 4; i++) ratio[i] = 0.0;
            }

            posMutex.WaitOne();

            if (TimeChanged())
            {
                saveCOP.X = COP.X;
                saveCOP.Y = COP.Y;
            }
            COP.X = copX;
            COP.Y = copY;
            posMutex.ReleaseMutex();

            ratioMutex.WaitOne();
            for (int i = 0; i < 4; i++) massRatio[i] = ratio[i];
            massTotal = totalMass;
            ratioMutex.ReleaseMutex();

            try { findPosition(data1); }
            catch (Exception e) { Console.WriteLine("findPisition Error : " + e.Message); }
        }

        //---------------------------------------------------------------------
        void calmass2(double[] data)
        {
            double UL = data[0];
            double UR = data[1];
            double DL = data[2];
            double DR = data[3];
            double totalMass = UL + UR + DL + DR;

            double leftDeltaY = (DL - UL) / (labelrow - 1);
            double rightDeltaY = (DR - UR) / (labelrow - 1);

            for (int i = 0; i < labelrow; i++)
            {
                double startMass = UL + leftDeltaY * i;
                double endMass = UR + rightDeltaY * i;
                double dx = (endMass - startMass) / (labelcol - 1);

                Dispatcher.Invoke(new Action(delegate
                {
                    for (int j = 0; j < labelcol; j++)
                    {
                        if (totalMass < 1.0) { Circle.Fill.Opacity = 0; }
                        else { Circle.Fill.Opacity = 1; }

                        massdist[i, j] = startMass + dx * j;
                        if (massdist[i, j] < 0.0) massdist[i, j] = 0.0;

                        int color = (int)(255 * (massdist[i, j] / totalMass * 2.0)) - 80;

                        if (color > 100) color = (int)(color * 3.0);

                        if (color > 255) color = 255;
                        if (color < 0 || totalMass <= 1.0) color = 0;
                        gridLabel[i, j].Content = ""; //grid에 아무것도 안넣기
                        gridLabel[i, j].Background = new SolidColorBrush(Color.FromArgb(255, (byte)color, (byte)color, (byte)color));
                    }
                }));
            }
        }

        Boolean TimeChanged()
        {
            ts = stopWatch.Elapsed;
         
            string MovedTime = String.Format("{0:00}:{1:00}", ts.Minutes, ts.Seconds);

            if (ts.Seconds > 3)
            {
                //stopWatch.Restart();
                MovedTime = String.Format("{0:00}:{1:00}", ts.Minutes, ts.Seconds);
            }

            if ((ts.Seconds + 1) % 4 == 0)
            {
                //stopWatch.Restart();
                MovedTime = String.Format("{0:00}:{1:00}", ts.Minutes, ts.Seconds);                

                return true;
            }
            else   return false;
        }

        // 무게중심이 움직인 거리 파악---------------------------------------------------------------------
        Boolean PatientMove() 
        {
            double dx = saveCOP.X - COP.X;
            double dy = saveCOP.Y - COP.Y;
            double distance = Math.Sqrt(dx * dx + dy * dy);

            if (distance > 0.2) return true;
            return false;
        }

        //********************내가 만듦************************
        void findPosition(double[] data)
        {
            calmass2(data); //계산하기
        }

        //---------------------------------------------------------------------
        void BedsoreTime()
        {
            ts = stopWatch.Elapsed;
            strTotalTimer = String.Format("{0:00}:{1:00}:{2:00}.{3:00}", ts.Hours, ts.Minutes, ts.Seconds, ts.Milliseconds / 10);
                        
            //욕창 소리 - 2분1초 지나면 소리
            ts = stopWatch.Elapsed;
            
            Dispatcher.Invoke(new Action(delegate
            {
                ProgressTimer.Text = strTotalTimer;
            }));            
        }

        //---------------------------------------------------------------------
        byte[] testData()
        {
            byte[] buffer16 = new byte[16];
            byte[] buffer4 = { 0, 0, 0, 0 };
            Int32 loadcellValue;
            double[] mass = new double[] { 0, 0, 0, 0 };

            if ((currentMode == xMode.IDLE) || (currentMode == xMode.MEASURE))
            {
                double TwoPI = Math.PI * 2.0;

                double thetaInc = TwoPI / 100.0;
                double totalMass = 70.0;// + (10.0 * rand.NextDouble() - 5.0);
                double halfMass = 0.0;
                double moreMass = 0.0;
                double rightMass = 0.0;
                double leftMass = 0.0;
                double yComponent = Math.Sin(theta) * radius;
                double xComponent = Math.Cos(theta) * radius;

                theta += thetaInc;
                if (theta >= TwoPI) theta -= TwoPI;

                radius += radiusInc;

                if (radius > 0.8 || radius < 0.3) radiusInc = -radiusInc;


                halfMass = totalMass / 2.0;
                moreMass = halfMass * xComponent;

                rightMass = halfMass + moreMass;
                leftMass = halfMass - moreMass;

                halfMass = rightMass / 2.0;
                moreMass = halfMass * yComponent;

                mass[3] = halfMass + moreMass;
                mass[2] = halfMass - moreMass;

                halfMass = leftMass / 2.0;
                moreMass = halfMass * yComponent;

                mass[0] = halfMass + moreMass;
                mass[1] = halfMass - moreMass;
            }


            for (int i = 0; i < 4; i++)
            {
                if ((currentMode == xMode.TARE) || (currentMode == xMode.TARE_INFO))
                {
                    loadcellValue = (Int32)(80000.0 + 3000.0 * rand.NextDouble() + 0.5);
                }
                else if ((currentMode == xMode.UNIT) || (currentMode == xMode.UNIT_INFO))
                {
                    loadcellValue = (Int32)(tare[i] + (30000.0) + (2000.0) * rand.NextDouble() + 0.5);
                }
                else if ((currentMode == xMode.IDLE) || (currentMode == xMode.MEASURE))
                {
                    loadcellValue = (Int32)(mass[i] * unit[i] + tare[i] + 0.5);
                }
                else
                {
                    loadcellValue = 0;
                }

                buffer4 = BitConverter.GetBytes(loadcellValue);

                Buffer.BlockCopy(buffer4, 0, buffer16, i * 4, 4);
            }
            return buffer16;
        }

        //---------------------------------------
        private void dataReader(object obj)
        {
            SerialPort serial = obj as SerialPort;

            //-----------------
            byte[] buffer16 = new byte[16];
            byte[] temp = new Byte[1] { CMD_DATA };
            byte[] command = new Byte[1];

            int readLen, totalRead;

            while (running)
            {
                if (testRun)
                {
                    buffer16 = testData();
                    Thread.Sleep(50);
                }
                else
                {
                    queueMutex.WaitOne();
                    while (sendQueue.isEmpty == false)
                    {
                        command[0] = sendQueue.data;
                        try
                        {
                            serial.Write(command, 0, 1);
                        }
                        catch (Exception e)
                        {
                            Console.WriteLine("send Command Error\n" + e.Message);
                            continue;
                        }
                    }//while
                    queueMutex.ReleaseMutex();

                    try
                    {
                        serial.Write(temp, 0, 1);

                        for (totalRead = 0; totalRead < 16; totalRead += readLen)
                        {
                            readLen = serial.Read(buffer16, totalRead, 16 - totalRead);
                        }
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine("datareader : " + e.Message); //console창에 시간초과했다고 뜬다 1월13일
                        continue;
                    }
                }
                dataMutex.WaitOne();
                Buffer.BlockCopy(buffer16, 0, buffer, 0, 16);
                newData = true;
                dataMutex.ReleaseMutex();
            }//while
        }

        //---------------------------------------
        private void dataUser()
        {
            byte[] buffer16 = new byte[16];
            byte[] buffer4 = new byte[4];
            double[] data4 = new double[4];
            bool newOne = false;

            while (running)
            {
                dataMutex.WaitOne();
                newOne = newData;
                dataMutex.ReleaseMutex();

                if (newOne == false)
                {
                    Thread.Sleep(10); //release CPU until new data arrive.
                    continue;
                }

                dataMutex.WaitOne();
                Buffer.BlockCopy(buffer, 0, buffer16, 0, 16);
                newData = false;
                dataMutex.ReleaseMutex();

                for (int i = 0; i < 4; i++)
                {
                    Buffer.BlockCopy(buffer16, i * 4, buffer4, 0, 4);

                    //use moving average if movement is unstable..
                    if (currentMode == xMode.IDLE || currentMode == xMode.MEASURE)
                    {
                        data4[inputSequence[i]] = movingAvr[i].add(BitConverter.ToInt32(buffer4, 0));
                    }
                    else
                    {
                        data4[inputSequence[i]] = (BitConverter.ToInt32(buffer4, 0));
                    }
                }

                switch (currentMode)
                {
                    case xMode.TARE_INFO:
                    case xMode.TARE:
                        Dispatcher.Invoke(new Action(delegate { getTareMass(data4); }));

                        break;
                    case xMode.UNIT_INFO:
                    case xMode.UNIT:
                        Dispatcher.Invoke(new Action(delegate { getUnitMass(data4); }));

                        break;
                    case xMode.IDLE:
                    case xMode.MEASURE:
                        Dispatcher.Invoke(new Action(delegate
                        {
                            User newUser = new User()
                            {
                                Time = (string)dateTimeString.Content,
                                DR = Math.Round(data1[3], 2),
                                DL = Math.Round(data1[2], 2),
                                UR = Math.Round(data1[1], 2),
                                UL = Math.Round(data1[0], 2),
                                TOTAL = Math.Round(massTotal, 2)
                            };


                            // 라즈베리파이 DB------------------------------------
                            //bool Dflag = true;
                            //if ( Dflag ) { insertTable(); }
                            //else { deleteTable(); }                            


                            users.Add(newUser);
                            dgUsers.Items.Add(newUser);
                        }));

                        BedsoreTime();
                        getCenterOfMass(data4);

                        if (testRun) Thread.Sleep(10);
                        break;
                }
                switch (currentMode)
                {
                    case xMode.TARE_INFO:
                    case xMode.UNIT_INFO:
                        Thread.Sleep(500);
                        break;

                    default:
                        Thread.Sleep(10);
                        break;
                }
            }
        }

        void deleteTable()
        {
            // 라즈베리파이 DB------------------------------------
            string source = "Server=192.168.123.127;Port=3306;Database=BED_DB;Uid=root;Pwd=0000;";

            MySqlConnection con = new MySqlConnection(source);

            try
            {
                con.Open();

                // delete data---------------------------------
                MySqlCommand query = new MySqlCommand("DELETE FROM rawdata1_Table", con);
                query.ExecuteNonQuery();

                query.CommandText = "DELETE FROM rawdata2_Table";
                query.ExecuteNonQuery();
            }
            catch (Exception) { MessageBox.Show("DB Error"); }
            finally { con.Close(); }
        }

        //int c = DateTime.Now.Second;
        void insertTable()
        {
            // 라즈베리파이 DB------------------------------------
            string source = "Server=192.168.0.54;Port=3306;Database=BED_DB;Uid=root;Pwd=0000;";
            //string source = "Server=192.168.123.127;Port=3306;Database=BED_DB;Uid=root;Pwd=0000;";

            MySqlConnection con = new MySqlConnection(source);

            while(true)
            {
                try
                {
                    con.Open();

                    // insert data---------------------------------------------
                    string querySentence = string.Format("INSERT INTO rawdata1_Table VALUES('{0}','{1}','{2}','{3}','{4}')",
                                     DateTime.Now.ToString("MM-dd HH:mm:ss"), Math.Round(data1[0], 2), Math.Round(data1[1], 2),
                                                                              Math.Round(data1[2], 2), Math.Round(data1[3], 2));

                    MySqlCommand query = new MySqlCommand(querySentence, con);
                    query.ExecuteNonQuery();

                    string querySentence2 = string.Format("INSERT INTO rawdata2_Table VALUES('{0}','{1}','{2}','{3}')",
                                     DateTime.Now.ToString("MM-dd HH:mm:ss"), Math.Round(COP.X, 2), Math.Round(COP.Y, 2), Math.Round(massTotal, 2));
                    query.CommandText = querySentence2;
                    query.ExecuteNonQuery();

                }
                catch (Exception) { MessageBox.Show("DB Error"); }
                finally { con.Close(); }

                Thread.Sleep(500);
            }
            
        }

        void copSeeker()
        {
            Point target = new Point();

            while (running)
            {
                if (seeking == false)
                {
                    Thread.Sleep(100);
                    continue;
                }

                posMutex.WaitOne();
                target.X = COP.X;
                target.Y = COP.Y;
                posMutex.ReleaseMutex();

                cursorMutex.WaitOne();
                cursorPos.X += (target.X - cursorPos.X) * 0.25;
                cursorPos.Y += (target.Y - cursorPos.Y) * 0.25;
                cursorMutex.ReleaseMutex();

                Dispatcher.Invoke(new Action(delegate { moveCursor(); }));

                Thread.Sleep(10);
            }
        }

        //--------------------------------------
        void timeLogger()
        {
            DateTime dt;

            while (running)
            {
                dt = DateTime.Now;

                Dispatcher.Invoke(delegate
                {
                    dateTimeString.Content = string.Format("{0} {1}", dt.ToString(), dt.DayOfWeek.ToString().Substring(0, 3));
                });
                Thread.Sleep(1000);
            }
        }

        //---------------------------------------
        void terminator(object obj)
        {
            Window mainWin = obj as Window;
            running = false;

            //주의!!
            //메인 스레드가 다른 쓰레드의 Join을 기다리고 있을 때 해당 쓰레드가 Invoke를 통해 호출된 메인 스레드의 함수의 종료를 대기 중
            //이라면 Deadlock 발생함.  왜냐하면 메인 스레드는 Join을 기다리느라 Invoke된 함수를 실행할 수 없고 해당 스레드는 
            //Invoke한 함수의 실행완료를 대기중이라 Join할 수 없기 때문.
            //따라서 Join을 다른 별도의 스레드를 사용하여 대기하는 방법을 테스트함...

            if (dataReaderThread != null)
            {
                dataReaderThread.Join();
                Console.WriteLine("dataReader joined");
            }

            if (dataUserThread != null)
            {
                dataUserThread.Join();
                Console.WriteLine("dataUser joined");
            }

            if (comSeekerThread != null)
            {
                comSeekerThread.Join();
                Console.WriteLine("copSeeker joined");
            }
            
            if (serial != null)
            {
                byte[] temp = new Byte[1] { CMD_STOP };
                try
                {
                    serial.Write(temp, 0, 1);
                    serial.Close();
                }
                catch (Exception e)
                {
                    Console.WriteLine("terminator " + e.Message);
                }
            }

            if (databaseThread != null)
            {
                databaseThread.Abort();
                Console.WriteLine("database Aborted");
            }

            if (testRun == false)
            {
                if (tareReady && unitReady) saveConfiguration();
            }

            stopWatch.Stop();

            Dispatcher.Invoke(new Action(delegate { mainWin.Close(); }));
        }

        //------------------------------------
        //scale발판상의 좌표 값을 화면 Canvas의 픽셀좌표로 변환
        Point getBedCanvasPos(Point boardPos)
        {
            Point canPos = new Point(); //Canvas position

            canPos.X = bedCanvasWidth * (boardPos.X - scaleBoardLeft) / scaleBoardWidth;
            canPos.Y = bedCanvasHeight - (bedCanvasHeight * (boardPos.Y - scaleBoardBottom) / scaleBoardHeight);

            return canPos;
        }

        //--------------------------------------
        void moveCursor()
        {
            Point canPos;
            double halfWidth = Center.Width * 0.5;
            double halfHeight = Center.Height * 0.5;

            cursorMutex.WaitOne();
            canPos = getBedCanvasPos(cursorPos);
            cursorMutex.ReleaseMutex();

            canvasMutex.WaitOne();

            if (canvasMode == xCanvasMode.MEASURE)
            {
                Canvas.SetLeft(Circle, canPos.X - (Circle.Width * 0.5));
                Canvas.SetTop(Circle, canPos.Y - (Circle.Height * 0.5));
                cross1.X1 = cross1.X2 = canPos.X;
                cross1.Y1 = canPos.Y - 10;
                cross1.Y2 = canPos.Y + 10;
                cross2.X1 = canPos.X - 10;
                cross2.X2 = canPos.X + 10;
                cross2.Y1 = cross2.Y2 = canPos.Y;
            }
            else if (canvasMode == xCanvasMode.IDLE)
            {
                Canvas.SetLeft(Center, canPos.X - (Center.Width * 0.5));
                Canvas.SetTop(Center, canPos.Y - (Center.Height * 0.5));
            }
            canvasMutex.ReleaseMutex();
        }

        //---------------------------------------
        private SerialPort getSerialPort()
        {
            SerialPort serial = new SerialPort();

            if (serial == null) throw new Exception("SerialPort 객체가 null입니다.");

            while (true)
            {
                xSerialPortDlg dlg = new xSerialPortDlg();

                dlg.Owner = this;

                dlg.InitializeDialogBox(serial);

                if (string.IsNullOrEmpty(serialPortName) == false) dlg.PortCombo.Text = serialPortName;
                if (string.IsNullOrEmpty(BaudRate) == false) dlg.BaudBox.Text = BaudRate;
                if (string.IsNullOrEmpty(Parity) == false) dlg.ParityCombo.Text = Parity;
                if (string.IsNullOrEmpty(DataBits) == false) dlg.DataBitsBox.Text = DataBits;
                if (string.IsNullOrEmpty(StopBits) == false) dlg.StopBitsCombo.Text = StopBits;
                if (string.IsNullOrEmpty(Handshake) == false) dlg.HandShakeCombo.Text = Handshake;

                dlg.ShowDialog();
                if (dlg.DialogResult == false) return null;

                serialPortName = dlg.PortCombo.Text;
                BaudRate = dlg.BaudBox.Text;
                Parity = dlg.ParityCombo.Text;
                DataBits = dlg.DataBitsBox.Text;
                StopBits = dlg.StopBitsCombo.Text;
                Handshake = dlg.HandShakeCombo.Text;

                serial.PortName = dlg.PortCombo.Text;
                serial.BaudRate = int.Parse(dlg.BaudBox.Text);
                serial.DataBits = int.Parse(dlg.DataBitsBox.Text);
                serial.Parity = (Parity)Enum.Parse(typeof(Parity), dlg.ParityCombo.Text);
                serial.StopBits = (StopBits)Enum.Parse(typeof(StopBits), dlg.StopBitsCombo.Text);
                serial.Handshake = (Handshake)Enum.Parse(typeof(Handshake), dlg.HandShakeCombo.Text);
                serial.ReadTimeout = 500;  //밀리초
                serial.WriteTimeout = 500; //밀리초

                try { serial.Open(); }
                catch (Exception e)
                {
                    MessageBox.Show("Serial Port Open Error:\n\n" + e.Message, "Error");
                    continue;
                }
                break;
            }//while true

            return serial;
        }

        //------------------------------------------------------
        private int roundUpInt(double x)
        {
            return (int)((x > 0.0) ? (x + 0.5) : (x - 0.5));
        }

        //------------------------------------------------------
        private void sendData(byte x)
        {
            bool done = false;

            while (!done)
            {
                queueMutex.WaitOne();
                if (sendQueue.isFull == false)
                {
                    sendQueue.data = x;
                    done = true;
                }
                queueMutex.ReleaseMutex();
                if (!done) Thread.Sleep(10);
            }
        }

        //------------------------------------------------------
        private void DataItem_Click(object sender, MouseButtonEventArgs e)
        {
            ListViewItem viewItem = sender as ListViewItem;
            if (viewItem == null) return;

            ListItem item = viewItem.Content as ListItem;
            if (item == null) return;
        }       
    }//class
}//class

class ListItem
{
    public ListItem(string date, string info)
    {
        measureDate = date;
        measureInfo = info;
    }
    public string measureDate { get; set; }
    public string measureInfo { get; set; }
}

//박성연
public class User
{
    public int Id { get; set; }
    //public DateTime Time { get; set; }
    public string Time { get; set; }
    public double UR { get; set; }
    public double UL { get; set; }
    public double DR { get; set; }
    public double DL { get; set; }
    public double TOTAL { get; set; }
}