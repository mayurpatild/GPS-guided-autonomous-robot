#include <LiquidCrystal.h>
#include <TinyGPS.h>
#include <Wire.h>

//wire pins on board( ATMega2560)	20 (SDA), 21 (SCL)
#define address 0x1E //0011110b, I2C 7bit address of HMC5883
TinyGPS gps;
#define rxPin 19 // to yellow wire in my gps
#define txPin 18

bool feedgps();

int echoPin = 9; // pingpin to Digital 8
int trigPin = 8; // inpin to Digital 9
int SAFE_ZONE = 10;

//Motor direction control pin def
#define right1 22
#define right2 24
#define left1 26
#define left2 28
#define pwm_left 13
#define pwm_right 12

LiquidCrystal lcd(6 ,7 ,2 ,3, 4, 5); // define our LCD and which pins to user

void setup()

{
  lcd.begin(20, 4); // need to specify how many columns and rows are in the LCD unit



  Serial.begin(9600);
  delay(100);                   // Power up delay

  Serial.println("GPS GUIDED ROBOT");
  Serial.println("By- Mayur Patil");
  Serial.println("PUNE University");
  Serial.println("Department of Electronics and Telecommunication Engineering");
  Serial.println("====================================================");
  Serial.println("                                                     ");
  Serial1.begin(9600);

  Wire.begin();

  Serial1.print("$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
  Serial1.print("$PMTK220,200*2C\r\n");

  // Set operating mode to continuous in compass
  Wire.beginTransmission(address); 
  Wire.write(byte(0x02));
  Wire.write(byte(0x00));
  Wire.endTransmission();

  pinMode(left1, OUTPUT);//LEFT  
  pinMode(left2, OUTPUT); //LEFT
  pinMode(right1, OUTPUT); //R
  pinMode(right1, OUTPUT); //R
  pinMode(pwm_left, OUTPUT); //LEFT
  pinMode(pwm_right, OUTPUT); //R
  pinMode(echoPin, OUTPUT);
  pinMode(trigPin, INPUT);

}

inline void setDir(bool L1, bool L2, bool R1, bool R2){
  digitalWrite(left1,L1);//L
  digitalWrite(left2,L2);//L
  digitalWrite(right1,R1);//R
  digitalWrite(right2,R2);//R
}

void forward()
{
  setDir(0,1,0,1);
  analogWrite(pwm_left,200);//L
  analogWrite(pwm_right,255);//R
}

void Backward()
{
  setDir(1,0,1,0);
  analogWrite(pwm_left,200);//L
  analogWrite(pwm_right,255);//R
}


void search()
{
  setDir(0,1,1,0);
  analogWrite(pwm_left,255);//R
  analogWrite(pwm_right,255);//L

}

void Stop()
{
  setDir(0,0,0,0);
  analogWrite(12,0);//L
  analogWrite(13,0);//R
}

void right()
{
  setDir(1,0,0,1);
  analogWrite(pwm_left,255);//L
  analogWrite(pwm_right,255);//R
}

void left()
{
  setDir(0,1,1,0);
  analogWrite(pwm_left,255);//L
  analogWrite(pwm_right,255);//R
}
long getDistance(){
  long duration;
  long cm;
  digitalWrite(echoPin, LOW);
  delayMicroseconds(2);
  digitalWrite(echoPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(echoPin, LOW);
  duration = pulseIn(trigPin, HIGH);
  return duration / 29 / 2;
   Serial.print(cm);
  Serial.print("cm"); 
}

void loop(){

  long cm=getDistance();
  feedgps();
  getDistance();


  unsigned char cc = Serial1.read();
  float flat, flon,x2lat,x2lon;
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);

  feedgps();
  getDistance();

  float flat1=flat;     // flat1 = our current latitude. flat is from the gps data. 
  float flon1=flon;  // flon1 = our current longitude. flon is from the fps data.
  float dist_calc=0;
  float angle_calc=0;
  float dist_calc2=0;
  float diflat=0;
  float diflon=0;
  x2lat=  18.5844321      ;  //enter a latitude point here   (my college location)
  x2lon= 18.5841197    ;  // enter a longitude point here  
  //------------------------------------------ distance formula below. Calculates distance from current location to waypoint

  dist_calc=sqrt((((flon1)-(x2lon))*((flon1)-(x2lon)))+(((x2lat-flat1)*(x2lat-flat1))));
  dist_calc*=110567 ; //Converting to meters
  //=======================angle==========================================
  angle_calc=atan2((x2lon-flon1),(x2lat-flat1));

  float declinationAngle2 = 57.29577951;
  angle_calc*= declinationAngle2;
  feedgps();
  getDistance();

  if(angle_calc < 0){
    angle_calc = 360 + angle_calc;
    feedgps();
    getDistance();
  }
  // Check for wrap due to addition of declination.
  if(angle_calc >0){
    angle_calc= angle_calc;
    feedgps();
    getDistance();
  }

  float angleDegrees = angle_calc;
  feedgps();
  //this is compass code==================================      
  int x, y, z;
  feedgps();
  getDistance();

  // Initiate communications with compass
  Wire.beginTransmission(address);
  Wire.write(byte(0x03));       // Send request to X MSB register
  Wire.endTransmission();

  Wire.requestFrom(address, 6);    // Request 6 bytes; 2 bytes per axis
  if(6<=Wire.available()) {    // If 6 bytes available
    x = Wire.read()<<8; //X msb
    x |= Wire.read(); //X lsb
    z = Wire.read()<<8; //Z msb
    z |= Wire.read(); //Z lsb
    y = Wire.read()<<8; //Y msb
    y |= Wire.read(); //Y lsb
    feedgps();
    getDistance();
  }

  float heading = atan2(y,x);
  float declinationAngle = 0.0457;
  heading += declinationAngle;
  feedgps();
    getDistance();

  // Correct for when signs are reversed.
  if(heading < 0){
    heading += 2*PI;
    feedgps();
    getDistance();
  }

  // Check for wrap due to addition of declination.
  if(heading > 2*PI){
    heading -= 2*PI;
    feedgps();
    getDistance();
  }

  float headingDegrees = heading * 180/M_PI;
  feedgps();
  getDistance();

  //================================LCD
  lcd.setCursor(0,0);
  lcd.print("E");
  lcd.print(flat,5);

  lcd.setCursor(10,0);
  lcd.print("N");
  lcd.print(flon,5);

  lcd.setCursor(0,1);
  lcd.print("d:");
  lcd.print(dist_calc,2);



  lcd.setCursor(0,2);
  lcd.print("H:");
  lcd.print(headingDegrees,2);

  lcd.setCursor(10,2);
  lcd.print("A:");
  lcd.print(angleDegrees,2);


  //==================================



  feedgps();
  getDistance();
  Serial.print("Lat: ");
  Serial.print(flat,8);
  Serial.print(", Long: ");
  Serial.println(flon,8);
  Serial.println("distance");
  Serial.println(dist_calc,8);    //print the distance in meters
  //compass========================================   
  Serial.print("Heading:\t");
  Serial.print(headingDegrees);
  Serial.println(" Degrees   \t");
  Serial.print(angleDegrees);
  Serial.println(" Degrees   \t");
  Serial.print(cm);
  Serial.print("cm"); 
  Serial.println("                 ");
  delay(500);


  // Take actions ==================================      
  bool condition = ((angleDegrees -10) < headingDegrees) && ((angleDegrees +10) > headingDegrees);  
  if(condition){    //condition == true?
    Serial.println("MOVE");
    lcd.setCursor(5,3);
    lcd.print("   MOVE   ");
    feedgps();
    getDistance();
    forward();
    long stTime = millis();
    while(millis()-stTime < 4000){
      getDistance();
      feedgps();
      if(getDistance() == 10)
      {
        feedgps();
        getDistance();
        Serial.println("OBS FORWARD");
        Serial.println("                 ");
        lcd.setCursor(5,3);
    lcd.print("OBS FORWARD");
        feedgps();
        getDistance();
        Stop();
         myDelay(1000);  //*** what to do if obstacle detected   
        feedgps();
        getDistance();
        Backward();
         myDelay(500);
        feedgps();
        getDistance();
        right();
         myDelay(800);
        feedgps();
        getDistance();
        forward();
         myDelay(2000);
        feedgps();
        getDistance();
        left();
         myDelay(800);
        feedgps();
        getDistance();
        forward();
       // break;              //*** get out of while loop

      }
      else{
      forward();
      }
      feedgps();
      getDistance();
      
    }
  }

  if(SAFE_ZONE>cm){    //obstacle in range?
    
   Serial.println("OBS");
Serial.println("                 ");
 lcd.setCursor(5,3);
    lcd.print("    OBS     ");
    feedgps();
        getDistance();
        Stop();
         myDelay(1000);  //*** what to do if obstacle detected   
        feedgps();
        getDistance();
        Backward();
         myDelay(500);
        feedgps();
        getDistance();
        right();
         myDelay(800);
        feedgps();
        getDistance();
        forward();
         myDelay(2000);
        feedgps();
        getDistance();
        left();
         myDelay(800);
        feedgps();
        getDistance();
        forward();
       
  }

  if(dist_calc<4)
  {
    feedgps();
    getDistance();
    Stop();
    lcd.setCursor(4,3);
    lcd.print("   STOP   ");
    Serial.println("      STOP    ");
  }
  
 /* if(flon==1000 && flat==1000 )
  {
    feedgps();
    Stop();
    lcd.setCursor(4,3);
    lcd.print("  NO SAT  ");
     Serial.println("NO SAT");
  }
  */

  else {
    lcd.setCursor(5,3);
    lcd.print("SEARCHING");
    search();
    feedgps();
    getDistance();
    delay(20);
    Stop();
    feedgps();
    getDistance();
    delay(30);
  }  

}


void myDelay(long duration){
  long stTime = millis();
  while(millis() - stTime <duration){
    feedgps();
    delay(1);
  }
}


static bool feedgps()        
{
  while (Serial1.available())
  {
    if (gps.encode(Serial1.read()))
      return true;
  }
  return false;
}
