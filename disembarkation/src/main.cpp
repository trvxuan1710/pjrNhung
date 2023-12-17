#include <SPI.h>
#include <Keypad.h>
#include <Wire.h>
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define ROW_NUM     4 
#define COLUMN_NUM  4

#define LED 2

#define RST_PIN 15
#define SS_PIN 5

MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "SonIT";
const char* password = "duahau123@";

#define HOST  "thongtinvetau-default-rtdb.firebaseio.com"
#define AUTH  "uWGelHudAqOppm0pvMWKSdBoL2I2bBH9qJml1vFU"

FirebaseData firebaseData;
FirebaseJson json;
String path = "/";
String cardUID;

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM] = {26, 25, 33, 32}; // connect to the row pins
byte pin_column[COLUMN_NUM] = {27, 14, 12, 13};  // connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

unsigned long time1 = 0;
unsigned long Time,tDisplay = 0;
int enteredAmount;
int cursorColumn = 0;
int ht = 1;
int depart;
int rd = 1;

String destination = "";
String departure = "";
int money = 0;

String readRFID();
void myDelay(int);

// Đặt địa chỉ IP của máy chủ NTP
const char* ntpServer = "pool.ntp.org";

// Tạo một đối tượng UDP để gửi và nhận gói tin NTP
WiFiUDP ntpUDP;

// Tạo đối tượng NTPClient để lấy thời gian từ máy chủ NTP
NTPClient timeClient(ntpUDP, ntpServer);

void setup()   {
  Serial.begin(9600);

  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(HOST, AUTH);
  Firebase.reconnectWiFi(true);
  if(!Firebase.beginStream(firebaseData, path));
  {
    Serial.println("Reason: " + firebaseData.errorReason());
    Serial.println();
  }

  lcd.init();
  lcd.backlight();
  pinMode(LED, OUTPUT);
  // Đặt múi giờ
  timeClient.setTimeOffset(7 * 3600);  // Điều chỉnh theo múi giờ của bạn (ở đây là UTC+7)
}

void loop() {
  timeClient.update();  // Cập nhật thời gian từ máy chủ NTP
  // Lấy thông tin về ngày, tháng, năm
  time_t rawTime = timeClient.getEpochTime();
  struct tm * timeinfo;
  timeinfo = localtime(&rawTime);
  char key = keypad.getKey();
  Time = millis();
  tDisplay = Time - time1;
  //Doc the RFID
  if(rd == 1) 
  {
    cardUID = readRFID();
    if(cardUID != "")
    {
      rd = 0;
    }
  } 
  Serial.println(cardUID);
  // Doc phim
  if(key >= '0' && key <= '9' && ht == 9) 
  {
    enteredAmount = enteredAmount * 10 + (key - '0');
    lcd.setCursor(6, 1);
    lcd.print(enteredAmount);
    time1 = Time;
  } 
  if(tDisplay < 4900 && ht == 1)
  {
    lcd.setCursor(3,0);
    lcd.print("GA DA NANG");
    lcd.setCursor(3,1);
    lcd.print("KINH CHAO");
  }
  else if(tDisplay < 5000 && ht == 1) lcd.clear();
  else if(tDisplay < 9900 && ht == 1)
  {
    lcd.setCursor(4,0);
    lcd.print("HE THONG");
    lcd.setCursor(3,1);
    lcd.print("BAN VE TAU");
  }
  else if(tDisplay < 10000 && ht == 1) lcd.clear();
  else if(tDisplay < 14900 && ht == 1)
  {
    lcd.setCursor(4,0);
    lcd.print("QUET THE");
    lcd.setCursor(2,1);
    lcd.print("DE XUONG TAU");
  }
  else if(tDisplay < 15000 && ht == 1)
  {
    lcd.clear();
    time1 = Time;
  }
  if(cardUID != "" && ht == 1)
  {
    Firebase.getString(firebaseData,path + "/hanhkhach/" + cardUID + "/diemlen");
    departure = firebaseData.stringData();
    Firebase.getInt(firebaseData,path + "/hanhkhach/" + cardUID + "/thanhtoan");
    money = firebaseData.intData();
    Firebase.getString(firebaseData,path + "/hanhkhach/" + cardUID + "/diemxuong");
    destination = firebaseData.stringData();
    ht = 0;
  }
  if(destination != "Da Nang" && ht == 0)
  {
    ht = 2;
    lcd.clear();
    time1 = Time;
    tDisplay = 0;
    lcd.setCursor(0,0);
    lcd.print("Sai Diem Den");
    lcd.setCursor(0,1);
    lcd.print("OK de tiep tuc");
  }
  else if (destination == "Da Nang" && ht == 0)
  {
    ht = 3;
  }
  if((ht == 2 && key == '#') || ht == 3)
  {
    lcd.clear();
    time1 = Time;
    tDisplay = 0;
    lcd.setCursor(0,0);
    lcd.print("Diem Bat Dau:");
    lcd.setCursor(0,1);
    lcd.print(departure);
    ht = 4;
    key = ' ';    
  }
  else if(ht == 2)
  {
    if(key == '*'||tDisplay > 20000)
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Len Lai Tau");
      lcd.setCursor(0,1);
      lcd.print("De Di Tiep");
      ht = 7;
      time1 = Time;
      tDisplay = 0;
    }
  }
  if(ht == 4)
  {
    if (key == '#'||tDisplay > 20000)
    {
      lcd.clear();
      time1 = Time;
      tDisplay = 0;
      lcd.setCursor(0,0);
      lcd.print("Diem Den:");
      lcd.setCursor(0,1);
      lcd.print("Da Nang");
      ht = 5;
      key = ' ';
    }
  }
  if(ht == 5)
  {
    if (key == '#'||tDisplay > 20000)
    {
      lcd.clear();
      time1 = Time;
      tDisplay = 0;
      lcd.setCursor(0,0);
      lcd.print("Gia Ve:");
      lcd.setCursor(0,1);
      lcd.print("200000");
      ht = 6;
      key = ' ';
    }
  }
  if(ht == 6 && (key == '#'||tDisplay > 20000))
  {
    lcd.clear();
    if(money > 200000)
    {
      time1 = Time;
      tDisplay = 0;
      lcd.setCursor(0,0);
      lcd.print("Vui long nhan");
      lcd.setCursor(0,1);
      lcd.print("lai tien thua");
      ht = 7;
    }
    else if(money == 200000)
    {
      lcd.setCursor(0,0);
      lcd.print("Da du tien ve");
      lcd.setCursor(0,1);
      lcd.print("Moi xuong tau");
      ht = 9;
    }
    else
    {
      lcd.setCursor(0,0);
      lcd.print("Thieu tien ve");
      lcd.setCursor(0,1);
      lcd.print("Nap them tien");
      ht = 8;
    }
    Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/diemxuong", "Đà Nẵng");
    Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/trangthai", "Đã xuống tàu");
    Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/gioxuong", timeClient.getFormattedTime());
    Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/ngayxuong", timeinfo->tm_mday);
    Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/thangxuong", timeinfo->tm_mon + 1);
    Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/namxuong", timeinfo->tm_year + 1900);
    key = ' ';
  }
  if(ht == 7 && (key == '#'||tDisplay > 20000))
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Tien thua:");
    lcd.setCursor(0,1);
    lcd.print(money - 200000);
    ht = 9;
    key = ' ';
  }
  if(ht == 8 && (key == '#'||tDisplay > 20000))
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Nap tien: ");
    lcd.setCursor(10,0);
    lcd.print(200000 - money);
    lcd.setCursor(0,1);
    lcd.print("Nhap: ");
    ht = 9;
    key = ' ';
  }
  if(ht == 9 && (key == '#'||tDisplay > 20000))
  {
    lcd.clear();
    //Firebase.deleteNode(firebaseData,path + "/id/" + cardUID);
    ht = 1;
    cardUID = "";
    time1 = Time;
    tDisplay = 0;
    rd = 1;
  }
}
String readRFID() {
  // Kiểm tra xem có thẻ RFID nào được đặt gần đầu đọc hay không
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    // Đọc mã thẻ RFID
    String rfidData = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      rfidData += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      rfidData += String(rfid.uid.uidByte[i], HEX);
    }
    
    // Tắt thẻ RFID
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    return rfidData;
  }

  return ""; // Trả về chuỗi rỗng nếu không có thẻ RFID được đọc
}