# LilyGo_T_Display_S3-Radio
Radio based on ESP32 S3 - LilyGo with Display 170x320

The prior version was developed in 2021 when platformio was based on arduino v2: https://github.com/ThomasH-W/ESP32-Radio-TTGO.
After moving to Arduino v3 basically nothing worked anymore and I had to re-establish the proejct setup from scratch using pioarduino.
In addition the guideline from schreibfaul1 did the final trick to get the fundamentals up and runnning again: https://github.com/schreibfaul1/ESP32_Arduino_ESPIDF/

The project is using an ESP32S3 with a better display. The Display is now attached to the board in a more solid way making it easier to mount into any housing.
https://www.lilygo.cc/products/t-display-s3

If you want to use a diufferent board please keep in mind to use an ESP32 with PSRAM as it is required by https://github.com/schreibfaul1/ESP32-audioI2S.
