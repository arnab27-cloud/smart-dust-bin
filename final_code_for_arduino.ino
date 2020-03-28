//****hc05 to direct tx rx of arduino
//bluetoothserial print
String stringOne, stringTwo, stringThree;

//****mpu6050
#include <Wire.h>
//Declaring some global variables
int gyro_x, gyro_y, gyro_z;
long gyro_x_cal, gyro_y_cal, gyro_z_cal;
boolean set_gyro_angles;
long acc_x, acc_y, acc_z, acc_total_vector;
float angle_roll_acc, angle_pitch_acc;
float angle_pitch, angle_roll;
int angle_pitch_buffer, angle_roll_buffer;
float angle_pitch_output, angle_roll_output;
long loop_timer;
int temp_g;

//****lcd
#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
int condtn;

//****lm35
const int sensorlm35=A1; // Assigning analog pin A1 to variable 'sensor'
float temp;  //variable to store temperature in degree Celsius
float vout;  //temporary variable to hold sensor reading

//****soil moisture
int sensor_pin = A0;
int moist ;

//****hc-sr04
// defines pins numbers
const int trigPin = 5;
const int echoPin = 6;
// defines variables
long duration;
int dist;


void setup() {
  //****mpu6050
  Wire.begin();                                                        //Start I2C as master
  setup_mpu_6050_registers();                                          //Setup the registers of the MPU-6050 
  for (int cal_int = 0; cal_int < 1000 ; cal_int ++){                  //Read the raw acc and gyro data from the MPU-6050 for 1000 times
    read_mpu_6050_data();                                             
    gyro_x_cal += gyro_x;                                              //Add the gyro x offset to the gyro_x_cal variable
    gyro_y_cal += gyro_y;                                              //Add the gyro y offset to the gyro_y_cal variable
    gyro_z_cal += gyro_z;                                              //Add the gyro z offset to the gyro_z_cal variable
    delay(3);                                                          //Delay 3us to have 250Hz for-loop
  }
  // divide by 1000 to get avarage offset
  gyro_x_cal /= 1000;                                                 
  gyro_y_cal /= 1000;                                                 
  gyro_z_cal /= 1000;
  loop_timer = micros();                                               //Reset the loop timer

  //****lcd
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("HI! USE ME!");

  //****lm35
  pinMode(sensorlm35,INPUT); // Configuring pin A1 as input

  //****hc-sr04
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  //****hc05
  //bluetoothserial print
  Serial.begin(9600);
  stringOne = String(",");
  stringTwo = String();
  stringThree = String();
  Serial.println("start....");
  Serial.flush();
}


void loop()
{
  //****mpu6050
  read_mpu_6050_data();
 //Subtract the offset values from the raw gyro values
  gyro_x -= gyro_x_cal;
  gyro_y -= gyro_y_cal;
  gyro_z -= gyro_z_cal;
  //Gyro angle calculations . Note 0.0000611 = 1 / (250Hz x 65.5)
  angle_pitch += gyro_x * 0.0000611;                                   //Calculate the traveled pitch angle and add this to the angle_pitch variable
  angle_roll += gyro_y * 0.0000611;                                    //Calculate the traveled roll angle and add this to the angle_roll variable
  //0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians
  angle_pitch += angle_roll * sin(gyro_z * 0.000001066);               //If the IMU has yawed transfer the roll angle to the pitch angel
  angle_roll -= angle_pitch * sin(gyro_z * 0.000001066);               //If the IMU has yawed transfer the pitch angle to the roll angel
  //Accelerometer angle calculations
  acc_total_vector = sqrt((acc_x*acc_x)+(acc_y*acc_y)+(acc_z*acc_z));  //Calculate the total accelerometer vector
  //57.296 = 1 / (3.142 / 180) The Arduino asin function is in radians
  angle_pitch_acc = asin((float)acc_y/acc_total_vector)* 57.296;       //Calculate the pitch angle
  angle_roll_acc = asin((float)acc_x/acc_total_vector)* -57.296;       //Calculate the roll angle
  angle_pitch_acc -= 0.0;                                              //Accelerometer calibration value for pitch
  angle_roll_acc -= 0.0;                                               //Accelerometer calibration value for roll
  if(set_gyro_angles){                                                 //If the IMU is already started
    angle_pitch = angle_pitch * 0.9996 + angle_pitch_acc * 0.0004;     //Correct the drift of the gyro pitch angle with the accelerometer pitch angle
    angle_roll = angle_roll * 0.9996 + angle_roll_acc * 0.0004;        //Correct the drift of the gyro roll angle with the accelerometer roll angle
  }
  else{                                                                //At first start
    angle_pitch = angle_pitch_acc;                                     //Set the gyro pitch angle equal to the accelerometer pitch angle 
    angle_roll = angle_roll_acc;                                       //Set the gyro roll angle equal to the accelerometer roll angle 
    set_gyro_angles = true;                                            //Set the IMU started flag
  }
  //To dampen the pitch and roll angles a complementary filter is used
  angle_pitch_output = angle_pitch_output * 0.9 + angle_pitch * 0.1;   //Take 90% of the output pitch value and add 10% of the raw pitch value
  //angle_roll_output = angle_roll_output * 0.9 + angle_roll * 0.1;      //Take 90% of the output roll value and add 10% of the raw roll value
  //Serial.print(" | Anglepitch  = "); Serial.print(angle_pitch_output);
 //while(micros() - loop_timer < 4000);                                 //Wait until the loop_timer reaches 4000us (250Hz) before starting the next loop
 //loop_timer = micros();//Reset the loop timer
  
  //****lm35
  vout=analogRead(sensorlm35);
  vout=(vout*500)/1023;//sensor value to temperature reading in celsius
  temp=vout; // Storing value in Degree Celsius

  //****soil moisture
  moist= analogRead(sensor_pin);
  moist = map(moist,725,281,0,100);

  //****hc-sr04
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  dist= duration*0.034/2;
  
  //****lcd
  // (note: line 1 is the second row, since counting begins with 0):
  //lcd.setCursor(0, 1);
  lcd.setCursor(11, 0);
  lcd.print(temp);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("HAVE A GOOD DAY");
    
  //****hc05
  //bluetoothserial print
  stringTwo = dist + stringOne;
  stringThree = stringTwo + temp;
  stringTwo = stringThree + stringOne;
  stringThree = stringTwo + moist;
  stringTwo = stringThree + stringOne;
  stringThree = stringTwo + angle_pitch_output;
  Serial.println(stringThree);
  delay(4000);//delay of 4 second
  Serial.flush();
  }
  
  //***functions of mpu6050
  void setup_mpu_6050_registers(){
  //Activate the MPU-6050
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x6B);                                                    //Send the requested starting register
  Wire.write(0x00);                                                    //Set the requested starting register
  Wire.endTransmission();                                             
  //Configure the accelerometer (+/-8g)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1C);                                                    //Send the requested starting register
  Wire.write(0x10);                                                    //Set the requested starting register
  Wire.endTransmission();                                             
  //Configure the gyro (500dps full scale)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1B);                                                    //Send the requested starting register
  Wire.write(0x08);                                                    //Set the requested starting register
  Wire.endTransmission();                                             
}

void read_mpu_6050_data(){                                             //Subroutine for reading the raw gyro and accelerometer data
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x3B);                                                    //Send the requested starting register
  Wire.endTransmission();                                              //End the transmission
  Wire.requestFrom(0x68,14);                                           //Request 14 bytes from the MPU-6050
  while(Wire.available() < 14);                                        //Wait until all the bytes are received
  acc_x = Wire.read()<<8|Wire.read();                                  
  acc_y = Wire.read()<<8|Wire.read();                                  
  acc_z = Wire.read()<<8|Wire.read();                                  
  temp_g = Wire.read()<<8|Wire.read();                                   
  gyro_x = Wire.read()<<8|Wire.read();                                 
  gyro_y = Wire.read()<<8|Wire.read();                                 
  gyro_z = Wire.read()<<8|Wire.read();                                 
}
