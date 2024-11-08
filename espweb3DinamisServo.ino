#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

const char* defaultSSID = "";
const char* defaultPassword = "";
//const char* ssid = "";
//const char* password = "";

Servo servo_4;
WiFiServer server(80);
int sensorValue,sensorValue2 = 0;  // Variable untuk menyimpan data dari sensor
int pServo;
int pos=0;
String ssid = defaultSSID;
String password = defaultPassword;
String lastValue = "Initial Value"; // Global variable to store the last value
IPAddress local_IP(192, 168, 1, 100); // IP statis
IPAddress gateway(192, 168, 1, 1);    // Gateway
IPAddress subnet(255, 255, 255, 0);   // Subnet mask

void setup() {
    pinMode(2,INPUT_PULLUP);
    pinMode(3,INPUT_PULLUP);
    pinMode(4,OUTPUT);
    pinMode(5,OUTPUT);
    servo_4.attach(4);
    Serial.begin(115200);
    //if (!WiFi.config(local_IP, gateway, subnet)) {
    //   Serial.println("Gagal mengatur IP statis");}
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting...");
    }
    
    Serial.println("Connected to WiFi");
    WiFi.softAP(ssid.c_str(), password.c_str());
    Serial.println("Hotspot aktif dengan SSID: " + ssid + " dan Password: " + password);
  
  // Menyiapkan halaman web
    //server.("/", handleRoot);
    //server.on("/set", HTTP_POST, handleSetCredentials);
    server.begin();
    Serial.println(WiFi.localIP());
    Serial.println("Server dimulai...");
    Serial.println("Servo diset...");
    initialServo();
}

void initialServo()
{	
        
       for (pos = 0; pos <179; ++pos)  
    // in steps of 1 degree
       { 
        //lcd.setCursor(0, 0);
        //lcd.print("Initial Position");
        servo_4.write(pos);              
        delay(5);                       
       }
  		
      for (pos = 179; pos > 0; --pos)  
    // in steps of 1 degree
       { 
        //lcd.setCursor(0, 0);
        //lcd.print("Initial Position");
        servo_4.write(pos);              
        delay(5);                       
       }

     
}

int turnRight(int pRight)
{  	 
    if (pRight>=0)
    		{ ++pRight;
          servo_4.write(pRight);
    		delay(20);}
     return pRight;   
}

int turnLeft(int pLeft)
{  	 
    if (pLeft<=180)
    		{ --pLeft;
          servo_4.write(pLeft);
    		delay(20);}
     return pLeft;   
}



void loop() {
    //sensorValue = analogRead(A0);
    sensorValue=digitalRead(2);
    //if (digitalRead(2)==LOW)
    //    pos=turnRight(pos);
    //if (digitalRead(3)==LOW)
    //    pos=turnLeft(pos);
    //if (sensorValue>180) sensorValue=180;
    //if (sensorValue<0) sensorValue=0;
    sensorValue2=digitalRead(3);
    WiFiClient client = server.available();
    if (client) {
        String request = client.readStringUntil('\r');
        Serial.println(request);
        client.flush();

        // Check if client is sending data (POST request)
        if (request.indexOf("POST /setData") != -1) {
            
             String body = "";
            while (client.available()) {
                body += client.readStringUntil('\n');
            }
            Serial.println(body);
             // Parse the body to get the value
            if (body.indexOf("value=") != -1) {
                sensorValue = body.substring(body.indexOf("value=") + 6).toInt();
                if (sensorValue>180) sensorValue=180;
                if (sensorValue<0) sensorValue=0;
                Serial.print("Received sensor value: ");
                Serial.println(sensorValue);
                if (sensorValue>=pos) 
                    while (pos<sensorValue)
                         {pos=turnRight(pos);
                           if (pos>180) pos=180; exit;}
                else
                    while (pos>sensorValue)
                         {pos=turnLeft(pos);
                           if (pos<0) pos=0; exit;}
            }
        }

        // Send HTML page with current sensor value
        client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
        client.print("<!DOCTYPE html><html>");
        client.print("<head><title>ESP8266 Web Server</title></head>");
        client.print("<h1>Last Value: " + lastValue + "</h1>");
        client.print("<body><h1>Posisi sudut Servo terbaca ESP: ");
        client.print(sensorValue/2);
        client.print("</h1><form method='POST' action='/setData'>");
        client.print("Set Posisi Servo (0- 180): <input type='text' name='value'><input type='submit'></form></body></html>");
        client.print("<br><h2>Current Sensor Value (digital Read): ");
        client.print(digitalRead(2)); // Menampilkan nilai sensor yang dibaca
        client.print("</h2></body></html>");
        client.print("<br><h2>posisi servo : ");
        //client.print("<meta http-equiv='refresh' content='10'>"); // Auto-refresh every 5 seconds
        client.print(pos); // Menampilkan nilai posisi servo
        client.print("</h2></body></html>");
        
        client.stop();
        Serial.println("Client Disconnected.");
    }
}

