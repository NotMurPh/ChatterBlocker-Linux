# ChatterBlocker-Linux

Blocks keyboard or any other device chatters/keybounce in linux

### What is a chatter ??

Well simply put when your stupid keyboard types twice or more after a single keypress 😡

### Why another chatter blocker though ?

First of all its written in c++ which means speed go brrrrrrrrrrrrr 💨 , but jokes aside there is only one linux chatter blocker that i found on github which is this one https://github.com/finkrer/KeyboardChatteringFix-Linux which is written in python and at the time i my self tried using it but since im a dum dum and the project wasn't well documented for me to understand how to use it and that it wasn't updated in like 3 years made me think that it was broken and it doesn't work hence i started writing my own and in the proccess i learned how do i have to use it which i have to say really really pissed me off 😤 but either way i finnished the project with the added bounes offff drum roll plssss....

- Way lighter on resources 🤏🏻
- Faster 🏃🏼‍♂️
- GameMode ️🕹️ ( Read about it [here](https://github.com/NotMurPh/ChatterBlocker-Linux?tab=readme-ov-file#gamemode-) )
- DelayedMode ⏱  ( Read about it [here](https://github.com/NotMurPh/ChatterBlocker-Linux?tab=readme-ov-file#delayedmode-) )
- Supports any device with keys ⌨️
- Showing proper logs 💬
- Better documented 🧾

### How does it work ?

#### NormalMode : 

I use libevdev library to read events from your device file e.g `/dev/input/event5` and then i create a virtual device which basically replicates your device but filters the chatters in the specified threshold which it starts ticking after each key up event and then it grabs your device so no one else can use it hence `xorg` uses the virtual one instead and if that didn't work or you are on wayland or somthing unknown like that you can also use the newly created device file like `/dev/input/event?` manually

#### GameMode :

And for the GameMode ️🕹️ well you just send a SIGUSR1 signal to ChatterBlocker to toggle the mode ON/OFF , GameMode then being ON stops the chatter blocking for `W A S D` keys why ? because in games usually these keys are being held for movement and when for example you press the `W` key to move forward and it decides to chatter , the second key down event responsable for starting the movement again after the chatter gets blocked by ChatterBlocker preventing you from moving forward , so basically GameMode lets the `W A S D` keys to chatter in game which isn't that important and ultimatly you can have a hotkey to toggle the mode for chatting as well 🙂

#### DelayedMode : 

This mode is interesting 🤔 basically it fixes every issue with chatters and chatterblockers including the holding issue and movement in games basically making it look like that chatters never happened 🎊 but unfourtunatly it adds delay to the key up events 😢 and that delay is determened by the threashold that you set in the start of the program so if you set 40ms threashold you get 40ms of key release delay 

#### Demo :

And if this is all confusing to you here is a demo 🎬 that showcases every mode in different senarios :

[![Video](https://i.imgur.com/vplxtO8.png)](https://www.youtube.com/watch?v=Ij7ffdDo89g)

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
then you can launch the program using a command like this but first read the instructions down blow 🫠

```
./ChatterBlocker /dev/input/by-id/usb-Logitech_G513_Carbon_Tactile_0B5238613437-event-kbd 30ms
```

***Attention! you may need to use sudo or give root access for it to work***

Required arguments in order : 
- Device file path ( Which you need to find and replace to your own device )
- Chatter threshold ( The time which you can not repeat a keydown after the last keyup event for that key )

It is recommended to use a device file from `/dev/input/by-id/` as it has consistant names and do not changes after each restart 
and also i have to mention that there might be two different devices with your keyboard name on it but if you pay close attention you will notice that one has `if01` attached to the name of it that one is responsable for handeling media keys 🎶 and handeling multitouch ☝🏻 more than 6 simultaneous keys so that means if you press 8 keys at the same time only the last 2 of those keys will be sent through this device so you basically dont want to use this device for chatter blocking and use the other one instead or run two different instances of the ChatterBlocker for each one unfortunately ChatterBlocker does not support two different devices at the same time 😢

you can find your threshold value by first setting threshold to somthing like 100ms and repeat pressing a key a fast as you can then according tho the logs lower the value until it doesn't block your fast keypresses keep in mind that lowering threshold too much may not block all of your chatters for me 30ms is where 99% my chatters are blocked and i cant repeat pressing the key any faster than that

If you get `Faild to use the virtual device!` error ❌ or you are still using your own keyboard device with chatters or you cant type/use your keyboard you can start using the virtual chatterless device manually either by using `xorg-xinput` program and disabling your main device so `xorg` starts using the virtual one or by adding the virtual device file path to the `xorg` config files which then you have to make sure that this program runs before `xorg` or basically use `/dev/input/event?` created by program in anyother way!

And to turn on any mode right away you can use these flags `--delayed` , `--gamemode`

And to toggle the modes on the fly 🐥 you can send `SIGUSR1` and `SIGUSR2` signals to ChatterBlocker to toggle GameMode and DelayedMode like so :

```bash
# Keep in mind that you need to use sudo here if you started the ChatterBlocker using sudo

# Toggle GameMode :
sudo pkill -USR1 ChatterBlocker

# Toggle DelayedMode :
sudo pkill -USR2 ChatterBlocker
```

***Note! you may need to reassign your hotkeys after changing to the new device***

Also here is a nice website that you can test your keyboard in https://keyboard.dmitrijs.lv/

### Enjoy your chatterless device 🙃

Fell free to contact me if you had any problem or an idea for an improvement

### Credits

This program is sponsored by ChatGPT 🌸

Thanks to @arian8j2 for nothing 😆
