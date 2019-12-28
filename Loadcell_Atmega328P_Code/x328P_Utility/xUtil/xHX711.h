#ifndef XHX711_H_2107_09_19_12_09_00
#define XHX711_H_2107_09_19_12_09_00

#include "../x328Util.h"

#define MSBFIRST 1
#define LSBFIRST 0

class xHX711
{
private:
	xPort PD_SCK;	// Power Down and Serial Clock Input Pin
	xPort DOUT;		// Serial Data Output Pin
	uint8_t GAIN;		// amplification factor

	// check if HX711 is ready
	// from the datasheet: When output data is not ready for retrieval, digital output pin DOUT is high. Serial clock
	// input PD_SCK should be low. When DOUT goes to low, it indicates data is ready for retrieval.
	bool is_ready();
	
	// Allows to set the pins and gain later than in the constructor
	void begin(uint8_t dout, uint8_t pd_sck, uint8_t gain = 128);
	
	// set the gain factor; takes effect only after a call to read()
	// channel A can be set for a 128 or 64 gain; channel B has a fixed 32 gain
	// depending on the parameter, the channel is also set to either A or B
	void set_gain(uint8_t gain = 128);
	
	uint8_t shiftIn(uint8_t dir = MSBFIRST);
	  
public:
    
	// define clock and data pin, channel, and gain factor
	// channel selection is made by passing the appropriate gain: 128 or 64 for channel A, 32 for channel B
	// gain: 128 or 64 for channel A; channel B works with 32 gain factor only
	xHX711(uint8_t dout, uint8_t pd_sck, uint8_t gain = 128);

	xHX711();

	 ~xHX711();

	// waits for the chip to be ready and returns a reading
	int32_t read();

	// returns an average reading; times = how many times to read
	int32_t read_average(uint8_t times = 10);


	// puts the chip into power down mode
	void power_down();

	// wakes up the chip after power down mode
	void power_up();
};

#endif //XHX711_H_2107_09_19_12_09_00