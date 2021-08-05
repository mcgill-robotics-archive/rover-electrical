#include <IfxMotorControlShield.h>

/*
 * ----------[PINOUT]----------
 * (Also see documentation: 
 * https://github.com/mcgill-robotics/rover-pcb/blob/science-dev-external-regulator/Projects/science_external_regulator.pdf)
 * 
 * ----[Fault Detection/Shutdown Lines]----
 * D51 - POWER_ON: Set high to enable the relay, allowing the rest of the board to get power.
 * 
 * D38 - notFAULT1: High when Stepper #1 is not in a fault condition
 * D36 - notFAULT2: High when Stepper #2 is not in a fault condition
 * D44 - STEPPER1_notENABLE: Stepper #1 is enabled when this pin is low.
 * D50 - STEPPER2_notENABLE: Stepper #2 is enabled when this pin is low.
 * 
 * ----[Stepper Control Lines]----
 * D42 - STEPPER1_STEP: Stepper #1 will advance one step on the rising edge of this pin.
 * D40 - STEPPER1_DIRECTION: Stepper #1 will change direction on the rising edge of this pin.
 * D48 - STEPPER2_STEP: Stepper #2 will advance one step on the rising edge of this pin.
 * D46 - STEPPER2_DIRECTION: Stepper #2 will change direction on the rising edge of this pin.
 * 
 * ----[Limit Switches]----
 * D34 - LIMIT_SWITCH_1
 * D52 - LIMIT_SWITCH_2
 * 
 * ----[DC Motor Control Lines]----
 * Will use this library: https://github.com/Infineon/DC-Motor-Control-BTN8982TA
 * 
 * D3 - IN_1: (Input Bridge 1)
 * D11 - IN_2: (Input Bridge 2)
 * D12 - INH_1: (Inhibit Bridge 1)
 * D13 - INH_2: (Inhibit Bridge 2)
 * A0 - AD_1
 * A1 - AD_2
 * 
 * ----[Laser]----
 * D8 - LASER1_CONTROL: Laser is on when this pin is high
 * 
 * ----[LED's]----
 * D4 - LED_CONTROL: LED's are on when this pin is high
 * 
 * ----[Solenoid]----
 * D22 - SOLENOID_ON: Solenoid is on when this pin is high
 * 
 * ----[CCD Sensor]----
 * TODO: Figure out how the sensor works
 * A3 - OutputSignal_out: “out” as in “out to Arduino”.
 * D2 - phiM(CCD_Clock): Clock output to CCD sensor.
 * D20 - SH: “Shift Gate” (???)
 * D21 - ICG: “Integration Clear Gate” (???)
 * 
 * ----[Peltier cooler]----
 * D14 - PELTIER_ON
 * 
 * ----[COMMUNICATION LINES]----
 * 
 * ----[USB Debugging]----
 * D0, D1 (already assigned by default)
 * 
 * ----[Serial 1]----
 * D18 - UART TX
 * D19 - UART RX
 * 
 * ----[ADC]----
 * Analog pins can do ADC by default.
 * 
 */

#include <Wire.h>
#include <SPI.h>
#include <IfxMotorControlShield.h>
#include "SerialInterface.h"

const int PROGMEM POWER_ON = 51;
const int PROGMEM LIMIT_SWITCH_1 = 34;
const int PROGMEM LIMIT_SWITCH_2 = 52;
const int PROGMEM notFAULT1 = 38;
const int PROGMEM notFAULT2 = 36;
const int PROGMEM STEPPER1_notENABLE = 44;
const int PROGMEM STEPPER2_notENABLE = 50;
const int PROGMEM STEPPER1_STEP = 42;
const int PROGMEM STEPPER1_DIRECTION = 40;
const int PROGMEM STEPPER2_STEP = 48;
const int PROGMEM STEPPER2_DIRECTION = 46;
const int PROGMEM IN_1 = 3;
const int PROGMEM IN_2 = 11;
const int PROGMEM INH_1 = 12;
const int PROGMEM INH_2 = 13;
const int PROGMEM LASER1_CONTROL = 8;
const int PROGMEM CCD_Clock = 2;
const int PROGMEM SH = 20;
const int PROGMEM ICG_PIN = 21;
const int PROGMEM SOLENOID_ON = 22;
const int PROGMEM LED_CONTROL = 4;
const int PROGMEM PELTIER_ON = 14;

#define AD_1 A0
#define AD_2 A1

// CCD sensor defines
#define OutputSignal_out A3
const int PROGMEM CCD_CLOCK_PERIOD = 50; // hundredth of usecs (1e-8 secs)
const int PROGMEM CCD_CLOCK_DUTY_CYCLE = 5; // 100 msecs in hundredth of usecs (1e-8 secs)
const int PROGMEM SH_PERIOD = 100; // some integer multiple of the clock period
const int PROGMEM SH_DUTY_CYCLE = 50;

const int PROGMEM ICG_PERIOD = 200;
const int PROGMEM ICG_DUTY_CYCLE = 50;

#define MCLK 0x10
#define ICG 0x01
#define SH  0x02
#define PIXELS 3691
#define MIN_SIGNAL 10

//16-bit buffer for pixels
uint16_t pixelBuffer[PIXELS];
uint16_t avg = 0;

char cmdBuffer[16];
int exposureTime = 20; 
int cmdIndex; 
int cmdR = 0;

// Failure conditions, in case they need to be communicated to the central computer
volatile bool STEPPER1_FAULT = false;
volatile bool STEPPER2_FAULT = false;
volatile bool one_ms_elapsed = false;

// Brushless DC motor constants
bool MOTOR_INIT_FAILED = false;
bool MOTOR_ON = false;
int speedvalue = 5;
int acceleration = 5;

// UART stuff
const int PROGMEM UART_BAUD_RATE = 9600;
const byte PROGMEM SCIENCE_BOARD_SYSTEM_ID = 3;
const uint64_t PROGMEM TIMEOUT = 1000; // ms
SerialInterface* UART = new SerialInterface(UART_BAUD_RATE, '5', TIMEOUT);

void setup() {
  // TODO: Look into CCD pins and figure out whether they are outputs or inputs
  pinMode(POWER_ON, OUTPUT);
  pinMode(SOLENOID_ON, OUTPUT);
  pinMode(LED_CONTROL, OUTPUT);
  pinMode(STEPPER1_notENABLE, OUTPUT);
  pinMode(STEPPER2_notENABLE, OUTPUT);
  pinMode(STEPPER1_STEP, OUTPUT);
  pinMode(STEPPER1_DIRECTION, OUTPUT);
  pinMode(STEPPER2_STEP, OUTPUT);
  pinMode(STEPPER2_DIRECTION, OUTPUT);
  pinMode(LASER1_CONTROL, OUTPUT);
  pinMode(OutputSignal_out, INPUT);
  pinMode(AD_1, INPUT);
  pinMode(AD_2, INPUT);
  pinMode(IN_1, OUTPUT);
  pinMode(IN_2, OUTPUT);
  pinMode(INH_1, OUTPUT);
  pinMode(INH_2, OUTPUT);
  pinMode(CCD_Clock, OUTPUT);
  pinMode(SH, OUTPUT);
  pinMode(ICG_PIN, OUTPUT);
  pinMode(LIMIT_SWITCH_1, INPUT);
  pinMode(LIMIT_SWITCH_2, INPUT);
  pinMode(PELTIER_ON, OUTPUT);

  randomSeed(analogRead(0));

  // Timer 4 setup for regular polling
  cli(); // Stop interrupts
  
  // Clear timer registers
  TCCR4A = 0;
  TCCR4B = 0;
  // Initialize counter value to 0
  TCNT4 = 0;
  // Set compare match register for 1 kHz increments
  OCR4A = 15999; // (16*10^6) / (1000*1) - 1
  // turn on CTC mode
  TCCR4B |= (1 << WGM12);
  // Set CS40 bit high for no prescaling
  TCCR4B |= (1 << CS40);
  // enable timer compare interrupt
  TIMSK4 |= (1 << OCIE4A);

  sei(); // Start interrupts

  // Start serial communications  
  UART->begin();

  // Initial conditions for active low pin
  digitalWrite(STEPPER1_notENABLE, HIGH);
  digitalWrite(STEPPER2_notENABLE, HIGH);

  // Update(18/03/2021): We now have an onboard relay
  digitalWrite(POWER_ON, HIGH);

  while(!(notFAULT1 and notFAULT2)){}           // Wait until both stepper IC's start up properly

  //Attach interrupts to their ISR's (Interrupt Service Routines):
  attachInterrupt(digitalPinToInterrupt(notFAULT1), stepperonefault_ISR, LOW);
  attachInterrupt(digitalPinToInterrupt(notFAULT2), steppertwofault_ISR, LOW);
  //(ISR's are at the bottom)

  // Initialize brushless DC motor controller.
  // Function returns true on failure, wtf?
  if(ifxMcsBiDirectionalMotor.begin()){
    MOTOR_INIT_FAILED = true;
  }

  // Turn on LED
  LEDstate(true);
  
  // CCD sensor setup ________________________________________________

  //Initialize the clocks
  DDRD |= (SH | ICG); //Set ICG and SH lines to outputs for port D
  DDRE |= (MCLK); //Set Master Clock line to output for port E
  PORTD |= ICG; //set ICG line high

  //TODO: Set up timer to generate frquency on MCLK pin D2
  //No clock prescaling, clear timer on compare mode
  TCCR2A = (0 << COM2A1) | (1 << COM2A0) | (1 << WGM21) | (0 << WGM20);
  TCCR2B = (0 << WGM22) |(1 << CS20); //no prescaling on the clock
  OCR2A = 3; //used to calculate frequency of output through MCLK pin (don't know what fclk is)
  TCNT2 = 0; //Reset timer2
  
  //Set ADC clock rate to sysclk/32
  ADCSRA &= ~((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0);
  
  // TODO: Tell the power board and the main computer that the science board is fully booted
  // and can receive commands
}

void loop() {
  // put your main code here, to run repeatedly:

  // Check for fault conditions:
    
  if(STEPPER1_FAULT){
    // Tell the main computer that something went wrong with stepper 1. See science board serial communication doc:
    const byte stepper1fault = 4;
    char* payload;
    memcpy(payload, &stepper1fault, 1);
    UART->send_message(2, payload);

  // Hang until power is cycled
  while(1){
    // Or maybe until the main computer tells the board to keep going?
    }
    
  }
  
  if(STEPPER2_FAULT){
    // Tell the main computer that something went wrong with stepper 1. See science board serial communication doc:
    const byte stepper2fault = 2;
    char* payload;
    memcpy(payload, &stepper2fault, 1);
    UART->send_message(2, payload);

  // Hang until power is cycled
  while(1){
    // Or maybe until the main computer tells the board to keep going?
    }
    
  }

  if(one_ms_elapsed){
    one_ms_elapsed = false;
    getStatesAndSend();
  }

  // Closed loop motor control
  if(MOTOR_ON){
    
    if(ifxMcsBiDirectionalMotor.getCurrentSense() < IFX_MCS_CRITICALCURRENTSENSE)
   {

      speedvalue += acceleration;
      if(speedvalue > 255 && acceleration > 0)
      {
        acceleration = -acceleration;
        speedvalue = 255;
      }
      if(speedvalue < -255 && acceleration < 0)
      {
        acceleration = -acceleration;
        speedvalue = -255;
      }
      
      ifxMcsBiDirectionalMotor.setBiDirectionalSpeed(speedvalue);
      
    //if speed was set to 0, the motor has to be restarted
      if(!ifxMcsBiDirectionalMotor.getRunning())
        ifxMcsBiDirectionalMotor.start();
   }
   else
   {
    // Something went wrong, stop the motor
     stopMotor();
   }
  }
  
  // Receive and parse commands from main computer
  UART->update();
  Message next_command = UART->get_next_message();
  parseUART(next_command);
}

// ___[SERIAL COMMS]_________________________________________________

void parseUART(Message message){
  
  // If a message that isn't supposed to be for the science board is received, ignore it.
  if(message.systemID != SCIENCE_BOARD_SYSTEM_ID){
    return;
  }
  
  int frame_id = message.frameID - '0';
  String argument = message.data;

  byte boolsteps, mask;
  char* CCDReadout;
  float motor_speed, angle;

  int stepper = argument.charAt(0) - '0';
  
  switch(frame_id)
  {
    case 1:
      motor_speed = message.data.toFloat();
    
      if(motor_speed == 0){
        stopMotor();
      }else if (!MOTOR_ON){
        startMotorWithSpeed(motor_speed);
      }else{
        ifxMcsBiDirectionalMotor.setBiDirectionalSpeed(speedvalue);
      }
      break;

    case 2:
      memcpy(&boolsteps, argument.c_str(), 1);
      
      mask = 8; // The first three bits are ignored. See science board serial communication doc:
      if(boolsteps & mask) shdn();

      mask <<= 1;
      peltierState(boolsteps & mask);
      mask <<= 1;
      solenoidState(boolsteps & mask);
      mask <<= 1;
      laserState(boolsteps & mask);
      mask <<= 1;
      LEDstate(boolsteps & mask);

      break;

    case 3:
      // There are two stepper motors in the science module.
      // If the stepper choice is something else than 0 or 1, message is simply ignored.
      
      if(!(stepper == 0 || stepper == 1)){
        break;
      }

      angle = argument.substring(2).toFloat();

      // If the angle is less than the minimum precision, clamp it to the min. precision:
      
      if(abs(angle) < 0.1125){
        if(angle < 0){
          angle = -0.1125;
        }else{
          angle = 0.1125;
        }
      }

      stepper_choice(stepper, angle);
      break;

    case 4:
      
      if(stepper == 0){
        stepper1_agitate();
      }else if(stepper == 1){
        stepper2_agitate();
      }

      break;

    case 5:
      CCDread();

      // Since the pixel buffer is an int16 array, the PC
      // knows how to interpret the received string char by char.
      // All the Arduino needs to do is to memcpy the array
      // onto a string.
      
      memcpy(CCDReadout, &pixelBuffer, PIXELS * 2);
      UART ->send_message(5, CCDReadout);
      
      break;

    default:
      // If some other frame type is received, the board ignores the command.
      break;
  }
}

// ___[BRUSHLESS DC MOTOR]___________________________________________
void startMotorWithSpeed(int speedvalue){
  if(!MOTOR_INIT_FAILED){
    ifxMcsBiDirectionalMotor.setBiDirectionalSpeed(speedvalue);
    ifxMcsBiDirectionalMotor.start();
    MOTOR_ON = true;
  }
}

void stopMotor(){
  ifxMcsBiDirectionalMotor.stop();
  speedvalue = 0;
  MOTOR_ON = false;
}

// ___[LED]____________________________________________________________

void LEDstate(bool state){
  if(state){
    digitalWrite(LED_CONTROL, HIGH);
  }else{
    digitalWrite(LED_CONTROL, LOW);
  }
}

// ___[PELTIER COOLER]_________________________________________________

void peltierState(bool state){
  if(state){
    digitalWrite(PELTIER_ON, HIGH);
  }else{
    digitalWrite(PELTIER_ON, LOW);
  }
}

// ___[SOLENOID]_______________________________________________________

void solenoidState(bool state){
  if(state){
    digitalWrite(SOLENOID_ON, HIGH);
  }else{
    digitalWrite(SOLENOID_ON, LOW);
  }
}

// ___[LASER]__________________________________________________________

void laserState(bool state){
  if(state){
    digitalWrite(LASER1_CONTROL, HIGH);
  }else{
    digitalWrite(LASER1_CONTROL, LOW);
  }
}

// ___[STEPPERS]_______________________________________________________

const unsigned int PROGMEM MINIMUM_STEP_PULSE = 3; // Microseconds
const int PROGMEM MINIMUM_DISABLE_TIME = 1;        // Milliseconds
const int PROGMEM MINIMUM_ENABLE_TIME = 1;         // Milliseconds
const double PROGMEM ANGLE_PER_STEP = 0.1125;
const int PROGMEM WIGGLE_STEPS = 100;
const int PROGMEM MAX_WIGGLE_DEVIATION = 16;       // (16 * 0.1125 = 1.8 deg)

// 0 for forward, 1 for backward
bool STEPPER1_FORWARD = true;
bool STEPPER2_FORWARD = true;
double STEPPER1_CURRENT_ANGLE = 45;
double STEPPER2_CURRENT_ANGLE = 45;

void stepper1_agitate(){
  
  for(int i = 0; i < WIGGLE_STEPS; i++){
    
    int steps = random(1, MAX_WIGGLE_DEVIATION);
    
    // Move forward
    for(int j = 0; j < steps; j++){
      stepper1_step();
    }
    
    stepper1_changedirection();
    
    // Move 2x backwards, ending up at the same deviation but
    // at the opposite end
    for(int j = 0; j < (2*steps); j++){
      stepper1_step();
    }

    stepper1_changedirection();
    
    // Move forward again, ending up where we started
    for(int j = 0; j < steps; j++){
      stepper1_step();
    }
    
  }
  
}

void stepper2_agitate(){

  for(int i = 0; i < WIGGLE_STEPS; i++){
    
    int steps = random(1, MAX_WIGGLE_DEVIATION);
    
    // Move forward
    for(int j = 0; j < steps; j++){
      stepper2_step();
    }
    
    stepper2_changedirection();
    
    // Move 2x backwards, ending up at the same deviation but
    // at the opposite end
    for(int j = 0; j < (2*steps); j++){
      stepper2_step();
    }

    stepper1_changedirection();
    
    // Move forward again, ending up where we started
    for(int j = 0; j < steps; j++){
      stepper2_step();
    }
    
  }
  
}

void stepper1_goto_angle(float angle){
  double diff = (angle - STEPPER1_CURRENT_ANGLE);
  stepper1_increment_angle(diff);
}

void stepper2_goto_angle(float angle){
  double diff = (angle - STEPPER1_CURRENT_ANGLE);
  stepper2_increment_angle(diff);
}

void stepper1_increment_angle(float angle){
  int n = round(angle/ANGLE_PER_STEP);
  
  if(n == 0) return;
  if(n < 0 && STEPPER1_FORWARD){
    stepper1_changedirection();
  }
  if(n > 0 && !STEPPER1_FORWARD){
    stepper1_changedirection();
  }
  
  for(int i = 0; i < n; i++){
    stepper1_step();
  }
}

void stepper2_increment_angle(float angle){
  int n = round(angle/ANGLE_PER_STEP);
  
  if(n == 0) return;
  if(n < 0 && STEPPER2_FORWARD){
    stepper2_changedirection();
  }
  if(n > 0 && !STEPPER2_FORWARD){
    stepper2_changedirection();
  }
  
  for(int i = 0; i < n; i++){
    stepper2_step();
  }
}

void stepper_choice(int stepper, float angle){
  if(stepper == 0){
    stepper1_increment_angle(angle);
  }else{
    stepper2_increment_angle(angle);
  }
}

void stepper1_step(){
  
  // Enable this stepper if it's not enabled and give it time to turn on
  if(digitalRead(STEPPER1_notENABLE) == HIGH){
    digitalWrite(STEPPER1_notENABLE, LOW);
    delay(MINIMUM_ENABLE_TIME);
  }

  // Pulse this stepper
  
  digitalWrite(STEPPER1_STEP, HIGH);
  delayMicroseconds(MINIMUM_STEP_PULSE);
  digitalWrite(STEPPER1_STEP, LOW);
  

  // Internally keep track of the current angle,
  // after everything else is done

  if(STEPPER1_FORWARD){
    
    STEPPER1_CURRENT_ANGLE += ANGLE_PER_STEP;
    if(STEPPER1_CURRENT_ANGLE >= 360){
      STEPPER1_CURRENT_ANGLE -= 360;
    }
    
  }else{
    
    STEPPER1_CURRENT_ANGLE -= ANGLE_PER_STEP;
    if(STEPPER1_CURRENT_ANGLE < 0){
      STEPPER1_CURRENT_ANGLE += 360;
    }
  }
  
}

void stepper2_step(){
  
  // Enable this stepper if it's not enabled and give it time to turn on
  if(digitalRead(STEPPER2_notENABLE) == HIGH){
    digitalWrite(STEPPER2_notENABLE, LOW);
    delay(MINIMUM_ENABLE_TIME);
  }

  // Pulse this stepper
  
  digitalWrite(STEPPER2_STEP, HIGH);
  delayMicroseconds(MINIMUM_STEP_PULSE);
  digitalWrite(STEPPER2_STEP, LOW);
  

  // Internally keep track of the current angle,
  // after everything else is done

  if(STEPPER2_FORWARD){
    
    STEPPER2_CURRENT_ANGLE += ANGLE_PER_STEP;
    if(STEPPER2_CURRENT_ANGLE >= 360){
      STEPPER2_CURRENT_ANGLE -= 360;
    }
    
  }else{
    
    STEPPER2_CURRENT_ANGLE -= ANGLE_PER_STEP;
    if(STEPPER2_CURRENT_ANGLE < 0){
      STEPPER2_CURRENT_ANGLE += 360;
    }
  }
  
}

void stepper1_changedirection(){
  
  digitalWrite(STEPPER1_DIRECTION, HIGH);
  delayMicroseconds(3);
  digitalWrite(STEPPER1_DIRECTION, LOW);
  
  STEPPER1_FORWARD = !STEPPER1_FORWARD;
}

void stepper2_changedirection(){
  
  digitalWrite(STEPPER2_DIRECTION, HIGH);
  delayMicroseconds(3);
  digitalWrite(STEPPER2_DIRECTION, LOW);
  
  STEPPER2_FORWARD = !STEPPER2_FORWARD;
}

//____[CCD SENSOR]______________________________________________________________________

void CCDread(){
  
//   PORTD &= ~ICG;      // set ICG line low
//   PORTD |= SH         // turn SH line high
//   TCNT2 = 0;          // On the rising edge of the CCD clock;
  
//   _delay_loop_1(8);   // delay 500ns on ATMega 2560
//   PORTD |= SH         // turn SH line high after delay
//   _delay_loop_1(16);  // delay 1000 ns on ATMega 2560
//   PORTD &= ~SH        // turn SH line low
//   _delay_loop_1(16);  // delay 1000 ns on ATMega 2560

//   TCNT2 = 0;                  // On the rising edge of the CCD clock;
//   PORTD |= ICG                // ICG signal goes high.
//   CCDSensorReadDataStream();  // Start listening to A3, keep listening for (INTEGRATION_TIME - 1500 ns)

  // On the rising edge of the CCD clock;
  PORTD &= ~ICG;       //set ICG line low
  _delay_loop_1(8);    // delay 500ns on ATMega 2560
  PORTD |= SH;         //turn SH line high after delay
  _delay_loop_1(16);   // delay 1000 ns on ATMega 2560
  PORTD &= ~SH;        //turn SH line low
  _delay_loop_1(16);   // delay 1000 ns on ATMega 2560
  PORTD |= ICG;        //set ICG line high

  // If the CCD sensor doesn't work, the timing is the first thing to look at

  int x; 
  uint16_t result;
  
  for (x = 0; x < PIXELS; x++) {
        PORTD |= SH;
        if (x == 0) {
            result = (uint16_t)(1023 - analogRead(A3));
            if (result > MIN_SIGNAL) {
                avg = result - MIN_SIGNAL;
            } else {
                avg = result;
            }
        } else {
            result = (uint16_t)(1023 - analogRead(A3));
        }
        if (result < avg) {
            result = 0;
        } else {
            result -= avg;
        }
        pixelBuffer[x] = result;
        delayMicroseconds(21);
    }
    PORTD &= ~SH; 
}

// ___[STATE GETTER]____________________________________________________________________
void getStatesAndSend(){

  // Encode the boolean states of the board to a single byte. The lower four bytes are not
  // used in regular polling because they either signal fault conditions or a shutdown command.
  
  byte boolstates = 0;
  byte roll = 3;

  bool peltier_state = ((PORTJ >> 1) & 0x01); // Digital Pin 14
  bool solenoid_state = (PORTA & 0x01);       // Digital Pin 22
  bool laser_state = ((PORTH >> 5) & 0x01);   // Digital Pin 8
  bool led_state = ((PORTG >> 5) & 0x01);     // Digital Pin 4

  boolstates |= (peltier_state << roll);
  roll++;

  boolstates |= (solenoid_state << roll);
  roll++;

  boolstates |= (laser_state << roll);
  roll++;

  boolstates |= (led_state << roll);

  // When the rover becomes autonomous, this is where the limit switches and other sensors will be polled.

  const byte conditions = boolstates;
  char* payload;
  memcpy(payload, &conditions, 1);
  UART->send_message(2, payload);
}

// ___[NORMAL SHUTDOWN]_________________________________________________________________

void shdn(){
  noInterrupts();
  
  stopMotor();
  laserState(false);
  // Turn off steppers
  digitalWrite(44, HIGH);
  digitalWrite(50, HIGH);
  peltierState(false);
  solenoidState(false);
  
  delay(1000);
  LEDstate(false);

  // Turn off relay
  digitalWrite(51, LOW);
  
  // Do nothing until power is lost
  while(1){}
  
}

ISR(TIMER4_COMPA_vect){
  one_ms_elapsed = true;
}
  
// ___[INTERRUPT SHUTDOWN]______________________________________________________________

void stepperonefault_ISR(){
  // Direct port manipulation to shut down everything quickly, starting with relay
  // which is on Port B. Does the same thing as the commented out portion below, but
  // more manually and turns off every pin (except active low ones)
  
  PORTB = B00001000; // PB3 -> high, PB2-PB7 low
  PORTA = B00000000; // PA0 -> low
  PORTD = B00000000; // PD1 -> low
  PORTE = B00000000; // PE4,PE5 -> low
  PORTG = B00000000; // PG1 -> low
  PORTH = B00000000; // PH5 -> low
  PORTJ = B00000000; // PJ1 -> low
  PORTL = B00100000; // PL3, PL7 -> low, PL5 -> high

  STEPPER1_FAULT = true;
}

void steppertwofault_ISR(){
  
  // Direct port manipulation to shut down everything quickly, starting with relay
  // which is on Port B. Turns off every pin except active low ones
  
  PORTB = B00001000; // PB3 -> high, PB2-PB7 low
  PORTA = B00000000; // PA0 -> low
  PORTD = B00000000; // PD1 -> low
  PORTE = B00000000; // PE4,PE5 -> low
  PORTG = B00000000; // PG1 -> low
  PORTH = B00000000; // PH5 -> low
  PORTJ = B00000000; // PJ1 -> low
  PORTL = B00100000; // PL3, PL7 -> low, PL5 -> high

  STEPPER2_FAULT = true;
}
