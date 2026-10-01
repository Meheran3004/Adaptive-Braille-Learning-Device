# Feedback audio

Copy these six MP3s into folder `03` at the root of the DFPlayer SD card.

| File | Spoken text |
| --- | --- |
| 001.mp3 | Select a mode. |
| 002.mp3 | Learning mode selected. |
| 003.mp3 | Quiz mode selected. |
| 004.mp3 | Word is correct. |
| 005.mp3 | Word is incorrect. |
| 006.mp3 | Enter Braille pattern. Played once after the selected Quiz word. |

Synthetic voice: Microsoft en-US-JennyNeural; speech rate -5%.
Format: MP3, mono, 44.1 kHz, 128 kbps.

Replace the previous 001/002 files: their meanings have changed.
Only copy the MP3 files. Legacy correct.wav and incorrect.wav are not used.
Generation source: `tools/generate_feedback.py`; old files are backed up in `audio_sources/`.
Regenerate `Adaptive_braille_project/AudioDurations.h` with `tools/measure_audio.py` after changing any recording.
