/*
 * Dr.TRonik / YouTube / ŞUBAT 2023 / İzmir / Türkiye 
 * ESP8266, P10 Led Panel, Mesaj Box, Eeprom, WebServer...
 * Güç kesildiğinde, son yazı Eeprom bellekte kalır...
 * The original source code to "running Text" : https://github.com/busel7/DMDESP/blob/master/examples/TeksDiamdanJalan/TeksDiamdanJalan.ino by  busel7
 * Credit to: YouTube @utehstr https://www.youtube.com/watch?v=6IQYDnwTjAo
 * Derleme ve karta yükleme öncesi, tüm kütüphaneler ve font dosyaları arduino ide'sine yüklenmiş olmalıdır...
*/

/********************************************************************
  GLOBALS___GLOBALS___GLOBALS___GLOBALS___GLOBALS___GLOBALS___
 ********************************************************************/
//Kütüphane ve font eklemeleri...
#include <ESP8266WebServer.h>
#include <DMDESP.h>  // https://github.com/busel7/DMDESP
#include <fonts/EMSans8x16.h>
#include <EEPROM.h>

//Acces Point erişim ad ve pass belirleme:
const char* ssid = "ESP_Test";
const char* password = "123456789";

//Prot 80 üzerinden server iletişimi...
ESP8266WebServer server(80);

//SetUp DMD
#define DISPLAYS_WIDE 3                     // Yatayda 3 panel...
#define DISPLAYS_HIGH 1                     // Dikeyde 1 panel...
DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);  // 3 panel eninde, 1 panel yüksekliğinde...

char* Text[] = { "P10 Led Panel MessageBox With ESP8266 AP Mode WebServer Dr.TRonik YouTube..." };  //Boş bırakılmamalı...
String Incoming_Text = "";
/* Varsayılan text i EEPROM a kayedetmek için String Text_To_EEPROM = ""; içine varsayılan text yazılarak, void setup() içindeki
   Text_From_EEPROM = read_String_from_EEPROM(0);   delay(100);    satırlarındaki yorum kaldırılarak yüklenmeli, ve sonrasında
   String Text_To_EEPROM = ""; içine yazılan text silinip,
   Text_From_EEPROM = read_String_from_EEPROM(0);   delay(100);    satırları yorumları yeniden açılarak ikinci yükleme yapılmalı...
  */

//_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-
String Text_To_EEPROM = "";
//_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-

String Text_From_EEPROM = "";

//WebPage tasarım kodları...
// Root web sayfamızı const char olarak tanımlayalım ve flash bellek için PROGMEM ile Raw String macro (R) kullanalım...
// const char Name[] PROGMEM = R"=====(//htmlcode here)=====";
// This function returns an HTML formated page in the correct type for display
// It uses the Raw string macro 'R' to place commands in PROGMEM
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
  <meta charset='utf-8'>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html {
        font-family: Arial;
        display: inline-block;
        margin: 0px auto;
        text-align: center;
      }

      h1 { font-size: 2.0rem; color:#2980b9;}
      
      .buttonSend {
        display: inline-block;
        padding: 15px 25px;
        font-size: 24px;
        font-weight: bold;
        cursor: pointer;
        text-align: center;
        text-decoration: none;
        outline: none;
        color: #fff;
        background-color: #4CAF50;
        border: none;
        border-radius: 15px;
        box-shadow: 0 5px #999;
      }
        .buttonSend:hover {background-color: #3e8e41}
        .buttonSend:active {
        background-color: #3e8e41;
        box-shadow: 0 1px #666;
        transform: translateY(4px);
      }
    </style>
  </head>
  
  <body>
    <div>
      <h1>KAYAR YAZI UYGULAMASI</h1>
      <h2>Türkçe Karekter Kullanmayınız:</h2><br>
      <textarea id="TextBox" name="TextBox" rows="4" cols="50"></textarea>
      <br><br>
    </div>
      
    <div>
      <button type="button" class="buttonSend" onclick="sendData()">Gönder</button>
    </div>

    <script>
      function sendData() {
        var Text = document.getElementById("TextBox").value;
        var xhttp = new XMLHttpRequest();
        xhttp.open("GET", "setText?TextContents="+Text, true);
        xhttp.send(); 
      }
    </script>
  </body>
</html>
)=====";


/********************************************************************
  SETUP___SETUP___SETUP___SETUP___SETUP___SETUP___SETUP___SETUP___
 ********************************************************************/
void setup() {
  Serial.begin(9600);
  delay(500);
  EEPROM.begin(512);
  delay(500);

  //DMDESP Setup
  Disp.start();
  Disp.setBrightness(100);  // Panel parlaklık ayarı (0 kapalı, 254 en parlak)...

  //ESP8266, Erişim Noktası (A.P.) olarak çalışmaya başlar:
  WiFi.softAP(ssid, password);

  Serial.println("");

  IPAddress apip = WiFi.softAPIP();  
  // Fonksiyon parantezi boş bırakılırsa varsayılan IP: 192.168.4.1 olur...
  Serial.print("ERiSiM NOKTASI: ");
  Serial.println(ssid);
  Serial.print("IP NO : ");
  Serial.print(apip);
  Serial.println(" OK.");

  //Web Sayfa tanımları:
  server.on("/", handleRoot);
  server.on("/setText", handle_Incoming_Text);

  //Server başlatma:
  server.begin();
  Serial.println("HTTP server OK...");

  /* "write_String_to_EEPROM" in void setup is used only once.
     After the code has been uploaded, comment / disable "write_String_to_EEPROM" and upload the code again.
     Then don't use "write_String_to_EEPROM" in void setup again.
  
    Takip eden iki satır, 
    GLOBAL'de yeralan String Text_To_EEPROM = ""; içine varsayılan text yazılıp sadece bir defa yüklenmelidir...
    Sonrasında, String Text_To_EEPROM = ""; içi silinerek yorumları açılarak yükleme tekrarlanmalıdır...
  */
  //_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-
  // write_String_to_EEPROM(0, Text_To_EEPROM); 
  // delay(10);
  //_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-

  Text_From_EEPROM = read_String_from_EEPROM(0);
  delay(100);
  Process_Incoming_Text(); 
}

/********************************************************************
  LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__
 ********************************************************************/
void loop() {
  server.handleClient();
  Disp.loop();
  Scrolling_Text(0, 50);  //Scrolling_Text(y konumu, hızı);"
}

/********************************************************************
  VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs
 ********************************************************************/
void Scrolling_Text(int y, uint8_t scrolling_speed) {
  static uint32_t pM;
  static uint32_t x;
  int width = Disp.width();
  Disp.setFont(EMSans8x16);
  int fullScroll = Disp.textWidth(Text[0]) + width;
  if ((millis() - pM) > scrolling_speed) {
    pM = millis();
    if (x < fullScroll) {
      ++x;
    } else {
      x = 0;
      return;
    }
    Disp.drawText(width - x, y, Text[0]);
  }
}

void handleRoot() {
  server.send(200, "text/html", MAIN_page);  // html 200 kodu ile web sayfası gönderimi...
}

void handle_Incoming_Text() {
  Incoming_Text = server.arg("TextContents");
  server.send(200, "text/plane", "");

  // "Send:" Gönderilecek text, güvenlik için bu kelime ile başlamalıdır!
  if (Incoming_Text.substring(0, 5) == "Send:") {
    Text_To_EEPROM = Incoming_Text.substring(5, Incoming_Text.length());
    delay(10);
    write_String_to_EEPROM(0, Text_To_EEPROM);  // Address "0" içine String data yazılıyor...
    delay(10);
    Text_From_EEPROM = read_String_from_EEPROM(0);  // Address "0" içinden String data okunuyor...
    delay(100);
    Process_Incoming_Text();  //String data, panelde gösteriliyor...
  }
}

//Gelen Text bilgisinin hazırlanması...
void Process_Incoming_Text() {
  delay(500);
  Serial.println("Gelen text : ");
  Serial.println(Incoming_Text);
  Serial.println();
  int str_len = Text_From_EEPROM.length() + 1;
  char char_array[str_len];
  Text_From_EEPROM.toCharArray(char_array, str_len);
  strcpy(Text[0], char_array);
  Incoming_Text = "";
  Text_To_EEPROM = "";
  Text_From_EEPROM = "";
}



//EEPROM yazma
void write_String_to_EEPROM(char add, String data) {
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++) {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0');  //Sonlandırma karekteri ekle...
  EEPROM.commit();
}

//EEPROM okuma
String read_String_from_EEPROM(char add) {
  int i;
  char data[100];  //Max 100 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 500)  //Sonlandırma karekterine kadar oku...
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}

/*
İletişim:
e-posta: bilgi@ronaer.com
https://www.instagram.com/dr.tronik2022/   
YouTube: Dr.TRonik: www.youtube.com/c/DrTRonik
PCBWay: https://www.pcbway.com/project/member/shareproject/?bmbno=A0E12018-0BBC-4C
*/
