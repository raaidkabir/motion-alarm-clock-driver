//************libraries**************//
#include <Wire.h>
#include <RtcDS1307.h>
#include <LiquidCrystal.h>
#include <LedControl.h>

//**********************************//
RtcDS1307<TwoWire> Rtc(Wire);
LedControl lc = LedControl(10, 9, 8, 1);

//************Buttons***************//
const int P1 = A1; // Button SET MENU'
const int P2 = 4; // Button +
const int P3 = 5; // Button -
const int P4 = 7; // SWITCH Alarm
const int alarmLED = A2; // Alarm Status

//***********Definitions************//
#define buzzer 6
#define echoPin 3 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 2 //attach pin D3 Arduino to pin Trig of HC-SR04
#define countof(a) (sizeof(a) / sizeof(a[0]))

//************Variables*************//
int hour_temp;
int min_temp;
int menu = 0;
int setAlarm = 0;
unsigned long duration = 0; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement
bool isAlarm; // variable that deterimines whether alarm is playing or not

uint8_t alarmHours = 0, alarmMinutes = 0;  // Holds the current alarm time



void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  pinMode(P1, INPUT);
  pinMode(P2, INPUT);
  pinMode(P3, INPUT);
  pinMode(P4, INPUT);
  pinMode(alarmLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  if (!Rtc.IsDateTimeValid()) {
    if (Rtc.LastError() != 0) {
      // we have a communications error
      // see https://www.arduino.cc/en/Reference/WireEndTransmission for
      // what the number means
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
    }
    else
    {
      // Common Causes:
      //    1) first time you ran and the device wasn't running yet
      //    2) the battery on the device is low or even missing
      Serial.println("RTC lost confidence in the DateTime!");
      // following line sets the RTC to the date & time this sketch was compiled
      // it will also reset the valid flag internally unless the Rtc device is
      // having an issue

      Rtc.SetDateTime(compiled);
    }
  }
  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  } else if (now > compiled) {
    Serial.println("RTC is newer than compile time. (this is expected)");
  } else if (now == compiled) {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }
  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.SetSquareWavePin(DS1307SquareWaveOut_Low);
  int menu = 0;

}

void loop() {
  if (digitalRead(P1) == LOW)
  {
    menu = menu + 1;
  }
  // in which subroutine should we go?
  if (menu == 0)
  {
    displayTime(); // void DisplayDateTime
    alarm(); // Alarm control
    checkMotion();
    OnAlarm();
  }
  if (menu == 1)
  {
    displaySetHour();
  }
  if (menu == 2)
  {
    displaySetMinute();
  }
  if (menu == 3)
  {
    storeAgg();
    delay(200);
    menu = 4;
  }
  if (menu == 4)
  {
    displaySetHourAlarm();
  }
  if (menu == 5)
  {
    displaySetMinuteAlarm();
  }
  if (menu == 6)
  {
    delay(200);
    menu = 0;
  }

  delay(100);

}

void displayTime() {
  RtcDateTime now = Rtc.GetDateTime();
  int dig_0 = now.Hour() / 10;
  int dig_1 = now.Hour() % 10;
  hour_temp = now.Hour();
  int dig_2 = now.Minute() / 10;
  int dig_3 = now.Minute() % 10;
  min_temp = now.Minute();
  lc.setDigit(0, 0, dig_0, false);
  lc.setDigit(0, 1, dig_1, false);
  lc.setDigit(0, 2, dig_2, false);
  lc.setDigit(0, 3, dig_3, false);
  lc.setDigit(0, 4, 0, true);
}

void displaySetHour() {
  lc.clearDisplay(0);
  if (digitalRead(P2) == LOW)
  {
    if (hour_temp == 23)
    {
      hour_temp = 0;
    }
    else
    {
      hour_temp = hour_temp + 1;
    }
  }
  if (digitalRead(P3) == LOW)
  {
    if (hour_temp == 0)
    {
      hour_temp = 23;
    }
    else
    {
      hour_temp = hour_temp - 1;
    }
  }
  int dig_0 = hour_temp / 10;
  int dig_1 = hour_temp % 10;
  lc.setDigit(0, 0, dig_0, false);
  lc.setDigit(0, 1, dig_1, false);
  lc.setDigit(0, 4, 0, true);
  delay(200);
}

void displaySetMinute() {
  lc.clearDisplay(0);
  if (digitalRead(P2) == LOW)
  {
    if (min_temp == 59)
    {
      min_temp = 0;
    }
    else
    {
      min_temp = min_temp + 1;
    }
  }
  if (digitalRead(P3) == LOW)
  {
    if (min_temp == 0)
    {
      min_temp = 59;
    }
    else
    {
      min_temp = min_temp - 1;
    }
  }
  int dig_2 = min_temp / 10;
  int dig_3 = min_temp % 10;
  lc.setDigit(0, 2, dig_2, false);
  lc.setDigit(0, 3, dig_3, false);
  lc.setDigit(0, 4, 0, true);
  delay(200);
}

void storeAgg() {
  RtcDateTime now = Rtc.GetDateTime();
  char timestring[9];
  snprintf_P(timestring,
             countof(timestring),
             PSTR("%02u:%02u:%02u"),
             hour_temp,
             min_temp,
             now.Second() );
  Rtc.SetDateTime(RtcDateTime(__DATE__, timestring));
}

void displaySetHourAlarm()// Setting the alarm minutes
{
  lc.clearDisplay(0);
  if (digitalRead(P2) == LOW)
  {
    if (alarmHours == 23)
    {
      alarmHours = 0;
    }
    else
    {
      alarmHours = alarmHours + 1;
    }
  }
  if (digitalRead(P3) == LOW)
  {
    if (alarmHours == 0)
    {
      alarmHours = 23;
    }
    else
    {
      alarmHours = alarmHours - 1;
    }
  }
  int dig_0 = alarmHours / 10;
  int dig_1 = alarmHours % 10;
  lc.setDigit(0, 0, dig_0, true);
  lc.setDigit(0, 1, dig_1, true);
  lc.setDigit(0, 4, 0, true);
  delay(200);
}

void displaySetMinuteAlarm()// Setting the alarm minutes
{
  lc.clearDisplay(0);
  if (digitalRead(P2) == LOW)
  {
    if (alarmMinutes == 59)
    {
      alarmMinutes = 0;
    }
    else
    {
      alarmMinutes = alarmMinutes + 1;
    }
  }
  if (digitalRead(P3) == LOW)
  {
    if (alarmMinutes == 0)
    {
      alarmMinutes = 59;
    }
    else
    {
      alarmMinutes = alarmMinutes - 1;
    }
  }
  int dig_2 = alarmMinutes / 10;
  int dig_3 = alarmMinutes % 10;
  lc.setDigit(0, 2, dig_2, true);
  lc.setDigit(0, 3, dig_3, true);
  lc.setDigit(0, 4, 0, true);
  delay(200);
}

void checkMotion() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  Serial.print("Distance: ");
  Serial.println(distance);
}

void OnAlarm() {
  if (isAlarm == true) {
    RtcDateTime now = Rtc.GetDateTime();
    checkMotion();
    tone(buzzer, 880); //play the note "A5" (LA5)
    delay (300);
    tone(buzzer, 698); //play the note "F6" (FA5)
    if (distance < 15) {
      isAlarm = false;
      noTone (buzzer);
      alarmMinutes += 5;
      if (alarmMinutes >= 60) {
        alarmMinutes -= 60;
      }
      if (alarmHours >= 24) {
        alarmHours -= 24;
      }

    }
  }
}

void alarm() {
  if (digitalRead(P4) == LOW)
  {
    setAlarm = setAlarm + 1;
  }
  if (setAlarm == 0)
  {
    isAlarm = false;
    noTone (buzzer);
    digitalWrite(alarmLED, LOW);
  }
  if (setAlarm == 1)
  {
    digitalWrite(alarmLED, HIGH);
    RtcDateTime now = Rtc.GetDateTime();
    if ( now.Hour() == alarmHours && now.Minute() == alarmMinutes )
    {
      isAlarm = true;
    }

  }
  if (setAlarm == 2)
  {
    setAlarm = 0;
  }
  delay(200);
}