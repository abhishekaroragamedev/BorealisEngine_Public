# BorealisEngine_Public
A public version of my Engine with selective feature branches.

# VolumetricClouds
An implementation of Volumetric Clouds using my engine.
----------------------------------------------------------------------------------------------------------

Structure
Engine/			-> All engine code
Thesis/Volumetric	-> "Game" code for Volumetric Clouds scene
----------------------------------------------------------------------------------------------------------

How to Use

To compile and run:
1. Go to Thesis/Volumetric
2. Open Volumetric.sln (made for VS 2017)
3. Compile and run in Release configuration (must be optimized; Debug configurations are slow)

To run:
1. Go to Thesis/Volumetric/Run_Win32
2. Open Volumetric.exe
----------------------------------------------------------------------------------------------------------

IMPORTANT:
Always run on your best graphics processor. This application is not optimized for use with Integrated GPUs.
----------------------------------------------------------------------------------------------------------

Controls:
WASD/Mouse/TG	- Look around/move
~		- Toggle developer console (type "help" in console for list of commands)
[]		- Change speed
4		- Toggle cloud movement
5		- Toggle mip level debug
6		- Toggle frame amortization
0		- Toggle debug overlay
Mouse wheel	- Change current option to tweak
M/N		- Tweak current option
----------------------------------------------------------------------------------------------------------