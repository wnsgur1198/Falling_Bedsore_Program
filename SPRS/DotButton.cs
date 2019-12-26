using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;

namespace Loadcell
{
    partial class DotButton
    {
        RadialGradientBrush buttonBrush = null;
        GradientStop stop1 = null;
        GradientStop stop2 = null;
        Color buttonColor = Colors.Red;
        Point buttonDownPosition = new Point(0.35, 0.35);
        Point buttonNormalPosition = new Point(0.3, 0.3);
        bool hollow = false;

        //---------------------------------------------------
        public Color ButtonColor
        {
            get { return buttonColor; }
            set
            {
                buttonColor = value;
                stop2.Color = value;
            }
        }

        //---------------------------------------------------
        public bool HollowButton
        {
            get { return hollow; }
            set {
                hollow = value;
                if (hollow) circle.Fill = Brushes.Transparent;
                else if(buttonBrush != null) circle.Fill = buttonBrush;
            }
        }

        //---------------------------------------------------
        public Brush ButtonStroke
        {
            get { return circle.Stroke; }
            set { circle.Stroke = value; }
        }

        //---------------------------------------------------
        public double ButtonStrokeThickness
        {
            get { return circle.StrokeThickness; }
            set { circle.StrokeThickness = value; }
        }

        //---------------------------------------------------
        public string ButtonText { get; set; }

        //---------------------------------------------------
        public Point ToolTipPos { get; set; }

        //--------------------------------------------------
        public DotButton()
        {
            InitializeComponent();

            buttonBrush =  new RadialGradientBrush();
            buttonBrush.GradientOrigin = buttonNormalPosition;
            buttonBrush.Center = new Point(0.5, 0.5);
            
            stop1 = new GradientStop();
            stop2 = new GradientStop();

            stop1.Offset = 0.0;
            stop2.Offset = 0.5;
            stop1.Color = Colors.White;
            stop2.Color = buttonColor;

            buttonBrush.GradientStops.Add(stop1);
            buttonBrush.GradientStops.Add(stop2);

            if (hollow) circle.Fill = Brushes.Transparent;
            else circle.Fill = buttonBrush;

        }//Constructor()

        private void mouseEnter(object sender, System.Windows.Input.MouseEventArgs e)
        {
            stop1.Offset = 0.18;

        }

        private void mouseLeave(object sender, System.Windows.Input.MouseEventArgs e)
        {
            stop1.Offset = 0.0;
            buttonBrush.GradientOrigin = buttonNormalPosition;
        }

        private void mouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            buttonBrush.GradientOrigin = buttonDownPosition;

        }

        private void mouseUp(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            buttonBrush.GradientOrigin = buttonNormalPosition;

        }

    }//class
}//ns