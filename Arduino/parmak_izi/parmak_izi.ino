#include <Sim800L.h>     
#include <Adafruit_Fingerprint.h>
#include <Keypad.h> 
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);
#else
#define mySerial Serial1
#endif
#define buzzer 52
#define relay 53
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
#define RX  18
#define TX  17
//#define DEBUG

const byte ROWS = 4; // Dört satır
const byte COLS = 4; // Dört sütun
// Keypad bağlantı pinleri
char keys[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'7','8','9', 'C'},
  {'4','5','6', 'B'},
  {'*','0','#', 'D'}
  
};
byte rowPins[ROWS] = {9, 8, 7, 6}; // Keypad'in satır pinleri Arduino'ya bağlı
byte colPins[COLS] = {5, 4, 3, 2}; // Keypad'in sütun pinleri Arduino'ya bağlı
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


const String correctPassword = "1337"; // Doğru şifre
String enteredPassword = "";
int check_keypad_error_count = 0;
int check_finger_error_count = 0;
const int error_number = 3;

bool finger_situation = false;
void setup()
{
  Serial.begin(9600);
  while (!Serial);  
  delay(100);
  #ifdef DEBUG
  Serial.println("\n\nAdafruit finger detect test");
  #endif
  pinMode(buzzer,OUTPUT);
  pinMode(relay,OUTPUT);
  digitalWrite(relay,HIGH);
  
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    #ifdef DEBUG
    Serial.println("Found fingerprint sensor!");
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("Did not find fingerprint sensor :(");
    #endif
    while (1) { delay(1); }
  }
  #ifdef DEBUG
  Serial.println(F("Reading sensor parameters"));
  #endif
  finger.getParameters();
  #ifdef DEBUG
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
  #endif
  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    #ifdef DEBUG
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    #endif
  }
  else {
    #ifdef DEBUG
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
    #endif
  }
}

void loop()                     // run over and over again
{
  int finger_key =getFingerprintID();
  char key = keypad.getKey();
  serial_read();
  if(finger_situation == true){
    buzzer_func("BUTTON");
    relay_func();
    check_finger_error_count = 0;
    finger_situation = false;
   }
  if(finger_key == 254){
    buzzer_func("ERROR");
    check_finger_error_count++;
   }
  if(check_finger_error_count == error_number ){
    send_error_serial();
    check_finger_error_count = 0;
    }
    
   if(check_keypad_error_count == error_number ){
   send_error_serial();
   check_keypad_error_count = 0;
   }
  #ifdef DEBUG
  Serial.println("Finger error = " + String(check_finger_error_count) + " Keypad error = " + String(check_keypad_error_count) );
  #endif
  if (key){
    #ifdef DEBUG
    Serial.println(key);
    #endif
    enteredPassword += key;
    buzzer_func("BUTTON");
  } 
  check_keypad();

  delay(50);            //don't ned to run this at full speed.
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      #ifdef DEBUG
      Serial.println("Image taken");
      #endif
      break;
    case FINGERPRINT_NOFINGER:
      return 255; 
    case FINGERPRINT_PACKETRECIEVEERR:
      #ifdef DEBUG
      Serial.println("Communication error");
      #endif
      return p;
    case FINGERPRINT_IMAGEFAIL:
      #ifdef DEBUG
      Serial.println("Imaging error");
      #endif
      return p;
    default:
      #ifdef DEBUG
      Serial.println("Unknown error");
      #endif
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      #ifdef DEBUG
      Serial.println("Image converted");
      #endif
      break;
    case FINGERPRINT_IMAGEMESS:
      #ifdef DEBUG
      Serial.println("Image too messy");
      #endif
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      #ifdef DEBUG
      Serial.println("Communication error");
      #endif
      return p;
    case FINGERPRINT_FEATUREFAIL:
      #ifdef DEBUG
      Serial.println("Could not find fingerprint features");
      #endif
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      #ifdef DEBUG
      Serial.println("Could not find fingerprint features");
      #endif
      return p;
    default:
      #ifdef DEBUG
      Serial.println("Unknown error");
      #endif
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    #ifdef DEBUG
    Serial.println("Found a print match!");
    #endif
    finger_situation = true;
    return finger.fingerID;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    #ifdef DEBUG
    Serial.println("Communication error");
    #endif
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    return 254; // Parmak izi tanınmadığında özel bir değer dön
  } else {
    #ifdef DEBUG
    Serial.println("Unknown error");
    #endif
    return p;
  }
}


// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  #ifdef DEBUG
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  #endif
  finger_situation = true;
  return finger.fingerID;
}

void buzzer_func(String situation){
  if(situation == "OK"){
    digitalWrite(buzzer,HIGH);
    delay(300);
    digitalWrite(buzzer,LOW);
    }
  else if (situation == "BUTTON"){
    digitalWrite(buzzer,HIGH);
    delay(20);
    digitalWrite(buzzer,LOW);
    }
  else if(situation == "ERROR"){
    for (int i =0 ; i <= 5; i++ ){
    digitalWrite(buzzer,HIGH);
    delay(100);
    digitalWrite(buzzer,LOW);
    delay(100);
   }}
  }

void check_keypad(){
  if(enteredPassword.length() == 4){
    #ifdef DEBUG
    Serial.println("Entered passaword " + enteredPassword + "Correct Password = " + correctPassword);
    #endif
    if(enteredPassword == correctPassword ){
      buzzer_func("OK");
      enteredPassword = "";
      check_keypad_error_count = 0;
      relay_func();
      
      }
     else{
      buzzer_func("ERROR");
      enteredPassword = "";
      check_keypad_error_count++;
      }
    }
  }

void relay_func(){
  digitalWrite(relay,LOW);
  delay(2000);
  digitalWrite(relay,HIGH);
}

void send_error_serial(){
  Serial.println("mail");
  }

bool serial_read(){
  if (Serial.available() > 0) {
    String data = Serial.readString(); 
    buzzer_func("OK");
    if (data == "ok") {
     relay_func();
    }
  }
 }
