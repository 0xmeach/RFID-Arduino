#include <SPI.h>                  // Include SPI library for communication with RFID module
#include <MFRC522.h>              // Include MFRC522 library for RFID
#include <LiquidCrystal_I2C.h>    // Include LiquidCrystal_I2C library for LCD
#include <Wire.h>                 // Include Wire library for I2C communication
#include <Servo.h>           // Include the ESP32 Arduino Servo Library instead of the original Arduino Servo Library

Servo myservo;  // create servo object to control a servo

#define SS_PIN 10                 // Define Slave Select pin for RFID module
#define RST_PIN 9                 // Define Reset pin for RFID module

#define LED_G 4                   // Define pin for green LED
#define LED_R 5                   // Define pin for red LED
#define Alarm A0                  // Define pin for Alarm
#define SERVO_PIN 3                    // Define pin for SERVO_PIN control

int angle = 0; // initial angle for servo
/*
Signal    Arduino Uno                                  Arduino Mega 
----------------------------------------------------------------------
SPI SS     SDA            Arduino Pin 10               Arduino Pin 53
SPI SCK    SCK            Arduino Pin 13               Arduino Pin 52
SPI MOSI   MOSI           Arduino Pin 11               Arduino Pin 51
SPI MISO   MISO           Arduino Pin 12               Arduino Pin 50
GND        GND            Arduino GND Pin              GND
RST/Reset  RST            Arduino Pin 9                Pin 5
3.3        3.3V           Arduino Pin 3.3V             3.3V
*/

int lcdRows = 2;                  // Number of rows in the LCD

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance with defined SS and RST pins

LiquidCrystal_I2C lcd(0x27, 16, 2); // Create LCD instance with I2C address 0x27, 16 columns, and 2 rows

void setup() 
{
  myservo.attach(3);

  Serial.begin(9600);             // Start serial communication at 9600 baud rate

  SPI.begin();                    // Initiate SPI bus

  mfrc522.PCD_Init();             // Initiate MFRC522 RFID module

  lcd.init();                     // Initialize the LCD
  lcd.backlight();                // Turn on the LCD backlight

  lcd.setCursor(0,0);             // Set cursor to the first row and column
  lcd.print("Your RFID Card!");   // Display initial message on LCD

  pinMode(LED_G, OUTPUT);         // Set green LED pin as output
  pinMode(LED_R, OUTPUT);         // Set red LED pin as output
  pinMode(Alarm, OUTPUT);         // Set Alarm pin as output
  pinMode(SERVO_PIN, OUTPUT);          // Set SERVO_PIN control pin as output

  digitalWrite(SERVO_PIN, HIGH);       // Set SERVO_PIN to locked position initially
}

void loop() 
{
   // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;                       // If no new card is present, exit the loop
  }

  // Display UID on Serial Monitor
  Serial.print("UID tag: ");
  String content = "";            // Create a string to hold the UID content
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX); // Print each byte of the UID in HEX format
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX)); // Append each byte to the content string
  }
  Serial.println();
  Serial.println();

  content.toUpperCase();          // Convert the UID content to uppercase
  if (content.substring(1) == "0B 23 9B 15" ) // Check if the UID matches an authorized card
  {
    digitalWrite(LED_G, HIGH);    // Turn on green LED
    myservo.write(180); // move the servo to desired angle

    lcd.clear();                  // Clear the LCD screen
    lcd.setCursor(0,0);           // Set cursor to the first row and column
    lcd.print("Your RFID Card: "); // Display message on LCD

    lcd.setCursor(0,1);           // Set cursor to the second row
    lcd.print("You can enter :) ");// Display access granted message

    delay(2500);                  // Wait for 2.5 seconds

    lcd.clear();                  // Clear the LCD screen
    lcd.setCursor(0,0);           // Set cursor to the first row and column
    lcd.print("Your RFID Card: "); // Display initial message on LCD

    digitalWrite(LED_G, LOW);     // Turn off green LED
    myservo.write(-180); // move the servo to desired angle
  }
  //52 EF E9 1C
  else if (content.substring(1) == "52 EF E9 1C" ){ // Check if the UID matches a wrong card
    digitalWrite(LED_R, HIGH);    // Turn on red LED
    digitalWrite(Alarm, HIGH);    // Activate alarm
    //myservo.write(0); // move the servo to desired angle

    lcd.clear();                  // Clear the LCD screen
    lcd.setCursor(0,0);           // Set cursor to the first row and column
    lcd.print("Your RFID Card!"); // Display message on LCD

    lcd.setCursor(0,1);           // Set cursor to the second row
    lcd.print("Wrong Card ;( ");  // Display access denied message

    delay(3000);                  // Wait for 3 seconds

    lcd.clear();                  // Clear the LCD screen
    lcd.setCursor(0,0);           // Set cursor to the first row and column
    lcd.print("Your RFID Card!"); // Display initial message on LCD

    digitalWrite(LED_R, LOW);     // Turn off red LED
    digitalWrite(Alarm, LOW);     // Deactivate alarm
  }
  // Halt PICC and stop encryption on PCD
  mfrc522.PICC_HaltA();           // Halt the PICC (card)
  mfrc522.PCD_StopCrypto1();      // Stop encryption on the PCD (reader)
} 
