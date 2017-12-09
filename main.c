#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define DDR_MASK 0b00000001 // PB0
#define PORT_MASK 0b00000000

#define SERVO_PIN DDR_MASK

#define COUNTER_MIN (F_CPU * 11 / 10 / 1000 / 256) // min point: 1.1ms
#define COUNTER_MAX (F_CPU * 19 / 10 / 1000 / 256) // max point: 1.9ms
#define COUNTER_RESET (F_CPU * 20 / 1000 / 256) // loop point: 20ms

volatile uint16_t timeCounter;
volatile uint16_t position;

int main(void)
{
	cli();

	DDRB = DDR_MASK;
	PORTB = PORT_MASK;
	
	timeCounter = 0;

	// init timer
	// prescale timer to 1/1st the clock rate
	TCCR0B |= (1<<CS00);
	// enable timer overflow interrupt
	TIMSK0 |= (1<<TOIE0);
	
	// init ADC
	ADMUX |= /*(1 << MUX1) |*/ (1 << MUX0); // MUX[1:0] = 01 //  ADC1 (PB2)
	ADCSRB |= (1 << ADATE); // Free running mode
	ADCSRA |= (1 << ADEN) | (1 << ADSC); // Start ADC

	position = COUNTER_MIN + COUNTER_MAX >> 1;
	
	sei();
	
	while(1)
	{
		uint32_t len = COUNTER_MAX - COUNTER_MIN;
		position = (uint16_t) (len * ADC / 1024) + COUNTER_MIN;
		ADCSRA |= (1 << ADSC); // test
	}
}

// Timer interrupt
ISR(TIM0_OVF_vect)
{
	timeCounter ++;

	if (timeCounter > COUNTER_RESET) {
		timeCounter = 0;
	}
	if (timeCounter < position) {
		PORTB = SERVO_PIN;
	} else {
		PORTB = ~SERVO_PIN;
	}

}

