/*******************************************************************************
* Author            : Josh Natis                                               *
* Last Modified     : 12/16/2020                                               *
* Description       : program to control a synth (i.e. read inputs and output  *
*                     and send proper signal to the speaker based on them).    *
* Technical Details : The program sends the output PWM audio signal to pin 11. *
*                     It receives inputs from pins 1-9,12,13 (an octave of     *
*                     buttons), as well as from A2-A5 (potentiometers).        *
*******************************************************************************/

/* waveform generation libary, i.e. synth.h, kindly provided via
   https://github.com/dzlonline/thesynth */
#include "synth.h"

#include "notes.h"
/* uncomment to view rudimentary debug information in the serial monitor */
/* #include "debug.h"
   #define DEBUG_MODE */

/*____  _    _ _______ _______ ____  _   _  _____
 |  _ \| |  | |__   __|__   __/ __ \| \ | |/ ____|
 | |_) | |  | |  | |     | | | |  | |  \| | (___  
 |  _ <| |  | |  | |     | | | |  | | . ` |\___ \
 | |_) | |__| |  | |     | | | |__| | |\  |____) |
 |____/ \____/   |_|     |_|  \____/|_| \_|_____/ ==================== */
 
#define BTN1 1
#define BTN2 2
#define BTN3 3
#define BTN4 4
#define BTN5 5
#define BTN6 6
#define BTN7 7
#define BTN8 8
#define BTN9 9
#define BTN10 10
#define BTN11 12
#define BTN12 13

#define NUM_BUTTONS 12
const int BUTTONS[] = {BTN1, BTN2, BTN3, BTN4, BTN5, BTN6, BTN7, BTN8, BTN9, BTN10, BTN11, BTN12};

/* we only want to act when a button's state changes (it's pressed or released)
   to see if something has changed, compare to state from previous read (i.e. cache) */
bool BUTTON_CURRENT_STATES[12] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
bool BUTTON_CACHED_STATES[12]  = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};

/* mystery button is mysterious... it's also red */
#define MYSTERY_BUTTON 0
/* ==================================================================== */

const int OCTAVE_TABLE[7][12] = {
  {C1, CS1, D1, DS1, E1, F1, FS1, G1, GS1, _A1, AS1, B1},
  {C2, CS2, D2, DS2, E2, F2, FS2, G2, GS2, _A2, AS2, B2},
  {C3, CS3, D3, DS3, E3, F3, FS3, G3, GS3, _A3, AS3, B3},
  {C4, CS4, D4, DS4, E4, F4, FS4, G4, GS4, _A4, AS4, B4},
  {C5, CS5, D5, DS5, E5, F5, FS5, G5, GS5, _A5, AS5, B5},
  {C6, CS6, D6, DS6, E6, F6, FS6, G6, GS6, _A6, AS6, B6},
  {C7, CS7, D7, DS7, E7, F7, FS7, G7, GS7, _A7, AS7, B7}
};

/* DEBOUNCE BUTTON READS */
/* using a software debouncing technique for button-presses as found on
   http://gammon.com.au/switches */
const unsigned long debounceDelay = 10;
unsigned long lastDebounceTime = 0;

/* AVAILABLE SIGNAL SETTINGS */
const int WAVEFORMS[] = {SINE, TRIANGLE, SQUARE, SAW, RAMP, NOISE};
const int ENVELOPES[] = {ENVELOPE0, ENVELOPE1, ENVELOPE2, ENVELOPE3};

/* synth.h provides 4 voices, we'll be using one per button pressed.
 * when a new button is pressed, we assign it to the next free voice,
 * as found in 'FREE_VOICE'. FREE_VOICE serves as an index to the
 * VOICES[] array -- so VOICES[0] represents voice 0. The value
 * in VOICES[i] is the frequency currently being played by that
 * voice.
 */
int VOICES[4] = {-1, -1, -1, -1};
int FREE_VOICE = 0;
int NUM_VOICES_PLAYING = 0;

/*_____   ____ _______ _____
 |  __ \ / __ \__   __/ ____|
 | |__) | |  | | | | | (___  
 |  ___/| |  | | | |  \___ \
 | |    | |__| | | |  ____) |
 |_|     \____/  |_| |_____/ ========================================= */

/* Out of the 4 pots on the right, only pot 2 (A3) is working with
   no issues. Pot 4 (A5) is completely not functional, pot 3 (A4)
   works somewhat when pressed down, and pot 1 (A2) has some effect
   but is affected by the movement of all other pots. Shame!
*/
#define POT_MODULATION A5
#define POT_PITCHBEND A4
#define POT_OCTAVE A3
#define POT_WAVEFORM A2

/* USER INPUT SETTINGS */
int INPUT_OCTAVE = 4;
int INPUT_MODULATION;
int INPUT_PITCHBEND;
int INPUT_WAVEFORM;
/* ==================================================================== */

synth winona;

void init_synth()
{
    winona.begin();
    /*             0-3             0-127            0-127  0-127 */
    /* setupVoice(voice, waveform, pitch, envelope, length, mod) */
    /* DEFAULT PATCH */
    winona.setupVoice(0, SQUARE, 0, ENVELOPE1, 0, 64);
    winona.setupVoice(1, SQUARE, 0, ENVELOPE1, 0, 64);
    winona.setupVoice(2, SQUARE, 0, ENVELOPE1, 0, 64);
    winona.setupVoice(3, SQUARE, 0, ENVELOPE1, 0, 64);
}

bool isPressed(int n)
{
    return BUTTON_CURRENT_STATES[n] && !BUTTON_CACHED_STATES[n]
        && (millis() - lastDebounceTime > debounceDelay);
}

bool isReleased(int n)
{
    return !BUTTON_CURRENT_STATES[n] && BUTTON_CACHED_STATES[n]
        && (millis() - lastDebounceTime > debounceDelay);
}

void read_button_states()
{
    for (int i = 0; i < NUM_BUTTONS; ++i)
        BUTTON_CURRENT_STATES[i] = digitalRead(BUTTONS[i]);
}

void cache_button_states()
{
    for (int i = 0; i < NUM_BUTTONS; ++i)
        BUTTON_CACHED_STATES[i] = BUTTON_CURRENT_STATES[i];
}

/* modulation: 0-127 passed into synth.setMod()
   pitchbend : 0-100 hertz added to each note
   waveform  : 0-5   serving as an index to the WAVEFORMS array
   octave    : 0-6   serving as an index to the octave table

   other possible parameters could include: length (0-127),
   and envelope (0-3, as an index to the ENVELOPES array)
*/
void read_pot_states()
{
    INPUT_MODULATION = map(analogRead(POT_MODULATION), 0, 1023, 0, 127);
    INPUT_PITCHBEND = map(analogRead(POT_PITCHBEND), 0, 1023, 0, 100);
    INPUT_WAVEFORM = WAVEFORMS[map(analogRead(POT_WAVEFORM), 0, 1023, 0, 5)];
    INPUT_OCTAVE = map(analogRead(POT_OCTAVE), 0, 1023, 0, 6);
}

void apply_pot_settings()
{
    for(int i = 0; i < 4; ++i)
    {
        winona.setMod(i, INPUT_MODULATION);
        winona.setWave(i, INPUT_WAVEFORM);
        winona.setFrequency(i, VOICES[i] + INPUT_PITCHBEND);
    }
}

/* the synth library does not support silencing a voice directly --  it can
only suspend() or resume() the entire audio interrupt at once. thus, to silence
a note, we resort to a trick -- set the length parameter of the voice to 0.
unfortunately this results in unwanted pops (probably due to the envelope of
the note). i figured out a voice setup that minimize this (as seen below). */
void silence(int note)
{
    for (int i = 0; i < 4; ++i)
    {
        if (VOICES[i] == note)
        {
            VOICES[i] = -1;
            winona.setupVoice(i, TRIANGLE, 0, ENVELOPE3, 0, 0);
            winona.trigger(i);
            NUM_VOICES_PLAYING--;
            break;
        }
    }
}

void play(int note)
{
    /* if the voice at index FREE_VOICE isn't available */
    if(VOICES[FREE_VOICE] != -1)
    {
        /* if not all the voices are being used */
        if(NUM_VOICES_PLAYING < 4)
        {
            /* find a free voice */
            for(int i = 0; i < 4; ++i)
                if(VOICES[i] == -1)
                    FREE_VOICE = i;
        }
        else
        {
            /* othewise, replace the oldest voice */
            FREE_VOICE = (FREE_VOICE + 1) % 4;
        }
    }
 
    VOICES[FREE_VOICE] = note;
    winona.setupVoice(FREE_VOICE, TRIANGLE, 127, ENVELOPE1, 127, INPUT_MODULATION);
    winona.setFrequency(FREE_VOICE, note);
    winona.trigger(FREE_VOICE);
    NUM_VOICES_PLAYING++;

    FREE_VOICE = (FREE_VOICE + 1) % 4;
}

/* without this function, notes would occasionally not stop playing even when
   their corresponding button was released. so, you'd be left with a "ghost"
   note, playing even when you're not touching anything (and it wouldn't stop).
   this was very unpleasant. i confirmed that we were detecting the button's
   release, so the cause was unknown. this function fixes that phenomenon by
   performing a sanity check -- are all the buttons off? if so, then make sure
   all the voices are off too.

  if the sum of the current states is 0, they are all low (off). if the sum of
  the cached button states is not 0, 1 or more of the buttons were were pressed
  previously, and have now been released. this is the right time to silence all
  of the voices just in case!
*/
void sanity_check()
{
    int sum = 0;
    int cache_sum = 0;
    for(int i = 0; i < NUM_BUTTONS; ++i)
    {
        sum += BUTTON_CURRENT_STATES[i];
        cache_sum += BUTTON_CACHED_STATES[i];
    }

    if(sum == 0 && cache_sum != 0) /* all buttons off, some just released */
        for(int i = 0; i < 4; ++i) /* turn voices all voices just in case */
        {
            VOICES[i] = -1;
            winona.setupVoice(i, TRIANGLE, 0, ENVELOPE3, 0, 0);
        }
}

void setup()
{
    delay(500); /* schweitzer's lemma */

    #ifdef DEBUG_MODE
        Serial.begin(9600);
        Serial.println("Start!");
    #endif
 
    init_synth();
}

void loop() {
    read_button_states();
    read_pot_states();
    apply_pot_settings();

    for (int btn = 0; btn < NUM_BUTTONS; ++btn)
    {
        if (isPressed(btn))
        {
            lastDebounceTime = millis();

            int note = OCTAVE_TABLE[INPUT_OCTAVE][btn];
            play(note);

            #ifdef DEBUG_MODE
                Serial.print("Button ");
                Serial.print(btn);
                Serial.println(" pressed!");
                debug_show_state();
            #endif
        }
        else if (isReleased(btn))
        {
            lastDebounceTime = millis();
 
            int note = OCTAVE_TABLE[INPUT_OCTAVE][btn];
            silence(note);

            #ifdef DEBUG_MODE
                Serial.print("Button ");
                Serial.print(btn);
                Serial.println(" released!");
            #endif
        }
    }
    sanity_check();
    cache_button_states();
}
