#include <ESP8266WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// Данные Wi-Fi
const char* ssid = "ssid";
const char* password = "password";

// Настройка Telegram-бота
const String BOT_TOKEN = "123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11";
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
const String CHAT_ID = "XXXXXXXXX";

// Объявление объекта для работы с Wi-Fi
WiFiClientSecure espClient;

// Создание объекта для работы с ботом в Telegram
UniversalTelegramBot bot(BOT_TOKEN, espClient);

int botRequestDelay = 100;
unsigned long lastTimeBotRan;

// Структура для работы со светодиодом
struct LED {
  uint8_t pin;
  bool state = LOW;
  int brightness = 255;

  LED(const int pin, bool state) : pin(pin), state(state) {}
  String getStatus() { return (state == LOW) ? "LED is OFF" : "LED is ON"; }
};

// Настройка пинов 
LED led1(5, LOW);
uint8_t LDRpin = A0;
uint8_t PIRpin = 4;

void handleNewMessages(int numNewMessages);

void setup() {

  // Инициализация последовательного порта для отладочных сообщений
  Serial.begin(115200);

  // Получаем сертификат api.telegram.org
  espClient.setTrustAnchors(&cert);  

  // Инициализация пинов на вход/выход
  pinMode(led1.pin, OUTPUT);
  pinMode(LDRpin, INPUT);
  pinMode(PIRpin, INPUT);

  // Подключение к Wi-Fi сети
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Вывод IP-адреса после успешного подключения
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Hello! Telegram bot is ready to go!", "");
}

void loop() {

  // Две переменные для хранения значения уровня света и наличия движения
  int light = analogRead(LDRpin);
  bool motion = digitalRead(PIRpin);

  Serial.println(light);
  Serial.println(motion);

  delay(500);

  if (motion == HIGH && light < 150) {
    digitalWrite(led1.pin, HIGH);
    led1.state = HIGH;
  }

  else if (motion == LOW || light >= 150) {
    digitalWrite(led1.pin, LOW);
    led1.state = LOW;
  }

  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Access denied! Please, contact @thesorcer", "");
      continue;
    }

    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/on") {
      digitalWrite(led1.pin, HIGH);
      bot.sendMessage(chat_id, "LED is ON", "");
      led1.state = HIGH;
      pinMode(led1.pin, OUTPUT);
    }

    else if (text == "/off") {
      digitalWrite(led1.pin, LOW);
      bot.sendMessage(chat_id, "LED is OFF", "");
      led1.state = LOW;
      pinMode(led1.pin, INPUT);
    }

    else if (text == "/start") {
      bot.sendMessage(chat_id, "Hello! I am a smart home control bot for user @thesorcer.\nClick /help for more information.", "");
    }

    else if (text == "/status") {
      bot.sendMessage(chat_id, led1.getStatus(), "");
    }

    else if (text == "/help") {
      bot.sendMessage(chat_id, "/on - switch ON the light\n/off - switch OFF the light\n/status - get LED status", "");
    }
  }
}
