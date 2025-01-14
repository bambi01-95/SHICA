#include <stdio.h>
#include <pigpio.h>

#define LED_PIN 17       // LED接続ピン
#define BUTTON_PIN 27    // ボタン接続ピン
#define COMM_PIN 22      // 通信ピン（共有）

void setup() {
    gpioSetMode(LED_PIN, PI_OUTPUT);       // LEDを出力モードに設定
    gpioSetMode(BUTTON_PIN, PI_INPUT);     // ボタンを入力モードに設定
    gpioSetPullUpDown(BUTTON_PIN, PI_PUD_UP);  // ボタンのプルアップ設定
    gpioSetMode(COMM_PIN, PI_INPUT);       // 通信ピンを入力モードに設定
    gpioSetPullUpDown(COMM_PIN, PI_PUD_DOWN);  // プルダウン設定
}

int main() {
    if (gpioInitialise() < 0) {
        printf("pigpioの初期化に失敗しました\n");
        return 1;
    }

    setup();
    printf("ラジオグループのシステムが起動しました。\n");

    while (1) {
        // 自分のボタンが押されたか確認
        if (gpioRead(BUTTON_PIN) == 0) {
            printf("ボタンが押されました！\n");

            // 自分のLEDをONにして通信ピンをHIGHに設定
            gpioWrite(LED_PIN, 1);
            gpioSetMode(COMM_PIN, PI_OUTPUT);
            gpioWrite(COMM_PIN, 1);

            // 通信ピンをしばらくHIGHにして、他のRaspberry Piに通知
            gpioDelay(100000);  // 100ms
            gpioWrite(COMM_PIN, 0);
            gpioSetMode(COMM_PIN, PI_INPUT);
        }

        // 他のRaspberry Piからの信号を確認
        if (gpioRead(COMM_PIN) == 1) {
            gpioWrite(LED_PIN, 0);  // 他のPiがボタンを押したら自分のLEDをOFF
        }

        gpioDelay(10000);  // 10msの遅延
    }

    gpioTerminate();
    return 0;
}
