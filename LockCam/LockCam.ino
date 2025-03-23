#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "base64.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

#define FIREBASE_HOST "https://projectdoor-9a3f5-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "mCbnfmo9gnWZpOK4qlVK3mXqlEnv3W1MtGzRVa4s"
#define FIREBASE_PATH "/captured_image.json"

const char *ssid = "JADELOCK";
const char *password = "Asdfghjkl54321";

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  camera_config_t cam_config;
  cam_config.ledc_channel = LEDC_CHANNEL_0;
  cam_config.ledc_timer = LEDC_TIMER_0;
  cam_config.pin_d0 = Y2_GPIO_NUM;
  cam_config.pin_d1 = Y3_GPIO_NUM;
  cam_config.pin_d2 = Y4_GPIO_NUM;
  cam_config.pin_d3 = Y5_GPIO_NUM;
  cam_config.pin_d4 = Y6_GPIO_NUM;
  cam_config.pin_d5 = Y7_GPIO_NUM;
  cam_config.pin_d6 = Y8_GPIO_NUM;
  cam_config.pin_d7 = Y9_GPIO_NUM;
  cam_config.pin_xclk = XCLK_GPIO_NUM;
  cam_config.pin_pclk = PCLK_GPIO_NUM;
  cam_config.pin_vsync = VSYNC_GPIO_NUM;
  cam_config.pin_href = HREF_GPIO_NUM;
  cam_config.pin_sccb_sda = SIOD_GPIO_NUM;
  cam_config.pin_sccb_scl = SIOC_GPIO_NUM;
  cam_config.pin_pwdn = PWDN_GPIO_NUM;
  cam_config.pin_reset = RESET_GPIO_NUM;
  cam_config.xclk_freq_hz = 10000000;
  cam_config.pixel_format = PIXFORMAT_JPEG;
  cam_config.frame_size = FRAMESIZE_QVGA;
  cam_config.jpeg_quality = 25;
  cam_config.fb_count = 1;

  esp_err_t err = esp_camera_init(&cam_config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }
  delay(1000); // Added delay after camera init

  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
  s->set_brightness(s, 0);
  s->set_contrast(s, 0);
  s->set_saturation(s, 0);
  s->set_special_effect(s, 0);
  s->set_whitebal(s, 1);
  s->set_awb_gain(s, 1);
  s->set_wb_mode(s, 0);
  s->set_exposure_ctrl(s, 1);
  s->set_aec2(s, 0);
  s->set_ae_level(s, 0);
  s->set_aec_value(s, 300);
  s->set_gain_ctrl(s, 1);
  s->set_agc_gain(s, 0);
  s->set_gainceiling(s, (gainceiling_t)6);
  s->set_bpc(s, 0);
  s->set_wpc(s, 1);
  s->set_raw_gma(s, 1);
  s->set_lenc(s, 1);
}

void uploadToFirebase(String base64Data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String firebaseURL = String(FIREBASE_HOST) + FIREBASE_PATH + "?auth=" + FIREBASE_AUTH;
    Serial.print("Firebase URL: ");
    Serial.println(firebaseURL);
    http.begin(firebaseURL);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"image\": \"" + base64Data + "\"}";
    Serial.print("JSON Payload: ");
    Serial.println(jsonPayload);
    int httpResponseCode = http.PUT(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void loop() {
  Serial.println("Capturing image...");
  delay(200);
  camera_fb_t *fb = NULL;
  for (int i = 0; i < 3; i++) {
    fb = esp_camera_fb_get();
    if (fb) {
      break;
    } else {
      Serial.println("Capture attempt failed, retrying...");
      delay(200);
    }
  }

  if (!fb) {
    Serial.println("Camera capture failed after multiple attempts");
    return;
  }

  String imageBase64 = base64::encode(fb->buf, fb->len);
  esp_camera_fb_return(fb);

  if (imageBase64.length() > 500000) {
    Serial.println("Base64 string too large, not uploading.");
    return;
  }

  uploadToFirebase(imageBase64);

  delay(5000); // Reduced delay
}