# weOS
System for wristwatches based on Arduino

This repository contains nothing more than children's creativity 🙂.
The development was carried out in 2015-2017 and its goal was a cheap alternative to electronic wristwatches.

The project was brought to a working prototype, but without a body, since it was not possible to obtain a satisfactory printing result.

Later, more advanced components were purchased for the project, and it was decided to refactor the existing source code, but due to lack of time, the project remained at the idea stage.

[Prototyping photos](pics/README.md) available.

## Branches
- [clock-face](https://github.com/taranovegor/weos/tree/clock-face) — pre-development of watch dial.
- [debug](https://github.com/taranovegor/weos/tree/debug) — various debugging scripts created during the development process.
- [v2](https://github.com/taranovegor/weos/tree/v2) — complete rethink of the code base.
- [menu](https://github.com/taranovegor/weos/tree/menu) — menu library development.

## Device information
### Technical specification
#### v0 — based on [Arduino Uno](https://docs.arduino.cc/hardware/uno-rev3)
| Component | Model name                   |
|-----------|------------------------------|
| CPU       | Atmega328P                   |
| Screen    | PCD8544 (1.6" 84*48 B/W LCD) |
#### v1 — based on [Arduino Uno](https://docs.arduino.cc/hardware/uno-rev3)
| Component | Model name                           |
|-----------|--------------------------------------|
| CPU       | Atmega328P                           |
| Clock     | DS1302                               |
| Screen    | ILI9163C (1.44" 128*128px Color LCD) |
| Bluetooth | HC-05                                |
| Vibration | similar to Nokia 3310                |
| Battery   | 260mAh                               |
#### v2 — based on [Teensy 3.2](https://www.pjrc.com/store/teensy32.html)
| Component     | Model name                              |
|---------------|-----------------------------------------|
| CPU           | MK20DX256VLH7 (ARM Cortex-M4 at 72 MHz) |
| Screen        | SSD1327 (1.5" 128*128px Color OLED)     |
| Bluetooth     | JDY-08 BLE                              |
| Accelerometer | GY-61                                   |
| Vibration     | similar to iPhone 4                     |
| Battery       | 260mAh (planned 300-320mAh)             |

### Electricity consumption information
| Consumer                        | 3.7V  | 5V  |
|---------------------------------|-------|-----|
| Atmega328P-PU                   | 7.2mA | N/A |
| Screen (ILI9163C)               | 0.7mA | N/A |
| Screen backlight (50% (125))    | 5.1mA | N/A |
| Vibration (50% (125))           | N/A   | N/A |
| Bluetooth                       | N/A   | N/A |
| Bluetooth (connected to device) | N/A   | N/A |

### Screen (system?) power consumption depending on brightness
| Brightness (%) | Consumption (mAh) | Battery life (h) |
|----------------|-------------------|------------------|
| 0              | 7.50              | N/A              |
| 5              | 7.99              | N/A              |
| 10             | 8.35              | N/A              |
| 15             | 8.65              | N/A              |
| 20             | 9.01              | N/A              |
| 25             | 9.33              | N/A              |
| 30             | 9.69              | N/A              |
| 35             | 10.00             | N/A              |
| 40             | 10.37             | N/A              |
| 45             | 10.68             | N/A              |
| 50             | 11.04             | N/A              |
| 55             | 11.36             | N/A              |
| 60             | 11.74             | N/A              |
| 65             | 12.07             | N/A              |
| 70             | 12.40             | N/A              |
| 75             | 12.73             | N/A              |
| 80             | 13.12             | N/A              |
| 85             | 13.45             | N/A              |
| 90             | 13.82             | N/A              |
| 95             | 14.16             | N/A              |
| 100            | 14.51             | N/A              |
| Avg            | 11.05             | N/A              |

## Used libraries
- [Adafruit_GFX](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit_PCD8544](https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library)
- [TFT_ILI9163C](https://github.com/sumotoy/TFT_ILI9163C) ❤️
- [MsTimer2](https://github.com/PaulStoffregen/MsTimer2)
- [MemoryFree](https://github.com/sudar/MemoryFree)
- [DS1302](https://github.com/Treboada/Ds1302)
- [HC05](https://github.com/jdunmire/HC05)
- [SSD_13XX](https://github.com/sumotoy/SSD_13XX)

## Documents
- [`Статистика.xlsx`](docs/Статистика.xlsx) — electricity consumption information.
- [`Яркость.docx`](docs/Яркость.docx) — measurements of electricity consumption depending on screen brightness.
- [`hm-11_bluetooth40_en.pdf`](docs/hm-11_bluetooth40_en.pdf)  — documentation for bluetooth modules HM series.
- [`ordered.csv`](docs/ordered.csv)  — list of ordered parts.

## Schematic & Layout Design
I couldn't find the files, I think they were lost 😢. All that remains is a photo of the developed printed circuit board. As far as I know, it was developed in [DipTrace](https://diptrace.com/) or Sprint Layout.

## 3D Model
The 3D models was developed using the [Компас-3D](https://kompas.ru/).

[`wclock_body.m3d`](model/wclock_body.m3d), [view on Sketchfab](https://skfb.ly/oKZsR)
![wClock — body (front side)](model/wclock_body_front.jpg)
![wClock — body (back side)](model/wclock_body_back.jpg)
[`wclock_back_cover.m3d`](model/wclock_back_cover.m3d), [view on Sketchfab](https://skfb.ly/oKZsX)
![wClock — back cover (front side)](model/wclock_back_cover_front.jpg)
![wClock — back cover (back side)](model/wclock_back_cover_back.jpg)
[`wclock_buttons.m3d`](model/wclock_buttons.m3d), [view on Sketchfab](https://skfb.ly/oKZtq)
![wClock — buttons](model/wclock_buttons.jpg)

## Special thanks
- To my father for his support and help.
- For [sumotoy](https://github.com/sumotoy) for really cool libraries for displays.
