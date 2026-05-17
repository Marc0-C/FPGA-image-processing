# Members

Timothy Cocks 33117357 tcoc0001@student.monash.edu Timothycocks

Hugo Knights-Burke 34990224 hkni0005@student.monash.edu hkni0005

Marco Carrera 36841633 mcar0097@student.monash.edu marco-carrera-monash
 
Flynn Kennedy 34977856 fken0004@student.monash.edu iHateGithubDesktop

---
# Instructions

## Hardware

1. Plug VGA cable into the VGA port of the DE-10 Lite, and a VGA compatible monitor.

2. Connect the ESP32 Cam into the DE-10 Lite GPIO Ports specified.

3. Connect the DE-10 Lite to your computer using the USB Type B to Type A Cable.


## Software

1. Open the quartus project file ".qpf"

2. In quartus, click on the .v IP Block files "sdram_pll", "vga_pll", and "pixel_buffa" and "finish" to create the IP Block qip and BB files.

3. Open platform designer and find the "nios_system.qsys" file

4. Click "Generate HDL", "Generate" and afterwards "close" and "finish"

5. Compile the quartus program, clicking the play icon.

6. With your DE-10 lite plugged in, open the Programmer via the icon or through the "tools" dropdown menu

7. With the correct .sof selected (This shuld be populated automatically) press "start" to program the DE-10 Lite.

8. Now open "Nios II Software Build Tools for Eclipse", set your workspace as the project folder.

9. Open the eclipse project in the software folder. Ensure you see all of "proccessor0", "proccessor1", "proccessor2" & their corresponding bsp folders.

10. In run configuration ensure Nios II Hardware is selected, double check that your board is found under "Target Connection" and that both checkboxes under the "System ID Checks" section are selected.

11. Press "Apply" and then "Run" to begin the program.

---
# Usage:
- Rotate the DE10 on the Y-axis to rotate the images being displayed in quad mode
- Press KEY0 to reset the Nios II Processors
- Press the pushbutton on the ESP-CAM to reset the ESP-CAM
- Press KEY1 to toggle between single and quad image modes

## Single Image Mode
- Use SW 0:1 to select which of the four image variants are displayed (Normal, Flipped, Blurred, Edge Detection)

## Quad Image Mode
- Use SW 0:1 to select displayed image for top left quadrant
- Use SW 2:3 to select displayed image for top right quadrant
- Use SW 4:5 to select displayed image for bottom left quadrant
- Use SW 6:7 to select displayed image for bottom right quadrant


---
# Tasks and Functionalities

ESP-CAM connected to DE-10 Lite over SPI and through FPGA based Nios-II processors, camera feed displayed on VGA Display with no artifacts. FPS benchmark displayed on onboard HEX and accelerometer data printed to console.
Uses multiple Nios II proccessors running in parallel. Capable of displaying either a single 320x240 image, or four downscaled 160x120 images in a 2x2 grid. Various image proccessing/filters can be applied to displayed images.
