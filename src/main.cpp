#include "WiFi.h"
#include "esp_wifi.h"
#include <Arduino.h>
#include <AsyncElegantOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#define FORMAT_LITTLEFS_IF_FAILED true

#define Button 0

// Replace with your network credentials
const char* ap_ssid     = "ESP32-Access-Point";
const char* ap_password = "123456789";

const char* sta_ssid     = "dxxy16-402-2";
const char* sta_password = "dxxy16402";

AsyncWebServer server(80);

/* 中断服务函数 */
void isr()
{
  Serial.println("Button pressed");
}

/* HTTP 服务初始化 */
void Server_Init()
{
  server.end();
  server.serveStatic("/", LittleFS, "/")
  .setCacheControl("max-age=31536000");  //挂载静态网页
  server.onNotFound([](AsyncWebServerRequest* request) {
    File file = LittleFS.open("/404/index.html", "r");
    request->send(LittleFS, "/404/index.html", "text/html", false);
    file.close();
  });
  AsyncElegantOTA.begin(&server);  //挂载 OTA
  server.begin();                  //开启 HTTP服务
  Serial.println("HTTP server started");
}

/* WiFi 初始化 */
void WiFi_Init()
{
  Serial.println("WiFi Setting ...");

  /* 同时开启 STA 和 AP 模式 */
  WiFi.mode(WIFI_MODE_APSTA);
  /*-------- Soft-AP --------*/
  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  /*-------- Station --------*/
  // WiFi.begin();
  WiFi.begin(sta_ssid, sta_password);

  //读取默认STA配置
  wifi_config_t current_conf;
  esp_wifi_get_config((wifi_interface_t)ESP_IF_WIFI_STA, &current_conf);
  Serial.print("Connecting to ");
  Serial.println((char*)current_conf.sta.ssid);
  Serial.print("password:");
  Serial.println((char*)current_conf.sta.password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/* 串口输出文件系统信息 */
void printDirectory(File dir, int numTabs)
{
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print("  ");
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    }
    else {
      Serial.print("  ");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

/* 文件系统初始化 */
void FS_Init()
{
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  unsigned int totalBytes = LittleFS.totalBytes();
  unsigned int usedBytes  = LittleFS.usedBytes();

  Serial.println("File sistem info.");
  Serial.print("Total space:      ");
  Serial.print(totalBytes);
  Serial.println("byte");
  Serial.print("Total space used: ");
  Serial.print(usedBytes);
  Serial.println("byte");

  // 输出文件系统信息
  File dir = LittleFS.open("/");
  printDirectory(dir, 0);
  Serial.println("File Content:");
  Serial.println("");

  //输出测试文件信息
  File file = LittleFS.open("/test.txt");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void setup()
{
  /* GPIO */
  Serial.begin(115200);
  pinMode(Button, INPUT);
  attachInterrupt(Button, isr, FALLING);

  /* LittleFS */
  FS_Init();

  /* WiFi */
  WiFi_Init();
  Server_Init();
}

void loop()
{
  delay(1);
}