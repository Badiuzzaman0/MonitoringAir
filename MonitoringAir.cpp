#include <Firebase_ESP_Client.h>
#include <Arduino.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define TURBIDITY_PIN 33

#define API_KEY "AIzaSyDqhmcjQu031hDcKVs0TZ14QIBCqzrSfNc"
#define DATABASE_URL "https://projectskripsi-ccef8-default-rtdb.firebaseio.com/"

const char *ssid = "Rozain";
const char *pass = "16111996";
int lastRequest = 0;

TaskHandle_t sensorInputHandle;
TaskHandle_t ledTaskHandle;
TaskHandle_t firebaseSetHandle;

void setupWifi();

void SensorTask(void *parameter);
void ledTask(void *parameter);
void FirebaseSet(void *parameter);


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

volatile int turbidityValue;

void setup() {
    // put your setup code here, to run once:
    pinMode(BUILTIN_LED, OUTPUT);
    Serial.begin(115200);
    vTaskDelay(1000);

    setupWifi();

    ESP_LOGI("SETUP", "Create freertos task!");

    // Create RTOS task
    xTaskCreate(SensorTask, "DHT11 Task", 2048, NULL, 1, &sensorInputHandle);
    xTaskCreate(ledTask, "LED Task", 2048, NULL, 2, &ledTaskHandle);

    //error nya ketika execute code dibawah ini:
    // xTaskCreate(FirebaseSet, "Firebase Task", 2048, NULL, 2, &firebaseSetHandle);
}

void loop() {
    // put your main code here, to run repeatedly:
    if (millis() - lastRequest > 10000) {
        if (WiFi.status() != WL_CONNECTED) {
            setupWifi();
        } else {
            ESP_LOGI("WIFI", "WiFi is already connected...");
        }

        lastRequest = millis();
    }
}

void SensorTask(void *parameter) {
    // EventBits_t clientBits;

    for (;;) {
        turbidityValue = analogRead(TURBIDITY_PIN);
        Serial.println(turbidityValue);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void ledTask(void *parameter) {
    for (;;) {
        if (WiFi.status() == WL_CONNECTED) {
            digitalWrite(BUILTIN_LED, HIGH);
            vTaskDelay(1000);
            digitalWrite(BUILTIN_LED, LOW);
            vTaskDelay(1000);
        } else {
            digitalWrite(BUILTIN_LED, HIGH);
            vTaskDelay(200);
            digitalWrite(BUILTIN_LED, LOW);
            vTaskDelay(200);
        }
    }
}

void FirebaseSet(void *parameter) {
    for (;;) {
        Serial.printf("Set float... %s\n", Firebase.RTDB.setFloat(&fbdo, F("/test/Turbidity"), turbidityValue) ? "ok" : fbdo.errorReason().c_str());
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void setupWifi() {
    vTaskDelay(10);
    // We start by connecting to a WiFi network
    ESP_LOGI("WIFI", "Connecting to %s", ssid);

    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(500);
        ESP_LOGI("WIFI", ".");
    }
    ESP_LOGI("WIFI", "WiFi is connected!");

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    //Connect to Firebase
    config.api_key = API_KEY;

    config.database_url = DATABASE_URL;

    Firebase.signUp(&config, &auth, "", "");
    config.token_status_callback = tokenStatusCallback;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}
