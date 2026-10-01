"""Generate the thirteen folder-03 prompts with Microsoft's online neural voice.

Run with tools/.venv/Scripts/python.exe tools/generate_feedback.py.
Requires edge-tts and ffmpeg. Existing audio is backed up before replacement.
"""
import asyncio
import argparse
from datetime import datetime
from pathlib import Path
import shutil
import subprocess
import edge_tts

ROOT = Path(__file__).resolve().parents[1]
PROMPTS = ["Select a mode.", "Learning mode selected.", "Quiz mode selected.",
           "Word is correct.", "Word is incorrect.", "Enter Braille pattern.",
           "Letter learning selected.", "Word learning selected.",
           "Letter quiz selected.", "Word quiz selected.",
           "Letter is correct.", "Letter is incorrect.", "Confirmed."]
VOICE = "en-US-JennyNeural"

async def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--start", type=int, default=1)
    start = parser.parse_args().start
    if not 1 <= start <= len(PROMPTS):
        raise ValueError("Invalid first track")
    staging = ROOT / "audio_sources" / datetime.now().strftime("%Y%m%d-%H%M%S")
    staging.mkdir(parents=True)
    for number, prompt in enumerate(PROMPTS, 1):
        if number < start:
            continue
        raw = staging / f"{number:03d}-neural.mp3"
        await asyncio.wait_for(edge_tts.Communicate(prompt, VOICE, rate="-5%").save(str(raw)), 45)
        final = staging / f"{number:03d}.mp3"
        subprocess.run(["ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-i", str(raw),
                        "-ar", "44100", "-ac", "1", "-codec:a", "libmp3lame", "-b:a", "128k", str(final)], check=True)
        print(f"Generated {number:03d}: {prompt}", flush=True)
    # Publish only after all requested files exist. Retain the previous audio set.
    backup = staging / "previous-03"
    shutil.copytree(ROOT / "03", backup)
    for number in range(start, len(PROMPTS) + 1):
        shutil.copy2(staging / f"{number:03d}.mp3", ROOT / "03" / f"{number:03d}.mp3")
    print(f"Published tracks {start} to {len(PROMPTS)}. Backup: {backup}", flush=True)

if __name__ == "__main__":
    asyncio.run(main())
