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
- Note: The development system is Windows. 
To configure the development environment, please refer to the official manual, the link is as follows.
```bash
https://circuitpython.org/
```
Development software download link.
```bash
https://thonny.org/
```

Copy the files in the PICO_SPI_CAM/Python/ path to the Pico device. 
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/3.png)
Open Thonny software, select environment and port number. 
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/4.png)
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/5.png)
Open the camera driver. 
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/6.png)
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/7.png)
Click Run, it displays [48], CameraType is OV2640, and SPI interface OK means that the camera is initialized. 
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/8.png)
Open the PC software, configure the port number, baud rate, camera model, and click Capyure to see the image. 
![EasyBehavior](https://github.com/UCTRONICS/pic/blob/master/pico/Spi%20Camera/9.png)