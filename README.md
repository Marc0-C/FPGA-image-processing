# Members

Timothy Cocks 33117357 tcoc0001@student.monash.edu Timothycocks

Hugo Knights-Burke 34990224 hkni0005@student.monash.edu hkni0005

Marco Carrera 36841633 mcar0097@student.monash.edu marco-carrera-monash
 
Flynn Kennedy 34977856 fken0004@student.monash.edu iHateGithubDesktop

#---
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

6. With your DE-10 lite plugged in, open the Programer via the icon or through the "tools" dropdown menu

7. With the correct .sof selected (This shuld be populated automatically) press "start" to program the DE-10 Lite.

8. Now open "Nios II Software Build Tools for Eclipse", set your workspace as the project folder.

9. Open the eclipse project in the software folder. Ensure you see both "NiosII_vga_Milestone1" & "NiosII_vga_Milestone1_bsp" folders.

10. In run configuration ensure Nios II Hardware is selected, double check that your board is found under "Target Connection" and that both checkboxes under the "System ID Checks" section are selected.

11. Press "Apply" and then "Run" to begin the program.

#---
# Tasks and Functionalities

DE-10 camera functioning over SPI and through FPGA based Nios-II processor, displayed on VGA Display with no artifacts. FPS benchmark displayed on onboard HEX and accelerometer data printed to console.