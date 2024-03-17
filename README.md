# 🎧 Arduino_Giga_R1___wav_player 🎧
The goal of this project is to have a minimalistic mp3-like player for the Arduino Giga R1 using files of the "wav" format. It is only compatible with a Arduino Giga R1 so keep that in mind.
<br>
<br>
## How to use ❓
First you need your Arduino Giga R1 and download the file onto your Pc to compile it to your Giga.<br>
After that you open the file in the Arduino IDE, compile it to your giga.⚠️ATTENTION⚠️ If you compile, everything on the Arduino will be gone. <br>
You maybe need to download libaries if your IDE says so. If thats the case here's the downloadlink to the libaries that I use:
- https://reference.arduino.cc/reference/en/libraries/arduino_advancedanalog/
- https://www.arduinolibraries.info/libraries/arduino_usb-host-mbed5<br>

After that you need a usb stick. The maximum capactity of the stick can be (I think) 32 Gigabyte. For reference I use a 16 Gig stick. The stick must be formated in FAT32 in order to be detected. You can store everything you want on the stick. The player will only use the wav files. <br>
I only tested it with the 3.5mm audiojack on the board. You can try to use the pins A12(DAC0) and A13(DAC1), but I can't guaranty that it will work, since I haven't tested it with.<br><br>
💥⚠️MASSIVE ATTENTION⚠️💥: Please only use speakers that have an amplifier inside or plugged in on an amp. If you don't do this your Arduino will break since it doesn't have an amp on the board. the only speakers that could work without an amp are headphone-speakers because of the resistance.
<br>
<br>
## Commands 🔧
If you have set up everything correctly with no compilation errors and no error on the "Serial monitor" you can use commands to modify the playback.<br>
Commands that currently are available:
- help: Shows you all commands that can be used.
- list: Lists all existing .wav files on the usb stick in PA_15.
- play: Starts at song 1 and plays all songs in order.
- shuffle: Creates a random order for the .wav files.
- pause: Pauses the current audio playback.
- continue: Continues the current audio playback.
- skip: Skips the current playback and stars the next in the queue.

more will be added in future ⌛.
<br>
<br>
## Having issues 🚩
Write me on discord (the.chair) since I am online there most of the day and isses will get fixed faster there. You can also leave a issue here on this repo, but I will check 2-3 time only a week, so keep that in mind.
<br>
<br>
## Want to download or modify the file ❓
Sure, do what you want with it. You don't need to mention me or so. Use as you want, but I am not liable or responsible for any fixes after modification, but you can still ask me to help you if you dont know what what means.
<br>
<br>
## Having ideas or want to submit to this project ❓
Since I am very limited due to time issues, it will unlikely be able for me currently to collab right now. But I dont know what near future holds since I am at Univerisity 🏛️🎓
