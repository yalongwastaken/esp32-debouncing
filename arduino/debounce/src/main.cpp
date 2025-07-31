// Author: Anthony Yalong
// Description: LED toggle control with software debounced button input.
//              Implements stable state detection to eliminate mechanical switch bounce.
//              LED toggles on each confirmed button press (rising edge detection).

#include <Arduino.h>

// Pin definitions
#define LED_NUM 2
#define SWITCH_NUM 4

// Software debounce class
class SoftwareDebounce {
  // Private variables
  private:
    uint8_t pin;                    // GPIO pin number
    bool inverted;                  // True for active-low logic
    bool cur_state;                 // Current raw pin state
    bool prev_stable_state;         // Last confirmed debounced state
    unsigned long last_change_time; // Timestamp of last state change
    unsigned long delay_time;       // Debounce delay in milliseconds
    bool state_changed;             // Flag for new debounced state available

  public:
    // Constructor
    SoftwareDebounce(uint8_t switch_pin = SWITCH_NUM, bool inverted_logic = true, unsigned long debounce_delay_time = 50) {
      pin = switch_pin;
      inverted = inverted_logic;
      last_change_time = 0;
      delay_time = debounce_delay_time;
      state_changed = false;
      pinMode(pin, inverted ? INPUT_PULLUP : INPUT_PULLDOWN);
      cur_state = prev_stable_state = read_raw_state(pin);
    }

    // Update debounce
    void update(void) {
      int raw_state = read_raw_state(pin);
      unsigned long cur_time = millis();
      
      // Reset timer if state changed
      if (raw_state != cur_state) {
        cur_state = raw_state;
        last_change_time = cur_time;
      }
      
      // Check if state has been stable long enough
      if (!state_changed && (cur_time - last_change_time) > delay_time) {
        if (cur_state != prev_stable_state) {
          prev_stable_state = cur_state;
          state_changed = true;
        }
      }
    }

    // Detect rising edge (button press)
    bool get_rising_edge(void) {
      return get_changed() && prev_stable_state;
    }

    // Detect falling edge (button release)
    bool get_falling_edge(void) {
      return get_changed() && !prev_stable_state;
    }

  private:
    // Read pin with inversion logic
    bool read_raw_state(int to_read) {
      bool raw = digitalRead(to_read);
      return inverted ? !raw : raw;
    }

    // Check if state changed (clears flag after reading)
    bool get_changed(void) {
      if (state_changed) {
        state_changed = false;
        return true;
      }
      return false;
    }
};

// Global objects and variables
SoftwareDebounce debouncer(SWITCH_NUM, true, 50);
bool led_state = false;

// Setup function
void setup() {
  Serial.begin(115200);
  Serial.println("Software Debounce (Arduino): Starting program.");
  pinMode(LED_NUM, OUTPUT);
  digitalWrite(LED_NUM, led_state);
}

// Main loop
void loop() {
  debouncer.update();
  
  // Toggle LED on button press
  if (debouncer.get_rising_edge()) {
    led_state = !led_state;
    digitalWrite(LED_NUM, led_state);
    Serial.println("Software Debounce (Arduino): " + String(led_state ? "LED ON" : "LED OFF"));
  }
}