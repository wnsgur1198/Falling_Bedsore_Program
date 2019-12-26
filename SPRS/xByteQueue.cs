using System;

namespace Loadcell
{
    class xByteQueue
    {
        int size;
        int count;
        int inPos;
        int outPos;

        byte[] buffer = null;

        public xByteQueue(int size)
        {
            if(size < 1) throw new Exception("Zero size ByteQueue Exception");

            this.size = size;
            buffer = new byte[size];
            clear();
        }

        public bool isEmpty
        {
            get { return (count==0); }
        }

        public bool isFull
        {
            get { return (count==size); }
        }

        public void clear()
        {
            count = inPos = outPos = 0;
        }


        public byte data
        {
            get
            {
                if(isEmpty) return 0xFF;  //should check before get
                byte b = buffer[outPos];
                outPos = (outPos + 1) % size;
                count--;
                return b;
            }

            set
            {
                if(isFull) return;
                buffer[inPos] = value;
                inPos = (inPos + 1) % size;
                count++;
            }
        }
    }
}
