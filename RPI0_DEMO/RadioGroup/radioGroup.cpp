#include <stdio.h>
#include <pigpio.h>

#define LED_PIN 17       // LED pin
#define BUTTON_PIN 27    // Button pin
#define COMM_PIN 22      // Communication pin

void setup() {
    gpioSetMode(LED_PIN, PI_OUTPUT);       // Set LED pin as output
    gpioSetMode(BUTTON_PIN, PI_INPUT);     // Set button pin as input
    gpioSetPullUpDown(BUTTON_PIN, PI_PUD_UP);  // Enable pull-up for button
    gpioSetMode(COMM_PIN, PI_INPUT);       // Set communication pin as input
    gpioSetPullUpDown(COMM_PIN, PI_PUD_DOWN);  // Enable pull-down for communication pin
}

int debounceButton(int pin) {
    if (gpioRead(pin) == 0) {
        gpioDelay(50000);  // 50ms debounce delay
        if (gpioRead(pin) == 0) {
            return 1;
        }
    }
    return 0;
}

int main() {
    if (gpioInitialise() < 0) {
        printf("Failed to initialize pigpio.\n");
        return 1;
    }

    setup();
    printf("Radio group system started.\n");

    while (1) {
        // Check if the local button is pressed
        if (debounceButton(BUTTON_PIN)) {
            printf("Button pressed on this Pi.\n");

            // Turn on the local LED and notify others
            gpioWrite(LED_PIN, 1);
            gpioSetMode(COMM_PIN, PI_OUTPUT);
            gpioWrite(COMM_PIN, 1);

            // Keep communication pin high for a short time
            gpioDelay(100000);  // 100ms
            gpioWrite(COMM_PIN, 0);
            gpioSetMode(COMM_PIN, PI_INPUT);
        }

        // Check if another Pi has pressed its button
        if (gpioRead(COMM_PIN) == 1) {
            gpioWrite(LED_PIN, 0);  // Turn off the local LED
            printf("Another Pi button pressed. Turning off LED.\n");
        }

        gpioDelay(10000);  // 10ms delay to reduce CPU usage
    }

    gpioTerminate();
    return 0;
}
