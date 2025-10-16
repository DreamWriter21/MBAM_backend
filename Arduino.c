#define Serial Serial0
#define ARDUINO_USB_CDC_ON_BOOT 1

#include "esp_camera.h"
#include <HardwareSerial.h>
#include <Adafruit_NeoPixel.h>

// Add these at the top of the file, after the includes
//static unsigned long photoCount = 0;

// accepter les modifs gpt.
// SXGA ça marche pas.
// Essai avec QVGA: ca marche mais serial affiche des erreurs

// COmprendre pourquoi j'ai du binaire en série...

// --- Config ESP32-CAM ---
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 34
#define SIOD_GPIO_NUM 15
#define SIOC_GPIO_NUM 16
#define Y9_GPIO_NUM 14
#define Y8_GPIO_NUM 13
#define Y7_GPIO_NUM 12
#define Y6_GPIO_NUM 11
#define Y5_GPIO_NUM 10
#define Y4_GPIO_NUM 9
#define Y3_GPIO_NUM 8
#define Y2_GPIO_NUM 7
#define VSYNC_GPIO_NUM 36
#define HREF_GPIO_NUM 35
#define PCLK_GPIO_NUM 37

#define PIN 21                   // LED Ring pin
#define NUM_PIXELS 8             // Remplace par le nombre de LEDs sur ton anneau
static int led_intensity = 128;  // Valeur par défaut

static unsigned long photoCount = 0;

Adafruit_NeoPixel strip(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

// --- Config UART modem ---
HardwareSerial modem(1);  // UART1
#define MODEM_TX 18
#define MODEM_RX 17
#define MODEM_BAUD 115200

// --- URL Webhook (remplacer par ta propre URL Webhook.site) ---
//const char* SERVER_URL = "http://stark-bayou-33265-3bff07a2985d.herokuapp.com/upload";
const char *SERVER_URL = "http://dev.ouisnap.com/upload";

bool sendAT(const char *cmd, const char *expected, uint32_t wait = 2000) {
  Serial.println(cmd);
  modem.println(cmd);
  unsigned long t0 = millis();
  String buffer = "";
  while (millis() - t0 < wait) {
    while (modem.available()) {
      char c = modem.read();
      Serial.write(c);
      buffer += c;
    }
  }
  return buffer.indexOf(expected) >= 0;
}

// Version "simple envoi + affichage"
void sendAT(const char *cmd, uint32_t wait = 2000) {
  Serial.println(cmd);
  modem.println(cmd);
  unsigned long t0 = millis();
  while (millis() - t0 < wait) {
    while (modem.available())
      Serial.write(modem.read());
  }
}

void sendMiniDataChunk(const uint8_t* data, size_t len, size_t chunkSize = 1024) {
  size_t sent = 0;
  while (sent < len) {
    size_t sendNow = (len - sent) > chunkSize ? chunkSize : (len - sent);
    modem.write(data + sent, sendNow);
    sent += sendNow;
    delay(20);  // éviter saturation UART
  }
}

void sendDataChunk(const uint8_t *data, size_t len, size_t chunkSize = 500000, String filename = "4Gcam_number_1.jpg") {
  size_t sent = 0;
  Serial.println("avant le while");

   Serial.printf("Photo capturée, taille: %u bytes\n", len);
  while (sent < len) {
    size_t sendNow = (len - sent) > chunkSize ? chunkSize : (len - sent);

    String url = String("AT+HTTPPARA=\"URL\",\"") + SERVER_URL + String("?filename=" + filename + "\"");
    sendAT(url.c_str(), 500);


    String httpDataCmd = "AT+HTTPDATA=" + String(sendNow) + ",30000";
    modem.println(httpDataCmd.c_str());

    // Wait for modem to be ready
    if (!modem.find("DOWNLOAD"))
    {
      Serial.println("❌ HTTPDATA non prêt !");
      return;
    }

    sendMiniDataChunk(data + sent, sendNow);

    modem.flush();
    delay(100);

    sendAT("AT+HTTPACTION=1", 15000);

    sent += sendNow;
    Serial.flush();
    delay(20);
  }
}

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  // config.frame_size = FRAMESIZE_SVGA;  // 800x600 - better balance between quality and size
  config.frame_size = FRAMESIZE_QXGA;  // 1600x1200
  config.jpeg_quality = 14;           // Reduced quality to ensure reliable transmission
  // config.fb_count = 1;                        // Use 2 frame buffers for better stability
  config.fb_count = 2;  // Use 2 frame buffers for better stability

  config.grab_mode = CAMERA_GRAB_LATEST;    // Grab frame when buffer is empty
  config.fb_location = CAMERA_FB_IN_PSRAM;  // Store frame buffers in PSRAM

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Erreur init camera!");
    while (true)
      delay(1000);
  } else {
    Serial.println("Camera initialized successfully!");

    sensor_t *s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated

    s->set_brightness(s, 0);      // -2 to 2
    s->set_contrast(s, 1);        // -2 to 2
    s->set_saturation(s, 1);      // -2 to 2
    s->set_special_effect(s, 0);  // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, 1);        // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);        // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);         // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_exposure_ctrl(s, 0);   // 0 = disable , 1 = enable
    s->set_ae_level(s, 0);        // -2 to 2
    s->set_aec_value(s, 900);     // 0 to 1200
    s->set_gain_ctrl(s, 1);       // 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);        // 0 to 30

    if (s->id.PID == OV3660_PID) {
      s->set_vflip(s, 1);  // flip it back
      // s->set_brightness(s, 1);   // up the brightness just a bit
      // s->set_saturation(s, -2);  // lower the saturation
    }
  }
}

void setupModem() {

  Serial.println("Init modem...");
  Serial.flush();
  delay(10);
  sendAT("AT", 1000);
  sendAT("ATE0", 500);  // désactive l’écho
  sendAT("AT+CPIN?", 1000);
  sendAT("AT+CSQ", 1000);
  // sendAT("AT+CNMP=38", 1000);  // mode réseau Auto
  sendAT("AT+CNMP=2", 1000);  // LTE only
  sendAT("AT+CGDCONT=1,\"IP\",\"mobiledata\"", 1000);

  // Attendre l’enregistrement réseau
  Serial.println("Attente enregistrement réseau...");
  bool registered = false;
  for (int i = 0; i < 30 && !registered; i++) {
    modem.println("AT+CREG?");
    if (sendAT("AT+CREG?", "+CREG: 0,1", 1000) || sendAT("AT+CREG?", "+CREG: 0,5", 1000)) {
      registered = true;
    } else {
      delay(1000);
    }
  }

  if (!registered) {
    Serial.println("⚠️ Réseau non prêt !");
  } else {
    Serial.println("✅ Réseau enregistré !");
    greenRing();
  }

  sendAT("AT+NETCLOSE", 1000);
  sendAT("AT+NETOPEN", 8000);
  delay(1000);
  sendAT("AT+IPADDR", 1000);

  // Vérifier le contexte PDP
  sendAT("AT+CGATT?");
  sendAT("AT+CGACT?");

  sendAT("AT+HTTPTERM", 500);
  sendAT("AT+HTTPINIT", 1000);
  sendAT("AT+HTTPPARA=\"CID\",8", 500);

  String urlCmd = String("AT+HTTPPARA=\"URL\",\"") + SERVER_URL + "\"";
  sendAT(urlCmd.c_str(), 500);
  sendAT("AT+HTTPPARA=\"CONTENT\",\"image/jpeg\"", 500);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  delay(3000);
  Serial.println("Démarrage...");
  Serial.flush();
  delay(10);
  modem.begin(MODEM_BAUD, SERIAL_8N1, MODEM_RX, MODEM_TX);

  setupCamera();
  setupModem();
}

void greenRing() {

  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 128, 0));
    strip.show();  // Afficher la couleur
    delay(200);
  }

  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();  // Afficher la couleur
  delay(2000);

  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 128, 0));
  }
  strip.show();  // Afficher la couleur
  delay(200);

  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();  // Afficher la couleur
}

void loop() {

  // Allumer toutes les LEDs avec l'intensité actuelle
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, strip.Color(led_intensity, led_intensity, led_intensity));
  }
  strip.show();  // Afficher la couleur
  delay(2000);

  Serial.println("taking picture");
  delay(1000);
  camera_fb_t *fb = esp_camera_fb_get();
  delay(1000);

  // Éteindre toutes les LEDs
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, 0);  // Noir (off)
  }
  strip.show();

  if (!fb) {
    Serial.println("Erreur capture camera !");
    Serial.flush();
    esp_camera_fb_return(fb);
    delay(2000);
    return;
  }

  // Create filename with timestamp and counter
  String filename = "4Gcam_number_1_img_" + String(++photoCount) + ".jpg";
  Serial.println("filename = " + filename);
 

  sendDataChunk(fb->buf, fb->len, 51200, filename);

  delay(100);
  esp_camera_fb_return(fb);

  Serial.println("Photo envoyée. Nouvelle capture dans 30s...");
  Serial.flush();
  delay(30000);
}