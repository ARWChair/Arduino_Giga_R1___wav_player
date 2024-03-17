#include <string.h>
#include <DigitalOut.h>
#include <FATFileSystem.h>
#include <Arduino_USBHostMbed5.h>
#include <Arduino_AdvancedAnalog.h>

AdvancedDAC dac0(A12);
USBHostMSD msd;
mbed::FATFileSystem usb("usb");

struct wav_header_t {
    char chunkID[4];
    unsigned long chunkSize;
    char format[4];
    char subchunk1ID[4];
    unsigned long subchunk1Size;
    unsigned short audioFormat;
    unsigned short numChannels;
    unsigned long sampleRate;
    unsigned long byteRate;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
};

struct chunk_t {
    char ID[4];
    unsigned long size;
};

struct audioplayer_t {
    FILE *        file;
    int           song_ids[10000];
    bool          open_drive;
    int           sample_size;
    int           samples_count;
    int           songs;
    int           s_id;
    bool          clean;
    bool          ready;
    bool          usb_stick;
    bool          instanciated;
    chunk_t       chunk;
    wav_header_t  header;
};

audioplayer_t audio;


//--------------------------------------------------------- UTILITY FUNCTIONS ---------------------------------------------------------\\'

// This function checks the length of a char array
int strlen(char *str) {
  int i = 0;

  while (str[i])
      i++;
  return i;
}

// This function checks if the file in the usb are the filetype .wav, because dac can only playback .wav files
int  ends_with(char *name, char *ending) {
    int i = 0;
    int j = 0;
    
    while (name[i++]);
    while (ending[j++]);
    i -= j;
    j = -1;
    while (ending[++j])
        if (!(name[i++] == ending[j]))
            return (-1);
    return (0);
}

// With this function you can get the name of the song in a static char array in order for the file pointer to be opened
char *get_name(char *str) {
    static char str2[2048];  // Declare static because we return a pointer to exactly this str2 for the name of the song. Please only change the size, or dont modify
    int j = 0;
    int i;

    str2[0] = '/'; str2[1] = 'u'; str2[2] = 's'; str2[3] = 'b'; str2[4] = '/';
    i = 5;
    j = 0;
    while (str[j]) {
        str2[i] = str[j];
        j++;
        i++;
    }
    str2[i] = 0;
    return (str2);
}

// With this function you can get the song that sits on that id in the audio.song_ids array
char *get_song(int id) {
    DIR* d = opendir("/usb/");
    char buf[256];
    int ammount = 0;

    while (true) {
        struct dirent* e = readdir(d);
          if (!e) {
              break;
          }
        snprintf(buf, sizeof(buf), "%s", e->d_name);
        if (ends_with(buf, ".wav") == 0) {
            if (ammount == id) {
                return (get_name(buf));
            }
            ammount++;
        }
    }
    return (nullptr);
}

// This function shuffles the songs in order to create a random playlist
int random_values() {
    int x = 0;
    int j = -1;
    int random;
    int max = list_files(0);

    while (++j < max) {
        x = -1;
        random = rand() % max;
        while (++x < j) {
            if (audio.song_ids[x] == random) {
                j--;
                break;
            }
        }
        if (x == j) {
            audio.song_ids[j] = random;
        }
    }
}

//------------------------------------------------------- UTILITY FUNCTIONS END -------------------------------------------------------\\'




//----------------------------------------------------------- ID  FUNCTIONS -----------------------------------------------------------\\'

// This function gets executed on startup to fill the audio.song_ids with accending number till it reaches max ammount of songs on the usb sticks
void  fill_ids() {
    DIR* d = opendir("/usb/");
    char buf[256];
    int  pos = 0;

    while (true) {
        struct dirent* e = readdir(d);
        if (!e) {
            break;
        }
        snprintf(buf, sizeof(buf), "%s", e->d_name);
        if (ends_with(buf, ".wav") == 0) {
            audio.song_ids[pos] = pos;
            pos++;
        }
    }
}

// This function prints every .wav file (audiofiles) and returns the ammount of songs
// 1 Argument = print: if using 1, it prints the files. if using 0 or other, it doesnt print
int  list_files(int print) {
    DIR* d = opendir("/usb/");
    char buf[256];
    int  temp = 0;

    while (true) {
        struct dirent* e = readdir(d);
        if (!e) {
            break;
        }
        snprintf(buf, sizeof(buf), "%s", e->d_name);
        if (ends_with(buf, ".wav") == 0) {
            if (print == 1)
                Serial.println(buf);
            temp++;
        }
    }
    Serial.println();
    return (temp);
}

//--------------------------------------------------------- ID FUNCTIONS END ---------------------------------------------------------\\'




//----------------------------------------------------------- USB FUNCTIONS -----------------------------------------------------------\\'

// Checks if the stick stasy connected or stays disconnected.
// If you remove the stick, then It will end all safely and if you reconnect. It reconnects safely
int status() {
    static bool last_status = true;
    static int connected;

    if (msd.connect() != last_status) {
        if (msd.connect()) {
            Serial.println("Connected");
            usb.mount(&msd);
            audio.songs = list_files(1);
            audio.instanciated = false;
            fill_ids();
        } else {
            Serial.println("Disconnected");
            usb.unmount();
            dac0.stop();
            audio.instanciated = false;
        }
        last_status = msd.connect();
        return (1);
    }
    return (0);
}

// This function tries to find the usb stick on pin PA_15. If one is found. it tries to reconnect
void reconnect() {
    if (msd.connect()) {
        audio.usb_stick = true;
        int err = usb.mount(&msd);
        if (err) {
            Serial.print("Error mounting USB device ");
            Serial.println(err);
            audio.ready = false;
        } else {
            audio.ready = true;
            audio.instanciated = false;
        }
    }
}

//----------------------------------------------------------- USB FUNCTIONS -----------------------------------------------------------\\'




//---------------------------------------------------------- MUSIC FUNCTIONS ----------------------------------------------------------\\'

// With this function, you start pülaying the music and starting the dac0 if it isn't started yet
void  play_music() {
    audio.file = fopen(get_song(audio.song_ids[audio.s_id]), "rb");
    if (audio.file == nullptr) {
        Serial.println("Failed to play music");
        return;
    } else {
      Serial.print("Next song: ");
      Serial.println(get_song(audio.song_ids[audio.s_id]));
    }
    fread(&audio.header, sizeof(audio.header), 1, audio.file);
    while (true)
    {
        fread(&audio.chunk, sizeof(audio.chunk), 1, audio.file);
        if (*(unsigned int *) &audio.chunk.ID == 0x61746164)
          break;
        fseek(audio.file, audio.chunk.size, SEEK_CUR);
    }
    audio.sample_size = audio.header.bitsPerSample / 8;
    audio.samples_count = audio.chunk.size * 8 / audio.header.bitsPerSample;
    if (!audio.instanciated) {
        if (!dac0.begin(AN_RESOLUTION_12, audio.header.sampleRate * 2, 256, 16))
        {
            Serial.println("Failed to start DAC1 !");
            audio.clean = false;
        }
        audio.instanciated = true;
    }
}

// This function checks if the audiofile is in pause mode, playing or has finished
int music_switch(bool pause, int finished_playing) {
    if (audio.clean && !pause && !feof(audio.file))
    {
        uint16_t sample_data[256] = {0};
        fread(sample_data, audio.sample_size, 256, audio.file);
        SampleBuffer buf = dac0.dequeue();
        for (size_t i = 0; i < buf.size(); i++)
        {
            uint16_t const dac_val = ((static_cast<unsigned int>(sample_data[i])+32768)>>4) & 0x0fff;
            buf[i] = dac_val;
        }
        dac0.write(buf);
        return (0);
    }
    else if (feof(audio.file)) {
        if (finished_playing == 0) {
            fclose(audio.file);
            if (audio.s_id < audio.songs) {
                audio.s_id++;
                if (audio.clean)
                    play_music();
            } else audio.s_id = 0;
            Serial.print("Done playing audio: (");
            Serial.print(audio.s_id);
            Serial.print("/");
            Serial.print(audio.songs);
            Serial.println(")");
            return (1);
        }
    }
}

//-------------------------------------------------------- MUSIC FUNCTIONS END --------------------------------------------------------\\'




//--------------------------------------------------------- GENERIC FUNCTIONS ---------------------------------------------------------\\'

bool cmd_tree(bool pause) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (audio.open_drive) {
        if (cmd == "list")
            list_files(1);
        else if (cmd == "pause") {
            if (pause == true)
                Serial.println("Music already paused");
            else {
                Serial.println("Music paused");
                return (true);
            }
        } else if (cmd == "continue") {
            if (pause == false)
                Serial.println("Music not in pause");
            else {
                Serial.println("Music continued");
                return (false);
            }
        } else if (cmd == "skip") {
            fclose(audio.file);
            if (audio.s_id < audio.songs)
                audio.s_id++;
            else
                audio.s_id = 0;
            Serial.print("Skipping audio (");
            Serial.print(audio.s_id);
            Serial.print("/");
            Serial.print(audio.songs);
            Serial.println(")");
            play_music();
        } else if (cmd == "play") {
            play_music();
        } else if (cmd == "shuffle") {
            random_values();
        } else if (cmd == "help") {
            Serial.println("Here are all existing commands:");
            Serial.println("\tlist: Lists all existing .wav files on the usb stick in PA_15.");
            Serial.println("\tplay: Starts at song 1 and plays all songs in order.");
            Serial.println("\tshuffle: Creates a random order for the .wav files.");
            Serial.println("\tpause: Pauses the current audio playback.");
            Serial.println("\tcontinue: Continues the current audio playback.");
            Serial.println("\tskip: Skips the current playback and stars the next in the queue.");
            Serial.println("\thelp: Gets and displays you a help box with all valid commands.\n");
        } else
            Serial.println("Command not found. Enter \"help\" to get all help commands.\n");
    }
}

// This function runs 1 time only at the start when a usb stick is connected. Its goal s to create a simple song queue and sets values for the dac
int instantiate_everything() {
    audio.file = nullptr;
    audio.open_drive = true;
    audio.sample_size = 0;
    audio.samples_count = 0;
    audio.s_id = 0;
    audio.songs = list_files(1);
    audio.clean = true;
    fill_ids();
    return (0);
}

//------------------------------------------------------- GENERIC FUNCTIONS END -------------------------------------------------------\\'




//--------------------------------------------------------- ARDUINO FUNCTIONS ---------------------------------------------------------\\'

void setup()
{
    int err;

    Serial.begin(115200);
    pinMode(PA_15, OUTPUT);
    digitalWrite(PA_15, HIGH);
    while (!Serial);
    if (!msd.connect()) {
        delay(1000);
        if (!msd.connect())
            audio.usb_stick = false;
        else
            audio.usb_stick = true;
    } else
        audio.usb_stick = true;
    audio.ready = false;
    if (audio.usb_stick) {
        err = usb.mount(&msd);
        if (err) {
            Serial.print("Error mounting USB device ");
            Serial.println(err);
        } else {
            audio.ready = true;
            audio.instanciated = false;
        }
    } else
        Serial.println("No usb stick inserted");
    fflush(stdout);
}

void loop() {
    static int  finished_playing = 0;
    static int  looped = 0;
    static bool pause = false;

    if (audio.usb_stick) {
        if (audio.ready) {
            if (looped == 0) {
                if (instantiate_everything() < 0)
                    audio.clean = false;
                looped++;
            }
            if (audio.clean && status() == 1)
                audio.open_drive = !audio.open_drive;
            if (audio.clean && Serial.available() > 0)
                pause = cmd_tree(pause);
            if (dac0.available())
                finished_playing = music_switch(pause, finished_playing);
                if (finished_playing == 1)
                    play_music();
        } else {
            Serial.println("Usb error. please reconnect");
        }
    } else {
        reconnect();
    }
}

//------------------------------------------------------- ARDUINO FUNCTIONS END -------------------------------------------------------\\'
