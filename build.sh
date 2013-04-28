



# gcc -g -w fakeston.c evemu.c wayland-util.c evdev.c evdev-touchpad.c filter.c -o fakeston  -lm -ldl
gcc -g fakeston.c fakeston_run.c evemu.c wayland-util.c compositor.h evdev.c evdev-touchpad.c filter.c -o fakeston_run  -lm -ldl


