# endless-sky-editor
Map editor for the Endless Sky universe.

To compile the editor, you will need to install Qt Creator. Binary releases will happen occasionally, but are not as high a priority right now as the development of the game itself.


## Editing a map file

To edit a map file, use File -> Open... and browse to the file you want to edit. If it is in a standard game data location (i.e. an “images” folder exists one level up) the editor will also load the landscape images and sprites for planets.
 
There are keyboard shortcuts (shown in the menus) for automatically generating systems, asteroids, commodity prices, etc. These shortcuts only work if you do not currently have a text entry box selected.
 
 
## The Galaxy tab
 
In the Galaxy tab, left click on a system to select it. You can then drag it around to reposition it, or double-click to switch to the System tab. To pan the view, click and drag on the background (i.e. the space in between systems). The large circle around the selected system shows the range of a jump drive.
 
To add a new star system, right click on the background. When a system is selected, you can toggle hyperlinks to that system by right clicking on other systems.
 
To randomize commodity prices, click on the name of one of the commodities in the list (not the price, or you’ll go into text editing mode) and then press ‘C’. The entire region of space connected to the currently selected system will have new commodity prices assigned. Keep randomizing the prices until you end up with something that makes sense, e.g. food and clothing cheaper in “frontier” regions and medical goods and equipment cheaper on more developed worlds.
 
You can delete the currently selected system by pressing the delete key.
 
The “galaxy” objects in the map file define background images, including the big image of the galaxy itself and the text labels for different regions of space. Right now, you need to add these to the map file manually. The existing labels use 24-point Zapfino font, with the fill color set to #AABBCCDD.

### Paint tool

The paint tool is a tool to quickly create large amounts of systems, links, and planets

When pressing ctrl+t or going to Galaxy > Set Tool Options, you can increase the paint tool range. When it is 0, the paint tool is disabled. Larger numbers will allow you to create more systems at the same time.

You can also change it using shift+scroll wheel

Most of the settings there are gaussian distribution parameters to tweak the random parameters of the paint tool. Other settings should be self-explanatory

Use right-click+drag on an empty place in the galaxy map to create more systems using the paint tool.
 
## The System tab
 
In the System tab, you can edit the stellar objects in a single star system, including their orbital positions and paths. Usually it’s sufficient to just generate random star systems (using the keyboard shortcuts in the “System” menu) until you get one that matches what you want the system to contain. You can also click and drag objects to change their orbits, and there are keyboard shortcuts for randomly changing sprites or adding objects to the system.
 
For some special objects (like wormholes and unusual stars) you will need to edit the map file manually to add them in. For example, to create a wormhole you might just create a gas giant as a placeholder, then manually change its sprite. You will also need to edit the map file manually if you want to specify a star system’s background haze or ambient music.
 
In the sidebar on the left, you can view and edit a system’s commodity prices, fleets, and minables. Fleet names need to match something defined in the game data files. The “period” of a fleet is the average number of frames in between times that that fleet appears. (A frame is a 60th of a second, so a fleet with a period of 3600 appears once a minute.) Minables can be randomized by pressing ‘H’; you can also edit them manually.
 
In the game data, a “planet” specifically means a stellar object that you can land on. To define planet data for an object, double-click it in the System view.
 
 
## The Planet tab
 
In the Planet tab, you can select a landscape photo for the planet by clicking an image in the gallery. If you hover your mouse over the current image, it will tell you how many planets that image is used for. (This is to help make sure certain photos don’t end up overused.)
 
Every planet must have a description and a landscape photo. The one exception is wormholes, since the planet dialog does not come up when you land on them. A wormhole should be defined in the map file only if its path should be shown on the map, and its spaceport text should be filled with a placeholder if NPC ships should land on it.
 
As with the fleet list, the shipyard and outfitter lists need to use names of shipyards and outfitters that are defined elsewhere in the data files.
 
The “bribe” amount is given as a fraction of your fleet’s net worth. Security is a value between 0 and 1 indicating the chance of your cargo and outfits being checked for illegal items. Required reputation controls whether you can land on a planet; for example, some high-security worlds may not let you land even if you’re friendly, and some low-security pirate worlds may let anyone land even if they’re hostile to you.
 
For tribute, the threshold is the raw combat rating you must have in order to demand tribute from a planet. The tribute amount is the daily payoff in credits. Currently each planet can only define a single defense fleet type.
