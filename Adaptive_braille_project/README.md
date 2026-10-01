# Adaptive Braille Learning Module

Open `Adaptive_braille_project.ino` in Arduino IDE. The previous sketch is preserved as `refreshable_braille_display.original.txt` so Arduino does not compile two copies of `setup()` and `loop()`.

The [earlier wiring diagram](wiring-diagram.svg) shows the original solenoid layout and GPIO2 for Quiz dot 2. The current sketch uses vibration motors and GPIO23 for Quiz dot 2; use the current pin table below.

## Controls

When SPELL confirms a selected Learning or Quiz activity, the device says “Confirmed.” This applies to both letter and word options. Wait for that recording to finish before pressing WORD to begin. Copy the new `03/013.mp3` (Confirmed) to the SD card's `03` folder and upload the updated sketch. SPELL during a lesson or quiz retains its normal repeat, advance, or submit action.

On startup, the device says “Select a mode.” Press MODE once to select Learning; press it again to select Quiz. Each later MODE press switches between the two and speaks the selected mode. All buttons connect their GPIO to GND when pressed and use `INPUT_PULLUP`. Each debounced press acts once; holding a button does not repeat it.

| Control | GPIO | Action |
| --- | --- | --- |
| WORD | 12 | Choose a learning activity, select a learning letter/word, or start a quiz word |
| SPELL | 13 | Confirm a learning activity, repeat a learning letter, advance a word, or start/submit a quiz answer |
| MODE | 21 | Select/toggle Learning/Quiz; clear the display and current attempt |
| EN/RST | Existing board button | Restart and restore all word weights to the base value |

Learning: after selecting Learning Mode, press WORD to hear and choose Letter Learning or Word Learning, then press SPELL to confirm. In Letter Learning, each WORD press advances through A to Z, returning to A after Z. Each new Letter Learning session starts at A. The device speaks the letter and shows its Braille pattern; SPELL repeats the same letter and pattern without advancing. In Word Learning, press WORD to select a word, then press SPELL for each letter. Each press speaks one letter and displays its Braille pattern. There is no automatic advance. The last letter stays displayed until SPELL clears it, WORD selects another word, or MODE resets the activity.

Quiz: press WORD and wait for the selected word. The device then says “Enter Braille pattern” once. Press SPELL to hear the first letter. After its recording finishes, use the six input buttons to toggle the corresponding virtual dots; press a dot again to deselect it. SPELL submits the pattern. A correct letter clears the guess and speaks the next letter without repeating the entry prompt. Completing the whole word plays `/03/004.mp3` (Word is correct). A wrong answer plays `/03/005.mp3` (Word is incorrect); after it finishes, each SPELL press reveals one letter of the correct word through the vibration motors. Correction spelling remains manual. Press WORD after audio finishes to start another attempt.

Audio timing comes from `AudioDurations.h`, measured from all 287 supplied MP3s, with a 300 ms allowance for the DFPlayer to start playback. WORD, SPELL, and dot presses during playback are ignored, not queued. MODE remains responsive and cancels any pending quiz prompt. There is no extra BUSY-pin wiring. If you replace an audio file, run `python tools/measure_audio.py` from the project root and recompile so its timing stays accurate. These are timed playback windows, not a check that the speaker or SD card actually worked.

Serial Monitor at 115200 baud reports mode, word, letter, selected quiz dots, updated weight, and errors. Quiz input does not actuate motors. There is no spoken echo for toggling individual dots.

## Letter Quiz and Word Quiz

After selecting Quiz Mode, press WORD to alternate between Letter Quiz and Word Quiz, then press SPELL to confirm. Wait for each audio prompt to finish before pressing the next button. To choose again, use MODE to leave Quiz and return to its menu.

In Letter Quiz, WORD selects and speaks a weighted random letter, followed once by “Enter Braille pattern.” After both recordings finish, enter the pattern with the six dot buttons and press SPELL to submit. The motors stay off during the question. Correct answers play “Letter is correct.” Incorrect answers play “Letter is incorrect,” after which SPELL speaks and displays the correct letter manually. WORD starts another question. Word Quiz follows the word quiz sequence described above.

Each of the 26 letters has its own integer weight, independent of the word weights. All start at 5. A correct letter decreases its weight by 1, never below 5. An incorrect letter increases its weight by 1, up to 1000. Selection probability equals that letter's weight divided by the total of all 26 letter weights. Immediate repeats are possible. Learning, correction, and abandoning a question do not change weights. Mode changes preserve weights, while EN/RST or power cycling resets both letter and word weights to 5. Letter Quiz works without a loaded word list.

Copy the newly generated `03/009.mp3` through `03/012.mp3` to the SD card's `03` folder and upload the updated sketch. Existing audio files remain unchanged by this update.

| SD location | Contents |
| --- | --- |
| `/03/009.mp3` | Letter quiz selected |
| `/03/010.mp3` | Word quiz selected |
| `/03/011.mp3` | Letter is correct |
| `/03/012.mp3` | Letter is incorrect |

## Adaptive word selection

Each word starts with integer `BASE_WEIGHT = 5`. A wrong pattern increases that word's weight by `WEIGHT_STEP = 1` once, then the attempt ends and correction begins. Only a completely correct word decreases its weight by 1. A partially correct word earns no decrease, correction does not change weights, and abandoning an attempt does not change them. Weights cannot fall below 5; `MAX_WEIGHT = 1000` prevents overflow during long sessions. These constants are near the top of the sketch.

In Quiz mode, probability = word weight / sum of all loaded word weights. For example, with just two words at weights 5 and 15, their probabilities are 25% and 75%. Quiz permits immediate repeats so those probabilities remain accurate. Learning mode retains uniform selection with no immediate repeat when multiple words exist.

Weights are held in RAM for the current session. Selecting another word or switching modes preserves them. Pressing the board's existing EN/RST button (or cycling power) restores all weights to 5 and plays “Select a mode.” No extra reset button, filesystem upload, or flash writes are needed for resetting weights.

## Dot wiring

The table uses standard Braille numbering, viewed from the reading side:

```text
1 4
2 5
3 6
```

| Dot | Vibration-motor MOSFET gate GPIO | Quiz input GPIO |
| --- | --- | --- |
| 1 | 32 | 15 |
| 2 | 33 | 23 |
| 3 | 25 | 4 |
| 4 | 26 | 5 |
| 5 | 27 | 18 |
| 6 | 14 | 19 |

These assignments target the original ESP32 layout. GPIO5, GPIO12, and GPIO15 have boot-strapping functions; release the buttons during reset/upload and do not add external pull-ups to GPIO12 without checking the module's flash-voltage requirements. ESP32 variants and modules with PSRAM may reserve different pins. See [Espressif's ESP32 datasheet](https://documentation.espressif.com/esp32_datasheet_en.html).

Keep the existing MOSFET motor drivers: motors use their rated external supply, a common ground with the ESP32, a flyback diode per motor, and a gate pull-down to keep each actuator off during reset. The vibration pattern stays active until the next control action.

DFPlayer TX connects to ESP32 RX GPIO16. ESP32 TX GPIO17 connects to DFPlayer RX through the existing 1 kΩ series resistor. Keep the existing speaker, power supply, and common ground connections.

## Files and upload

1. Install the Espressif ESP32 board package and the `DFRobotDFPlayerMini` Arduino library. Select the exact ESP32 board and serial port you use.
2. Open `Adaptive_braille_project.ino`, compile, and upload the sketch.
3. Use your ESP32 LittleFS uploader to upload the contents of `data/` to a filesystem partition. Firmware upload alone does not upload the word list. The runtime file is `/words.txt` in LittleFS. Edit `data/words.txt` for future uploads; the adjacent original `words.txt` is retained as a reference.
4. Copy folders `01`, `02`, and `03` from the project root to the DFPlayer SD card root. Copy only the MP3 files from `03`; WAV files, if present, are source recordings.

| SD location | Contents |
| --- | --- |
| `/01/001.mp3` through `/01/026.mp3` | Letters a–z |
| `/02/001.mp3` through `/02/255.mp3` | Words in word-list order |
| `/03/001.mp3` | Select a mode |
| `/03/002.mp3` | Learning mode selected |
| `/03/003.mp3` | Quiz mode selected |
| `/03/004.mp3` | Word is correct |
| `/03/005.mp3` | Word is incorrect |
| `/03/006.mp3` | Enter Braille pattern |
| `/03/007.mp3` | Letter learning selected |
| `/03/008.mp3` | Word learning selected |

Replace the old `03/001.mp3` and `03/002.mp3` on the SD card along with adding tracks 003–008. The old numbering is incompatible with this firmware. All eight recordings use the synthetic Microsoft Jenny neural voice at 5% slower than its default rate, encoded as mono 44.1 kHz, 128 kbps MP3. Previous recordings are backed up under `audio_sources/`. The old WAV files in folder 03 are not used by the firmware.

To regenerate the prompts, run `tools/.venv/Scripts/python.exe tools/generate_feedback.py` from the project root (internet required), then run `python tools/measure_audio.py` and recompile. The generator uses [edge-tts](https://github.com/rany2/edge-tts) and keeps a backup before replacing any prompt audio.

Each nonempty word-list line corresponds to the next audio track in folder 02. Keep that order unchanged unless you also update the audio files. The loader accepts a–z, converts uppercase to lowercase, ignores blank lines, and rejects punctuation while preserving the subsequent track numbers. The existing word list contains 255 alphabetic entries. The folder/track calls follow the [DFRobot DFPlayer API](https://wiki.dfrobot.com/dfr0299/docs/20905).

## Validation

Compiled successfully for `esp32:esp32:esp32` (ESP32 Dev Module), Espressif core 3.3.11: 318,779 bytes flash (24%) and 27,732 bytes global RAM (8%). Select your actual board when uploading. No firmware has been uploaded by this update.

The desktop test includes the actual sketch with lightweight Arduino, storage, and audio substitutes. From the project root:

```powershell
g++ -std=c++17 -Wall -Wextra -Werror -I tests tests/test_sketch.cpp -o tests/test_sketch.exe
.\tests\test_sketch.exe
```

Tests cover exact weighted-draw probabilities, whole-word rewards, incorrect-answer penalties, weight limits and reset, no reward during correction, startup/mode/entry audio sequencing, ignored early input, manual spelling, debounce, word loading, timer wraparound, and all 26 pattern constants. This does not replace testing physical wiring, audio, and motors.

On the device, check WORD/SPELL in Learning, toggle MODE, submit both correct and incorrect quiz answers, and verify the dot numbering on the physical display. Switch MODE midway through an answer to verify the pattern and attempt clear.
