/*
参考 帰宅したら自動で解錠するスマートロックを自作する①～オートロック編～
https://ppdr.softether.net/smartlock-1
*/
#include <stdio.h>
#include <string.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ルータ設定
IPAddress ip(192, 168, 10, 102);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);

//施錠認識
char num[5] = "";
const char pass[5] = "1111";

// ESP8266のピン番号
const int led_close = 16;    //D0
const int led_open = 15;     //D8
const int key_0 = 3;         //RX
const int key_1 = 5;         //D1
const int key_2 = 4;         //D2
const int servo_pin = 14;    //D5
const int close_button = 12; //D6
const int open_button = 13;  //D7

Servo myservo; // サーボオブジェクトを生成
int open_sensor = LOW;
int open_sensor_last = LOW;
int close_sensor = LOW;
int close_sensor_last = LOW;

int key_0_sensor = LOW;
int key_0_sensor_last = LOW;
int key_1_sensor = LOW;
int key_1_sensor_last = LOW;
int key_2_sensor = LOW;
int key_2_sensor_last = LOW;

ESP8266WebServer server(80); //サーバーオブジェクト

void door_open()
{
    myservo.write(0);
    delay(1000);
    myservo.write(180);
    digitalWrite(led_close, LOW);
    digitalWrite(led_open, HIGH);
    Serial.println("opend");
}

void door_close()
{
    myservo.write(180);
    delay(1000);
    myservo.write(0);
    digitalWrite(led_open, LOW);
    digitalWrite(led_close, HIGH);
    Serial.println("closed");
}

void handle_open()
{
    door_open();
    server.send(200, "text/html", "opened");
}

void handle_close()
{
    door_close();
    server.send(200, "text/html", "closed");
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    //固定IPで運用するときの設定
    WiFi.config(ip, gateway, subnet);
    WiFi.begin("2.4Ghz", "a12345678");

    // WiFiに接続するまで待つ
    Serial.println("");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Webサーバを設定
    server.on("/open", handle_open);
    server.on("/close", handle_close);
    server.begin();

    // 開閉センサの入力を内部プルアップにする
    pinMode(close_button, INPUT_PULLUP);
    pinMode(open_button, INPUT_PULLUP);
    pinMode(key_0, INPUT_PULLUP);
    pinMode(key_1, INPUT_PULLUP);
    pinMode(key_2, INPUT_PULLUP);
    pinMode(led_close, OUTPUT);
    pinMode(led_open, OUTPUT);

    // サーボ変数をピンに割り当て
    myservo.attach(servo_pin);
    door_close();
}

void loop()
{
    // サーバとして待ち受ける
    server.handleClient();

    // 現在のドアの開閉を検知
    // LOW＝閉
    // HIGH＝開
    close_sensor = digitalRead(close_button);
    open_sensor = digitalRead(open_button);
    key_0_sensor = digitalRead(key_0);
    key_1_sensor = digitalRead(key_1);
    key_2_sensor = digitalRead(key_2);

    // 「開」→「閉」になったタイミングでサーボモータを回す
    if (close_sensor == LOW && close_sensor_last == HIGH)
    {
        door_close();
    }
    else if (open_sensor == LOW && open_sensor_last == HIGH)
    {
        door_open();
    }

    if (key_0_sensor == LOW && key_0_sensor_last == HIGH)
    {
        strcat(num, "0");
    }
    else if (key_1_sensor == LOW && key_1_sensor_last == HIGH)
    {
        strcat(num, "1");
    }
    else if (key_2_sensor == LOW && key_2_sensor_last == HIGH)
    {
        strcat(num, "2");
    }
    if (strlen(num) >= 4)
    {
        if (strcmp(num, pass) == 0)
        {
            door_open();
            memset(num, 0, sizeof(num));
        }
        else
        {
            for (int i = 0; i < 3; i++)
            {
                digitalWrite(led_close, HIGH);
                delay(200);
                digitalWrite(led_close, LOW);
                delay(200);
            }
            Serial.print("num=");
            Serial.print(num);
            Serial.print("\n");
            memset(num, 0, sizeof(num));
        }
    }
    delay(150); //key

    key_0_sensor_last = key_0_sensor;
    key_1_sensor_last = key_1_sensor;
    key_2_sensor_last = key_2_sensor;
    close_sensor_last = close_sensor;
    open_sensor_last = open_sensor;
}