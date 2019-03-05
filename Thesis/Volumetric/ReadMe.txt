Volumetric Clouds
Abhishek Arora

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Known Issues
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Building from Volumetric.sln - project won't fully build/run
Workaround:
- Open Volumetric.sln
- Build the Engine project
- Once done, build the Game project
- Run the executable outside Visual Studio by double-clicking on Run_Win32/Volumetric.exe

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
How to Use
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

1. Run the Executable - Run_Win32/Volumetric.exe
2. View source - open Volumetric.sln
	- Build project if needed

Input bindings
--------------
Mouse    	- Camera look
WASD, TG	- Camera movement
Shift (hold)	- Move faster
Arrow keys	- Move sun
Mouse wheel	- Select toggle option
M/N		- Tweak selected toggle option
C		- Toggle clouds
V		- Toggle god rays
6		- Toggle temporal reprojection
~		- Open/close Developer Console

Developer Console commands
--------------------------
NOTE: Only relevant commands described for brevity.
goto <x,y,z>   -	Go to the specified world position. Ex. "goto 0,5200,0" will take you to (0,5200,0).
goin <x,y,z>   -	Go in the specified (camera relative) direction. Ex. "goin 0,0,100" will take you 100 units in the camera's forward direction.
lookat <x,y,z> -	Make the camera look at the specified position. World up is assumed to be the reference up direction, so you cannot use a position directly above or below the camera.
