/*
参考 帰宅したら自動で解錠するスマートロックを自作する①～オートロック編～
https://ppdr.softether.net/smartlock-1
*/

#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ルータ設定
IPAddress ip(192, 168, 10, 102);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);

// ESP8266のピン番号
const int servo_pin = 14;    //D5
const int open_button = 13;  //D7
const int close_button = 12; //D6

Servo myservo; // サーボオブジェクトを生成
int open_sensor = LOW;
int open_sensor_last = LOW;
int close_sensor = LOW;
int close_sensor_last = LOW;
ESP8266WebServer server(80); //サーバーオブジェクト

void door_open()
{
    myservo.write(180);
    delay(1000);
    myservo.write(0);
}

void door_close()
{
    myservo.write(180);
    delay(1000);
    myservo.write(0);
}

void handle_open()
{
    door_open();
    server.send(200, "text/html", "opened");
    Serial.println("opend");
}

void handle_close()
{
    door_close();
    server.send(200, "text/html", "closed");
    Serial.println("closed");
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

    // 「開」→「閉」になったタイミングでサーボモータを回す
    if (close_sensor == LOW && close_sensor_last == HIGH)
    {
        delay(1000);
        door_close();
    }
    else if (open_sensor == LOW && open_sensor_last == HIGH)
    {
        delay(1000);
        door_open();
    }

    close_sensor_last = close_sensor;
    open_sensor_last = open_sensor;
}