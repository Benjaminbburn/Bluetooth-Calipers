// Adapted from: 
// 1. https://www.instructables.com/id/Interfacing-a-Digital-Micrometer-to-a-Microcontrol/
// 2. https://github.com/Roger-random/mitutoyo

#include <Arduino.h>
#include <BleKeyboard.h>  //used to send keystrokes to connected device
#include <Bounce2.h>      //used to manage button behavior

// Pins, from RIGHT to LEFT on calipers
// 1. GND - Ground
// 2. DATA - Measured data
// 3. CK - Clock
// 4. NA - No Connection
// 5. REQ - Request 

int dat = 4; //Arduino pin for Data line, connects directly to SPC port pin 2 (DATA)
int clk = 0; //Arduino pin for Clock line, connects directly to SPC port pin 3 (~CLK)
int req = 2; //Arduino pin for REQ line, drives transistor to ground SPC port pin 5 (~REQ)

int btn = 5; //esp32 pin for button

int i = 0;  //used to count digits from calipers
int j = 0;  //used to count bits in each digit
int k = 0;  //value of the bit

int sign = 0;  //default value used to track if value is positive or negative
int decimal;   //determines where decimal is placed on returned value
float dpp;      //decimal point position, number used as denominator that moves decimal to correct position
int units;     //value that determines mm's or in's

byte mydata[14];    //stores data from calipers
String value_str;   //string representation of the value
long value_int;     //was an int, could not measure over 32mm
float value;         //value as a float, used to position decimal
String units_str;   //unit, as a string
String output_val;  //output value, as a string

int alreadyReported = 0;  //tracks if a value has already been returned, prevents duplicate values when long pressing 

BleKeyboard bleKeyboard;   //creates a bleKeyboard object
Bounce bounce = Bounce();  //creates a bounce object
String get_data();         //inits the get_data function

void setup() {
  Serial.begin(115200);  //starts the serial monitor, useful for debugging
  
  pinMode(req, OUTPUT);              //sets req pin to output
  pinMode(clk, INPUT_PULLUP);        //sets clk to input_pullup
  pinMode(dat, INPUT_PULLUP);        //sets dat to input pullup
  digitalWrite(req,LOW);             // set req to low by default
  bounce.attach(btn, INPUT_PULLUP);  //sets button to input_pullup
  bounce.interval(1);                //sets the bounce interval to 1ms
  
  bleKeyboard.begin();  //init the blekeyboard
}

void loop() {
  if(bleKeyboard.isConnected()) {  //only checks button once bluetooth is connected
    
    bounce.update();  //checks button status
    int debouncedState = bounce.read();

    if (debouncedState == LOW && bounce.currentDuration() > 500 && alreadyReported == 0) {  //checks if button is pressed, has been held for 500ms, and a number has not been reported
      //Serial.println("Detected long press");  //DEBUG line
      String caliper_val = get_data();          //gets data from calipers
      bleKeyboard.print(caliper_val);           //types reported value to connected device
      bleKeyboard.write(KEY_RETURN);            //hits enter on connected device
      alreadyReported = 1;                      //ensures value is only reported once per press

    } else if (bounce.rose() && alreadyReported == 1) {   //checks if the button is released and a number is reported
      //Serial.println("Reset values after long press");  //DEBUG line
      alreadyReported = 0;                                //resets the variable that prevents multiple reports from long presses

    } else if (bounce.rose() && alreadyReported == 0) {  //checks if button is released and a number has not been reported
      //Serial.println("Detected short press");          //DEBUG line
      String caliper_val = get_data();                   //gets data from calipers
      bleKeyboard.print(caliper_val);                    //types reported values to connected device
    }
  }
}

String get_data() {
  //Serial.print("REQ value when button is pressed: "); Serial.println(digitalRead(req));  //DEBUG line
  digitalWrite(req, HIGH); //requests the current value from calipers  
  //Serial.print("REQ value after being set to high: "); Serial.println(digitalRead(req)); //DEBUG line
  
  //Following section parses the data stream reported from the calipers to generate a value. 
  //See supporting docs: https://www.instructables.com/Interfacing-a-Digital-Micrometer-to-a-Microcontrol/ 
  //Step 3: Reading the Mitutoyo Output -> #2 Data format
  for( i = 0; i < 13; i++ ) {  //data consists of 13 digits
    k = 0;
    for (j = 0; j < 4; j++) {  //each digit consists of 4 bits.
      //Serial.println("waiting for clock: ");  //DEBUG line
      while( digitalRead(clk) == LOW) {  // hold until clock is high
        //Serial.println("waiting for clock to not be LOW");  //DEBUG line
        } 
      while( digitalRead(clk) == HIGH) {  // hold until clock is low
        //Serial.println("waiting for clock to not be HIGH");  //DEBUG line
        } 
      
    bitWrite(k, j, (digitalRead(dat) & 0x1)); //Each clock cycle, we record the bits on the data line
    }
  
  mydata[i] = k;
  }
  
  sign = mydata[4];      //this digit tells us if data is positive or negative
  value_str = String(mydata[5]) + String(mydata[6]) + String(mydata[7]) + String(mydata[8] + String(mydata[9] + String(mydata[10]))) ; //this combines the data digits to give the raw number value reported
  decimal = mydata[11];  //this digit gives the location of the decimal
  units = mydata[12];    //this digit gives the units (mm or in)

  //following if statements are used to move our decimal to the correct place in the reported value
  if (decimal == 0) dpp = 1.0;
  if (decimal == 1) dpp = 10.0;
  if (decimal == 2) dpp = 100.0;
  if (decimal == 3) dpp = 1000.0;
  if (decimal == 4) dpp = 10000.0;
  if (decimal == 5) dpp = 100000.0;

  value_int = value_str.toInt();       //converts the value to an integer
  value = value_int / dpp;             //gets value with decimal place
  value_str = String(value, decimal);  //converts back to a string for reporting
  
  // following if statements add appropriate units depending on units value
  if (units == 0) units_str = " mm";  
  if (units == 1) units_str = " in";  

  // following if statements apply appropriate sign depending on sign value
  if (sign == 0) {
    output_val = "";
    //Serial.print(value,decimal); Serial.println(units_str);  //DEBUG line
  }
  if (sign == 8) {
    output_val = "-";
    //Serial.print("-"); Serial.print(value,decimal); Serial.println(units_str);  //DEBUG line
  }
  
  output_val.concat(value_str);  //combines the sign with the value
  output_val.concat(units_str);  //combines result from above with the units
  
  digitalWrite(req,LOW);  //resets request line

  return(output_val);    //returns the output value as a string
}