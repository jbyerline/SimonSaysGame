#include "pitches.h"

#define CHOICE_OFF      0 //Used to control LEDs
#define CHOICE_NONE     0 //Used to check buttons
#define CHOICE_SW0     (1 << 0)
#define CHOICE_SW1     (1 << 1)
#define CHOICE_SW2     (1 << 2)
#define CHOICE_SW3     (1 << 3)

#define LED0     11
#define LED1     9
#define LED2     6
#define LED3     3

// Button pin definitions
#define SW0     12
#define SW1     8
#define SW2     7
#define SW3     4

// Buzzer pin definitions
#define SPKR    5

// Define game parameters
#define ROUNDS_TO_WIN      16//Number of rounds to succesfully remember before you win. 13 is do-able.
#define ENTRY_TIME_LIMIT   30000 //Amount of time to press a button before game times out. 3000ms = 3 sec

// Game state variables
int gameBoard[32]; //Contains the combination of buttons as we advance
int gameRound = 0; //Counts the number of succesful rounds the player has made it through

void setup()
{
  //Setup hardware inputs/outputs. These pins are defined in the hardware_versions header file

  pinMode(SW0, INPUT_PULLUP);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  pinMode(SPKR, OUTPUT);
  pinMode(SPKR, OUTPUT);
}

void loop()
{
  attractMode(); // Blink lights while waiting for user to press a button

  // Indicate the start of game play
  setLEDs(CHOICE_SW0 | CHOICE_SW1 | CHOICE_SW2 | CHOICE_SW3); // Turn all LEDs on
  delay(1000);
  setLEDs(CHOICE_OFF); // Turn off LEDs
  delay(250);
  
    // Play memory game and handle result
    if (play_memory() == true) 
      play_winner(); // Player won, play winner tones
    else 
      endMode(); // Player lost, play loser tones
}

// Play the regular memory game
// Returns 0 if player loses, or 1 if player wins
boolean play_memory(void)
{
  randomSeed(millis()); // Seed the random generator with random amount of millis()

  gameRound = 0; // Reset the game to the beginning

  while (gameRound < ROUNDS_TO_WIN) 
  {
    add_to_moves(); // Add a button to the current moves, then play them back

    playMoves(); // Play back the current game board

    // Then require the player to repeat the sequence.
    for (int currentMove = 0 ; currentMove < gameRound ; currentMove++)
    {
      int choice = wait_for_button(); // See what button the user presses

      if (choice == 0) return false; // If wait timed out, player loses

      if (choice != gameBoard[currentMove]) return false; // If the choice is incorect, player loses
    }

    delay(1000); // Player was correct, delay before playing moves
  }

  return true; // Player made it through all the rounds to win!
}

// Plays the current contents of the game moves
void playMoves(void)
{
  for (int currentMove = 0 ; currentMove < gameRound ; currentMove++) 
  {
    toner(gameBoard[currentMove], 150);

    // Wait some amount of time between button playback
    // Shorten this to make game harder
    delay(150); // 150 works well. 75 gets fast.
  }
}

// Adds a new random button to the game sequence, by sampling the timer
void add_to_moves(void)
{
  int newButton = random(0, 3); //min (included), max (exluded)

  // We have to convert this number, 0 to 3, to CHOICEs
  if(newButton == 0) newButton = CHOICE_SW0;
  else if(newButton == 1) newButton = CHOICE_SW1;
  else if(newButton == 2) newButton = CHOICE_SW2;

  gameBoard[gameRound++] = newButton; // Add this new button to the game array
}

// Lights a given LEDs
// Pass in a int that is made up from CHOICE_SW0, CHOICE_SW3, etc
void setLEDs(int leds)
{
  if ((leds & CHOICE_SW0) != 0)
    digitalWrite(LED0, HIGH);
  else
    digitalWrite(LED0, LOW);

  if ((leds & CHOICE_SW1) != 0)
    digitalWrite(LED1, HIGH);
  else
    digitalWrite(LED1, LOW);

  if ((leds & CHOICE_SW2) != 0)
    digitalWrite(LED2, HIGH);
  else
    digitalWrite(LED2, LOW);

  if ((leds & CHOICE_SW3) != 0)
    digitalWrite(LED3, HIGH);
  else
    digitalWrite(LED3, LOW);
}

// Wait for a button to be pressed. 
// Returns one of LED colors (LED0, etc.) if successful, 0 if timed out
int wait_for_button(void)
{
  long startTime = millis(); // Remember the time we started the this loop

  while ( (millis() - startTime) < ENTRY_TIME_LIMIT) // Loop until too much time has passed
  {
    int button = checkButton();

    if (button != CHOICE_NONE)
    { 
      toner(button, 150); // Play the button the user just pressed

      while(checkButton() != CHOICE_NONE) ;  // Now let's wait for user to release button

      delay(10); // This helps with debouncing and accidental double taps

      return button;
    }
  }
  
  return CHOICE_NONE; // If we get here, we've timed out!
}

// Returns a '1' bit in the position corresponding to CHOICE_SW0, CHOICE_SW1, etc.
int checkButton(void)
{
  if (digitalRead(SW0) == 0) return(CHOICE_SW0); 
  else if (digitalRead(SW1) == 0) return(CHOICE_SW1); 
  else if (digitalRead(SW2) == 0) return(CHOICE_SW2); 
  else if (digitalRead(SW3) == 0) return(CHOICE_SW3);

  return(CHOICE_NONE); // If no button is pressed, return none
}

// Light an LED and play tone
void toner(int which, int buzz_length_ms)
{
  setLEDs(which); //Turn on a given LED

  //Play the sound associated with the given LED
  switch(which) 
  {
  case CHOICE_SW0:
    buzz_sound(buzz_length_ms, 1136); 
    break;
  case CHOICE_SW1:
    buzz_sound(buzz_length_ms, 568); 
    break;
  case CHOICE_SW2:
    buzz_sound(buzz_length_ms, 851); 
    break;
  case CHOICE_SW3:
    buzz_sound(buzz_length_ms, 638); 
    break;
  }

  setLEDs(CHOICE_OFF); // Turn off all LEDs
}

// Toggle buzzer every buzz_delay_us, for a duration of buzz_length_ms.
void buzz_sound(int buzz_length_ms, int buzz_delay_us)
{
  // Convert total play time from milliseconds to microseconds
  long buzz_length_us = buzz_length_ms * (long)1000;

  // Loop until the remaining play time is less than a single buzz_delay_us
  while (buzz_length_us > (buzz_delay_us * 2))
  {
    buzz_length_us -= buzz_delay_us * 2; //Decrease the remaining play time

    // Toggle the buzzer at various speeds
    digitalWrite(SPKR, LOW);
    digitalWrite(SPKR, HIGH);
    delayMicroseconds(buzz_delay_us);

    digitalWrite(SPKR, HIGH);
    digitalWrite(SPKR, LOW);
    delayMicroseconds(buzz_delay_us);
  }
}

// Play the winner sound and lights
void play_winner(void)
{
  setLEDs(CHOICE_SW0 | CHOICE_SW1 | CHOICE_SW2| CHOICE_SW3);
  winner_sound();
  setLEDs(CHOICE_SW0 | CHOICE_SW1 | CHOICE_SW2| CHOICE_SW3);
  winner_sound();
  setLEDs(CHOICE_SW0 | CHOICE_SW1 | CHOICE_SW2| CHOICE_SW3);
  winner_sound();
  setLEDs(CHOICE_SW0 | CHOICE_SW1 | CHOICE_SW2| CHOICE_SW3);
  winner_sound();
}

// Play the winner sound
// This is just a unique (annoying) sound we came up with, there is no magic to it
void winner_sound(void)
{
  // Toggle the buzzer at various speeds
  for (int x = 250 ; x > 70 ; x--)
  {
    for (int y = 0 ; y < 3 ; y++)
    {
      digitalWrite(SPKR, HIGH);
      digitalWrite(SPKR, LOW);
      delayMicroseconds(x);

      digitalWrite(SPKR, LOW);
      digitalWrite(SPKR, HIGH);
      delayMicroseconds(x);
    }
  }
}

// Show an "attract mode" display while waiting for user to press button.
void attractMode(void)
{
  while(1) 
  {
    setLEDs(CHOICE_SW0);
    delay(100);
    if (checkButton() == CHOICE_SW3) return;

    setLEDs(CHOICE_SW2);
    delay(100);
    if (checkButton() == CHOICE_SW3) return;

    setLEDs(CHOICE_SW1);
    delay(100);
    if (checkButton() == CHOICE_SW3) return;

    setLEDs(CHOICE_SW3);
    delay(100);
    if (checkButton() == CHOICE_SW3) return;
  }
}


void endMode(void)
{
  while(1) 
  {
    setLEDs(CHOICE_SW0);
    if (checkButton() == CHOICE_SW3) 
    {
    buzz_sound(255, 1500);
    return;
    }
    setLEDs(CHOICE_SW3);
    if (checkButton() == CHOICE_SW3) 
    {
    buzz_sound(255, 1500);
    return;
    }
  }
}

int LEDnumber = 0; // Keeps track of which LED we are on during the beegees loop

// Each time this function is called the board moves to the next LED
void changeLED(void)
{
  setLEDs(1 << LEDnumber); // Change the LED

  LEDnumber++; // Goto the next LED
  if(LEDnumber > 3) LEDnumber = 0; // Wrap the counter if needed
}
