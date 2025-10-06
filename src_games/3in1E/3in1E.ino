//                >>>>>  3in1E for ARDUBOY  GPL v3 <<<<
//                  Programmer: Daniel Champagne 2021
//            Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                    https://www.tinyjoypad.com
//           https://sites.google.com/view/arduino-collection

//  3in1E is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// 3in1E =(TINY-MISSILE + TINY-MORPION + TINY-PIPE)
// Reference in file "COPYING.txt".
// -__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-


// The 3in1E source code include commands referencing to librairy 
// Arduboy2 who is not include in the source code.

// Reference in file "Arduboy2_library_LICENSE.txt".
// https://github.com/MLXXXp/Arduboy2

// A HEX file "3in1E.ino.leonardo.hex" is provided with the source code which includes
// compiled code from the Arduboy2 library.
// Reference in the file "Arduboy2_library_LICENSE.txt".

#include <Arduboy2.h> // https://github.com/MLXXXp/Arduboy2
Arduboy2 arduboy;
#include "PIC_3in1E.h"
#include "Tiny-Missile_ARDUBOY/Tiny-Missile_ARDUBOY.h"
#include "tiny_Morpion_ARDUBOY/tiny_Morpion_ARDUBOY.h"
#include "Tiny-Pipe_ARDUBOY/Tiny-Pipe_ARDUBOY.h"


//public var
int8_t POS_Cursor;

void setup() {
arduboy.begin();
}

void loop() {
POS_Cursor=13;
int8_t Trig_3in1E=0;
while(1){
if (TINYJOYPAD_LEFT) {Trig_3in1E=-1;}
if (TINYJOYPAD_RIGHT) {Trig_3in1E=1;}

if ((BUTTON_DOWN)&&(Trig_3in1E==0)) {Select_GAMES_3in1E();}
Check_POS_3in1E(&Trig_3in1E);
Tiny_Flip_3in1E();
}
}
////////////////////////////////// main end /////////////////////////////////

void Select_GAMES_3in1E(){
switch(POS_Cursor){
  case(13):UNPUSH_WAIT();loop_TMISSILE();break;
  case(56):UNPUSH_WAIT();loop_TMORPION();break;
  case(99):UNPUSH_WAIT();loop_TPIPE();break;
  default:break;
}
}

void UNPUSH_WAIT(void){
Sound(100,255);
while(1){
if (BUTTON_UP) {Sound(1,255);break;}
}
}

uint8_t Check_POS_3in1E(int8_t *trig_){ 
if (*trig_==1) POS_Cursor=(POS_Cursor<99)?POS_Cursor+1:99;
if (*trig_==-1) POS_Cursor=(POS_Cursor>13)?POS_Cursor-1:13;
  switch(POS_Cursor){
   case(13):*trig_=0;return 1;break;
   case(56):*trig_=0;return 1;break;
   case(99):*trig_=0;return 1;break;
   default:return 0;break;
  }
 
}

uint8_t Menu_3in1E(uint8_t xPASS,uint8_t yPASS){
return pgm_read_byte(&MENU_3in1E[xPASS+(yPASS*128)]);
}

uint8_t Selector_3in1E(uint8_t xPASS,uint8_t yPASS){
return blitzSprite(POS_Cursor,55,xPASS,yPASS,0,SELECTOR_3in1E);
}

uint8_t Recupe_3in1E(uint8_t xPASS,uint8_t yPASS){
return (Menu_3in1E(xPASS,yPASS)|(Selector_3in1E(xPASS,yPASS)));
}

void Tiny_Flip_3in1E(void){
uint8_t y,x; 
for (y = 0; y < 8; y++){ 
for (x = 0; x <128; x++){
arduboy.SPItransfer(Recupe_3in1E(x,y));}
}}

