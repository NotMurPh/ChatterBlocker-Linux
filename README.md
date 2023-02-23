# ChatterBlocker-Linux

Blocks keyboard or any other device chatters/keybounce in linux

### What is a chatter ??

Well simply put when your stupid keyboard types twice or more after a single keypress 😡

### Why another chatter blocker though ?

First of all its written in c++ which means speed go brrrrrrrrrrrr 💨 , but jokes aside there is only one linux chatter blocker that i found on github which is this one https://github.com/finkrer/KeyboardChatteringFix-Linux which is written in python and i my self tried using it but since im a dum dum and the project wasn't well documented for me to understand how to use it and that it wasn't updated in like 3 years made me think that it was broken and doesn't work hence i started writing my own and in the proccess i learned how i have to use it which i have to say really really pissed me off 😤 but either way i finnished the project with the added bounes offff drum roll plssss.... automating the usage prosses in `xorg` using `xorg-xinput` and removed the need to restart and i have to say that you can use this on other devices too not just a keyboard and it shows logs about blocked chatters and their times

### How does it work ?

I use libevdev library to read from your device file e.g `/dev/input/event5` and create a virtual device which replicates your device but filters the chatters in the specified threshold in the proccess after keydown events and then it uses `xorg-xinput` to disable your main device hence `xorg` uses the virtual one if that didn't work or you are on wayland or somthing unknown like that you can also use the newly created `/dev/input/event?` device manually

### How do i install and use ?

Just download latest release binary from the release section of the github page or clone and compile the project using these commands :

```
git clone https://github.com/NotMurPh/ChatterBlocker-Linux.git
cd ChatterBlocker-Linux
# you need to install libevdev on your system for a successfull compile 
# and if you are on arch you need to make a symlink like this
# sudo ln -sf /usr/include/libevdev-1.0/libevdev /usr/include/libevdev
gcc -lstdc++ -levdev ChatterBlocker.cpp -o ChatterBlocker
```
then you can launch the program using a command like this but first read the requirements down blow 🫠

```
./ChatterBlocker /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_0B5238613437-event-kbd 80ms
```

***Attention! you may need to use sudo or give root access for it to work but you can simply try it***

Required arguments in order : 
- Device file path ( Which you need to find and replace to your own device )
- Chatter threshold ( The time which you can not repeat a keypress after the last keydown event for that key )

It is recommended to use a device file from `/dev/input/by-id/` as it has consistant names and do not changes after restart also i have to mention that i had two keyboard devices in there which only one of them worked

and lastly you need to install `xorg-xinput`  if you haven't already for the automation of switching to virtual device

you can also do it manually either by using `xorg-xinput` ( you just need to disable your main device ) or by adding virtual device file path to `xorg` config files which then you have to make sure that this program runs before `xorg`

***Note! you may need to reassign your hotkeys after changing to new keyboard device***

### Enjoy your chatterless device 🙃

fell free to contact me if you had any problem or an idea for an improvement

### Credits

This program is sponsored by ChatGPT 🌸

Thanks to @arian8j2 for nothing 😆
