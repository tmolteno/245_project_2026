//                >>>>>  3in1C for ARDUBOY  GPL v3 <<<<
//                   Programmer: Daniel C 2019-2021
//             Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                    https://www.tinyjoypad.com
//           https://sites.google.com/view/arduino-collection

//  3in1C is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// Reference in file "COPYING.txt".
// -__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-


// The 3in1C source code include commands referencing to librairy 
// Arduboy2 who is not include in the source code.

// Reference in file "Arduboy2_library_LICENSE.txt".
// https://github.com/MLXXXp/Arduboy2

// A HEX file "3in1C.ino.leonardo.hex" is provided with the source code which includes
// compiled code from the Arduboy2 library.
// Reference in the file "Arduboy2_library_LICENSE.txt".

// 3in1C = (TINY-BERT + TINY-BIKE + TINY-TRICK)

#include <Arduboy2.h> //https://github.com/MLXXXp/Arduboy2
Arduboy2 arduboy;
#include "PIC_3in1C.h"
#include "TINY_BERT_ARDUBOY/TINY_BERT.h"
#include "TINY_BIKE_ARDUBOY/TINY_BIKE.h"
#include "TINY_TRICK_ARDUBOY/TINY_TRICK.h"

//public var
int8_t POS_Cursor;


void setup() {
arduboy.begin();
}

void loop() {
POS_Cursor=13;
int8_t Trig_3in1C=0;
while(1){
if (TINYJOYPAD_LEFT) {Trig_3in1C=-1;}
if (TINYJOYPAD_RIGHT) {Trig_3in1C=1;}

if ((BUTTON_DOWN)&&(Trig_3in1C==0)) {Select_GAMES_3in1C();}
Check_POS_3in1C(&Trig_3in1C);
Tiny_Flip_3in1C();
}
}
////////////////////////////////// main end /////////////////////////////////

void Select_GAMES_3in1C(){
switch(POS_Cursor){
  case(13):UNPUSH_WAIT();loop_TBERT();break;
  case(56):UNPUSH_WAIT();loop_TBIKE();break;
  case(99):UNPUSH_WAIT();loop_TTRICK();break;
  default:break;
}
}

void UNPUSH_WAIT(void){
Sound(100,255);
while(1){
if (BUTTON_UP) {Sound(1,255);break;}
}
}

uint8_t Check_POS_3in1C(int8_t *trig_){ 
if (*trig_==1) POS_Cursor=(POS_Cursor<99)?POS_Cursor+1:99;
if (*trig_==-1) POS_Cursor=(POS_Cursor>13)?POS_Cursor-1:13;
  switch(POS_Cursor){
   case(13):*trig_=0;return 1;break;
   case(56):*trig_=0;return 1;break;
   case(99):*trig_=0;return 1;break;
   default:return 0;break;
  }
 
}


uint8_t Menu_3in1C(uint8_t xPASS,uint8_t yPASS){
return pgm_read_byte(&MENU_3in1C[xPASS+(yPASS*128)]);
}

uint8_t Selector_3in1C(uint8_t xPASS,uint8_t yPASS){
return blitzSprite(POS_Cursor,56,xPASS,yPASS,0,SELECTOR_3in1C);
}

uint8_t Recupe_3in1C(uint8_t xPASS,uint8_t yPASS){
return (Menu_3in1C(xPASS,yPASS)|(Selector_3in1C(xPASS,yPASS)));
}

uint8_t blitzSprite(int8_t xPos,int8_t yPos,uint8_t xPASS,uint8_t yPASS,uint8_t FRAME,const uint8_t *SPRITES){
uint8_t OUTBYTE;
uint8_t WSPRITE=(pgm_read_byte(&SPRITES[0]));
uint8_t HSPRITE=(pgm_read_byte(&SPRITES[1]));
uint16_t Wmax=((HSPRITE*WSPRITE)+1);
uint16_t PICBYTE=FRAME*(Wmax-1);
int8_t RECUPELINEY=RecupeLineY(yPos);
if ((xPASS>((xPos+(WSPRITE-1))))||(xPASS<xPos)||((RECUPELINEY>yPASS)||((RECUPELINEY+(HSPRITE))<yPASS))) {return 0x00;}
int8_t SPRITEyLINE=(yPASS-(RECUPELINEY));
uint8_t SPRITEyDECALAGE=(RecupeDecalageY(yPos));
uint16_t ScanA=(((xPASS-xPos)+(SPRITEyLINE*WSPRITE))+2);
uint16_t ScanB=(((xPASS-xPos)+((SPRITEyLINE-1)*WSPRITE))+2);
if (ScanA>Wmax) {OUTBYTE=0x00;}else{OUTBYTE=SplitSpriteDecalageY(SPRITEyDECALAGE,pgm_read_byte(&SPRITES[ScanA+(PICBYTE)]),1);}
if ((SPRITEyLINE>0)) {
uint8_t OUTBYTE2=SplitSpriteDecalageY(SPRITEyDECALAGE,pgm_read_byte(&SPRITES[ScanB+(PICBYTE)]),0);
if (ScanB>Wmax) {return OUTBYTE;}else{return OUTBYTE|OUTBYTE2;}
}else{return OUTBYTE;}
}

uint8_t SplitSpriteDecalageY(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN){
if (UPorDOWN) {return Input<<decalage;}
return Input>>(8-decalage);
}

int8_t RecupeLineY(int8_t Valeur){
return (Valeur>>3); 
}

uint8_t RecupeDecalageY(uint8_t Valeur){
return (Valeur-((Valeur>>3)<<3));
}


void Tiny_Flip_3in1C(void){
uint8_t y,x; 
for (y = 0; y < 8; y++){ 
for (x = 0; x <128; x++){
arduboy.SPItransfer(Recupe_3in1C(x,y));}
}}

void Sound(uint8_t freq,uint8_t dur){
   DanCSound(freq, dur);
}
