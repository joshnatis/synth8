void debug_show_voices_table()
{
  Serial.println("--VOICES-------");
  for (int i = 0; i < 4; ++i)
  {
    Serial.print("Voice ");
    Serial.print(i);
    Serial.print(" :");
    Serial.println(VOICES[i]);
  }
  Serial.println("---------");
}

void debug_show_buttons_table()
{
  Serial.println("--BUTTONS-------");
  for (int i = 0; i < NUM_BUTTONS; ++i)
  {
    Serial.print("Button ");
    Serial.print(i);
    Serial.print(" :");
    Serial.println(BUTTON_CURRENT_STATES[i]);
  }
  Serial.println("---------");
}

void debug_show_pots_table()
{
  Serial.println("--POTS----------");

  Serial.print("MODULATION (");
  Serial.print(POT_MODULATION);
  Serial.print("): ");
  Serial.println(INPUT_MODULATION);

  Serial.print("PITCHBEND (");
  Serial.print(POT_PITCHBEND);
  Serial.print("): ");
  Serial.println(INPUT_PITCHBEND);

  Serial.print("WAVEFORM (");
  Serial.print(POT_WAVEFORM);
  Serial.print("): ");
  Serial.println(INPUT_WAVEFORM);

  Serial.print("OCTAVE (");
  Serial.print(POT_OCTAVE);
  Serial.print("): ");
  Serial.println(INPUT_OCTAVE);

  Serial.println("---------");
}

//void debug_pressed(int btn)
//{
//  Serial.print("Button ");
//  Serial.print(btn);
//  Serial.print(" pressed! Playing [Octave: ");
//  Serial.print(CURRENT_OCTAVE);
//  Serial.print(", Button: ");
//  Serial.print(btn);
//  Serial.print(", Voice:");
//  Serial.print(FREE_VOICE - 1);
//  Serial.print("] = ");
//  Serial.println(OCTAVE_TABLE[CURRENT_OCTAVE][btn]);
//  print_voice_states();
//}

void debug_show_state()
{
  Serial.println("==========");
  debug_show_buttons_table();
  debug_show_voices_table();
  debug_show_pots_table();
  Serial.println("==========\n");
}
