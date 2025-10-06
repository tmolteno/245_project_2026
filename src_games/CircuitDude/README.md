Have to download the sketch yourself from https://community.arduboy.com/t/circuit-dude-3-2/2490 / http://crait.net/download.php?file=CircuitDude.ino.

To make sure that it saves which level you are up to, 

 - Add `EEPROM.commit();` after `EEPROM.put(SAVELOCATION, maxlevel);` around line 1450
 - `#define SAVELOCATION	EEPROM_STORAGE_SPACE_START + 5` at line 841 instead of it's original value.
