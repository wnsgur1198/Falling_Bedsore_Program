using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Loadcell
{
    class xMovingAverage
    {
        int size;
        int total;
        int shiftCount;
        int index;
        int[] buffer;

        public xMovingAverage(int size)
        {
            if(size < 1) throw new Exception("Zero size MovingAverage Queue Exception");

            this.size = size;
            buffer = new int[size];

            shiftCount = -1;

            clear();

            switch(size)
            {
                case 1:
                case 2:
                case 4:
                case 8:
                case 16:
                case 32:
                case 64:
                case 128:
                    
                    for(int i=0; i<8; i++)
                    {
                        size >>= 1;
                        if(size==0)
                        {
                            shiftCount = i;
                            break;
                        }
                    }
                    break;
            }//switch

            
        }

        public void clear()
        {
            total = 0;
            index = -1;
            isNew = false;
        }

        public bool isNew
        {
            get;
            private set;
        }

        public int add(int val)
        {
            movingAverage = val;
            return movingAverage;
        }

        public int movingAverage
        {
            get
            {
                isNew = false;

                if(shiftCount < 0) return (total / size);

                return (total >> shiftCount);
            }//get

            set
            {
                isNew = true;

                // first entry
                if(index < 0) {
                    for(int i=0; i<size; i++)  {
                        buffer[i] = value;
                        total += value;
                    }
                    index = 0;
                }
                else {
                    total = total - buffer[index] + value;
                    buffer[index] = value;
                    index = (index + 1) % size;
                }
                
            }//set
        }//average
    }//class
}//ns
