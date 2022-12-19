# lucesDomotica

## Project description

Little domotics project consisting of an esp32 with a led strip connected that can communicate with an android app and with google assistant to change the led strip's config (color, intensity, animations, etc)

The esp32 hosts a wifi server that listens to http queries sent from and android app and a google assistant embedded into a raspberry pi.

The esp32 uses both cores. 
- Core 0 listens to the queries and updates global variables containing the led strip's options (mainly color and mode)
- Core 1 reads those variables and controls the led strip

This was made this way beacause some animations leave blocked the core they execute in until they finish.
This way no query is lost as one core is always listening while the other handles the lighting functionality.

A movement sensor was also added to include a "sleep mode" functionality so that lights only trigger when i'm near.

The google assistant code was not included. Neither was the app because it was made with visual studio temporarely as i want to port it to flutter

## To do
  - [ ] port app to flutter
  - [x] refactor code using another library
