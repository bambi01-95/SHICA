#include <stdio.h>
#include <pigpio.h>

#define LED_PIN 17       // GPIO17
#define BUTTON_PIN 27    // GPIO27

int main() {
    // pigpioの初期化
    if (gpioInitialise() < 0) {
        printf("pigpioの初期化に失敗しました\n");
        return 1;
    }

    // ピンの設定
    gpioSetMode(LED_PIN, PI_OUTPUT);
    gpioSetMode(BUTTON_PIN, PI_INPUT);
    gpioSetPullUpDown(BUTTON_PIN, PI_PUD_UP);  // ボタンのプルアップ設定

    printf("ボタンを押すとLEDがON/OFFします。\n");

    int ledState = 0;  // LEDの状態（0: OFF, 1: ON）
    int lastButtonState = 1;  // ボタンの前回の状態（1: 押されていない, 0: 押された）
    
    while (1) {
        int currentButtonState = gpioRead(BUTTON_PIN);  // 現在のボタンの状態

        // ボタンが押された瞬間を検出する（押された -> 離されたときの立ち上がりエッジ）
        if (lastButtonState == 1 && currentButtonState == 0) {
            // LEDの状態を切り替える
            ledState = !ledState;
            gpioWrite(LED_PIN, ledState);
            printf("LEDの状態: %s\n", ledState ? "ON" : "OFF");
        }

        // ボタンの状態を更新
        lastButtonState = currentButtonState;

        // デバウンスのための遅延
        gpioDelay(50000);  // 50msの遅延
    }

    // pigpioの終了
    gpioTerminate();

    return 0;
}