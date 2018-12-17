SD1: Adventure - Gold
Abhishek Arora

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Known Issues
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

1. The Xbox controller's analog joystick may flip its direction (perfectly diametrically) if it is snapped rapidly in one direction and released immediately.
Investigation: The value coming directly from the controller joystick itself is more than 30% of the way from the center, in the wrong direction. Unsure of how to correct this, since it doesn't fall into the deadzone, but is in the wrong direction.

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
How to Use
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

"Adventure" and its code can be consumed in the following ways:

1. Playing the game directly using the executable:
	- Navigate to the "Run_Win32/" folder and double-click on "Adventure.exe" (this is the release build of the game).
2. Building the game in Visual Studio:
	- Double-click on either the "Adventure.sln" file in the root folder of the project or on the "Game.vcxproj" file under the "Code/Game/" directory to open the solution in Visual Studio.
	- Select a build configuration (Debug or Release) in the drop down located in the toolbar.
	- Right click on the "Adventure" Solution in the Solution Explorer and click "Rebuild All".
	- Once the build is done, click on the Play button in the toolbar that reads "Local Windows Debugger" to play the game.
3. When the game starts, you will see the main menu, with the instructions and controls.
4. You can start an Adventure by using WASD, arrow keys, or the D-PAD on the Xbox controller, to highlight an Adventure, and pressing Space or A on the Controller to finalize your selection.
5. You can use the keyboard or the Xbox controller to play (controls printed on the main main menu screen and on other screens, where useful).
6. The mission objective is displayed in the pause menu.

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Features Implemented
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
•	Rendering features
o	glDrawArrays instead of glBegin
o	Map::RenderTiles() optimized
o	Two sprites per Tile
o	Entity sorted draw order

•	Physics features
o	Actor-vs-Actor disc corrective
o	Momentum, velocity, friction, knock-back

•	Map metadata features
o	Tags (on Tiles and Entities)
o	TileExtraInfo structure

•	Stats & Combat features
o	Stats
o	Combat (fully-featured)

•	Items, Equipment, and Loot features
o	Items (affect stats)
o	Inventory and Equipment (get, drop, equip)
o	Loot (data-driven control)

•	Dialogue features
o	Dialogue object
o	Dialogue game state
o	Adventure, Entity, Item, and/or Portal texts

•	Map Generation features
o	MapGenStep_SpawnActor
o	MapGenStep_SpawnItem

•	Overlay features
o	UI (main menu, inventory menu)
o	HUD (health bars, onscreen status info)

•	Custom feature(s)/Juice
o	Attack/Damage visual feedback - Attack/Damage Quantities/"Miss"/"Dead"/"No Spell" display as text for a short while above actors immediately after attack/damage attempts
o	Status Effect visual feedback - Actors are colored based on one of the four status effects (Poison/Fire/Ice/Electricity, in that order of priority, in case they have multiple status effects). The red flash for physical damage takes an even higher priority than these.
	Also, in the Inventory menu, affected stat(s) will be colored with the status effect color if the Player is currently under any status effect(s).
o	Scrollable/paginated lists with selection boxes - The Adventure list and Inventory list display only a certain number of items (each specified separately in GameConfig.xml), and items can be selected by moving a selection box.
	Other items need to be scrolled through. If more items are visible above and/or below the shown view, arrows are shown (above and/or below) the scrollable view.
	In the Inventory list, you can switch between the Inventory and Equipment lists with the relevant directional keys.
o	Running dialogues - The predominant dialogue mode features text that displays progressively, ie. a letter at a time, based on settings in GameConfig.xml.
o	Equipment visualization - Actors display their equipped inventory (except for their Spell slots) on top of their sprites.

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Credits
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Character sprites/Item sprites - LPC Sprite Creator http://gaurav.munjal.us/Universal-LPC-Spritesheet-Character-Generator/
Item sprites - https://opengameart.org/content/roguelikerpg-items, twitter (@JoeCreates)
Terrain sprites - Prof. Eiserloh