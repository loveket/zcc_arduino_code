#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>        // 本程序使用 ESP8266WiFi库
#include <ESP8266WiFiMulti.h>   //  ESP8266WiFiMulti库
#include <ESP8266WebServer.h>   //  ESP8266WebServer库
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <DHT.h>
#define PIN  D2 
#define NUMPIXELS 90 
#define SLOWSPEED 100 // Time (in milliseconds) to pause between pixels
#define FLOWSPEED 30

#define DHTPIN D6 
#define DHTTYPE DHT11
ESP8266WiFiMulti wifiMulti;     // 建立ESP8266WiFiMulti对象,对象名称是'wifiMulti'
ESP8266WebServer esp8266_server(3333);// 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）
IPAddress local_IP(192, 168, 0, 33); // 设置ESP8266-NodeMCU联网后的IP
IPAddress gateway(192, 168, 0, 1);    // 设置网关IP（通常网关IP是WiFI路由IP）
IPAddress subnet(255, 255, 255, 0);   // 设置子网掩码
IPAddress dns(192,168,0,1);           // 设置局域网DNS的IP（通常局域网DNS的IP是WiFI路由IP）
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Ticker ticker;
DHT dht(DHTPIN, DHTTYPE);
int pinState =1;                 // 存储引脚状态用变量
int LedLightness=200;//设置灯亮度
int typeNum=0;
int red=150;
int green=150;
int blue=150;
float wetGet=0.0;
float tempGet=0.0;
int startflag=1;
void setup() {
  Serial.begin(9600);
  //pinMode(PIN,OUTPUT);
  //初始化http服务
  initWebServer(); 
  ticker.attach(10, resetWifiConn);
  //连接温度传感器解开
  dht.begin();
}
void initWebServer(){
  // 设置开发板网络环境
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Failed to ESP8266 IP"); 
  }
  wifiMulti.addAP("zcc333", "15144099398b"); // 将需要连接的一系列WiFi ID和密码输入这里
  Serial.println("Connecting ..."); 
  int i = 0;
  bool isExe=false;                                 
  while (wifiMulti.run() != WL_CONNECTED) {  // 此处的wifiMulti.run()是重点。通过wifiMulti.run()，NodeMCU将会在当前
    delay(1000);                             // 环境中搜索addAP函数所存储的WiFi。如果搜到多个存储的WiFi那么NodeMCU
    Serial.print(i++); Serial.print(' ');    // 将会连接信号最强的那一个WiFi信号。
    if(i>=5){
      isExe=true;
      break;
     }
  }                                          // 一旦连接WiFI成功，wifiMulti.run()将会返回“WL_CONNECTED”。这也是
                                             // 此处while循环判断是否跳出循环的条件。
  if(!isExe){
    // WiFi连接成功后将通过串口监视器输出连接成功信息 
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP8266-NodeMCU的IP
  esp8266_server.begin();                           // 启动网站服务
  esp8266_server.on("/off", handleLEDOff);  // 设置处理LED控制请求的函数'handleLED'
  esp8266_server.on("/on",  handleLEDOn);  // 设置处理LED控制请求的函数'handleLED'
  esp8266_server.on("/lightness" , handleLEDLightness);
  esp8266_server.on("/ledtype" ,handleLEDType);
  esp8266_server.on("/isonlineheart" ,handleOnlineHeart);
  esp8266_server.onNotFound(handleNotFound);        // 设置处理404情况的函数'handleNotFound'
  //连接温度传感器解开api
  esp8266_server.on("/gettw", getTempWet);
  Serial.println("HTTP esp8266_server started");//  告知用户ESP8266网络服务功能已经启动
   }
  
}
//******************************
//******************************
//******************************
//******************************
//http-api
//******************************
//******************************
//void testHttpRequest(){
//  WiFiClient client;
//  HTTPClient http;
//  http.begin(client, serverName);
//  int httpResponseCode = http.GET();
//      
//      if (httpResponseCode>0) {
//        Serial.print("HTTP Response code: ");
//        Serial.println(httpResponseCode);
//        String payload = http.getString();
//        Serial.println(payload);
//      }
//      else {
//        Serial.print("Error code: ");
//        Serial.println(httpResponseCode);
//      }
//      // Free resources
//      http.end();
// }
// void testHandle(){
//  Serial.println("test http success");
//  }
//void resetHttpServer(){
//  Serial.println("reset http server");
//  esp8266_server.begin();  
//  }
void resetWifiConn(){
  if(!WiFi.isConnected()){
    Serial.println("reset wifi conn.........");
    initWebServer();
   }
  }
void handleLEDOff(){
  Serial.println("off");
  pinState=0;
  esp8266_server.send(200, "text/plain", "shut led");
}
void handleLEDOn(){
  Serial.println("on");
   pinState=1;
   esp8266_server.send(200, "text/plain", "open led");
}
void handleLEDLightness(){
  //Serial.println(esp8266_server.arg("lightnum"));
  LedLightness=esp8266_server.arg("lightnum").toInt();
  esp8266_server.send(200, "text/plain", "success set up lightness");
  }
 void handleOnlineHeart(){
  esp8266_server.send(200, "text/plain", "online");
  }
// 设置处理404情况的函数'handleNotFound'
void handleNotFound(){
  esp8266_server.send(404, "text/plain", "404: Not found"); // 发送 HTTP 状态 404 (未找到页面) 并向浏览器发送文字 "404: Not found"
}
void handleLEDType(){
 pixels.clear();
  typeNum=esp8266_server.arg("typenum").toInt();
  red=esp8266_server.arg("red").toInt();
  green=esp8266_server.arg("green").toInt();
  blue=esp8266_server.arg("blue").toInt();
  //Serial.println(typeNum);
  esp8266_server.send(200, "text/plain", "success switch");
 }
 void getTempWet(){
  StaticJsonDocument<32> doc;
  // 添加键值对到JSON文档
  doc["temp"] = tempGet;
  doc["wet"] = wetGet;
  // 序列化JSON文档为字符串
  String jsonBuffer;
  serializeJson(doc, jsonBuffer);
  // 通过串口发送JSON字符串
  Serial.println(jsonBuffer);
  esp8266_server.send(200, "application/json", jsonBuffer);
  }
 //获取温湿度
 void getTempAndWet(){
  wetGet = dht.readHumidity();//读湿度
  tempGet = dht.readTemperature();//读温度，默认为摄氏度
 }
 //******************************
//******************************
//******************************
//******************************
//灯光执行逻辑
//******************************
//******************************
void loop() {
  
  //wifi是否断开
//  if(WiFi.status()!=WL_CONNECTED){
//    Serial.println("wifi disconn................");
//    initWebServer();
//    }
  //pixels.clear(); // Set all pixel colors to 'off'
  esp8266_server.handleClient();
  //获得温度湿度,连接温度传感器解开
  getTempAndWet();
  //灯光逻辑
  if(pinState==1){
     //开启灯光
       pixels.begin();
       if(startflag==1){
      //开机动画
      ledStart();
      startflag=2;
    }
    if(typeNum==0){//常亮模式
       normal();
     }else if(typeNum==1){
      flowled(FLOWSPEED);
      }else if(typeNum==2){
      goBack();
      }else if(typeNum==3){
      misssplit();
      }
    //breath();
   }else if(pinState==0){
    pixels.show();
    pixels.clear();
    delay(1000);
   }
}
//*******************************
//*******************************
//*******************************
//*******************************
//灯光代码
//*******************************
//灯光开机动画
void ledStart(){
  pixels.setBrightness(LedLightness);
   for(int i=0;i<NUMPIXELS;i++)//for循环控制几色流水灯，三个for循环就是三色，增加颜色可继续往下copy
  {
    pixels.setPixelColor(i, pixels.Color(0,163,255)); // 改颜色
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(20); 
  }
}
//
void normal(){
  pixels.setBrightness(LedLightness);
   for(int i=0;i<NUMPIXELS;i++)//for循环控制几色流水灯，三个for循环就是三色，增加颜色可继续往下copy
  {
    pixels.setPixelColor(i, pixels.Color(red,green,blue)); // 改颜色
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
  delay(1000);
}
void flowled(int wait){
  pixels.setBrightness(LedLightness);
  for(int i=0;i<NUMPIXELS;i++)//for循环控制几色流水灯，三个for循环就是三色，增加颜色可继续往下copy
  {
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,167,255)); // 改颜色
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(wait); // 控制流水灯的速度
  }
  pixels.clear();
//  for(int i=0;i<NUMPIXELS;i++){
//    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
//    pixels.setPixelColor(i, pixels.Color(150,150,0)); // Moderately bright blue color.
//    pixels.show(); // This sends the updated pixel color to the hardware.
//    delay(wait); // Delay for a period of time (in milliseconds).
//  }
// for(int i=0;i<NUMPIXELS;i++){
//    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
//    pixels.setPixelColor(i, pixels.Color(150,0,150)); // Moderately bright blue color.
//    pixels.show(); // This sends the updated pixel color to the hardware.
//    delay(wait); // Delay for a period of time (in milliseconds).
//  }
}
void goBack(){
  pixels.setBrightness(LedLightness);
  for(int i=0;i<NUMPIXELS;i++)//for循环控制几色流水灯，三个for循环就是三色，增加颜色可继续往下copy
  {
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,150,150)); // 改颜色
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(5); // 控制流水灯的速度
  }
  for(int i=NUMPIXELS-1;i>=0;i--)//for循环控制几色流水灯，三个for循环就是三色，增加颜色可继续往下copy
  {
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(150,0,150)); // 改颜色
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(5); // 控制流水灯的速度
  }
 }
 void misssplit(){
  pixels.setBrightness(LedLightness);
  for(int i=0;i<NUMPIXELS/2;i++)//for循环控制几色流水灯，三个for循环就是三色，增加颜色可继续往下copy
  {
    for(int j=i;j<i+2;j++){
       // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(j, pixels.Color(0,150,150)); // 改颜色
      pixels.setPixelColor(NUMPIXELS-j, pixels.Color(150,0,150)); // 改颜色
      pixels.show(); // This sends the updated pixel color to the hardware.
      delay(30); // 控制流水灯的速度
     }
  }
   for(int i=0;i<NUMPIXELS/2;i++)//for循环控制几色流水灯，三个for循环就是三色，增加颜色可继续往下copy
  {
    for(int j=i;j<i+2;j++){
       // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(j, pixels.Color(200,50,150)); // 改颜色
      pixels.setPixelColor(NUMPIXELS-j, pixels.Color(10,150,200)); // 改颜色
      pixels.show(); // This sends the updated pixel color to the hardware.
      delay(30); // 控制流水灯的速度
     }
  }
 }
//void breath(){
//  int timeNums=5;
//  int ledLight=200;
//  while(timeNums>=0){
//    pixels.setPixelColor(NUMPIXELS, pixels.Color(150,150,0));
//    pixels.setBrightness(ledLight);
//    pixels.show();
//    delay(200);
//    ledLight-=30;
//    timeNums--;
//  }
//  while(timeNums!=5){
//    pixels.setPixelColor(NUMPIXELS, pixels.Color(0,150,150));
//    pixels.setBrightness(ledLight);
//    pixels.show();
//    delay(200);
//    ledLight+=30;
//    timeNums++;
//  }
//}
