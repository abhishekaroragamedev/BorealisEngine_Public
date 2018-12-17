SD1: Asteroids, Assignment 3
Abhishek Arora
09/21/2017

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Known Issues
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

1. When the Xbox Controller joystick is snapped to the end rapidly and released, the orientation of the ship is (sometimes) set wrongly after the release.
2. The Release builds freeze up after a few seconds (crash) when Xbox controllers are connected, but the Debug builds don't.

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
How to Use
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

"Asteroids Gold" and its code can be consumed in the following ways:

1. Playing the game directly using the executable:
	- Navigate to the "Run_Win32/" folder and double-click on "Asteroids.exe" (this is the release build of the game).
2. Building the game in Visual Studio:
	- Double-click on either the "Asteroids.sln" file in the root folder of the project or on the "Game.vcxproj" file under the "Code/Game/" directory to open the solution in Visual Studio.
	- Select a build configuration (Debug or Release) in the drop down located in the toolbar.
	- Right click on the "Asteroids" Solution in the Solution Explorer and click "Rebuild All".
	- Once the build is done, click on the Play button in the toolbar that reads "Local Windows Debugger" to play the game.

This is a 1-5 player game, with one keyboard input and 0-4 Xbox controller inputs. An Xbox controller can be bluetooth-paired or connected via USB.
Once the game is loaded, you will see a black screen with 1-5 "Ships" in the center (overlapping, and dependent upon how many players are connected), pointing upward, and 4-6 "Asteroids" moving into the screen.
If an asteroid collides with a ship, it will "die", and needs to be respawned. The ships can fire "Bullets" to destroy asteroids.
You can press the following keys to perform the associated actions:

1. Pressing 'Esc' will quit the game.
2. Pressing 'P' will pause the game. Pressing it again will unpause it.
3. Pressing and holding down 'T' will slow the asteroids down. Releasing the button will restore their speed.
4. Pressing 'I' will spawn a new asteroid in a random position.
5. Pressing 'O' will destroy an existing asteroid.
6. Pressing the up arrow key or 'E' (or using the controller's analog joystick) will make the ship "thrust" forward in the direction that it is facing.
7. Pressing the left arrow key or 'S' (or using the controller's analog joystick) will rotate the player ship counter-clockwise.
8. Pressing the right arrow key or 'F' will rotate the player ship clockwise. The ships controlled by the analog joystick will automatically snap to the direction in which the joystick is pulled.
9. Pressing the space key (or the 'A' button on the controller) will fire bullets from the ship's nose, in the direction that the ship is facing.
10. Pressing the 'N' key (or the 'Start' button on the controller) will respawn the ship, if it is dead.
11. Pressing the F1 key will toggle "Developer Mode", which will display the velocity vectors, collision circles and cosmetic circles of the Ship, the Asteroids and the Bullets.

Asteroids Gold comes with three cool, juicy features!
1. Multiplayer with 1-4 Xbox controllers, in addition to the keyboard
2. Screen shake when ships die
3. Controller vibration when ships die