SD1: Incursion Gold
Abhishek Arora

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Known Issues
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

1. While using the speed up key (Y), physics, particularly bullet physics, may (occasionally) break down and cause the game to crash. However, this is not scoped to be fixed, as the speed key is intended to be a debug tool, and works effectively enough for this purpose.

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
How to Use
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

"Incursion" and its code can be consumed in the following ways:

1. Playing the game directly using the executable:
	- Navigate to the "Run_Win32/" folder and double-click on "Incursion.exe" (this is the release build of the game).
2. Building the game in Visual Studio:
	- Double-click on either the "Incursion.sln" file in the root folder of the project or on the "Game.vcxproj" file under the "Code/Game/" directory to open the solution in Visual Studio.
	- Select a build configuration (Debug or Release) in the drop down located in the toolbar.
	- Right click on the "Incursion" Solution in the Solution Explorer and click "Rebuild All".
	- Once the build is done, click on the Play button in the toolbar that reads "Local Windows Debugger" to play the game.

Once the game is launched, a small delay will occur owing to NVIDIA optimizations that are hardcoded for EXEs named "Incursion.exe" (props to our classmate Hongjin Yu for figuring this out).
After that small delay, you will see a tank sprite in a field of textured tiles, and enemy tanks and turrets randomly located around the "map".
Your aim is to make your way to a "portal" at the far right corner of the map. There are three maps in total, with the same objective for each of them, and the next one will load immediately after you hit the portal for the current one.

Controls:
1. The tank can be moved in the 2D plane with a connected Xbox controller's left analog joystick.
2. The tank's turret can be moved in the 2D plane with the Xbox controller's right analog joystick.
3. Bullets can be fired by holding down the controller's right trigger at more than 50% of its reach.
4. Pressing Space, or the start button on the controller, in the main menu, will start the game.
5. Pressing P, or the start button on the controller, while the game is in session (not in the main menu) will pause the game, and show a pause screen. If pressed after the death screen shows up (a few seconds after your tank dies), it will respawn the tank.
6. Pressing Esc, or the back button on the controller, will quit the game if in the main menu. If pressed while the game is paused or when your tank has died, it will open the main menu and reset your progress. If pressed while the game is in session, it will pause the game.

Cheats/Debug tools:
7. Pressing F1 will toggle "Developer mode", in which the cosmetic and physical (collision) circles of the tanks, turrets and bullets will be rendered above them.
8. Holding down T will slow the game down to 10% of its normal speed.
9. Holding down Y will speed the game up to 200% of its normal speed (note that weird stuff can happen with the physics if you do this).
10. Pressing G will (silently) toggle God Mode on or off.
11. Pressing R will toggle between two camera modes - a close-up of the player, and a full view of the map.

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Juice Features chosen for Incursion Gold
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

1. Health conveyance through screen border - As the player takes damage, a red border starts to intrude toward the center of the screen, growing in size and pulsating faster and faster as you get close to death.
2. Health regeneration - About 3 seconds after the player gets hit, health will start regenerating (visually apparent through the gradual fading away of the damage border). If you get hit again before those 3 seconds, the regen will be prevented, and the timer reset!
3. Health conveyance through gameplay audio muting - To complement (1) above, all Game SFX (explosions and gunfire) are progressively muted as the player takes damage (while the background music isn't). As health regenerates, you will gradually hear more of the SFX again.
4. Balance sounds based on direction - Explosion and firing sounds are more balanced in the x-direction from which they "originate", relative to the player; eg. an Enemy tank exploding to the left side of the player will be audible more on the left speaker than the right.
5. Screen shake on damage/death - The screen is lightly nudged every time you take damage, but is more violently shaken the moment you die.
6. Controller vibration - The right part of the controller vibrates a little every time you fire (since the right trigger is used to fire). The left part vibrates a little every time you're hit. Both parts vibrate more when you die, synced with the screen shake.
7. Turret Recoil - Every time a turret (whether on a tank or a stationary turret) is fired, it recoils in the direction opposite to the bullet movement (just visually). If not firing anymore, it slowly comes back to its original position.
8. Flowing liquid tiles - Apart from solid tiles (that block movement and bullets), there are liquid tiles, which block only movement (and thus, stationary turrets can reside in them). To convey that they are liquid, their textures "flow" back and forth in the horizontal direction.

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Credits
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Menu/Pause/Death/Victory screen text(ures): Abhishek Arora (using Photoshop, with the "Courier" font)
Other In-game sprites: Prof. Eiserloh's demo
Menu SFX: Prof. Eiserloh's demo
Menu and level music tracks: Guildhall sound library - Mark Mercury (ASCAP) & Blue Chromium Music (ASCAP)
Game SFX (all explosion, firing sounds): Guildhall sound library

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Deep Learning
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Incursion Gold was a fairly smooth ride, given that my code was written fairly well early on, and not prone to regressions from new features.
A point of suffering did occur as I was completing my juice, when I had started adding liquid tiles. The thought of adding "flowing" tiles occurred to me at 12AM one night,
as I was about to go to sleep. And this idea excited me at that very wrong time to get excited. As a result of this, I sat down to prototype it at that very moment, not
preparing for the onslaught of implementation issues it would throw at me. I had the basic horizontal flow working by 1AM. But I was not satisfied with that - passion knows no bounds!
I wanted to make the waves bump up and down in a sine wave pattern. I started implementing the code for it, and by 1:30AM, I realized that I was drawing textures with quads, so I had to go
back and discrete-divide the texture horizontally if I wanted it to behave like a pseudo-sine wave - thus impacting both my excitement for the idea and my estimated sleep time. But I persevered, pressing
on into the dead of the night till I was prepared to concede defeat - the sine wave movement was not to be. I went to sleep at 4AM. The next day, I settled for a to-and-fro flow instead of a more complex pattern,
and it took much less time to implement than the struggle did the night before. My takeaway from this as a programmer is to use my excitement, but to tame it, as, much like fire, it is a good servant that helps me
get work done, but a bad master when it directs my work entirely. Before I jump into something that's potentially exciting, I will evaluate the repercussions of trying it out, as well as when I should actually work on it, not
compromising everything else in life for it.