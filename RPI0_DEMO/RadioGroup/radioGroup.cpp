#include <stdio.h>
#include <pigpio.h>

#define LED_PIN 17       // LED pin
#define BUTTON_PIN 27    // Button pin
#define COMM_PIN 22      // Communication pin

// Function to initialize GPIO pins
void setup() {
    gpioSetMode(LED_PIN, PI_OUTPUT);       // Set LED pin as output
    gpioSetMode(BUTTON_PIN, PI_INPUT);     // Set button pin as input
    gpioSetPullUpDown(BUTTON_PIN, PI_PUD_UP);  // Enable pull-up for button
    gpioSetMode(COMM_PIN, PI_INPUT);       // Set communication pin as input
    gpioSetPullUpDown(COMM_PIN, PI_PUD_DOWN);  // Enable pull-down for communication pin
}

// Function to debounce button
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

    int ledState = 0;  // Track the LED state (0 = OFF, 1 = ON)

    while (1) {
        // Check if the button is pressed
        if (debounceButton(BUTTON_PIN)) {
            // Toggle the LED state
            ledState = !ledState;
            gpioWrite(LED_PIN, ledState);

            // Print the current state of the LED
            printf("LED is now %s.\n", ledState ? "ON" : "OFF");
            
            // Notify other Raspberry Pis
            gpioSetMode(COMM_PIN, PI_OUTPUT);
            gpioWrite(COMM_PIN, 1);
            gpioDelay(100000);  // 100ms delay
            gpioWrite(COMM_PIN, 0);
            gpioSetMode(COMM_PIN, PI_INPUT);
        }

        // Check if another Pi has sent a signal to turn off the LED
        if (gpioRead(COMM_PIN) == 1) {
            gpioWrite(LED_PIN, 0);  // Turn off the local LED
            printf("Received signal from another Pi. LED turned OFF.\n");
            ledState = 0;  // Update the LED state
        }

        gpioDelay(10000);  // 10ms delay to reduce CPU usage
    }

    gpioTerminate();
    return 0;
}
