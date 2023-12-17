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

String readRFID();

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
  if(rd == 1) 
  {
    cardUID = readRFID();
    if(cardUID != "")
    {
      rd = 0;
    }
  } 
  Serial.println(cardUID);
  if(ht == 3 && key >= '0' && key <= '9') 
  {
    enteredAmount = enteredAmount * 10 + (key - '0');
    lcd.setCursor(0, 1);
    lcd.print(enteredAmount);
    time1 = Time;
  } 
  if(key == '#' && ht == 3)
  {
    switch(depart)
    {
      case 1:
      {
        if(enteredAmount >= 100000)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Thanh Toan Xong");
          lcd.setCursor(0,1);
          lcd.print("Moi len tau");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/diemlen", "Hà Nội");
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/thanhtoan", enteredAmount);
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/diemxuong", "Thanh Hoá");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/trangthai", "Đã Lên tàu");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/giolen", timeClient.getFormattedTime());
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/ngaylen", timeinfo->tm_mday);
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/thanglen", timeinfo->tm_mon + 1);
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/namlen", timeinfo->tm_year + 1900);
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/gioxuong", " ");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/ngayxuong", " ");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/thangxuong"," ");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/namxuong", " ");
          ht = 4;
        }
        else{
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Thanh Toan Loi");
          lcd.setCursor(0,1);
          lcd.print("Thuc hien lai");
          //digitalWrite(LED, HIGH);
          ht = 2;
          key = ' ';
        }
        break;
        }
      case 2:
      {
        if(enteredAmount >= 200000)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Thanh Toan Xong");
          lcd.setCursor(0,1);
          lcd.print("Moi len tau");
                    Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/diemlen", "Hà Nội");
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/thanhtoan", enteredAmount);
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/diemxuong", "Đà Nẵng");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/trangthai", "Đã Lên tàu");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/giolen", timeClient.getFormattedTime());
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/ngaylen", timeinfo->tm_mday);
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/thanglen", timeinfo->tm_mon + 1);
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/namlen", timeinfo->tm_year + 1900);
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/gioxuong", " ");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/ngayxuong", " ");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/thangxuong"," ");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/namxuong", " ");
          ht = 4;
        }
        else{
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Thanh Toan Loi");
          lcd.setCursor(0,1);
          lcd.print("Thuc hien lai");
          // digitalWrite(LED, HIGH);
          ht = 2;
          key = ' ';
        }
        break;
        }
        case 3:
        {
        if(enteredAmount >= 300000)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Thanh Toan Xong");
          lcd.setCursor(0,1);
          lcd.print("Moi len tau");
                    Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/diemlen", "Hà Nội");
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/thanhtoan", enteredAmount);
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/diemxuong", "Huế");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/trangthai", "Đã Lên tàu");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/giolen", timeClient.getFormattedTime());
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/ngaylen", timeinfo->tm_mday);
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/thanglen", timeinfo->tm_mon + 1);
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/namlen", timeinfo->tm_year + 1900);
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/gioxuong", " ");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/ngayxuong", " ");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/thangxuong"," ");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/namxuong", " ");
          ht = 4;
        }
        else{
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Thanh Toan Loi");
          lcd.setCursor(0,1);
          lcd.print("Thuc hien lai");
          //digitalWrite(LED, HIGH);
          ht = 2;
          key = ' ';
        }
        break;
        }
        case 4:
        {
        if(enteredAmount >= 400000)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Thanh Toan Xong");
          lcd.setCursor(0,1);
          lcd.print("Moi len tau");
                    Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/diemlen", "Hà Nội");
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/thanhtoan", enteredAmount);
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/diemxuong", "TP HCM");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/trangthai", "Đã Lên tàu");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/giolen", timeClient.getFormattedTime());
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/ngaylen", timeinfo->tm_mday);
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/thanglen", timeinfo->tm_mon + 1);
          Firebase.setInt(firebaseData,path + "/hanhkhach/" + cardUID + "/namlen", timeinfo->tm_year + 1900);
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/gioxuong", " ");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/ngayxuong", " ");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/thangxuong"," ");
          Firebase.setString(firebaseData,path + "/hanhkhach/" + cardUID + "/namxuong", " ");
          ht = 4;
        }
        else{
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Thanh Toan Loi");
          lcd.setCursor(0,1);
          lcd.print("Thuc hien lai");
          //digitalWrite(LED, HIGH);
          ht = 2;
          key = ' ';
        }
        break;
        }
        default: break;
    }
    enteredAmount = 0;
  }
  if(tDisplay >= 5000 && ht == 4)
  {
    digitalWrite(LED, LOW);
    lcd.clear();
    ht = 1;
    time1 = Time;
    tDisplay = 0;
    rd = 1;
    cardUID = "";
  }
  if(tDisplay < 4900 && ht == 1)
  {
    lcd.setCursor(4,0);
    lcd.print("HE THONG");
    lcd.setCursor(3,1);
    lcd.print("BAN VE TAU");
  }
  else if(tDisplay < 5000 && ht == 1) lcd.clear();
  else if(tDisplay < 9900 && ht == 1)
  {
    lcd.setCursor(0,0);
    lcd.print("1.Thanh Hoa");
    lcd.setCursor(0,1);
    lcd.print("2.Da Nang");
  }
  else if(tDisplay < 10000 && ht == 1) lcd.clear();
  else if(tDisplay < 14900 && ht == 1)
  {
    lcd.setCursor(0,0);
    lcd.print("3.Hue");
    lcd.setCursor(0,1);
    lcd.print("4.TP HCM");
  }
  else if(tDisplay < 15000 && ht == 1)
  {
    lcd.clear();
    time1 = Time;
    ht = 1;
  }
  if(cardUID != "" && ht == 1)
  {
    ht = 0;
    lcd.clear();
    time1 = Time;
    tDisplay = 0;
    lcd.setCursor(0,0);
    lcd.print("Chon diem den");
    lcd.setCursor(0,1);
    lcd.print("F: Ha Noi");
  }
  if(key == '1' && ht == 0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("T: Thanh Hoa");
    lcd.setCursor(0,1);
    lcd.print("Fee: 100.000");
    ht = 2;
    depart = 1;
    tDisplay = 0;
    time1 = Time;
  }
  if(key == '2' && ht == 0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("T: Da Nang");
    lcd.setCursor(0,1);
    lcd.print("Fee: 200.000");
    ht = 2;
    depart = 2;
    tDisplay = 0;
    time1 = Time;
  }
  if(key == '3' && ht == 0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("T: Hue");
    lcd.setCursor(0,1);
    lcd.print("Fee: 300.000");
    ht = 2;
    depart = 3;
    tDisplay = 0;
    time1 = Time;
  }
  if(key == '4' && ht == 0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("T: TPHCM");
    lcd.setCursor(0,1);
    lcd.print("Fee: 400.000");
    ht = 2;
    depart = 4;
    tDisplay = 0;
    time1 = Time;
  }
  if(ht == 2 && key == '#')
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Thanh Toan:");
    time1 = Time;
    ht = 3;
  }
  if((key == '*' || tDisplay > 10000) && (ht == 0 || ht == 2 || ht == 3))
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Tac vu da bi huy");
    lcd.setCursor(0,1);
    lcd.print("Thuc hien lai");
    tDisplay = 0;
    time1 = Time;
    ht = 5;
  }    
  if(ht == 5)
  {
    if(tDisplay > 6000 || key == '#')
    {
      lcd.clear();
      rd = 1;
      cardUID = "";
      ht = 1;
      time1 = Time;
      tDisplay = 0;
    }
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