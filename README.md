# weOS
System for wristwatches based on Arduino

[Prototyping photos](pics/README.md)

## Device information
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

## Documents
- [`Статистика.xlsx` — electricity consumption information.](docs/Статистика.xlsx)
- [`Яркость.docx` — measurements of electricity consumption depending on screen brightness.](docs/Яркость.docx)

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

## Schematic & Layout Design
I couldn't find the files, I think they were lost 😢. All that remains is a photo of the developed printed circuit board. As far as I know, it was developed in [DipTrace](https://diptrace.com/) or Sprint Layout.
