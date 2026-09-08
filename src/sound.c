// Pirates sound mock-up. See sound.h for what this is for.
//
// Compiles to stubs unless DY_SOUND_AVAILABLE is defined, which user.cmake does
// only when the lib/dy_sound_same51 submodule is actually populated. That keeps
// the project building before the driver has been pushed.

#include "sound.h"

#ifdef DY_SOUND_AVAILABLE

#include "definitions.h"
#include "FreeRTOS.h"
#include "task.h"

#include "dy_sound/dy_sound.h"
#include "dy_sound/dy_sound_bank.h"
#include "dy_sound/dy_sound_freertos.h"

#include <stdio.h>

// ---------------------------------------------------------------------------
// SERCOM
//
// All eight SERCOMs are the same peripheral and any of them can be a USART.
// What is NOT free is the pins, and on this board three are already spoken for:
//
//     SERCOM1  PA16 MOSI / PA17 SCK      NeoPixel strip
//     SERCOM2  PA12 SDA  / PA13 SCL      I2C - silkscreened, do not take
//     SERCOM5  PB16 TX   / PB17 RX       this console (CDC)
//
// SERCOM3 on PA22/PA23 is the pick: PA22 is SERCOM3 PAD[0] and PA23 is PAD[1]
// on peripheral function C, and neither pin appears anywhere else in this
// project. Verified against ATSAME51J20A.atdf, not from memory.
//
// If PA22/PA23 are not broken out on your board, these are the other pairs the
// ATDF says are free of every pin this project claims - change the two lines
// below and the MCC pin assignment to match, nothing else:
//
//     SERCOM0   PA04 (PAD0) / PA05 (PAD1)   function D
//     SERCOM0   PA08 (PAD0) / PA09 (PAD1)   function C
//     SERCOM4   PB08 (PAD0) / PB09 (PAD1)   function D
//
// Not SERCOM4 on PB12/PB13, the pair the board labels CAN1: PB13 is the fog
// relay (FOG_RELAY_PIN in plib_port.h) and taking it would kill the fog.
//
// MCC still has to create it - this driver binds to a SERCOM, it does not
// configure one. In MCC Melody add a SERCOM3 USART:
//
//     Mode                 USART with internal clock
//     Baud                 9600
//     Data / parity / stop  8 / none / 1
//     Receive enable       on          <- needed for the confirming query
//     Interrupt driven     OFF         <- see the note in dy_sound.h
//     TX pin  PA22 (PAD0)   RX pin  PA23 (PAD1)
//
// Until that exists the SERCOM never raises "data register empty" and every cue
// reports DY_EVENT_TX_TIMEOUT on the console after ~20 ms. It will not hang.
// ---------------------------------------------------------------------------
#ifndef SOUND_SERCOM
#define SOUND_SERCOM       SERCOM3_REGS
#define SOUND_SERCOM_NAME  "SERCOM3"
#endif

// Wiring, and the strap that is easy to miss:
//
//     module IO0/TX  ->  PA23   (MCU RX)
//     module IO1/RX  ->  PA22   (MCU TX)
//     common ground
//
// UART mode must be strapped or the module ignores serial completely - powered,
// healthy and totally deaf. CON1->GND, CON2->GND, CON3->3V3, 10k each. Left
// floating it sits in button mode. Logic is 3.3 V, so no level shifting.
//
// Leaving the module's TX disconnected is fine and costs one 120 ms wait per
// cue. Sounds still play; the console just says "no reply" instead of
// "confirmed", and Sound_ModuleIsTalking() stays false.

static dy_sound_t   s_module;
static volatile bool s_talking = false;

// ---------------------------------------------------------------------------
// The card, in one table.
//
// This is the whole of what used to be chestTrackFor()'s switch, mastLoops()
// and a pair of LastSoundVariable latches. Index, repeat behaviour and name on
// one line each.
//
// Indices are the copy order on the card and are NOT edited here to match
// anything - they are what the Pirates sketch already sends, so the existing
// card works untouched.
// ---------------------------------------------------------------------------
#if SOUND_CARD_MAST

static const dy_track_t s_tracks[] = {
    DY_TRACK    (SND_PIRATES_ON,    1, "pirates on (intro)"     ),
    DY_TRACK    (SND_RESET_MODE,    2, "reset mode"             ),
    DY_TRACK    (SND_RESET_DONE,    3, "reset done"             ),
    DY_TRACK    (SND_WIND,          4, "wind"                   ),
    DY_TRACK    (SND_CBR_START,     5, "CBR start"              ),
    DY_TRACK_BED(SND_CBR_AMBIENT,   6, "CBR ambient bed"        ),
    DY_TRACK    (SND_MAST_DIALS,    7, "mast dials complete"    ),
    DY_TRACK    (SND_MAST_TABLE,    8, "mast table complete"    ),
    DY_TRACK    (SND_AMBIENT_DONE,  9, "ambient puzzle done"    ),
    DY_TRACK    (SND_SWORDS_0,     10, "no swords"              ),
    DY_TRACK    (SND_SWORDS_1,     11, "one sword"              ),
    DY_TRACK    (SND_SWORDS_2,     12, "two swords"             ),
    DY_TRACK    (SND_SWORDS_ALL,   13, "all swords"             ),
    DY_TRACK    (SND_CONSTELLATION,14, "constellation complete" ),
    DY_TRACK    (SND_COMPASS_TRUE, 15, "compass points true"    ),
    DY_TRACK    (SND_COMPASS_FAIL, 16, "compass fail"           ),
    DY_TRACK    (SND_COMPASS_DING, 17, "compass ding"           ),
    DY_TRACK    (SND_SIZZLE,       18, "sizzle"                 ),
    DY_TRACK    (SND_CANNON,       19, "cannon fire"            ),
    DY_TRACK    (SND_FAIL,         20, "fail"                   ),
};

#define SOUND_EXPECTED_FILES  20

#else   // chest card

static const dy_track_t s_tracks[] = {
    DY_TRACK    (SND_WIND,          1, "wind"                   ),
    DY_TRACK    (SND_CREAK,         2, "chest creaking"         ),
    // Two entries, one id. The bank alternates them round-robin, which is what
    // the sketch was doing by hand with codes 250/251 and a 30 s timer.
    DY_TRACK    (SND_PLATFORM,      3, "platform change a"      ),
    DY_TRACK    (SND_PLATFORM,      4, "platform change b"      ),
    DY_TRACK_BED(SND_AMBIENT,       5, "ambient a"              ),
    DY_TRACK_BED(SND_AMBIENT,       6, "ambient b"              ),
    DY_TRACK    (SND_SIZZLE,        7, "sizzling"               ),
    DY_TRACK    (SND_CANNON,        8, "cannon fire"            ),
    DY_TRACK    (SND_HINT_GENERIC,  9, "hint: generic"          ),
    DY_TRACK    (SND_HINT_NOD,     10, "hint: captains nod"     ),
    DY_TRACK    (SND_HINT_WEE,     11, "hint: fancy a wee hint" ),
    DY_TRACK    (SND_HINT_CANNON,  12, "hint: hands in cannon"  ),
    DY_TRACK    (SND_HINT_NAY,     13, "hint: nay not right"    ),
    DY_TRACK    (SND_HINT_NOWHERE, 14, "hint: leads nowhere"    ),
    DY_TRACK    (SND_HINT_NUMBERS, 15, "hint: no numbers yet"   ),
    DY_TRACK    (SND_HINT_CLEVER,  16, "hint: aye clever dog"   ),
};

#define SOUND_EXPECTED_FILES  16

#endif

static const dy_bank_t s_bank = DY_BANK(s_tracks);

#define SOUND_TRACK_COUNT  (sizeof(s_tracks) / sizeof(s_tracks[0]))

// ---------------------------------------------------------------------------
// Console. Runs in the sound task, never an ISR, so printf is fine - the task
// is given the stack for it below.
// ---------------------------------------------------------------------------
static void on_sound_event(uintptr_t ctx, dy_event_t ev, uint16_t track, int reported)
{
    (void)ctx;

    switch (ev)
    {
        case DY_EVENT_READY:
            printf("sound: module ready, volume %d/30\n", reported);
            break;

        case DY_EVENT_CONFIRMED:
            s_talking = true;
            printf("sound:   confirmed %u\n", track);
            break;

        case DY_EVENT_NO_REPLY:
            // Expected, and harmless, when the module's TX is not wired back.
            printf("sound:   no reply (playing anyway)\n");
            break;

        case DY_EVENT_MISMATCH:
            // Not necessarily wrong: a short clip can finish before the query
            // lands, and the module then reports whatever it is holding.
            s_talking = true;
            printf("sound:   module says %d, not %u\n", reported, track);
            break;

        case DY_EVENT_DROPPED:
            printf("sound:   DROPPED %u - queue full\n", track);
            break;

        case DY_EVENT_TX_TIMEOUT:
        default:
            printf("sound:   TX timeout - is %s set up in MCC?\n", SOUND_SERCOM_NAME);
            break;
    }
}

// ---------------------------------------------------------------------------

void Sound_Init(void)
{
    dy_sound_cfg_t      cfg;
    dy_sound_rtos_cfg_t rtos;

    dy_sound_cfg_init(&cfg);          // every default, then override
    cfg.sercom   = SOUND_SERCOM;
    cfg.now_ms   = xTaskGetTickCount; // configTICK_RATE_HZ is 1000, so ticks are ms
    cfg.on_event = on_sound_event;
    cfg.volume   = 22u;               // 0..30. Loud enough to hear, not to argue with.

    if (!dy_sound_init(&s_module, &cfg))
    {
        printf("sound: dy_sound_init FAILED\n");
        return;
    }

    if (!dy_sound_bank_attach(&s_module, &s_bank))
    {
        printf("sound: bank attach FAILED\n");
        return;
    }

    dy_sound_rtos_cfg_init(&rtos);
    rtos.name        = "sound";
    rtos.priority    = 1;             // below the puzzle logic, above idle
    rtos.stack_words = 512;           // printf in on_sound_event needs the room

    if (!dy_sound_rtos_start(&s_module, &rtos))
    {
        printf("sound: task FAILED to start\n");
    }
}

bool Sound_Play(uint16_t id)
{
    return dy_sound_bank_play(&s_module, id);
}

bool Sound_Select(uint16_t id)
{
    return dy_sound_bank_select(&s_module, id);
}

bool Sound_ModuleIsTalking(void)
{
    return s_talking;
}

// ---------------------------------------------------------------------------
// The walk.
//
// Plays by raw track index rather than by cue id, deliberately: the point is to
// hear what index N actually is, and playing by id would rotate past the second
// half of any variant pair instead of visiting it.
// ---------------------------------------------------------------------------
void SoundWalk_Task(void *pvParameters)
{
    uint16_t on_card = 0u;
    uint32_t pass    = 0u;
    uint32_t i;

    (void)pvParameters;

    // The sound task is already waiting out the module's 2 s card-mount time
    // and setting volume. Give it room to finish before talking over it.
    vTaskDelay(pdMS_TO_TICKS(3000));

    printf("\n=== pirate card walk: %s, %u tracks ===\n",
           SOUND_CARD_MAST ? "mast" : "chest",
           (unsigned)SOUND_TRACK_COUNT);

    // Ask the module how many files it can see. This is one of the seven
    // commands it actually answers, and it is the cheapest way to tell which
    // card is in: 20 files is the mast, 16 the chest.
    if (dy_sound_query_song_count(&s_module, &on_card))
    {
        s_talking = true;
        printf("sound: module reports %u files on the card\n", on_card);

        if (on_card != SOUND_EXPECTED_FILES)
        {
            printf("sound: expected %u for this bank - wrong SOUND_CARD_MAST?\n",
                   (unsigned)SOUND_EXPECTED_FILES);
        }
    }
    else
    {
        printf("sound: module did not answer - TX unwired, or button mode.\n"
               "sound: sounds will still play, names below are the assumption.\n");
    }

    for (;;)
    {
        printf("\n--- pass %lu ---\n", (unsigned long)(++pass));

        for (i = 0u; i < SOUND_TRACK_COUNT; i++)
        {
#if DY_SOUND_LABELS
            printf("sound: track %2u  %s\n",
                   (unsigned)s_tracks[i].track, s_tracks[i].label);
#else
            printf("sound: track %2u\n", (unsigned)s_tracks[i].track);
#endif

            // Cycle mode comes from the table, so the ambient bed loops and
            // everything else stops on its own - exactly as it would in the
            // room. The next track interrupts whatever is still going.
            (void)dy_sound_rtos_play_ex(&s_module,
                                        s_tracks[i].track,
                                        s_tracks[i].cycle,
                                        true);

            vTaskDelay(pdMS_TO_TICKS(SOUND_WALK_MS));
        }
    }
}

#else  /* DY_SOUND_AVAILABLE */

// Submodule not populated yet. Stubs keep main.c free of #ifdef.

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

void Sound_Init(void)
{
    printf("sound: driver submodule not populated - sound disabled\n");
}

void SoundWalk_Task(void *pvParameters)
{
    (void)pvParameters;
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}

bool Sound_Play(uint16_t id)          { (void)id; return false; }
bool Sound_Select(uint16_t id)        { (void)id; return false; }
bool Sound_ModuleIsTalking(void)      { return false; }

#endif /* DY_SOUND_AVAILABLE */
