#ifndef SOUND_H
#define SOUND_H

// Pirates sound mock-up, for listening to the existing DY module card in the
// Coffin rig without touching what is on it.
//
// The card is READ ONLY as far as this code is concerned. Track indices below
// were reverse-engineered from PiratesB5v2.ino, so nothing here needs the card
// re-copied - which matters, because the numbering IS the copy order and
// re-copying it is what would destroy the other project.
//
// WHAT IT DOES
//   SoundWalk_Task walks the selected card one track at a time, prints what it
//   is about to play, waits SOUND_WALK_MS, and moves on. Sit with the serial
//   console open and you can hear which index is which clip.
//
// HARDWARE
//   The DY module needs its own SERCOM, and its own pins. SERCOM1 is the
//   NeoPixel SPI (PA16/17), SERCOM2 is I2C (PA12 SDA / PA13 SCL) and SERCOM5 is
//   this console (PB16/17). SERCOM3 on PA22/PA23 is what sound.c picks, with
//   the verified alternates listed there.

#include <stdbool.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// Which card is in the module. The two Pirates modules had different cards.
//   1 = mast  (20 tracks, PiratesB5v2 played these by index directly)
//   0 = chest (16 tracks, the sketch's chestTrackFor() switch)
// Sound_Init prints the module's own file count so you can confirm the pick.
// #ifndef so it can also be set from the build without editing this file.
// ---------------------------------------------------------------------------
#ifndef SOUND_CARD_MAST
#define SOUND_CARD_MAST   1
#endif

// Seconds each track gets during the walk before the next one interrupts it.
#define SOUND_WALK_MS     12000u

// ---------------------------------------------------------------------------
// Cue ids. These are the bank's ids, not raw track numbers - the mapping to
// indices lives in the table in sound.c, which is the only place that has to
// know the card's copy order.
// ---------------------------------------------------------------------------
#if SOUND_CARD_MAST
enum
{
    SND_NONE = 0,
    SND_PIRATES_ON = 1,      // 1  intro sting
    SND_RESET_MODE,          // 2  reset started
    SND_RESET_DONE,          // 3  reset finished / CBR restored
    SND_WIND,                // 4
    SND_CBR_START,           // 5  lights out, ambient timer starts
    SND_CBR_AMBIENT,         // 6  the bed - LOOPS
    SND_MAST_DIALS,          // 7  mast dials complete
    SND_MAST_TABLE,          // 8  mast table complete
    SND_AMBIENT_DONE,        // 9  ambient puzzle complete
    SND_SWORDS_0,            // 10 no swords
    SND_SWORDS_1,            // 11 one sword
    SND_SWORDS_2,            // 12 two swords
    SND_SWORDS_ALL,          // 13 all swords
    SND_CONSTELLATION,       // 14 constellation complete
    SND_COMPASS_TRUE,        // 15 compass points north
    SND_COMPASS_FAIL,        // 16
    SND_COMPASS_DING,        // 17 never triggered by the sketch
    SND_SIZZLE,              // 18
    SND_CANNON,              // 19 cannon fire, no epic music
    SND_FAIL                 // 20
};
#else
enum
{
    SND_NONE = 0,
    SND_WIND = 1,            // 1
    SND_CREAK,               // 2  chest creaking
    SND_PLATFORM,            // 3+4  two variants, alternated
    SND_AMBIENT,             // 5+6  two variants, alternated - LOOP
    SND_SIZZLE,              // 7
    SND_CANNON,              // 8
    SND_HINT_GENERIC,        // 9
    SND_HINT_NOD,            // 10 captains nod
    SND_HINT_WEE,            // 11 fancy a wee hint
    SND_HINT_CANNON,         // 12 hands in cannon
    SND_HINT_NAY,            // 13 nay not right
    SND_HINT_NOWHERE,        // 14 leads nowhere
    SND_HINT_NUMBERS,        // 15 no numbers yet
    SND_HINT_CLEVER          // 16 aye clever dog
};
#endif

// Bind the module and start its sound task. Call before vTaskStartScheduler().
// Safe to call when the driver submodule is absent - it does nothing.
void Sound_Init(void);

// Walks the card, one track at a time, printing each before it plays. Create it
// as a task; it never returns. This is not the driver's sound task -
// Sound_Init() creates that one, and this only posts cues to it.
void SoundWalk_Task(void *pvParameters);

// Play a cue now, whether or not it is already selected. Any task, never blocks.
bool Sound_Play(uint16_t id);

// Say what the room's sound should be. Plays only on a change, so this is safe
// to call every pass of a loop - no LastSoundVariable needed.
bool Sound_Select(uint16_t id);

// True once the module has answered at least one confirming query. A module
// with its TX unwired plays perfectly and never sets this, so do not gate
// anything on it - it is for the console line only.
bool Sound_ModuleIsTalking(void);

#endif /* SOUND_H */
