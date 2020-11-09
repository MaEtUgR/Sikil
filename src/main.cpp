#include <arduino.h>
#include <math.h>

//Include libraries
#include <VescUart.h>
#include "wiring_private.h"

// Pins
#define PEDAL_SWITCH_PIN 4

//Serial Ports
// Create the new UART instance assigning it to pin 0 and 1
Uart SerialVesc2 (&sercom3, 0, 1, SERCOM_RX_PAD_1, UART_TX_PAD_0);

VescUart VESCG;
VescUart VESCM;

#define SERIALVESCG SerialVesc2
#define SERIALVESCM Serial1
#define DEBUGSERIAL Serial // usb serial

// Options
//#define CYCLE_COMPENSATION 0

#define SAMPLING_FREQ_HZ 50
#define NB_SAMPLES 50

#define SET_RPM 75

#define FULL_TACHOMETER_ROT 600

#define SIN_PHASE -0.1
#define SIN_INTENS 3 // 2 is highest, infinity is lowest

#define KP_AVERAGE 0.2
#define KI_AVERAGE 0.003
#define KP_CYCLE 0.8

//variables
float current_rpm = 0;
float moving_average_rpm = 0;
float average_rpm_error = 0;
float cycle_rpm_error = 0;
float integral_average_braking_current = 0;
float braking_current = 0;
float derivative_rpm = 0;

float rpm_samples[NB_SAMPLES];

unsigned int samples_head = 0;

volatile int zero_tach;
float pedal_position;

int j = 0;

// Functions

void zero_pedal_position();


void setup() {

	DEBUGSERIAL.begin(9600);
	#ifdef DEBUG
	//SEtup debug port
	SetDebugSerialPort(&DEBUGSERIAL);
	#endif

	//Setup UART port for VESC motor
	SERIALVESCM.begin(115200);

	//Setup UART Port for VESC generator
	SERIALVESCG.begin(115200);
	pinPeripheral(0, PIO_SERCOM); //Assign RX function to pin 0
	pinPeripheral(1, PIO_SERCOM); //Assign TX function to pin 1
	
	VESCG.setSerialPort(&SERIALVESCG);
	VESCM.setSerialPort(&SERIALVESCM);

	attachInterrupt(digitalPinToInterrupt(PEDAL_SWITCH_PIN), zero_pedal_position, RISING);
}
	

void loop() {
	/** Call the function getVescValues() to acquire data from VESC */
	if ( VESCG.getVescValues() ) {

		current_rpm = VESCG.data.rpm / 111; // /7 poles * 15.9 reduction

		// Remove old value from moving average
    	moving_average_rpm = moving_average_rpm - rpm_samples[samples_head]/NB_SAMPLES;
		// Insert new sample value
		rpm_samples[samples_head] = current_rpm; 
    	// Compute new moving average
    	moving_average_rpm = moving_average_rpm + rpm_samples[samples_head]/NB_SAMPLES;
		// Compute cycle error
		cycle_rpm_error = rpm_samples[samples_head] - moving_average_rpm;
		// Compute average error
		average_rpm_error = moving_average_rpm - SET_RPM;
		
		// Move head up
    	samples_head = (samples_head + 1) % NB_SAMPLES;

		// Compute pedal position in radians
		pedal_position = (float) (VESCG.data.tachometer - zero_tach) / FULL_TACHOMETER_ROT * TWO_PI;
		
	} else
	{
    	Serial.println("Failed to get data 1!");
	}
	
	// Compute average braking current
	integral_average_braking_current += KI_AVERAGE * average_rpm_error;
	float average_braking_current = KP_AVERAGE * average_rpm_error + integral_average_braking_current;

	// Braking current can't be smaller than 0
	integral_average_braking_current = (integral_average_braking_current <= 0) ? 0 : integral_average_braking_current;

	// Mitigate cycle compensation at low rpm
	float cycle_start_comp = 2 * (0.5 - ((SET_RPM - current_rpm) / SET_RPM)); 
	cycle_start_comp = (cycle_start_comp > 1) ? 1 : cycle_start_comp;
	cycle_start_comp = (cycle_start_comp < 0) ? 0 : cycle_start_comp; 

	// Compute the sinusoidal braking
	// Math explained here: https://www.desmos.com/calculator/wa0o7zkayl
	float sinus_braking = (cos(2*(pedal_position + SIN_PHASE)) + SIN_INTENS - 1) / SIN_INTENS;

	// Compute total braking current
	braking_current = average_braking_current * sinus_braking;

	//braking_current *= sinus_braking;

	#ifdef CYCLE_COMPENSATION
		braking_current += KP_CYCLE * cycle_rpm_error * cycle_start_comp;
		#error
	#endif
	
	if (braking_current <= 0) {
		braking_current = 0;
	}
	
	// Set braking current
	VESCG.setBrakeCurrent(braking_current);
  	//VESCM.setCurrent(-VESCG.data.avgInputCurrent);


	if (j == 5) {
		Serial.print(moving_average_rpm);
		Serial.print(", ");
		Serial.print(SET_RPM);
		Serial.print(", ");
		Serial.print(cycle_rpm_error);
		Serial.print(", ");
		Serial.print(VESCG.data.rpm / 111); //7 * 15.9
		Serial.print(", ");
		Serial.print(braking_current);
		Serial.print(", ");
		Serial.print(10 * cycle_start_comp);
		Serial.print(", ");
		Serial.print(sinus_braking);
		Serial.println();

		j = 0;
	}
	
	j++;
	
	delay(5);
}

// Attach the interrupt handler to the SERCOM
void SERCOM3_Handler()
{
	SERIALVESCG.IrqHandler();
}

void zero_pedal_position(){
	if (pedal_position > PI) {
		zero_tach = VESCG.data.tachometer;
	}
};