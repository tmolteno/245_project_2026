# Arduboy 1010
Just a little 1010 puzzle game.

    Written by Mike Meyer (mwm@mired.org)
    Original files from:
https://chiselapp.com/user/mwm/repository/1010

    Modified by Scott Allen to lower the RGB LED brightness
    Modified version at:
https://github.com/MLXXXp/Tiny-1010

----------

Here's how to play the current version.

## Moving mode
Once you've selected a shape (see below), the arrow keys will move it
around on the board. The A button will drop it in that position and
switch back to selection mode.

While in moving mode, the LED color indicates if the shape will fit
where it is: *green* for yes or go ahead and place, *red* for no.

## Selection mode
You start the game in selection mode, choosing from up to three
shapes. The current shape is show on the left side of the screen. Each
available shape is indicated by a *-* to the left of the displayed
shape, with the current shape being a *+*. Once a shape is placed,
it's symbol is gone.

The left arrow places the current shape in the upper right corner and
changes to moving mode.

The A button will try and find a place where it fits, making the LED
red while it searches. If it finds one, it switches to moving mode. If
the shape will not fit on the current board, the LED goes off and you
are left in selection mode.

## Pause
At any time, you can hit the B button and pause the game. This opens
the menu so you can start a new game if you want.

## Menu
The menu entries are:

* Help - how to play the game
* Resume - restart a game if paused.
* New - start a new game
* Keys - a table of button actions
* About - info about the game

## Done
If it ever becomes impossible to place any of the available pieces, the game ends.
