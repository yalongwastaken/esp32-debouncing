# ESP32 Debouncing

A collection of ESP32 button debouncing implementations demonstrating various programming approaches, from basic switch handling to advanced interrupt-driven debouncing with RTOS integration. These examples showcase different debouncing techniques including finite state machines, hybrid approaches, and interrupt-based solutions.

## Project Structure

### Arduino Framework (`arduino/`)
* `debounce_hybrid/` - Hybrid debouncing approach combining polling and timing
* `switch/` - Basic switch handling implementation

### ESP-IDF Framework (`esp-idf/`)
* `debounce_fsm/` - Finite state machine-based debouncing implementation
* `debounce_hybrid/` - Hybrid debouncing approach using FreeRTOS features
* `debounce_interrupt/` - Advanced interrupt-driven debouncing with timer callbacks
* `switch/` - Basic switch handling using ESP-IDF GPIO drivers

## Development Environment

**Primary Development:** PlatformIO IDE

**Compatibility:**
* **Arduino projects:** Compatible with Arduino IDE
* **ESP-IDF projects:** Compatible with native `idf.py` toolchain

**Hardware Tested:** ESP32-WROOM-32E module

## Potential Improvements & Add-ons

* Multiple button support with individual debouncing
* Capacitive touch sensor integration
* Button press pattern recognition (single, double, long press)
* External interrupt optimization for ultra-low power applications
* Button matrix support for multiple inputs
* Configuration persistence using NVS storage
* Remote button state monitoring via WiFi
* Hardware debouncing circuit examples

## Author

Anthony Yalong