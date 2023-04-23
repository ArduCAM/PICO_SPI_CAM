add_executable(Arducam_mini_2mp_plus_videostreaming
        Arducam_MINI_2MP_Plus_Videostreaming.cpp 
        )

# Pull in our pico_stdlib which pulls in commonly used features
include_directories(../../ArduCAM)
target_link_libraries(Arducam_mini_2mp_plus_videostreaming
	pico_stdlib
	hardware_dma
	hardware_i2c
	hardware_pwm
	ArduCAM
)
pico_enable_stdio_usb(Arducam_mini_2mp_plus_videostreaming 1)
pico_enable_stdio_uart(Arducam_mini_2mp_plus_videostreaming 0)
# create map/bin/hex file etc.
pico_add_extra_outputs(Arducam_mini_2mp_plus_videostreaming)


