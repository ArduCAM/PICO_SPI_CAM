# PICO_SPI_CAM Examples
The example demonstrates how to use C or python language to drive the SPI Camera mini camera on the Pcio platform. 
This demonstration is made for ArduCAM_Mini_2MP/5MP, and it needs to be used in conjunction with PC software.

# Download driver
```bash
git clone https://github.com/ArduCAM/PICO_SPI_CAM.git
```

# Use C language to drive SPI Camera
To configure the development environment, please refer to the official manual, the link is as follows.
```bash
https://www.raspberrypi.org/documentation/rp2040/getting-started/#getting-started-with-c
```
- Compile
Select the driver to be compiled, the default is Arducam_MINI_2MP_Plus_Videostreaing.
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/1.png)
```bash
cd  PICO_SPI_CAM/C
mkdir build
cd build
cmake ..
make 
```
Copy PICO_SPI_CAM/C/build/Examples/Arducam_MINI_2MP_Plus_Videostreaing/Arducam_mini_2mp_plus_videostreaming.uf2 to pico to run the test.
Open the PC software, configure the port number, baud rate, camera model, and click Capyure to see the image.
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/2.png)

# Use Python to drive SPI camera
## Note: The development system is Windows. 
To configure the development environment, please refer to the official manual, the link is as follows.
```bash
https://circuitpython.org/
```
Development software download link.
```bash
https://thonny.org/
```

## Copy the file under PICO_SPI_CAM/Python/ path except boot.py to the Pico device.
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/3.png)
## Open Thonny software, select environment and port number. 
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/4.png)
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/5.png)
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/10.png)
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/11.png)
## Copy the boot.py from the PICO_SPI_CAM/Python/ path to the Pico device, restart the Pico, open the device manager, and use the new port number for USB communication
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/12.png)	
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/13.png)	
## Open the camera driver. 
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/6.png)
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/7.png)
## Click Run, it displays [48], CameraType is OV2640, and SPI interface OK means that the camera is initialized. 
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/8.png)
## Open hostapp.exe in the HostApp file,Select the port number for USB communication and click Image. 
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/9.png)