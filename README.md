# ChatterBlocker-Linux

Blocks keyboard or any other device chatters/keybounce in linux

### What is a chatter ??

Well simply put when your stupid keyboard types twice or more after a single keypress ๐ก

### Why another chatter blocker though ?

First of all its written in c++ which means speed go brrrrrrrrrrrrr ๐จ , but jokes aside there is only one linux chatter blocker that i found on github  which is this one https://github.com/finkrer/KeyboardChatteringFix-Linux which is written in python and i my self tried using it but since im a dum dum   and the project wasn't well documented for me to understand how to use it and that it wasn't updated in like 3 years made me think that it was broken     and doesn't work hence i started writing my own and in the proccess i learned how i have to use it which i have to say really really pissed me off ๐ค     but either way i finnished the project with the added bounes offff drum roll plssss....

- Way lighter on resources ๐ค๐ป
- Faster ๐๐ผโโ๏ธ
- Fixing key holds not working after chatters ๏ธ๐ ๏ธ
- Gamemode ๏ธ๐น๏ธ
- Supports any device with keys โจ๏ธ
- Showing proper logs ๐ฌ
- Better documented ๐งพ

### How does it work ?

I use libevdev library to read from your device file e.g `/dev/input/event5` and create a virtual device which replicates your device but filters the chatters in the specified threshold after keyup events and then it grabs your device so no one else can use it hence `xorg` uses the virtual one instead and if that didn't work or you are on wayland or somthing unknown like that you can also use the newly created `/dev/input/event?` device manually

And for the gamemode ๏ธ๐น๏ธ well you just send a SIGUSR1 signal to ChatterBlocker and ChatterBlocker toggles the gamemode to on which then stops the chatter blocking for `W A S D` keys why ? because in games usually these keys are being held , and when for example you press `W` key and it decides to chatters the key down event responsable for starting the hold events gets blocked by ChatterBlocker preventing the game from detecting a key hold and preventing you from moving forward until it is detected as a hold and fixed in ChatterBlocker which this process takes a while and it is huge in games where every thing is fast paced and since i couldn't fix the issue without reproducing the chatter i came up with this alternative way which lets the `W A S D` keys chatter in game which isn't that important and ultimatly the choice is yours ๐

### How do i install and use ?

Just download latest release binary from the release section of the github page or clone and compile the project using these commands :

```
git clone https://github.com/NotMurPh/ChatterBlocker-Linux.git
cd ChatterBlocker-Linux
# you need to install libevdev on your system for a successfull compile 
# and then you might need to make a symlink like so
# sudo ln -sf /usr/include/libevdev-1.0/libevdev /usr/include/libevdev
g++ -l evdev ChatterBlocker.cpp -o ChatterBlocker
```
then you can launch the program using a command like this but first read the instructions down blow ๐ซ 

```
./ChatterBlocker /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_0B5238613437-event-kbd 30ms
```

***Attention! you may need to use sudo or give root access for it to work***

Required arguments in order : 
- Device file path ( Which you need to find and replace to your own device )
- Chatter threshold ( The time which you can not repeat a keydown after the last keyup event for that key )

It is recommended to use a device file from `/dev/input/by-id/` as it has consistant names and do not changes after each restart 
and also i have to mention that there might be two different devices with your keyboard name on it but if you pay close attention you will notice that one has `if01` attached to the name of it that one is responsable for handeling media keys ๐ถ and handeling multitouch โ๐ป more than 6 simultaneous keys so that means if you press 8 keys at the same time only the last 2 of those keys will be sent through this device so you basically dont want to use this device for chatter blocking and use the other one instead or run two different instances of the ChatterBlocker for each one unfortunately ChatterBlocker does not support two different devices at the same time ๐ข

you can find your threshold value by first setting threshold to somthing like 100ms and repeat pressing a key a fast as you can then according tho the logs lower the value until it doesn't block your fast keypresses keep in mind that lowering threshold too much may not block all of your chatters for me 30ms is where 99% my chatters are blocked and i cant repeat pressing the key any faster than that

If you get `Faild to use the virtual device!` error โ or you are still using your own keyboard device with chatters or you cant type/use your keyboard you can start using the virtual chatterless device manually either by using `xorg-xinput` program and disabling your main device so `xorg` starts using the virtual one or by adding the virtual device file path to the `xorg` config files which then you have to make sure that this program runs before `xorg` or basically use `/dev/input/event?` created by program in anyother way!

And to toggle the gamemode ๏ธ๐น๏ธ whenever you want you can just send SIGUSR1 signal to ChatterBlocker like so :

```
sudo pkill -USR1 ChatterBlocker
# Keep in mind that you need to use sudo if you started the ChatterBlocker using sudo
```

***Note! you may need to reassign your hotkeys after changing to the new device***

Also here is a nice website that you can test your keyboard in https://keyboard.dmitrijs.lv/

### Enjoy your chatterless device ๐

Fell free to contact me if you had any problem or an idea for an improvement

### Credits

This program is sponsored by ChatGPT ๐ธ

Thanks to @arian8j2 for nothing ๐
