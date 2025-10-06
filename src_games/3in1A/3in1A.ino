// 3in1A (Tiny Space invaders + Tiny Pinball + Tiny Pacman) for ARDUBOY  GPL v3
//                          Programmer: Daniel C 2018-2021
//                    Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                           https://www.tinyjoypad.com
//                  https://sites.google.com/view/arduino-collection

//   3in1A (Tiny Space invaders + Tiny Pinball + Tiny Pacman)
//   is free software: you can redistribute it and/or modify
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

//-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-

// The 3in1A source code include commands referencing to librairy 
// Arduboy2 who is not include in the source code.

// Reference in file "Arduboy2_library_LICENSE.txt".
// https://github.com/MLXXXp/Arduboy2

// A HEX file "3in1A.hex" is provided with the source code which includes
// compiled code from the Arduboy2 library.
// Reference in the file "Arduboy2_library_LICENSE.txt".

#include <Arduboy2.h> //https://github.com/MLXXXp/Arduboy2
Arduboy2 arduboy;
#include "spritebank.h"

#include "extras.h"

uint8_t MyShoot(uint8_t x,uint8_t y,SPACE *space);

#define BUTTON_DOWN ((arduboy.pressed(A_BUTTON)==1)||(arduboy.pressed(B_BUTTON)==1))
#define BUTTON_UP ((arduboy.pressed(A_BUTTON)==0) && (arduboy.pressed(B_BUTTON)==0))

// var public Pacman
uint8_t LEVELSPEED;
uint8_t GobbingEND;
uint8_t LIVEPacman;
uint8_t INGAME;
uint8_t Gobeactive;
uint8_t TimerGobeactive;
uint8_t add;
uint8_t dotsMem[9];
int8_t dotscount;
uint8_t Frame;
enum {PACMAN=0,FANTOME=1,FRUIT=2};
// fin var public

// var public space invaders
#define MAXLEVELSHIELDED 3
uint8_t Live=0;
uint8_t ShieldRemoved=0;
uint8_t MONSTERrest=0;
uint8_t LEVELS=0;
uint8_t SpeedShootMonster=0;
uint8_t ShipDead=0;
uint8_t ShipPos=64;
// fin var public

//<<<<<<<<<<<<<<<<<<<VARIABLE PUBLIQUE>>>>>>>>>>>>>>>>>
//pinball
#define SPRINGLONG 72
uint8_t SCANLINE=0;
uint8_t totalBall=4;
uint8_t totalpush=0;
uint8_t FIRSTTIME=0;
uint16_t bouncePush=0; 
uint8_t frameCount=0; 
uint8_t SpringBar=SPRINGLONG;
uint8_t MemExa[3][3]={{0}};
int8_t trigFlipG=0;
int8_t trigFlipD=0;
typedef struct BALL{
float x=0;
float y=0;
float Speedx=0;
float Speedy=0;
float SIMx=0;
float SIMy=0;
float SIMSpeedx=0;
float SIMSpeedy=0;
float DecelX=0;
float DecelY=0;
uint8_t resetBall=0;
uint8_t grid[2][3]={{0}};
}BALL;
//<<<<<<<<<<<<<<<<<<<FIN VARIABLE PUBLIQUE>>>>>>>>>>>>>>>>>


void setup() {
 arduboy.begin();
}

void loop(){
uint8_t menuselect=0;
while(1){
if (arduboy.pressed(RIGHT_BUTTON)) {while(1){if (arduboy.pressed(RIGHT_BUTTON)==0) {break;}}
if (menuselect<3) {menuselect++;Sound(60,55);}else{Sound(60,55);menuselect=0;}
}
if (arduboy.pressed(LEFT_BUTTON)) {while(1){if (arduboy.pressed(LEFT_BUTTON)==0) {break;}}
if (menuselect>0) {Sound(2,55);menuselect--;}else{Sound(2,55);menuselect=3;}
}
switch(menuselect){
case 0:SSD1306_ssd1306_draw_bmp(0,1,128,7,MENU);break;
case 1:SSD1306_ssd1306_draw_bmp(0, 0,128,8,intro);break;
case 2:Tiny_Flip2(1,0);break;
case 3:SSD1306_ssd1306_draw_bmp(0,2,128,6,MENUPAC);break;
default:break;
}
if BUTTON_DOWN {while(1){if BUTTON_UP { for(uint8_t r=0;r<60;r++){Sound(2+r,10);Sound(255-r,20);}break;}}
  switch(menuselect){
  case 0:break;
  case 1:loopSpace();break;
  case 2:looppinball();break;
  case 3:loopPacman();break;
  default:break;
}}}}

void ResetVar(void){
LEVELSPEED=200;
GobbingEND=0;
LIVEPacman=3;
Gobeactive=0;
TimerGobeactive=0;
add=0;
INGAME=0;
for(uint8_t t=0;t<9;t++){
dotsMem[t]=0xff;}
dotscount=0;
Frame=0;}

void StartGame(PERSONAGE *Sprite){
if (INGAME==0) {
Sprite[1].x=76;
Sprite[1].y=3;
Sprite[2].x=75;
Sprite[2].y=4;
Sprite[3].x=77;
Sprite[3].y=3;
Sprite[4].x=76;
Sprite[4].y=4;
INGAME=1;}}

void loopPacman() {
uint8_t t;
PERSONAGE Sprite[5];
NEWGAME:
ResetVar();
LIVEPacman=3;
goto New;
NEWLEVEL:
if (LEVELSPEED>10) {LEVELSPEED=LEVELSPEED-10;
if ((LEVELSPEED==160)||(LEVELSPEED==120)||(LEVELSPEED==80)||(LEVELSPEED==40)||(LEVELSPEED==10)){    
if (LIVEPacman<3) {LIVEPacman++; for(t=0;t<=4;t++){
Sound(80,100);delay(300);
}}}}
New:
GobbingEND=(LEVELSPEED/2);
for(t=0;t<9;t++){
dotsMem[t]=0xff;
}
RESTARTLEVEL:
Gobeactive=0;
Sprite[0].type=PACMAN;
Sprite[0].x=64;
Sprite[0].y=3;
Sprite[0].Decalagey=5;
Sprite[0].DirectionV=2;
Sprite[0].DirectionH=2;
Sprite[0].DirectionAnim=0;
Sprite[1].type=FANTOME;
Sprite[1].x=76;
Sprite[1].y=4;
Sprite[1].guber=0;
Sprite[2].type=FANTOME;
Sprite[2].x=75;
Sprite[2].y=5;
Sprite[2].guber=0;
Sprite[3].type=FANTOME;
Sprite[3].x=77;
Sprite[3].y=4;
Sprite[3].guber=0;
Sprite[4].type=FANTOME;
Sprite[4].x=76;
Sprite[4].y=5;
Sprite[4].guber=0;
while(1){
//arduboy.clear();
//joystick
if BUTTON_DOWN {StartGame(&Sprite[0]);}
if (INGAME) {
if (arduboy.pressed(LEFT_BUTTON)) {Sprite[0].DirectionV=0;}
else if (arduboy.pressed(RIGHT_BUTTON)) {Sprite[0].DirectionV=1;}
if (arduboy.pressed(DOWN_BUTTON)) {Sprite[0].DirectionH=1;}
else if (arduboy.pressed(UP_BUTTON)) {Sprite[0].DirectionH=0;}
//fin joystick
if (TimerGobeactive>1)  {TimerGobeactive--;}else{if (TimerGobeactive==1) {TimerGobeactive=0;Gobeactive=0;}}}
if (Frame<24) {Frame++;}else{Frame=0;}
if (CollisionPac2Caracter(&Sprite[0])==0) {RefreshCaracter(&Sprite[0]);}else{
Sound(100,200);Sound(75,200);Sound(50,200);Sound(25,200);Sound(12,200);delay(400);
if (LIVEPacman>0) {LIVEPacman--;goto RESTARTLEVEL;}else{goto NEWGAME;}}
if (Frame%2==0) {
Tiny_FlipPacman(0,&Sprite[0]);
if (INGAME==1){
for (uint8_t t=0;t<=139;t=t+2){
Sound(pgm_read_byte(&Music[t])-8,(pgm_read_byte(&Music[t+1])-100)); 
}INGAME=2;}
}else{
for(t=0;t<63;t++){
if (checkDotPresent(t)) {break;}else{if (t==62) { for(uint8_t r=0;r<60;r++){Sound(2+r,10);Sound(255-r,20);}delay(1000);goto NEWLEVEL;}}
}}
if ((Gobeactive)&&(Frame%2==0)) {Sound((255-TimerGobeactive),1);}
delay(10);
}

}
////////////////////////////////// main end /////////////////////////////////

uint8_t CollisionPac2Caracter(PERSONAGE *Sprite){
uint8_t ReturnCollision=0;
#define xmax(I) (Sprite[I].x+6)
#define xmin(I) (Sprite[I].x)
#define ymax(I) ((Sprite[I].y*8)+Sprite[I].Decalagey+6)
#define ymin(I) ((Sprite[I].y*8)+Sprite[I].Decalagey)
if ((INGAME)) {    
for (uint8_t t=1;t<=4;t++){
if ((xmax(0)<xmin(t))||(xmin(0)>xmax(t))||(ymax(0)<ymin(t))||(ymin(0)>ymax(t))) {}else{ 
if (Gobeactive) {if (Sprite[t].guber!=1) {Sound(20,100);Sound(2,100);}Sprite[t].guber=1;ReturnCollision=0;}else{ if (Sprite[t].guber==1) {ReturnCollision=0;}else{ReturnCollision=1;}}
}}}return ReturnCollision;}

void RefreshCaracter(PERSONAGE *Sprite){
uint8_t memx,memy,memdecalagey;
for (uint8_t t=0;t<=4;t++){
memx=Sprite[t].x;
memy=Sprite[t].y;
memdecalagey=Sprite[t].Decalagey;
if ((Sprite[t].y>-1)&&(Sprite[t].y<8)) {
if ((Frame%2==0)||(t==0)||(LEVELSPEED<=160)) {
if (Sprite[t].DirectionV==1) {Sprite[t].x++;}
if (Sprite[t].DirectionV==0) {
if (t==0) {
if ((Sprite[0].y==3)&&(Sprite[0].x==86)){}else{Sprite[t].x--;}
}else{Sprite[t].x--;}
}}}
if (CheckCollisionWithBack(t,1,Sprite)) {
if (t!=0) {Sprite[t].DirectionV=random()%2;}else{ Sprite[t].DirectionV=2;}
Sprite[t].x=memx;
}
if ((Frame%2==0)||(t==0)||(LEVELSPEED<=160)) {
if (Sprite[t].DirectionH==1) {if (Sprite[t].Decalagey<7) {Sprite[t].Decalagey++;}else{Sprite[t].Decalagey=0;Sprite[t].y++;if (Sprite[t].y==9) {Sprite[t].y=-1;}}}
if (Sprite[t].DirectionH==0) {if (Sprite[t].Decalagey>0) {Sprite[t].Decalagey--;}else{Sprite[t].Decalagey=7;Sprite[t].y--;if (Sprite[t].y==-2) {Sprite[t].y=8;}}}
}
if (CheckCollisionWithBack(t,0,Sprite)) {
if (t!=0) {Sprite[t].DirectionH=random()%2;}else{Sprite[t].DirectionH=2;}
Sprite[t].y=memy;
Sprite[t].Decalagey=memdecalagey;
}
if (t==0) {
if (Frame%2==0) {
if (Sprite[t].DirectionH==1) {Sprite[t].DirectionAnim=0;}
if (Sprite[t].DirectionH==0) {Sprite[t].DirectionAnim=(2*3);} 
if (Sprite[t].DirectionV==1) {Sprite[t].DirectionAnim=(3*3);}
if (Sprite[t].DirectionV==0) {Sprite[t].DirectionAnim=(1*3);}
}}else{
if ((Frame==0)||(Frame==12)) {
Sprite[t].DirectionAnim=0;
if (Sprite[t].DirectionH==1) {Sprite[t].DirectionAnim=0;}
if (Sprite[t].DirectionH==0) {Sprite[t].DirectionAnim=2;}
}}
if (t==0) {
if (Frame%2==0) {
if (Sprite[0].switchanim==0) {
if (Sprite[0].anim<2) {Sprite[0].anim++;}else{Sprite[0].switchanim=1;} 
}else{
if (Sprite[0].anim>0) {Sprite[0].anim--;}else{Sprite[0].switchanim=0;}  
}}}else{
if ((Sprite[t].guber==1)&&(Sprite[t].x>=74)&&(Sprite[t].x<=76)&&(Sprite[t].y>=2)&&(Sprite[t].y<=4)) {Sprite[t].guber=0;}
if  (Frame%2==0) {
if (Sprite[t].anim<1) {Sprite[t].anim++;}else{Sprite[t].anim=0; } 
}}}}

uint8_t CheckCollisionWithBack(uint8_t SpriteCheck,uint8_t HorVcheck,PERSONAGE *Sprite){
uint8_t BacktoComp;
if (HorVcheck==1) {
BacktoComp=RecupeBacktoCompV(SpriteCheck,Sprite); 
}else{
BacktoComp=RecupeBacktoCompH(SpriteCheck,Sprite);}
if ((BacktoComp)!=0) {return 1;}else{return 0;}}

uint8_t RecupeBacktoCompV(uint8_t SpriteCheck,PERSONAGE *Sprite){
uint8_t Y1=0b00000000;
uint8_t Y2=Y1;
#define SpriteWide 6
#define MAXV (Sprite[SpriteCheck].x+SpriteWide)
#define MINV (Sprite[SpriteCheck].x)
if (Sprite[SpriteCheck].DirectionV==1) {
Y1=pgm_read_byte(&backPac[((Sprite[SpriteCheck].y)*128)+(MAXV)]);
Y2=pgm_read_byte(&backPac[((Sprite[SpriteCheck].y+1)*128)+(MAXV)]);
}else if (Sprite[SpriteCheck].DirectionV==0) {
Y1=pgm_read_byte(&backPac[((Sprite[SpriteCheck].y)*128)+(MINV)]);
Y2=pgm_read_byte(&backPac[((Sprite[SpriteCheck].y+1)*128)+(MINV)]);
}else{Y1=0;Y2=0;}
//decortique
Y1=Trim(0,Y1,Sprite[SpriteCheck].Decalagey);
Y2=Trim(1,Y2,Sprite[SpriteCheck].Decalagey);
if (((Y1)!=0b00000000)||((Y2)!=0b00000000) ) {return 1;}else{return 0;}
}

uint8_t Trim(uint8_t Y1orY2,uint8_t TrimValue,uint8_t Decalage){
uint8_t Comp;
if (Y1orY2==0) {
Comp=0b01111111<<Decalage;
return (TrimValue&Comp);
}else{
Comp=(0b01111111>>(8-Decalage));
return (TrimValue&Comp);
}}

uint8_t ScanHRecupe(uint8_t UporDown,uint8_t Decalage){
if (UporDown==0){
return 0b01111111<<Decalage;}
else{
return 0b01111111>>(8-Decalage);
}}

uint8_t RecupeBacktoCompH(uint8_t SpriteCheck,PERSONAGE *Sprite){
uint8_t TempPGMByte;
if (Sprite[SpriteCheck].DirectionH==0) {
uint8_t RECUPE=(ScanHRecupe(0,Sprite[SpriteCheck].Decalagey));
for(uint8_t t=0;t<=6;t++){
if ((((Sprite[SpriteCheck].y)*128)+(Sprite[SpriteCheck].x+t)>1023)||(((Sprite[SpriteCheck].y)*128)+(Sprite[SpriteCheck].x+t)<0)) {TempPGMByte=0x00;}else{
 TempPGMByte=(pgm_read_byte(&backPac[((Sprite[SpriteCheck].y)*128)+(Sprite[SpriteCheck].x+t)])); 
}
#define CHECKCOLLISION ((RECUPE)&(TempPGMByte))
if  (CHECKCOLLISION!=0) {return 1;}
}}else if (Sprite[SpriteCheck].DirectionH==1) {
uint8_t tadd=0;
if (Sprite[SpriteCheck].Decalagey>2) { tadd=1;}else{tadd=0;}
uint8_t RECUPE=(ScanHRecupe(tadd,Sprite[SpriteCheck].Decalagey));
for(uint8_t t=0;t<=6;t++){
if (((((Sprite[SpriteCheck].y+tadd)*128)+(Sprite[SpriteCheck].x+t))>1023)||((((Sprite[SpriteCheck].y+tadd)*128)+(Sprite[SpriteCheck].x+t))<0)) {TempPGMByte=0x00;}else{
TempPGMByte=pgm_read_byte(&backPac[((Sprite[SpriteCheck].y+tadd)*128)+(Sprite[SpriteCheck].x+t)]);
}
#define CHECKCOLLISION2 ((RECUPE)&(TempPGMByte))
if  (CHECKCOLLISION2!=0) {return 1;}
}}return 0;}

void Tiny_FlipPacman(uint8_t render0_picture1,PERSONAGE *Sprite){
uint8_t y,x; 
dotscount=-1;
for (y = 0; y < 8; y++){ 

for (x = 0; x < 128; x++){
if (render0_picture1==0) {
if (INGAME) {
arduboy.SPItransfer(background(x,y)|SpriteWrite(x,y,Sprite)|DotsWrite(x,y,Sprite)|LiveWrite(x,y)|FruitWrite(x,y));
  }else{
arduboy.SPItransfer(0xff-(background(x,y)|SpriteWrite(x,y,Sprite)));
}}else if (render0_picture1==1){
}}

}}

uint8_t FruitWrite(uint8_t x,uint8_t y){
switch(y){
  case 7:if (x<=7) {return pgm_read_byte(&fruits[x]);}break;
  case 6:if ((LEVELSPEED<=190)&&(x<=7)) {return pgm_read_byte(&fruits[x+8]);}break;
  case 5:if ((LEVELSPEED<=180)&&(x<=7)) {return pgm_read_byte(&fruits[x+16]);}break;
  case 4:if ((LEVELSPEED<=170)&&(x<=7)) {return pgm_read_byte(&fruits[x+24]);}break;
}return 0;}

uint8_t LiveWrite(uint8_t x,uint8_t y){
if (y<LIVEPacman) {if (x<=7) {return pgm_read_byte(&caracters[x+(1*8)]);}else{return 0x00;}}
return 0x00;}

uint8_t DotsWrite(uint8_t x,uint8_t y,PERSONAGE *Sprite){
uint8_t Menreturn=0;
uint8_t mem1=pgm_read_byte(&dots[x+(128*y)]);
if (mem1!=0b00000000) {
dotscount++;
 switch(dotscount){
  case 0:  
  case 1: 
  case 12: 
  case 13: 
  case 50:
  case 51:
  case 62:
  case 63:Menreturn=1;break;
  default:Menreturn=0;break;
}
if (checkDotPresent(dotscount)==0b00000000) {mem1=0x00;}else{
if ((Sprite[0].type==PACMAN)&&((Sprite[0].x<x)&&(Sprite[0].x>x-6))&&(((Sprite[0].y==y)&&(Sprite[0].Decalagey<6))||((Sprite[0].y==y-1)&&(Sprite[0].Decalagey>5)))) {DotsDestroy(dotscount);if (Menreturn==1) {TimerGobeactive=LEVELSPEED;Gobeactive=1;}else{Sound(10,10);Sound(50,10);}}
}}
if (Menreturn==1) {
if (((Frame>=6)&&(Frame<=12))||((Frame>=18)&&(Frame<=24))) {
return 0x00;
}else{return mem1;}
}else{return mem1;}}

uint8_t checkDotPresent(uint8_t  DotsNumber){
uint8_t REST=DotsNumber;
uint8_t DOTBOOLPOSITION=0;
DECREASE:
if (REST>=8) {REST=REST-8;DOTBOOLPOSITION++;goto DECREASE;}
return ((dotsMem[DOTBOOLPOSITION])&(0b10000000>>REST));
}

void DotsDestroy(uint8_t DotsNumber){
uint8_t REST=DotsNumber;
uint8_t DOTBOOLPOSITION=0;
uint8_t SOUSTRAIRE;
DECREASE:
if (REST>=8) {REST=REST-8;DOTBOOLPOSITION=DOTBOOLPOSITION+1;goto DECREASE;}
switch(REST){
  case (0):SOUSTRAIRE=0b01111111;break;
  case (1):SOUSTRAIRE=0b10111111;break;
  case (2):SOUSTRAIRE=0b11011111;break;
  case (3):SOUSTRAIRE=0b11101111;break;
  case (4):SOUSTRAIRE=0b11110111;break;
  case (5):SOUSTRAIRE=0b11111011;break;
  case (6):SOUSTRAIRE=0b11111101;break;
  case (7):SOUSTRAIRE=0b11111110;break;
}
dotsMem[DOTBOOLPOSITION]=dotsMem[DOTBOOLPOSITION]&SOUSTRAIRE;
}
//jhjhjhjhjhjhjhjhjkkkkllklklklklklklklkkl
uint8_t SplitSpriteDecalageY_pac(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN){
if (UPorDOWN) {
return Input<<decalage;
}else{
return Input>>(8-decalage); 
}}

uint8_t SpriteWrite(uint8_t x,uint8_t y,PERSONAGE  *Sprite){
uint8_t var1=0;
uint8_t AddBin=0b00000000;
while(1){ 
if (Sprite[var1].y==y) {
AddBin=AddBin|SplitSpriteDecalageY_pac(Sprite[var1].Decalagey,return_if_sprite_present(x,Sprite,var1),1);
}else if (((Sprite[var1].y+1)==y)&&(Sprite[var1].Decalagey!=0)) {
AddBin=AddBin|SplitSpriteDecalageY_pac(Sprite[var1].Decalagey,return_if_sprite_present(x,Sprite,var1),0);
}
var1++;
if (var1==5) {break;}
}return AddBin;}
  
uint8_t return_if_sprite_present(uint8_t x,PERSONAGE  *Sprite,uint8_t SpriteNumber){
uint8_t ADDgobActive;
uint8_t ADDGober;
if  ((x>=Sprite[SpriteNumber].x)&&(x<(Sprite[SpriteNumber].x+8))) { 
if (SpriteNumber!=0) { 
if (Sprite[SpriteNumber].guber==1) {
ADDgobActive=1*(4*8);
ADDGober=Sprite[SpriteNumber].guber*(4*8);
}else{
if ((((Frame>=6)&&(Frame<=12))||((Frame>=18)&&(Frame<=24)))||(TimerGobeactive>GobbingEND)) {ADDgobActive=Gobeactive*(4*8);}else{ADDgobActive=0;}
ADDGober=0;
}}else{
ADDGober=0;
ADDgobActive=0;
}
if ((INGAME==0)&&(SpriteNumber==0)) {  return 0;}     
return pgm_read_byte(&caracters[((x-Sprite[SpriteNumber].x)+(8*(Sprite[SpriteNumber].type*12)))+(Sprite[SpriteNumber].anim*8)+(Sprite[SpriteNumber].DirectionAnim*8)+(ADDgobActive)+(ADDGober)]);
}return 0;}

uint8_t background(uint8_t x,uint8_t y){
return pgm_read_byte(&BackBlitz[((y)*128)+((x))]);
}

void Sound(uint8_t freq,uint8_t dur){
   DanCSound(freq, dur);
}

//Fin Pacman
//Fin Pacman
//Fin Pacman
//Fin Pacman

//Fin Pacman
//Fin Pacman
//Fin Pacman
//Fin Pacman

void LoadMonstersLevels(int8_t Levels,SPACE *space){
uint8_t x,y;
for (y=0;y<4;y++){
for (x=0;x<6;x++){
space->MonsterGrid[y][x]=pgm_read_byte(&MonstersLevels[(Levels*24)+((y)*6)+(x)]); 
}}}

void loopSpace() {
  #define SHOOTS 2
  uint8_t Decompte=0;
  uint8_t VarPot;
  uint8_t MyShootReady=SHOOTS;
SPACE space;
NEWGAME:;
Live=3;
LEVELS=0;
while(1){
Tiny_FlipSpace(1,&space);
if BUTTON_DOWN {Sound(100,125);Sound(50,125);_delay_ms(500);goto BYPASS2;}
}
NEWLEVEL:
_delay_ms(1000);
BYPASS2:
VarResetNewLevel(&space);
SpeedControle(&space);
VarPot=54;
ShipPos=56;
space.ScrBackV=(ShipPos/14)+52;
goto Bypass;
RestartLevel:
if (Live>0) {Live--;}else{goto NEWGAME;}
Bypass:
ShipDead=0;
Decompte=0;
Tiny_FlipSpace(0,&space);
_delay_ms(1000);
while(1){
if (MONSTERrest==0) { 
Sound(110,255);_delay_ms(40);Sound(130,255);_delay_ms(40);Sound(100,255);
_delay_ms(40);Sound(1,155);_delay_ms(20);Sound(60,255);Sound(60,255);
if (LEVELS<9) {LEVELS++;}
goto NEWLEVEL;}
if ((((space.MonsterGroupeYpos)+(space.MonsterFloorMax+1))==7)&&(Decompte==0)) {ShipDead=1;}
if (SpeedShootMonster<=((9-LEVELS))) {SpeedShootMonster++;}else{SpeedShootMonster=0;MonsterShootGenerate(&space);}
 space.ScrBackV= (ShipPos/14)+52;
Tiny_FlipSpace(0,&space);
space.oneFrame=!space.oneFrame;
RemoveExplodOnMonsterGrid(&space);
MonsterShootupdate(&space);
UFOUpdate(&space);
if (((space.MonsterGroupeXpos>=26)&&(space.MonsterGroupeXpos<=28))&&(space.MonsterGroupeYpos==2)&&(space.DecalageY8==4)) {space.UFOxPos=127;}
if (VarPot>(ShipPos+2)) {ShipPos=ShipPos+((VarPot-ShipPos)/3);}
if (VarPot<(ShipPos-2)) {ShipPos=ShipPos-((ShipPos-VarPot)/3);}
if (ShipDead!=1) {
if (space.frame<space.frameMax) {space.frame++;}else{GRIDMonsterFloorY(&space);space.anim=!space.anim;if (space.anim==0){SnD(space.UFOxPos,200);}else{SnD(space.UFOxPos,100);}MonsterRefreshMove(&space);space.frame=0;}

//VarPot=map(analogRead(A3),0,1023,0,114);
if ((arduboy.pressed(LEFT_BUTTON))) {if (VarPot>5) {VarPot=VarPot-6;}}
if ((arduboy.pressed(RIGHT_BUTTON))) {if (VarPot<108) {VarPot=VarPot+6;}}

if ((BUTTON_DOWN)&&(MyShootReady==SHOOTS)) {Sound(200,4);MyShootReady=0;space.MyShootBall=6;space.MyShootBallxpos=ShipPos+6;}
}else{
Sound(80,1);Sound(100,1); 
Decompte++;
if (Decompte>=30) {_delay_ms(600);if (((space.MonsterGroupeYpos)+(space.MonsterFloorMax+1))==7) {goto NEWGAME;}else{goto RestartLevel;}}}
if (space.MyShootBall==-1) {if (MyShootReady<SHOOTS) {MyShootReady++;}
}
delay(26);
}}
////////////////////////////////// main end /////////////////////////////////

void SnD(int8_t Sp_,uint8_t SN){
if (Sp_!=-120) {Sound(220,8);Sound(200,4);}else{Sound(SN,1);}
}

void SpeedControle(SPACE *space){
uint8_t xx=00,yy=0;
MONSTERrest=0;
for (yy=0;yy<4;yy++){
for (xx=0;xx<6;xx++){ 
if ((space->MonsterGrid[yy][xx]!=-1)&&((space->MonsterGrid[yy][xx]<=5)) ){MONSTERrest++;}
}}space->frameMax=(MONSTERrest/8 );}

/*Thanks to Sven Bruns for informing me of an error in this function!*/
void GRIDMonsterFloorY(SPACE *space){
uint8_t y,x; 
space->MonsterFloorMax=3;
for (y=0;y<4;y++){
for (x=0;x<6;x++){
if (space->MonsterGrid[3-y][x]!=-1) {goto FIN;}}
space->MonsterFloorMax=space->MonsterFloorMax-1;
}FIN:;}

uint8_t LivePrint(uint8_t x,uint8_t y){
#define XLIVEWIDE ((5*Live)-1)
if (((0>=(x-XLIVEWIDE)))&&(y==7)) {return pgm_read_byte(&LIVESpace[(x)]);}
return 0x00;}

void Tiny_FlipSpace(uint8_t render0_picture1,SPACE *space){
uint8_t y,x; 
uint8_t MYSHIELD=0x00;
for (y = 0; y < 8; y++)
{

for (x = 0; x < 128; x++)
{
if (render0_picture1==0) {
if (ShieldRemoved==0) {MYSHIELD=MyShield(x,y,space);}else{MYSHIELD=0x00;}
arduboy.SPItransfer(background(x,y,space)|LivePrint(x,y)|Vesso(x,y,space)|UFOWrite(x,y,space)|Monster(x,y,space)|MyShoot(x,y,space)|MonsterShoot(x,y,space)|MYSHIELD);
}else{arduboy.SPItransfer(pgm_read_byte(&intro[x+(y*128)]));}
}
if (render0_picture1==0) {
if (ShieldRemoved==0) {ShieldDestroy(0,space->MyShootBallxpos,space->MyShootBall,space);}

}}
if (render0_picture1==0) {
if ((space->MonsterGroupeYpos<(2+(4-(space->MonsterFloorMax+1))))/*&&(LEVELS<=MAXLEVELSHIELDED)*/) {}else{
if (ShieldRemoved!=1) {
   space->Shield[0]=0x00;
   space->Shield[1]=0x00;
   space->Shield[2]=0x00;
   space->Shield[3]=0x00;
   space->Shield[4]=0x00;
   space->Shield[5]=0x00;
  ShieldRemoved=1;}}}}

uint8_t UFOWrite(uint8_t x,uint8_t y,SPACE *space){
if ((space->UFOxPos!=-120)&&(y==0)&&((space->UFOxPos<=(x))&&(space->UFOxPos>=(x-14)))) {return pgm_read_byte(&Monsters[(x-space->UFOxPos)+(6*14)+(space->oneFrame*14)]); }
return 0x00;
}

void UFOUpdate(SPACE *space){
if (space->UFOxPos!=-120) {space->UFOxPos=space->UFOxPos-2;SnD(space->UFOxPos,0);if (space->UFOxPos<=-20) {space->UFOxPos=-120;} }
}

void ShipDestroyByMonster(SPACE *space){ 
if ((space->MonsterShoot[1][0]>=14)&&(space->MonsterShoot[1][0]<=15)&&(space->MonsterShoot[0][0]>=ShipPos)&&(space->MonsterShoot[0][0]<=ShipPos+14)) {
ShipDead=1;
}}

void MonsterShootupdate(SPACE *space){
if (space->MonsterShoot[1][0]!=16) {
ShipDestroyByMonster(space);
if (ShieldDestroy(1,space->MonsterShoot[0][0],space->MonsterShoot[1][0]/2,space)) {
space->MonsterShoot[1][0]=16;
}else{
space->MonsterShoot[1][0]=space->MonsterShoot[1][0]+1;
}}}

void MonsterShootGenerate(SPACE *space){ 
uint8_t a=random()%3; 
uint8_t b=random()%6; 
if (b>=5) {b=5;}
if (space->MonsterShoot[1][0]==16) {  
if (space->MonsterGrid[a][b]!=-1) {
space->MonsterShoot[0][0]=(space->MonsterGroupeXpos+7)+(b*14);
space->MonsterShoot[1][0]=(((space->MonsterGroupeYpos)+a)*2)+1;
}  
}
}

uint8_t MonsterShoot(uint8_t x,uint8_t y,SPACE *space){
if ((((space->MonsterShoot[1][0])/2)==y)&&(space->MonsterShoot[0][0]==x) ) {if (((space->MonsterShoot[1][0 ])%2)==0) {return 0b00001111;}else{return 0b11110000;}}
return 0x00;
}

uint8_t ShieldDestroy(uint8_t Origine,uint8_t VarX,uint8_t VarY,SPACE *space){
#define OFFSETXSHIELD -1
if (VarY==6) {
if (((VarX>=(20+OFFSETXSHIELD))&&(VarX<=(27+OFFSETXSHIELD)))) {
if ((BOOLREAD(0,(VarX-(20+OFFSETXSHIELD)),space))) {ShieldDestroyWrite(0,(VarX-(20+OFFSETXSHIELD)),space,Origine);return 1;}
}
if (((VarX>=(28+OFFSETXSHIELD))&&(VarX<=(35+OFFSETXSHIELD)))) {
if ((BOOLREAD(1,(VarX-(28+OFFSETXSHIELD)),space))) {ShieldDestroyWrite(1,(VarX-(28+OFFSETXSHIELD)),space,Origine);return 1;}
}
if (((VarX>=(55+OFFSETXSHIELD))&&(VarX<=(62+OFFSETXSHIELD)))) {
if ((BOOLREAD(2,(VarX-(55+OFFSETXSHIELD)),space))) {ShieldDestroyWrite(2,(VarX-(55+OFFSETXSHIELD)),space,Origine);return 1;}
}
if (((VarX>=(63+OFFSETXSHIELD))&&(VarX<=(70+OFFSETXSHIELD)))) {
if ((BOOLREAD(3,(VarX-(63+OFFSETXSHIELD)),space))) {ShieldDestroyWrite(3,(VarX-(63+OFFSETXSHIELD)),space,Origine);return 1;}
}
if (((VarX>=(90+OFFSETXSHIELD))&&(VarX<=(97+OFFSETXSHIELD)))) {
if ((BOOLREAD(4,(VarX-(90+OFFSETXSHIELD)),space))) {ShieldDestroyWrite(4,(VarX-(90+OFFSETXSHIELD)),space,Origine);return 1;}
}
if (((VarX>=(98+OFFSETXSHIELD))&&(VarX<=(105+OFFSETXSHIELD)))) {
if ((BOOLREAD(5,(VarX-(98+OFFSETXSHIELD)),space))) {ShieldDestroyWrite(5,(VarX-(98+OFFSETXSHIELD)),space,Origine);return 1;}
}
}
return 0;
}

  
void ShieldDestroyWrite(uint8_t BOOLWRITE,uint8_t line,SPACE *space,uint8_t Origine){
switch (line){
  case 0:space->Shield[BOOLWRITE]=space->Shield[BOOLWRITE]-128;if (Origine==0) {space->MyShootBall=-1;}break;
  case 1:space->Shield[BOOLWRITE]=space->Shield[BOOLWRITE]-64;if (Origine==0) {space->MyShootBall=-1;}break;
  case 2:space->Shield[BOOLWRITE]=space->Shield[BOOLWRITE]-32;if (Origine==0) {space->MyShootBall=-1;}break;
  case 3:space->Shield[BOOLWRITE]=space->Shield[BOOLWRITE]-16;if (Origine==0) {space->MyShootBall=-1;}break;
  case 4:space->Shield[BOOLWRITE]=space->Shield[BOOLWRITE]-8;if (Origine==0) {space->MyShootBall=-1;}break;
  case 5:space->Shield[BOOLWRITE]=space->Shield[BOOLWRITE]-4;if (Origine==0) {space->MyShootBall=-1;}break;
  case 6:space->Shield[BOOLWRITE]=space->Shield[BOOLWRITE]-2;if (Origine==0) {space->MyShootBall=-1;}break;
  case 7:space->Shield[BOOLWRITE]=space->Shield[BOOLWRITE]-1;if (Origine==0) {space->MyShootBall=-1;}break;
default:break;
}}


uint8_t MyShield(uint8_t x,uint8_t y,SPACE *space){
#define OFFSETXSHIELD -1
if (((x>=(20+OFFSETXSHIELD))&&(x<=(27+OFFSETXSHIELD)))&&(y==6)) {
if ((BOOLREAD(0,(x-(20+OFFSETXSHIELD)),space))) {return ShieldBlitz(0,(x-(20+OFFSETXSHIELD)));}else{return 0x00;}
}
if (((x>=(28+OFFSETXSHIELD))&&(x<=(35+OFFSETXSHIELD)))&&(y==6)) {
if ((BOOLREAD(1,(x-(28+OFFSETXSHIELD)),space))) {return ShieldBlitz(1,(x-(28+OFFSETXSHIELD)));}else{return 0x00;}
}
if (((x>=(55+OFFSETXSHIELD))&&(x<=(62+OFFSETXSHIELD)))&&(y==6)) {
if ((BOOLREAD(2,(x-(55+OFFSETXSHIELD)),space))) {return ShieldBlitz(0,(x-(55+OFFSETXSHIELD)));}else{return 0x00;}
}
if (((x>=(63+OFFSETXSHIELD))&&(x<=(70+OFFSETXSHIELD)))&&(y==6)) {
if ((BOOLREAD(3,(x-(63+OFFSETXSHIELD)),space))) {return ShieldBlitz(1,(x-(63+OFFSETXSHIELD)));}else{return 0x00;}
}
if (((x>=(90+OFFSETXSHIELD))&&(x<=(97+OFFSETXSHIELD)))&&(y==6)) {
if ((BOOLREAD(4,(x-(90+OFFSETXSHIELD)),space))) {return ShieldBlitz(0,(x-(90+OFFSETXSHIELD)));}else{return 0x00;}
}
if (((x>=(98+OFFSETXSHIELD))&&(x<=(105+OFFSETXSHIELD)))&&(y==6)) {
if ((BOOLREAD(5,(x-(98+OFFSETXSHIELD)),space))) {return ShieldBlitz(1,(x-(98+OFFSETXSHIELD)));}else{return 0x00;}
}
return 0x00;
}

uint8_t ShieldBlitz(uint8_t Part,uint8_t LineSH ){
uint8_t Var0=0;
switch (LineSH){
  case 0:if (Part==0) {Var0=0b11110000;}else{Var0=0b00001111;}break;
  case 1:if (Part==0) {Var0=0b11111100;}else{Var0=0b00001111;}break;
  case 2:
  case 3:
  case 4:
  case 5:Var0=0b00001111;break;
  case 6:if (Part==1) {Var0=0b11111100;}else{Var0=0b00001111;}break;
  case 7:if (Part==1) {Var0=0b11110000;}else{Var0=0b00001111;}break;
  default:Var0=0b00000000;break;
}
return Var0;
}

uint8_t BOOLREAD(uint8_t SHnum,uint8_t LineSH,SPACE *space ){
uint8_t Var0=0;
switch (LineSH){
  case 0:Var0=0b10000000;break;
  case 1:Var0=0b01000000;break;
  case 2:Var0=0b00100000;break;
  case 3:Var0=0b00010000;break;
  case 4:Var0=0b00001000;break;
  case 5:Var0=0b00000100;break;
  case 6:Var0=0b00000010;break;
  case 7:Var0=0b00000001;break;
  default:Var0=0b00000000;break;
}
if ((space->Shield[SHnum]&Var0)!=0) {return 1;}else{return 0;}
}

void RemoveExplodOnMonsterGrid(SPACE *space){
uint8_t x=0,y=0;
for (y=0;y<=3;y++){ //était a 5
for (x=0;x<=5;x++){
if (space->MonsterGrid[y][x]>=11) {space->MonsterGrid[y][x]=-1;} 
if (space->MonsterGrid[y][x]>=8) {space->MonsterGrid[y][x]=space->MonsterGrid[y][x]+1;}
}}}
  
uint8_t background(uint8_t x,uint8_t y,SPACE *space){
uint8_t scr=(space->ScrBackV+x);
if ((scr)>127) {scr=(space->ScrBackV+x)-128;}
return 0xff-pgm_read_byte(&backSpace[((y)*128)+((scr))]);
}
 
uint8_t Vesso(uint8_t x,uint8_t y,SPACE *space){
if (((x-ShipPos)>=0)&&((x-ShipPos)<13)&&(y==7)) {
if (ShipDead==0) {return pgm_read_byte(&vesso[(x-ShipPos)]);}else{return pgm_read_byte(&vesso[(x-ShipPos)+(12*space->oneFrame)]);}
}
return 0;}

void UFO_Attack_Check(uint8_t x,SPACE *space){
if (space->MyShootBall==0) {
if ((space->MyShootBallxpos>=space->UFOxPos)&&(space->MyShootBallxpos<=(space->UFOxPos+14))) {
for (x=1;x<100;x++){
Sound(x,1);
}
if (Live<3) Live++;
space->UFOxPos=-120;}
}
}

 uint8_t MyShoot(uint8_t x,uint8_t y,SPACE *space){
if ((space->MyShootBallxpos==x)&&(y==((space->MyShootBall)))) {
if (space->MyShootBall>-1) {space->MyShootBallFrame=!space->MyShootBallFrame;}else{return 0x00;}
if (space->MyShootBallFrame==1) {space->MyShootBall--;} 
Monster_Attack_Check(space);
UFO_Attack_Check(x,space);
return pgm_read_byte(&SHOOT[(space->MyShootBallFrame)]);
}
return 0x00;
}

void Monster_Attack_Check(SPACE *space){
int8_t Varx=0,Vary=0;
#define Xmouin (space->MonsterGroupeXpos) 
#define Ymouin ((space->MonsterGroupeYpos)*8)//-space->DecalageY8
#define XPlus (Xmouin+84)
#define YPlus (Ymouin+(4*8))
#define MYSHOOTX (space->MyShootBallxpos)
#define MYSHOOTY ((space->MyShootBall*8)+(((space->MyShootBallFrame)+1)*4))
if ((MYSHOOTX>=Xmouin)&&(MYSHOOTX<=XPlus)&&(MYSHOOTY>=(Ymouin))&&(MYSHOOTY<=YPlus)){
//enter in the monster zone
Vary= (round((MYSHOOTY-Ymouin)/8));
Varx= (round((MYSHOOTX-Xmouin)/14));
if (Varx<0) {Varx=0;}
if (Vary<0) {Vary=0;}
if (Varx>5) {goto End;}
if (Vary>3) {goto End;}
if ((space->MonsterGrid[Vary][Varx]>-1) && (space->MonsterGrid[Vary][Varx]<6)) {
Sound(50,10);
space->MonsterGrid[Vary][Varx]=8;
space->MyShootBall=-1;
SpeedControle(space);
}
//fin monster zone
}
End:;
}

int8_t OuDansLaGrilleMonster(uint8_t x,uint8_t y,SPACE *space){
if (x<space->MonsterGroupeXpos) {return -1;}
if (y<space->MonsterGroupeYpos) {return -1;}
space->PositionDansGrilleMonsterX=(x-space->MonsterGroupeXpos)/14; 
space->PositionDansGrilleMonsterY=(y-space->MonsterGroupeYpos);
if ((space->PositionDansGrilleMonsterX)>5)  {return -1;}
if ((space->PositionDansGrilleMonsterY)>4)  {return -1;}
return 0;
}

uint8_t SplitSpriteDecalageY(uint8_t Input,uint8_t UPorDOWN,SPACE *space){
if (UPorDOWN) {
return Input<<space->DecalageY8;
}else{
return Input>>(8-space->DecalageY8); 
}}

uint8_t Murge_Split_UP_DOWN(uint8_t x,SPACE *space){
int8_t SpriteType=-1;
int8_t ANIMs=-1;
uint8_t Murge1=0;
uint8_t Murge2=0;
if (space->DecalageY8==0) {
SpriteType=space->MonsterGrid[space->PositionDansGrilleMonsterY][space->PositionDansGrilleMonsterX];
if (SpriteType<8) {ANIMs=(space->anim*14);}else{ANIMs=0;}
if (SpriteType==-1) {return 0x00;}
return pgm_read_byte(&Monsters[(WriteMonster14(x-space->MonsterGroupeXpos)+SpriteType*14)+ANIMs]);
}else{ //debut
if (space->PositionDansGrilleMonsterY==0) {
SpriteType=space->MonsterGrid[space->PositionDansGrilleMonsterY][space->PositionDansGrilleMonsterX];
if (SpriteType<8) {ANIMs=(space->anim*14);}else{ANIMs=0;}
if (SpriteType!=-1) { Murge2=SplitSpriteDecalageY(pgm_read_byte(&Monsters[(WriteMonster14(x-space->MonsterGroupeXpos)+SpriteType*14)+ANIMs]),1,space);}else{
Murge2=0x00;
}
return Murge2;    
}else{
SpriteType=space->MonsterGrid[space->PositionDansGrilleMonsterY-1][space->PositionDansGrilleMonsterX];
if (SpriteType<8) {ANIMs=(space->anim*14);}else{ANIMs=0;}
if (SpriteType!=-1) {Murge1=SplitSpriteDecalageY(pgm_read_byte(&Monsters[(WriteMonster14(x-space->MonsterGroupeXpos)+SpriteType*14)+ANIMs]),0,space);
}else{Murge1=0x00;}
SpriteType=space->MonsterGrid[space->PositionDansGrilleMonsterY][space->PositionDansGrilleMonsterX];
if (SpriteType<8) {ANIMs=(space->anim*14);}else{ANIMs=0;}
if (SpriteType!=-1) { Murge2=SplitSpriteDecalageY(pgm_read_byte(&Monsters[(WriteMonster14(x-space->MonsterGroupeXpos)+SpriteType*14)+ANIMs]),1,space);
}else{Murge2=0x00;}
return Murge1|Murge2;    
}  
} //fin
}

uint8_t WriteMonster14(uint8_t x){
while(1){
if ((x-14)>=0) {x=x-14;}else{break;}
}return x;}

uint8_t Monster(uint8_t x,uint8_t y,SPACE *space){

if (OuDansLaGrilleMonster(x,y,space)!=-1) {
}else{return 0x00;} //quiter la fonction si pas dans la grille
return  Murge_Split_UP_DOWN(x,space);
return 0x00;
}//end Monster();

uint8_t MonsterRefreshMove(SPACE *space){
if (space->Direction==1) {
if ((space->MonsterGroupeXpos<space->MonsterOffsetDroite)) {space->MonsterGroupeXpos=space->MonsterGroupeXpos+2;return 0;}else{
if (space->DecalageY8<7) {space->DecalageY8=space->DecalageY8+4;if (space->DecalageY8>7) {space->DecalageY8=7;} }else{space->MonsterGroupeYpos++;space->DecalageY8=0;}
space->Direction=0;return 0;
}}else{
if ((space->MonsterGroupeXpos>space->MonsterOffsetGauche)) {space->MonsterGroupeXpos=space->MonsterGroupeXpos-2;return 0;}else{
if (space->DecalageY8<7) {space->DecalageY8=space->DecalageY8+4;if (space->DecalageY8>7) {space->DecalageY8=7;} }else{space->MonsterGroupeYpos++;space->DecalageY8=0;}
space->Direction=1;return 0;
}}}


void VarResetNewLevel(SPACE *space){
//space->ScrBackV=0;
ShieldRemoved=0;
SpeedShootMonster=0;
MONSTERrest=24;
LoadMonstersLevels(LEVELS,space);
space->Shield[0]=255;  
space->Shield[1]=255;  
space->Shield[2]=255;  
space->Shield[3]=255;  
space->Shield[4]=255;  
space->Shield[5]=255;  
space->MonsterShoot[0][0]=16;
space->MonsterShoot[1][0]=16;
space->UFOxPos=-120;

space->MyShootBall=-1;
space->MyShootBallxpos=0;
space->MyShootBallFrame=0;
space->anim=0;
 space->frame=0;
space->PositionDansGrilleMonsterX=0;
space->PositionDansGrilleMonsterY=0;
space->MonsterFloorMax=3;
space->MonsterOffsetGauche=0;
space->MonsterOffsetDroite=44;
space->MonsterGroupeXpos=20;
if (LEVELS>3) {space->MonsterGroupeYpos=1;}else{space->MonsterGroupeYpos=0;}
space->DecalageY8=0;
space->frameMax=8;
space->Direction=1; //1 right 0 gauche  
space->oneFrame=0;}

//debut pinball

void looppinball() {
NEWGAME:
totalpush=0;
totalBall=5;
delay(500);
Tiny_Flip2(1,0);
for (uint8_t t=255;t>0;t--){
Sound(t,2);}
for (uint8_t t=0;t<255;t++){
Sound(t,2);}
delay(1000);
while(1){
random(255);
if ((arduboy.pressed(DOWN_BUTTON))||(arduboy.pressed(UP_BUTTON))||(arduboy.pressed(LEFT_BUTTON))||(arduboy.pressed(RIGHT_BUTTON))||(arduboy.pressed(B_BUTTON))||(arduboy.pressed(A_BUTTON))) {

SSD1306_ssd1306_draw_bmp(27+32, 3, 37+32, 5, &READY_IMG[0]);

FIRSTTIME=1;
delay(1000);
break;}}
start:
if (totalBall!=0) {totalBall--;}else{
delay(300);
SSD1306_ssd1306_draw_bmp(22+32, 2, 41+32, 6, GameOver);

 for(uint8_t t=0;t<5;t++){
  Sound (100,100);
  Sound (1,100);}
delay(1000);goto NEWGAME;}

#define XSTART 46.0
#define YSTART 30.0;
#define XSTARTSPEED 0
#define YSTARTSPEED 0
BALL ball;
ball.resetBall=0;
ball.x=XSTART;
ball.y=YSTART;
ball.Speedx=XSTARTSPEED ;
ball.Speedy=YSTARTSPEED;
ball.SIMx=XSTART;
ball.SIMy=YSTART;
ball.SIMSpeedx=XSTARTSPEED ;
ball.SIMSpeedy=YSTARTSPEED ;

while(1){
BallupDate(&ball);
Tiny_Flip2(0,&ball);
delay(3);
if (SCANLINE<1) {SCANLINE++;}else{SCANLINE=0;}
if (ball.x<.5) {falseBall();FIRSTTIME=1;goto start;}
if ((ball.y>=29)&&(ball.x>=18)) {if (arduboy.pressed(DOWN_BUTTON)) {if (SpringBar>54) {SpringBar=SpringBar-2;}}else{if ((SpringBar>SPRINGLONG-10)&&(SpringBar<SPRINGLONG)) {SpringBar=SPRINGLONG;} if (SpringBar<SPRINGLONG) {SpringBar=SpringBar+8;}}}else{SpringBar=SPRINGLONG;}
if (arduboy.pressed(UP_BUTTON))  {if (trigFlipG<3) {trigFlipG++;}}else{if (trigFlipG>0) {trigFlipG--;}}
if (arduboy.pressed(DOWN_BUTTON))  {if (trigFlipD<3) {trigFlipD++;}}else{if (trigFlipD>0) {trigFlipD--;}}
}}
//<<<<<<<<<<<<<<<<<<<MAIN FIN>>>>>>>>>>>>>>>

void falseBall(void){
uint8_t t;
for (t=50;t>0;t--){
Sound(t,6);}}


void BallupDate(BALL *B){
SimulateMove(B);
if (ColisionCheck(B->SIMx,B->SIMy,B)) {
CheckColisionType(B);
if ((B->y<7)||(B->y>24)||(B->x<31)||(B->x>48)){
WriteMove(B);
if (B->y<29) {Sound(1,6);}
}else{
WriteMoveBounce(B);
bouncePush=256;
if (totalpush<7) {totalpush++;}else{
 if (totalBall<4) {totalBall++;}
 totalpush=0;
for(uint8_t ttt=60;ttt<240;ttt=ttt+20){Sound(ttt,6);}
}}
if ((B->SIMSpeedy>=0.15)) {B->SIMSpeedy=(B->SIMSpeedy-.1);} 
if ((B->SIMSpeedy<=-0.15)) {B->SIMSpeedy=(B->SIMSpeedy+.1);}
}else{WriteMove(B);}
if ((B->y>29)&&(B->x>22)) {B->y=30;}
if ((round(B->x)>=49)&&(round(B->y)==30)) {
B->x=50; 
B->y=29; 
B->Speedx=0.8-(random(10.0)/100.0);  
B->Speedy=-1.0+(random(30.0)/100.0);  
}}

uint8_t SelectByte(uint8_t ByteSelect,uint8_t FFx0){
uint8_t ByteRecup=0;
switch (ByteSelect){
  case(0):ByteRecup=0b10000000;break;
  case(1):ByteRecup=0b01000000;break;
  case(2):ByteRecup=0b00100000;break;
  case(3):ByteRecup=0b00010000;break;
  case(4):ByteRecup=0b00001000;break;
  case(5):ByteRecup=0b00000100;break;
  case(6):ByteRecup=0b00000010;break;
  case(7):ByteRecup=0b00000001;break;
  default:ByteRecup=0;break;
}
if ((ByteRecup&FFx0)!=0b00000000) {return 1;}else{return 0;}
}

uint8_t ScanMem(uint8_t xxx,uint8_t yyy){
 return pgm_read_byte(&startpinball[(uint8_t)(xxx)+(yyy)]);
}

void TrimXY(BALL *B){
if (B->Speedx>1) B->Speedx=1;
if (B->Speedx<-1) B->Speedx=-1;
if (B->Speedy>1) B->Speedy=1;
if (B->Speedy<-1) B->Speedy=-1;}

void SimulateRebounce(uint8_t Sim,BALL *B){
B->SIMx=B->x;
B->SIMy=B->y;
switch(Sim){
  case (0):B->SIMSpeedx=B->Speedx;B->SIMSpeedy=B->Speedy;break;
  case (1):B->SIMSpeedx=-B->Speedx;B->SIMSpeedy=B->Speedy;break;
  case (2):B->SIMSpeedx=B->Speedx;B->SIMSpeedy=-B->Speedy;break;
  case (3):B->SIMSpeedx=-B->Speedx;B->SIMSpeedy=-B->Speedy;break;
  case (4):B->SIMSpeedx=-B->Speedy;B->SIMSpeedy=-B->Speedx;break;
  case (5):B->SIMx=B->x+1;B->SIMy=B->y;B->SIMSpeedx=-0.2;B->SIMSpeedy=0.2;break;
  case (6):B->SIMx=B->x+1;B->SIMy=B->y;B->SIMSpeedx=-0.2;B->SIMSpeedy=-0.2;break;
  default:break;}}

uint8_t CheckColisionType(BALL *B){
TrimXY(B);
B->SIMx=B->x;
B->SIMy=B->y;
B->SIMSpeedx=B->Speedx;
B->SIMSpeedy=B->Speedy;
if ((B->SIMy==30)&&(B->SIMx<=(79-32))){ 
if (!ColisionCheck(B->SIMx,B->SIMy,B)){
SimulateMove(B);
return 0;}else{
TrimBallOnSpring(B);
if (B->SIMSpeedx<=0) {B->SIMSpeedx=-B->SIMSpeedx;}
SimulateMove(B);
return 0;}}
uint8_t sim=0;
while(1){                       
SimulateRebounce(sim,B);          
SimulateMove(B);
if (!ColisionCheck(B->SIMx,B->SIMy,B)) {return 0;}
sim++;if (sim==7) {sim=0;}
}}

void TrimBallOnSpring(BALL *B){
uint8_t counter=0;
if ((B->SIMx+32)<(SpringBar-1)) {
DAS:
if ((B->SIMx+32)<(SpringBar-1)) { B->SIMx++;counter++;goto DAS;}
if (counter>4) {
B->SIMSpeedx=2;
}}}

void WriteMove(BALL *B){ 
B->x=B->SIMx;
B->y=B->SIMy;
B->Speedx=B->SIMSpeedx;
B->Speedy=B->SIMSpeedy;}

void WriteMoveBounce(BALL *B){ 
B->x=B->SIMx;
B->y=B->SIMy;
B->SIMSpeedx=B->SIMSpeedx*8;
B->SIMSpeedy=B->SIMSpeedy*8;
if (B->SIMSpeedx>1.4) B->SIMSpeedx=1.4;
if (B->SIMSpeedx<-1.4) B->SIMSpeedx=-1.4;
if (B->SIMSpeedy>1.4) B->SIMSpeedy=1.4;
if (B->SIMSpeedy<-1.4) B->SIMSpeedy=-1.4;
B->Speedx=B->SIMSpeedx;
B->Speedy=B->SIMSpeedy;}

void SimulateMove(BALL *B){
TrimXY(B);
if (B->SIMx>=0) {
if ((B->SIMSpeedx>-1)) {if (B->SIMSpeedx-0.05>-1) {B->SIMSpeedx=B->SIMSpeedx-0.05;}}
B->SIMx=B->SIMx+B->SIMSpeedx;}else{B->resetBall=1;}
B->SIMy=B->SIMy+B->SIMSpeedy;}

uint8_t ColisionCheck(float x,float y,BALL *B){
TrimXY(B);
uint16_t serialcount=0;
uint8_t X=round(x);
uint8_t Y=round(y);
uint8_t verticalStrip=((Y)/8); //0 to 3
serialcount=((verticalStrip*64)+X);
#define BACK (pgm_read_byte(&startpinball[serialcount])&PixelAsign(Y))!=0b00000000)
if ((X>=4)&&(X<=9)&&(Y>=11)&&(Y<=14)){
if (trigFlipG==0) {
if ((((pgm_read_byte(&FLIPGAUCHE[(X-4)+(trigFlipG*6)]))&(PixelAsign(Y)))!=0b00000000)||(BACK) {return 1;}else{return 0;} 
}else if ((((pgm_read_byte(&FLIPDETGAUCHE[(X-4)]))&(PixelAsign(Y)))!=0b00000000)||(BACK){if (trigFlipG!=3) {B->SIMSpeedx=2;
Sound(20,12);
}else{B->SIMSpeedx=1;Sound(1,6);}return 0;}
} else if ((X>=4)&&(X<=9)&&(Y>=17)&&(Y<=20)){
if (trigFlipD==0) {
if ((((pgm_read_byte(&FLIPDROITE[(X-4)+(trigFlipD*6)]))&(PixelAsign(Y)))!=0b00000000)||(BACK) {return 1;}else{return 0;} 
}else if ((((pgm_read_byte(&FLIPDETDROIT[(X-4)]))&(PixelAsign(Y)))!=0b00000000)||(BACK){if (trigFlipD!=3) {B->SIMSpeedx=2;
Sound(20,12);
}else{B->SIMSpeedx=1;Sound(1,6);}return 0;}
}else{
if (((X+32)<(SpringBar))&&(B->y==30)) {return 1;}
if ((pgm_read_byte(&startpinball[serialcount])&PixelAsign(Y))!=0b00000000) {return 1;}
return 0;}
return 0;}

uint8_t RecupeScreen(uint8_t nn,uint8_t mm){
uint8_t ScreenCount=nn-90;  
switch(mm){
case(4): return pgm_read_byte(&ScreenBallA[ScreenCount+(totalBall*6)]);break;
case(5): return pgm_read_byte(&ScreenBallB[ScreenCount+(totalBall*6)]);break;
case(6): return pgm_read_byte(&PusherA[ScreenCount+(totalpush*6)]);break;
case(7): return pgm_read_byte(&PusherB[ScreenCount+(totalpush*6)]);break;
default:return 0;break;
}
return 0;
}


uint8_t addBin(uint8_t a,uint8_t b,uint8_t c,uint8_t d){
return a|b|c|d;}

uint8_t RecupeByte(uint8_t x,uint8_t y){
return pgm_read_byte(&startpinball [((x-32)+(((y)-4)*64))+bouncePush]);}

uint8_t RecupeFlip(uint8_t x,uint8_t y){
uint8_t TRIGG=0;
uint8_t TRIDD=0;
if (trigFlipG>2) {TRIGG=2;}else{TRIGG=trigFlipG;}
if (trigFlipD>2) {TRIDD=2;}else{TRIDD=trigFlipD;}
if ((y==5)&&(x>35&&x<42)){
return pgm_read_byte(&FLIPGAUCHE[(x-36)+(TRIGG*6)]);
}else if ((y==6)&&(x>35&&x<42)) {
return pgm_read_byte(&FLIPDROITE[(x-36)+(TRIDD*6)]);}else{
return 0x00;}}

uint8_t RecupeSpring(uint8_t x,uint8_t y){
if (((y==7)&&(x>53&&x<SpringBar))){
return 0b01000000;}else{
return 0x00;}
}

uint8_t PixelAsign(uint8_t Value){
while(Value>=8){
Value=Value-8;}
switch(Value){
case  (0):return 0b00000001;break;
case  (1):return 0b00000010;break;
case  (2):return 0b00000100;break;
case  (3):return 0b00001000;break;
case  (4):return 0b00010000;break;
case  (5):return 0b00100000;break;
case  (6):return 0b01000000;break;
case  (7):return 0b10000000;break;
default:return 0x00;break;
}}

uint8_t PixelConvert(uint8_t horiz,uint8_t verti,BALL *B){
uint8_t ballx (round(B->x)+32);
uint8_t bally (round(B->y)+32);
if ((horiz>31)&&(horiz<97)&&(verti>3)) {
if ((trunc((bally)/8)==verti)&&(ballx==horiz)) {return PixelAsign(bally);}else{return 0x00;}
}else{return 0x00;}}

uint8_t SliceByte(uint8_t Vertical,uint8_t Byte){
uint8_t toto=0;
 if ((Vertical%2)!=0)  {
 if ((Byte&0b10000000)!=0) {toto=toto|0b11000000;}
 if ((Byte&0b01000000)!=0) {toto=toto|0b00110000;}
 if ((Byte&0b00100000)!=0) {toto=toto|0b00001100;}
 if ((Byte&0b00010000)!=0) {toto=toto|0b00000011;}
 } else {
 if ((Byte&0b00001000)!=0) {toto=toto|0b11000000;}
 if ((Byte&0b00000100)!=0) {toto=toto|0b00110000;}
 if ((Byte&0b00000010)!=0) {toto=toto|0b00001100;}
 if ((Byte&0b00000001)!=0) {toto=toto|0b00000011;}
  }return toto;}

void Tiny_Flip2(uint8_t select,BALL *B){
uint8_t m,x,y,n,mem;
for (y = 0; y <= 7; y++){  
m=(trunc(y/2))+4;
for (x = 0; x < 64; x++){
n=(x)+32;
if (select==0) {
if ((n<90)) {
mem=SliceByte(y,addBin(PixelConvert(n,m,B),RecupeByte(n,m),RecupeFlip(n,m),RecupeSpring(n,m)));
arduboy.SPItransfer(mem);
arduboy.SPItransfer(mem);
// SSD1306.ssd1306_send_byte(SliceByte(y,BUFFER128[n-32]));    
}else{    
mem=SliceByte(y,RecupeScreen(n,m));
arduboy.SPItransfer(mem);
arduboy.SPItransfer(mem);
}}else{
 mem=SliceByte(y,Recupeintro(n,m));
arduboy.SPItransfer(mem);
arduboy.SPItransfer(mem); 
  }
}}
if (bouncePush==256) {
frameCount++;
if (frameCount>1) {
Sound(1,20);Sound(20,20);Sound(1,20);
bouncePush=0;frameCount=0;}}
FIRSTTIME=0;}

uint8_t Recupeintro(uint8_t x,uint8_t y){
return pgm_read_byte(&intropinball [((x-32)+(((y)-4)*64))]);}



void SSD1306_ssd1306_fillscreen(uint8_t BYTE){
for(uint16_t t=0;t<1024;t++){
arduboy.SPItransfer(BYTE);
}}

void SSD1306_ssd1306_draw_bmp(uint8_t x,uint8_t y,uint8_t w,uint8_t h,const uint8_t *PIC){
i2c_start();
i2c_send_byte(0x40);
for(uint8_t yy=0;yy<8;yy++){
for(uint8_t xx=0;xx<128;xx++){
if ((xx>=x)&&(xx<w)&&(yy>=y)&&(yy<h)) { i2c_send_byte(pgm_read_byte(&PIC[(xx-x)+(((w-x)*(yy-y)))]));}else{
i2c_send_byte(0x00);
}}}
i2c_stop();
}
