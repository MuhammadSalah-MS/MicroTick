/*
 * 	Project : MicroTick - Digtal Clock
 *
 *	Author : Muhammad Salah Ahmed
 *
 */

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>

//MODES
#define STOPWATCH_MODE 0
#define CLOCK_MODE 1
#define STOP_COUNT 2
#define RESUME_COUNT 3
#define ADJUST_MODE 4
char state; // holds the current mode
// Clock time adjusting enable
#define ADJUST_ENABLE 0
#define ADJUST_SAVE 1
// Seven Segments Enable Pins
#define SC1 0
#define SC2 1
#define MN1 2
#define MN2 3
#define HR1 4
#define HR2 5
// Stop Watch Control Buttons Pins
#define resetButton 2
#define stopButton 3
#define resumeButton 0
// Clock Control Buttons Pins
#define switchButton 4
#define secondsButton 5
#define minutesButton 6
#define hoursButton 7
// Clock Adjusting Buttons Pins
#define adjustButton 0
#define sec_adjButton 5
#define min_adjButton 6
#define hour_adjButton 7
// Leds & Buzzers Pins
#define stopwatchLed 6
#define clockLed 7
#define stopwatchBuzzer 6
#define clockBuzzer 7
// Time variables for the Stop Watch
char seconds=0,minutes=0,hours=0;
// Time variables for the Clock
char c_seconds=12,c_minutes=12,c_hours=12;
// Time Temp Variables
char temp_sec=0,temp_min=0,temp_hour=0;
char adj_sec=0,adj_min=0,adj_hour=0;
char stop_flag =0;

// Interrupts (INT0,1,2) Enable Function
void interrupts_enable()
{
	// Enable Interrupt 0
	MCUCR|=(1<<ISC01);
	GICR|=(1<<INT0);
	// Enable Interrupt 1
	MCUCR|=(1<<ISC11)|(1<<ISC10);
	GICR|=(1<<INT1);
	// Enable Interrupt 2
	MCUCSR&=~(1<<ISC2);
	GICR|=(1<<INT2);
	// Enable Golbal Interrupt
	SREG|=(1<<7);
}
//  Timer 1 Enabling Function
void timer1_enable()
{
	// Set the timer mode to the compare mode
	// Adjust the counter to make interrupt each one second
	OCR1A=977;
	// Initialize the counter
	TCNT1 = 0;
	// Adjust the clock with 1024 pre-scaler
	TCCR1B=(1<<WGM12)|(1<<CS10)|(1<<CS12);

	// enable the timer1 interrupt
	TIMSK |= (1<<OCIE1A);

}
// Disable timer 1 ( Stop the Clock Source )
void timer1_disable()
{
	TCCR1B=0;
}
// Interrupt Service Routine for timer 1 to count the seconds
ISR(TIMER1_COMPA_vect)
{
	// Increase number of seconds each seconds
	seconds++;
	c_seconds++;
}
// the reset button ISR returns the time to start point (00:00:00)
ISR(INT0_vect)
{
	seconds=0;
	minutes=0;
	hours=0;
}
// the stop button ISR disables the timer and stops counting
ISR(INT1_vect)
{
	//timer1_disable();
	state=STOP_COUNT;
	temp_sec=seconds,temp_min=minutes,temp_hour=hours;
	PORTB|=(1<<5);
	stop_flag=1;

}
// the resume button ISR enable the timer again and resumes counting
ISR(INT2_vect)
{
	//timer1_enable();
	if (stop_flag)
	{
		seconds=temp_sec,minutes=temp_min,hours=temp_hour;
		state=STOPWATCH_MODE;
		PORTB&=~(1<<5);
		stop_flag=0;
	}

}

// Seconds Updating function, update and displays the seconds on the seconds 7-segments
void seconds_update(char *sec1,char *sec2)
{
	// enable the seconds segments , update their numbers
	// disable previous
	PORTA&=~(1<<SC2);
	PORTA&=~(1<<MN1);
	PORTA&=~(1<<MN2);
	PORTA&=~(1<<HR1);
	PORTA&=~(1<<HR2);
	// enable seg
	PORTA|=(1<<SC1);
	// display
	PORTC=(0x0f)&(*sec1);
	_delay_ms(5);
	// disable seg and enable seg2
	PORTA&=~(1<<SC1);
	PORTA|=(1<<SC2);
	// display
	PORTC=(0x0f)&(*sec2);
	_delay_ms(5);
	// disable seg2
	PORTA&=~(1<<SC2);

}
// Minutes Updating function, update and displays the seconds on the minutes 7-segments
void minutes_update(char *min1,char *min2)
{
	// enable the seconds segments , update their numbers
	// disable seconds segments
	PORTA&=~(1<<SC2);
	PORTA&=~(1<<SC1);
	PORTA&=~(1<<MN2);
	PORTA&=~(1<<HR1);
	PORTA&=~(1<<HR2);

	// enable seg
	PORTA|=(1<<MN1);
	// display
	PORTC=(0x0f)&(*min1);
	_delay_ms(5);
	// disable seg and enable seg2
	PORTA&=~(1<<MN1);
	PORTA|=(1<<MN2);
	// display
	PORTC=(0x0f)&(*min2);
	_delay_ms(5);
	// disable seg2
	PORTA&=~(1<<MN2);
}
// hours Updating function, update and displays the seconds on the hours 7-segments
void hours_update(char *hr1,char *hr2)
{
	// enable the seconds segments , update their numbers
	// disable seconds segments
	PORTA&=~(1<<SC2);
	PORTA&=~(1<<SC1);
	PORTA&=~(1<<MN1);
	PORTA&=~(1<<MN2);
	PORTA&=~(1<<HR2);

	// enable seg
	PORTA|=(1<<HR1);
	// display
	PORTC=(0x0f)&(*hr1);
	_delay_ms(5);
	// disable seg and enable seg2
	PORTA&=~(1<<HR1);
	PORTA|=(1<<HR2);
	// display
	PORTC=(0x0f)&(*hr2);
	_delay_ms(5);
	// disable seg2
	PORTA&=~(1<<HR2);

}

// stopwatch times adjust function
// increases the minutes and hours every 60 seconds and minutes
void swTime_adjust()
{
	if (seconds>=60)
	{
		seconds=0;
		minutes++;
	}
	if (minutes>=60)
	{
		minutes=0;
		seconds=0;
		hours++;
	}
	if (hours>=24)
	{
		hours=0;
		seconds=0;
		minutes=0;
	}

}
//clock time adjust function
// increases the minutes and hours every 60 seconds and minutes
void clockTime_adjust()
{
	if (c_seconds>=60)
	{
		c_seconds=0;
		c_minutes++;
	}
	if (c_minutes>=60)
	{
		c_minutes=0;
		c_seconds=0;
		c_hours++;
	}
	if (c_hours>=24)
	{
		c_hours=0;
		c_seconds=0;
		c_minutes=0;
	}


}


void seg_display(char s,char m,char h)
{
	char sec1=s%10;
	char sec2=s/10;
	seconds_update(&sec1,&sec2);
	char min1=m%10;
	char min2=m/10;
	minutes_update(&min1,&min2);
	char hr1=h%10;
	char hr2=h/10;
	hours_update(&hr1,&hr2);
}
// save the current units entered from user
void save_time()
{
	state=CLOCK_MODE;
	c_seconds=adj_sec;
	c_minutes=adj_min;
	c_hours=adj_hour;
}
// adjust time
void adjust_time()
{
	seg_display(adj_sec,adj_min,adj_hour);
	// static flags to save the button states
	static char flag1=1,flag2=1,flag3=1;

	char sec_read=PIND&(1<<sec_adjButton);

	if (!sec_read)
	{
		if (flag1&&!sec_read)
		{
			adj_sec++;
			flag1=0;
		}
	}
	else flag1=1;


	char min_read=PIND&(1<<min_adjButton);

	if (!min_read)
	{
		if (flag2&&!min_read)
		{
			adj_min++;
			flag2=0;
		}
	}
	else flag2=1;


	char hour_read=PIND&(1<<hour_adjButton);

	if (!hour_read)
	{
		if (flag3&&!hour_read)
		{
			adj_hour++;
			flag3=0;
		}
	}
	else flag3=1;


	if (adj_sec==60) adj_sec=0;
	if (adj_min==60) adj_min=0;
	if (adj_hour==60) adj_hour=0;

	seg_display(adj_sec,adj_min,adj_hour);

}

// Display function is the main function adjusting the time and displays it
void display(char state)
{
	if (state==STOP_COUNT)
	{
		seg_display(temp_sec,temp_min,temp_hour);
	}

	if (state==ADJUST_MODE)
	{
		adjust_time();
	}

	if (state==STOPWATCH_MODE)
	{
		swTime_adjust();
		seg_display(seconds,minutes,hours);

	}
	else if (state==CLOCK_MODE)
	{
		clockTime_adjust();
		seg_display(c_seconds,c_minutes,c_hours);

	}

}

// 7-Segments And Decoder PINS Initializations
void segments_enable()
{
	/*** Seven Segment Pins ***/
	// set the Seven segments ENABLE pins as output
	DDRA|=0x3F;
	// set the DECODER pins as output
	DDRC|=0x0F;


}
void swControl_enable()
{
	/*** Push Buttons Inputs ***/
	// set the buttons as input
	DDRD&=~(1<<resetButton);
	DDRD&=~(1<<stopButton);
	DDRB&=~(1<<resumeButton);
	// enable the internal pull up resistors
	PORTD|=(1<<resetButton);
	PORTD|=(1<<resumeButton);

}
void clockControl_enable()
{
	// setting the 4 buttons pins as input
	DDRD&=~(0xF0);
	// setting the 4 buttons pull up resistors
	PORTD|=0xF0;
}
void buzzers_enable()
{
	// setting the two buzzers pins as output
	DDRC|=(1<<clockBuzzer)|(1<<stopwatchBuzzer);
	// ensuring two pins are low (Buzzers off)
	PORTC&=~(1<<clockBuzzer)&~(1<<stopwatchBuzzer);
}

void leds_enable()
{
	DDRB|=0xC0;
	PORTB&=~0xC0;
}
void adjust_enable()
{
	DDRD&=~(1<<adjustButton);
	DDRD&=~0xE0;
}
void switching()
{

}
int main()
{
	/***** Initialization *****/
	// enable functions
	swControl_enable();		 // enabling stopwatch control buttons
	clockControl_enable(); 	 // enabling digtal clock control buttons
	segments_enable();		 // enabling the seven segments& decoder
	interrupts_enable();	 // enabling interrupts
	timer1_enable(); 		 // enabling timer1
	leds_enable();       	 // enabling leds
	adjust_enable();		 // enable adjust buttons

	char switching_read;
	char adjust_read;
	char flag=1;
	char flag2=1;
	char adjust_state=ADJUST_ENABLE;
	state=CLOCK_MODE; // the initial mode is the clock
	PORTB|=(1<<clockLed);
	/***** Application *****/

	while (1)
	{
		switching_read=PIND&(1<<4); // read button state
		if (!switching_read)        // check if the switching button is pressed (PULL-UP)
		{
			if (flag&&!switching_read)
			{
				if (state==STOPWATCH_MODE)
				{
					state=CLOCK_MODE;
					PORTB|=(1<<clockLed);
					PORTB&=~(1<<stopwatchLed);
				}
				else {
					state = STOPWATCH_MODE;
					PORTB|=(1<<stopwatchLed);
					PORTB&=~(1<<clockLed);
				}
				flag=0;
			}
		}
		else flag=1;

		adjust_read=PIND&(1<<0); // read  button state
		if (!adjust_read)   	 // check if adjust button is pressed
		{
			if (flag2&&!adjust_read)
			{
				if (adjust_state==ADJUST_ENABLE)
				{
					state=ADJUST_MODE;
					adjust_state=ADJUST_SAVE;
					adj_sec=c_seconds,adj_min=c_minutes,adj_hour=c_hours;
				}
				else {
					save_time();
					adjust_state=ADJUST_ENABLE;
				}
				flag2=0;
			}
		}
		else flag2=1;

		display(state);

	}
}
