# shuvabihani

Shuvabihani is a custom esp32-c3 environmental dashboard and smart alarm clock featuring a WIFI ntp sync with the mx switch inputs and BME680 air quality monitoring. 

## Overview:
Shuvabihani is a Hackclub-BLARE inspired smart alarm clock which introduces ntp time syncing with multi alarm state function. Also this clock measures the environmental state like temperature,humidity and gas resistance and displays the real data in the clock. Also it has some cool smart features like master kill switch that disables all alarms set in the clock. 

### Features:

1. NTP-synced time:
    The clock automatically connects to your home wifi and fetches the precise local time which eliminates the manual clock drift. 

2. Environmental Features:
    It integrates a BME680 sensor to track the temperature, humidity and VOC gas resistance and displays in human readable air quaity ratings. (fresh,fair,poor)

3. Multi alarms:
    The clock supports uoto 5 configurable alarms with onboard UI menu to set hour, min. and active states.

4. Programmed inputs:
    The clock uses the four mx switch in the back very efficiently by using multi button combinations for complex actions like snooze and other normal toggles replacing the need of more buttons and switches.

### Basic clock functions:

The clock uses 4 MX switches for navigation and the alarm control.

- SW1(bottom-left): Up / Increase time
- SW2(bottom-right): Down / Decrease time
- SW3(top-left): Select / Enter Menu
- SW4(bottom-right): Back / Cancel
- Dismiss/Stop Alarm: Press SW1+SW4 simultaneously
- Snooze Alarm(5 mins.): Press SW2+SW3 simultaneously
- Master kill switch: Press SW1+SW4 from the clock screen to disable all alarms.(note: this will work only when you are in clock screen)  

## Hardware assembly and flashing

### 1. Mounting the pcb and components into enclosure:

- Solder the all smd and tht components as required in pcb.
- There are mounting holes in the pcb. So fabricate the pcb and mount them into the base using 4xM3 screws. 
- The front part with display is a bit tricky. You need to mount both the display and the front enclosure into the base using the 2xM2.5 screws at the same time as in the 3Dmodel.
- The front lid is mounted into the base using the 2xM3 screws in the left part and the right part is adjusted automatically with the display mounts.
- And the back part is adjusted by screwing a M3 screws in the middle-top then the 4xM1 screws.
- Then the last part the keycaps are auto adjusted as i have made the different mounting holes for 4 switches which can be inserted as in mechanical keyboard.

### 2. Firmware update and dependencies:

You must install these dependencies in your cpp env. or Arduino ide before flashing the code.
- `Adafruit_GFX`
- `Adafruit_ST7789`
- `Adafruit_BME680`
- `Adafruit_Sensor`
- `WiFi`
- `SPI`
- `Wire`

Also note that you must update your WiFi password and ssid in the firmware before flashing so that the time auto updates fron ntp connecting to your wifi.
code:
``` cpp
    const char* ssid ="insert_ssid_of_wifi";
    const char* password ="insert_password_of_wifi";
```

Also you should update your `gmtOffset_sec` in the code so that it matches with your country's code. For eg: 

``` cpp 
    const long  gmtOffset_sec = 20700; // GMT+5:45 for  Nepal my country(put your country's gmt respectively here)
```

### 3. Flashing the Esp32-c3:
1. Open firmware.ino in the firmware folder in arduino ide.
2. Update the wifi credentials and the timezone gmt in firmware.
3. Select Xiao-esp32c3 as your boarrd.
4. Compile and upload using usb-c.

## How it Works:

The main working principle of the clock is inspired from [Hack Club BLARE](https://blare.hackclub.com/) but i have modified some things and they are:

1. I2C/SPI Pin Constraint:
The standard xiao ESP32-C3 has 11 usable pins. so to support a high res. TFT display, I2C environmental sensor, 4 mechanical switches, and piezo buzzer,a strict pin economy was required. So the display's Reset and Backlight lines are hardwired to the 3.3V power traces on the PCB. This helped two pins (D6 and D8) to be used as  custom I2C bus for the BME680 without dropping any other components.

2. UI change:
The normal clock fails because a delay()  pauses the entire mcu. So shuvabihani uses a non blocking loop and a certain UI states (like CLOCK_VIEW, MENU_SELECT_ALARM, MENU_SET_HOUR, etc.). This thing allows the clock to pulse buzzer, fetch NTP data, read  BME680 , and listen for multi-button  inputs simultaneously without freezing the UI.


## Images:
### CAD:
1. Photos:

2. Link:
[FUSION 360](https://a360.co/4z7ZS5Q)

### PCB: 


## Credits:
- [Hack Club BLARE](https://blare.hackclub.com/): For the hardware inspiration.