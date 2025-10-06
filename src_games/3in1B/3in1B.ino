// 3in1B (Tiny Bomber + Tiny Arkanoid + Tiny Gilbert) for ARDUBOY  GPL v3
//                    Programmer: Daniel C 2018-2021
//              Contact EMAIL: electro_l.i.b@tinyjoypad.com
//                     https://www.tinyjoypad.com
//          https://sites.google.com/view/arduino-collection

//   3in1B (Tiny Bomber + Tiny Arkanoid + Tiny Gilbert)
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
// -__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-__-


// The 3in1B source code include commands referencing to librairy 
// Arduboy2 who is not include in the source code.

// Reference in file "Arduboy2_library_LICENSE.txt".
// https://github.com/MLXXXp/Arduboy2

// A HEX file "3in1B.hex" is provided with the source code which includes
// compiled code from the Arduboy2 library.
// Reference in the file "Arduboy2_library_LICENSE.txt".

#include <Arduboy2.h> //https://github.com/MLXXXp/Arduboy2
Arduboy2 arduboy;
#include "spritebank.h"

// var public
uint8_t SPEEDBOMBER=0;
uint8_t RNDcount;
int8_t BOMBXY[4]={-10,-10,0,0};
uint8_t live;
uint8_t INGAMEBomber;
int8_t Level;
uint8_t FrameBomber;

uint8_t BlocBombMem[14];  
// fin var public

uint8_t Map[8][34]={{0}};
uint8_t timer=0;
int8_t scrool=0;
int8_t step4=0;
uint8_t MainAnim=0,LorR=1;
int8_t Jump=0;
int8_t  jumpcancel=0;
const float VSlide[9]={1,2,4,8,16,32,64,128,256};
uint8_t key[20][2]={{0}};
uint8_t keyS=0;
float VSlideOut=0;
uint8_t LevelMult=0;
uint8_t ByteMem=0;
uint8_t visible=1;
uint8_t injur=0;

#define BUTTON_DOWN ((arduboy.pressed(A_BUTTON)==1)||(arduboy.pressed(B_BUTTON)==1))
#define BUTTON_UP ((arduboy.pressed(A_BUTTON)==0) && (arduboy.pressed(B_BUTTON)==0))
void setup() {
 arduboy.begin();
}

void Sound(uint8_t freq,uint8_t dur){
   DanCSound(freq, dur);
}

void sound(uint8_t SND){
if (SND==1) {Sound(210,10);Sound(240,2);Sound(180,5);}
if (SND==2){for (uint8_t x=255;x>2;x--){ Sound(x,1);}
}
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
case 0:SSD1306_ssd1306_draw_bmp(0,0,1,128,7,MENU);break;
case 1:SSD1306_ssd1306_draw_bmp(0,0, 0,128,8,BackBlitzBomber);break;
case 2:SSD1306_ssd1306_draw_bmp(0,0, 0,128,8,MAIN);break;
case 3:SSD1306_ssd1306_draw_bmp(1,32, 4, 96, 8,start);break;
default:break;
}
if (BUTTON_DOWN) {while(1){if (BUTTON_UP) { for(uint8_t r=0;r<60;r++){Sound(2+r,10);Sound(255-r,20);}break;}}
  switch(menuselect){
  case 0:break;
  case 1:looptinybomber();
  case 2:looptinyarkanoid();
  case 3:looptinygilbert();
  default:break;
}}}}
//////////////////fin loop

void LoadLevelBomber(uint8_t LeveL){
for (uint8_t t=0;t<14;t++){
BlocBombMem[t]=pgm_read_byte(&levelBomber[t+(LeveL*14)]);
}
}

void ResetVarBomber(PERSONAGEBomber * Sprite){
live=3;
Level=-1;
INGAMEBomber=0;
SPEEDBOMBER=0;
resetMonster(Sprite);
}

void resetMonster(PERSONAGEBomber *Sprite){
uint8_t t;
for (t=1;t<=4;t++){
Sprite[t].dead=0;

}
}
void resetBomb(void){
BOMBXY[0]=-10; 
BOMBXY[1]=-10; 
BOMBXY[2]=0; 
BOMBXY[3]=0; 
}

void StartGameBomber(void){
if (INGAMEBomber==0) {
 while(1){if (BUTTON_UP) {break;}}
INGAMEBomber=1;}
}

void looptinybomber(void){
PERSONAGEBomber Sprite[5];
NEWGAME:
ResetVarBomber(&Sprite[0]);
while(1){
Tiny_FlipBomber(1,&Sprite[0]);
if (BUTTON_DOWN) {StartGameBomber();break;}
}
NEWLEVEL:
if (Level<2) {
if (Level>-1) {{for(uint8_t t=0;t<=4;t++){Sound(80,100);delay(300);}}}
Level++;
}else{{for(uint8_t t=0;t<=4;t++){Sound(80,100);delay(300);}}SPEEDBOMBER=1;Level=0;}
resetMonster(Sprite);
LoadLevelBomber(Level);
RESTARTLEVEL:
resetBomb();
Sprite[0].type=0;
Sprite[0].x=9;
Sprite[0].y=7;
Sprite[0].Decalagey=0;
Sprite[0].DirectionV=2;
Sprite[0].DirectionH=2;
Sprite[0].DirectionAnim=0;
Sprite[0].dead=0;
Sprite[1].type=1;
Sprite[1].x=56;
Sprite[1].y=1;
Sprite[1].Decalagey=0;
Sprite[2].type=1;
Sprite[2].x=120;
Sprite[2].y=2;
Sprite[2].Decalagey=0;
Sprite[3].type=1;
Sprite[3].x=119;
Sprite[3].y=7;
Sprite[3].Decalagey=0;
Sprite[4].type=1;
Sprite[4].x=24;
Sprite[4].y=5;
Sprite[4].Decalagey=0;

//INGAME:

while(1){
//joystick
//if (arduboy.pressed(B_BUTTON)) {StartGame();}
if (INGAMEBomber) {

if (arduboy.pressed(LEFT_BUTTON)) {Sprite[0].DirectionV=0;}
else if (arduboy.pressed(RIGHT_BUTTON))  {Sprite[0].DirectionV=1;}else{if (((Sprite[0].x)%8)==0) Sprite[0].DirectionV=2;}
if (arduboy.pressed(DOWN_BUTTON))  {Sprite[0].DirectionH=1;}
else if (arduboy.pressed(UP_BUTTON))  {Sprite[0].DirectionH=0;}else{if (Sprite[0].Decalagey==0) Sprite[0].DirectionH=2;}
if ((BUTTON_DOWN)&&(BOMBXY[2]==0)) {
  BOMBXY[0]=(uint8_t((Sprite[0].x+3)/8))*8;
  BOMBXY[1]=(uint8_t(((Sprite[0].y*8)+(Sprite[0].Decalagey)+3)/8));
  BOMBXY[2]=5;
{
  uint8_t t;
 for(t=155;t<255;t++){Sound(2+t,2);Sound(25+t,1);}
}

}

//fin joystick

}
if (FrameBomber<24) {FrameBomber++;}else{FrameBomber=0;}
if (CollisionTiny2Caracter(&Sprite[0])==0) {
RefreshCaracterBomber(&Sprite[0]);
  }
else{
DeadSong();
if (live>0) {live--;goto RESTARTLEVEL;}else{
  delay(200);
 {for(uint8_t t=0;t<5;t++){Sound (100,100);Sound (1,100);}} 
  goto NEWGAME;}
}
if ((FrameBomber==0)||(FrameBomber==8)||(FrameBomber==16)) {
  if (BOMBXY[2]>1) {                               
 if (BOMBXY[3]<2) {BOMBXY[3]++;}else{BOMBXY[3]=0;BOMBXY[2]--;} 
}}

if (FrameBomber%4==0) {
  if (BOMBXY[2]==1) {                             
  if (BOMBXY[3]<2) {
   if (BOMBXY[3]==1) {DestroyEnemy(&Sprite[0]);DestroyBloc();}
    BOMBXY[3]++;
    }else{
resetBomb();
{for(uint8_t t=210;t<255;t++){Sound(2+t,1);Sound(255-t,2);}
}}}}
if (FrameBomber%2==0) {
Tiny_FlipBomber(0,&Sprite[0]);
if (INGAMEBomber==1){INGAMEBomber=2;
{for (uint8_t t=0;t<=41;t=t+2){
  uint8_t r=pgm_read_byte(&MusicBomber[t]);
  if (r==0) {}else{r=r+10;}
  Sound(r,pgm_read_byte(&MusicBomber[t+1])-20);}
delay(500);
}}}
if (Sprite[0].dead==1) {if (live>0) {DeadSong();live--;goto RESTARTLEVEL;}else{DeadSong();
delay(200);
{for(uint8_t t=0;t<5;t++){Sound (100,100);Sound (1,100);}}
goto NEWGAME;}}
if (FrameBomber==12) {
if (CheckLevelEnded(&Sprite[0])==1) goto NEWLEVEL;
}
delay(11);
}  
}

void RsVarNewGame(GROUPE *VAR){
VAR->LEVELSPEED=16;
VAR->LEVEL=1;
VAR->live=3;
VAR->ANIMREFLECT=0;
LoadLevel(0,VAR);
}


void looptinyarkanoid(void){
GROUPE VARIABLE;
NEWGAME:;
Tiny_Flip(1,&VARIABLE);
while(1){
if (BUTTON_DOWN) {break;}  
}
RsVarNewGame(&VARIABLE);
Tiny_Flip(2,&VARIABLE);
PLAYMUSIC(0);
LoadLevel(VARIABLE.LEVEL-1,&VARIABLE);
goto ONE;
NEXTLEVEL:;
Sound(60,100);
Sound(80,100);
Sound(100,100);
Sound(120,100);
Sound(140,100);
if (VARIABLE.LEVELSPEED>8) {VARIABLE.LEVELSPEED=VARIABLE.LEVELSPEED-2;}
Tiny_Flip(2,&VARIABLE);
delay(400);
ResetVar(&VARIABLE);
VARIABLE.LEVEL++;
goto ONE;
RESTARTLEVEL:;
Sound(200,100);
Sound(150,100);
Sound(100,100);
Sound(50,100);

if (VARIABLE.live>0) {VARIABLE.live--;}else{goto NEWGAME;}
ONE:;

ResetBall(&VARIABLE);


while(1){
if (VARIABLE.Frame%8==0) {
if (arduboy.pressed(DOWN_BUTTON)) {if (VARIABLE.TrackBaryDecal<7) {if (VARIABLE.TrackBaryDecal+(VARIABLE.TrackBary*8)<44){ VARIABLE.TrackBaryDecal++;}}else{VARIABLE.TrackBaryDecal=0;VARIABLE.TrackBary++;}}
if (arduboy.pressed(UP_BUTTON)) {if (VARIABLE.TrackBaryDecal>0) {if (VARIABLE.TrackBaryDecal+(VARIABLE.TrackBary*8)>4){ VARIABLE.TrackBaryDecal--;}}else{VARIABLE.TrackBaryDecal=7;VARIABLE.TrackBary--;}}
if ((VARIABLE.launch==0)&&((BUTTON_DOWN))) {VARIABLE.launch=1;}
if (VARIABLE.launch==0) {VARIABLE.Ballypos=((VARIABLE.TrackBary*8)+VARIABLE.TrackBaryDecal)+10;VARIABLE.SIMBallypos=VARIABLE.Ballypos;}
}
if ((VARIABLE.Frame%VARIABLE.LEVELSPEED==0)) {UpdateBall(&VARIABLE);} //1,2,4,8,
if (VARIABLE.Frame%32==0) {Tiny_Flip(0,&VARIABLE);}
if (VARIABLE.Frame==48) {
if (VARIABLE.ANIMREFLECT<3) {VARIABLE.ANIMREFLECT++;}
if (BallMissing(&VARIABLE)) {goto RESTARTLEVEL;}
if (CheckLevelEnded(&VARIABLE)) {goto NEXTLEVEL;}
}
if (VARIABLE.Frame<64) {VARIABLE.Frame++;}else{VARIABLE.Frame=1;}
_delay_us(600);
}  
}

void looptinygilbert(void){
RESTARTGAME:
sound(1);sound(2);sound(2);sound(1);sound(2);sound(2);
delay(200);
ResetVar();
//SSD1306_ssd1306_fillscreen(0xff);

SSD1306_ssd1306_draw_bmp(1,32, 4, 96, 8,start);
delay(400);
 while(1){
if (BUTTON_DOWN)  {sound(1);break;}}
RESTARTLEVEL:
ResetVarNextLevel();
NEXTLEVEL:
if (Level>9) {goto RESTARTGAME;}
delay(400);
DriftSprite MainSprite;
SpriteShiftInitialise(&MainSprite);
SSD1306_ssd1306_fillscreen(0x00);
#define RBACKUP  if  ((CollisionCheck(&MainSprite)==1)){MainSprite.x4decalage--;}
#define LBACKUP  if  ((CollisionCheck(&MainSprite)==1)) {MainSprite.x4decalage++;}   
while(1){
if (arduboy.pressed(RIGHT_BUTTON))  {
if (timer%4==0) {
MainAnim++;if (MainAnim>2) {MainAnim=0;}
}
LorR=0;
MainSprite.x4decalage++;RBACKUP;if (MainSprite.x4decalage>3) {MainSprite.x4decalage=0;MainSprite.MainPositionOnGridH++;}
}
if (arduboy.pressed(LEFT_BUTTON)){
if (timer%4==0) {
MainAnim++;if (MainAnim>2) {MainAnim=0;}
}
LorR=1;
MainSprite.x4decalage--;LBACKUP;if (MainSprite.x4decalage<0) {MainSprite.x4decalage=3;MainSprite.MainPositionOnGridH--;}
} 
ScrollUpdate(&MainSprite); 
#define exclude(Spick) (ByteMem==Spick)
#define SpritePickup (exclude(11))
if (MainSprite.MainPositionOnGridV>=7) {sound(2);live--;if (live==0) {goto RESTARTGAME;}goto RESTARTLEVEL;}
for (uint8_t x = 0; x < 33; x++)
{
#define LevelS (x+scrool)/4
  switch (Level){
  case(0):LevelMult=pgm_read_byte(&Level0[LevelS]);break;
  case(1):LevelMult=pgm_read_byte(&Level1[LevelS]);break;
  case(2):LevelMult=pgm_read_byte(&Level2[LevelS]);break;
  case(3):LevelMult=pgm_read_byte(&Level3[LevelS]);break;
  case(4):LevelMult=pgm_read_byte(&Level4[LevelS]);break;
  case(5):LevelMult=pgm_read_byte(&Level5[LevelS]);break;
  case(6):LevelMult=pgm_read_byte(&Level6[LevelS]);break;
  case(7):LevelMult=pgm_read_byte(&Level7[LevelS]);break;
  case(8):LevelMult=pgm_read_byte(&Level8[LevelS]);break;
  case(9):LevelMult=pgm_read_byte(&Level9[LevelS]);break;
  default:goto RESTARTGAME;}
#define LevelShift (((x+scrool)%4)+(LevelMult*4))
  ByteMem=pgm_read_byte(&map1couche2[LevelShift]);
  if ((SpritePickup)) {Map[1][x]=delKey(x+scrool,1);}else{Map[1][x]=ByteMem;}
  ByteMem=pgm_read_byte(&map1couche3[LevelShift]);
  if ((SpritePickup)) {Map[2][x]=delKey(x+scrool,2);}else{Map[2][x]=ByteMem;}
  ByteMem=pgm_read_byte(&map1couche4[LevelShift]);
  if ((SpritePickup)) {Map[3][x]=delKey(x+scrool,3);}else{Map[3][x]=ByteMem;}
  ByteMem=pgm_read_byte(&map1couche5[LevelShift]);
  if ((SpritePickup)) {Map[4][x]=delKey(x+scrool,4);}else{Map[4][x]=ByteMem;}
  ByteMem=pgm_read_byte(&map1couche6[LevelShift]);
  if ((SpritePickup)) {Map[5][x]=delKey(x+scrool,5);}else{Map[5][x]=ByteMem;}
  ByteMem=pgm_read_byte(&map1couche7[LevelShift]);
  if ((SpritePickup)) {Map[6][x]=delKey(x+scrool,6);}else{Map[6][x]=ByteMem;}
  ByteMem=pgm_read_byte(&map1couche8[LevelShift]);
  if ((SpritePickup)) {Map[7][x]=delKey(x+scrool,7);}else{Map[7][x]=ByteMem;}
}
if (Jump==0) {GravityUpdate(&MainSprite);}
if (((BUTTON_DOWN))&&(Jump==0)&&(jumpcancel==0)&&(CollisionCheck(&MainSprite)==0)){
if (MainSprite.y8decalage==0) {Jump=3;}
}
if (BUTTON_UP) {jumpcancel=0;}
if (Jump>0) {JumpProcedure(&MainSprite);}
#define pickup(Vadd,Hadd,SPRITE) (Map[MainSprite.MainPositionOnGridV+Vadd][MainSprite.MainPositionOnGridH+Hadd]==SPRITE)
#define Pictup2(SP) ((pickup(0,0,SP))||(pickup(0,1,SP))||(pickup(0,2,SP)))
#define Pictup4(SP2) (Pictup2(SP2)||(pickup(1,0,SP2))||(pickup(1,1,SP2))||(pickup(1,2,SP2)))
if ((MainSprite.y8decalage==0)) {
if( pickup(0,0,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH;keyS++;sound(1);}
if( pickup(0,1,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH+1;keyS++;sound(1);}
if( pickup(0,2,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH+2;keyS++;sound(1);}
if ((Pictup2(8))&&(injur==0)) {live--;if (live==0) {goto RESTARTGAME;}Jump=2;injur=30;sound(2);}
if (Pictup2(13)||Pictup2(14)) {if ((pgm_read_byte(&KeyinLevel[Level])==keyS)&&(Jump>0)) {NextLevel();goto NEXTLEVEL;}}
Map[MainSprite.MainPositionOnGridV][MainSprite.MainPositionOnGridH]=MainSprite.DriftGrid[0][0];
Map[MainSprite.MainPositionOnGridV][MainSprite.MainPositionOnGridH+1]=MainSprite.DriftGrid[0][1];}else{
if( pickup(0,0,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH;keyS++;sound(1);}
if( pickup(0,1,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH+1;keyS++;sound(1);}
if( pickup(0,2,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH+2;keyS++;sound(1);}
if( pickup(1,0,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV+1;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH;keyS++;sound(1);}
if( pickup(1,1,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV+1;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH+1;keyS++;sound(1);}
if( pickup(1,2,11)) {key[keyS][1]=MainSprite.MainPositionOnGridV+1;key[keyS][0]=scrool+MainSprite.MainPositionOnGridH+2;keyS++;sound(1);}   
if ((Pictup2(8))&&(injur==0)) {live--;if (live==0) {goto RESTARTGAME;}Jump=2;injur=30;sound(2);}
if (Pictup4(13)||Pictup4(14)) {if ((pgm_read_byte(&KeyinLevel[Level])==keyS)&&(Jump>0)) {NextLevel();goto NEXTLEVEL;}}
Map[MainSprite.MainPositionOnGridV][MainSprite.MainPositionOnGridH]=MainSprite.DriftGrid[0][0];
Map[MainSprite.MainPositionOnGridV][MainSprite.MainPositionOnGridH+1]=MainSprite.DriftGrid[0][1];
Map[MainSprite.MainPositionOnGridV+1][MainSprite.MainPositionOnGridH]=MainSprite.DriftGrid[1][0];
Map[MainSprite.MainPositionOnGridV+1][MainSprite.MainPositionOnGridH+1]=MainSprite.DriftGrid[1][1];
}
if (timer%2==0)  {if (injur>0) {visible=!visible;injur--;}}
UpdateVerticalSlide(&MainSprite);
Tiny_Flip(&MainSprite);
timer++;
if (timer>60) {timer=0;}
delay(18);
}  
}
uint8_t delKey(uint8_t Xin,uint8_t Yin){
int8_t x=0;  
for (x=0;x<23;x++){
if   ((key[x][0]==0) && (key[x][1]==0)) {return 11;}
if ((key[x][0]==Xin) && (key[x][1]==Yin)) {return 0;}  }
return 11;}




void SSD1306_ssd1306_draw_bmp(uint8_t COL0or1,uint8_t x,uint8_t y,uint8_t w,uint8_t h,const uint8_t *PIC){
for(uint8_t yy=0;yy<8;yy++){
for(uint8_t xx=0;xx<128;xx++){
if ((xx>=x)&&(xx<w)&&(yy>=y)&&(yy<h)) { arduboy.SPItransfer(pgm_read_byte(&PIC[(xx-x)+(((w-x)*(yy-y)))]));}else{

if (COL0or1==1) {arduboy.SPItransfer(0xff);}else{arduboy.SPItransfer(0x00);}

}}}}


void PLAYMUSIC(uint8_t track){
  if (track==0) {
for (uint8_t t=0;t<92;t=t+2){ 
Sound(pgm_read_byte(&Music1[t]),(pgm_read_byte(&Music1[t+1])-100));
}}
  if (track==1) {
 for (uint8_t t=0;t<148;t=t+2){ 
Sound(pgm_read_byte(&Music0[t]),(pgm_read_byte(&Music0[t+1])-100));
}   
  }


}


uint8_t BallMissing(GROUPE *VAR){
if (VAR->Ballxpos<0) {return 1;}
return 0; 
}

uint8_t CheckLevelEnded(GROUPE *VAR){
uint8_t h,v,res=1;
for(v=0;v<5;v++){
for(h=0;h<6;h++){
if ((VAR->BlocsGrid[h][v]!=255)&&(VAR->BlocsGrid[h][v]!=5)) {res=0;}
}}
return res;
}

void UpdateBall(GROUPE *VAR){
   VAR->TrackAngleOut=0;
for (uint8_t T1=0;T1<=6;T1++) {
RecupeBALLPosForSIM(VAR);
if (VAR->launch==0) goto FIN;
SimulMove(T1,VAR);
TestMoveBALL(VAR);
if (CheckCollisionBall(VAR)==0) {goto FIN;}
}
 FIN:;
WriteBallMove(VAR);

}

void RecupeBALLPosForSIM(GROUPE *VAR){

VAR->SIMBallxpos=VAR->Ballxpos;
VAR->SIMBallypos=VAR->Ballypos;
VAR->SIMBallSpeedx=VAR->BallSpeedx;
VAR->SIMBallSpeedy=VAR->BallSpeedy;
}
void TestMoveBALL(GROUPE *VAR){
VAR->SIMBallxpos=VAR->SIMBallxpos+VAR->SIMBallSpeedx;
VAR->SIMBallypos=VAR->SIMBallypos+VAR->SIMBallSpeedy;
}
void SimulMove(uint8_t Sim,GROUPE *VAR){
  switch(Sim){
  case (0):VAR->SIMBallSpeedx=VAR->BallSpeedx;VAR->SIMBallSpeedy=VAR->BallSpeedy;break;
  case (1):VAR->SIMBallSpeedx=-VAR->BallSpeedx;VAR->SIMBallSpeedy=VAR->BallSpeedy;break;
  case (2):VAR->SIMBallSpeedx=VAR->BallSpeedx;VAR->SIMBallSpeedy=-VAR->BallSpeedy;break;
  case (3):VAR->SIMBallSpeedx=-VAR->BallSpeedx;VAR->SIMBallSpeedy=-VAR->BallSpeedy;break;
  case (4):VAR->SIMBallSpeedx=-VAR->BallSpeedy;VAR->SIMBallSpeedy=-VAR->BallSpeedx;break;
  case (5):VAR->SIMBallxpos=VAR->Ballxpos+1;VAR->SIMBallypos=VAR->Ballypos;VAR->SIMBallSpeedx=-1;VAR->SIMBallSpeedy=1;break;
  case (6):VAR->SIMBallxpos=VAR->Ballxpos+1;VAR->SIMBallypos=VAR->Ballypos;VAR->SIMBallSpeedx=-1;VAR->SIMBallSpeedy=-1;break;
  default:break;
}
 
}

uint8_t CheckCollisionBall(GROUPE *VAR){
//tableau limite
if (VAR->SIMBallxpos>106) {return 1;}
if (VAR->SIMBallypos>59) {return 1;}
if (VAR->SIMBallypos<4) {return 1;}
//fin
//TRACKBAR Detect
if (CheckCollisionWithTRACKBAR(VAR)) {Sound(60,10);return 1;}
//fin
if (CheckCollisionWithBLOCK(VAR)) {return 1;}
return 0;
}

uint8_t CheckCollisionWithBLOCK(GROUPE *VAR){
RecupePositionOnGrid(VAR);
if ((VAR->Px==255)||(VAR->Py==255)) {return 0;}

if (VAR->BlocsGrid[VAR->Py][VAR->Px]==5) {Sound(210,50);VAR->ANIMREFLECT=0;return 1;}
if (VAR->BlocsGrid[VAR->Py][VAR->Px]==255) {return 0;}
Sound(150,10);
VAR->BlocsGrid[VAR->Py][VAR->Px]=255;
return 1;
}

void RecupePositionOnGrid(GROUPE *VAR){
VAR->Px=RecupeXPositionOnGrid(VAR);
VAR->Py=RecupeYPositionOnGrid(VAR);
}

uint8_t RecupeXPositionOnGrid(GROUPE *VAR){
if ((VAR->SIMBallxpos>=66)&&(VAR->SIMBallxpos<72))  return 0;
else if ((VAR->SIMBallxpos>=72)&&(VAR->SIMBallxpos<78)) return 1;
else if ((VAR->SIMBallxpos>=78)&&(VAR->SIMBallxpos<84)) return 2;
else if ((VAR->SIMBallxpos>=84)&&(VAR->SIMBallxpos<90))  return 3;
else if ((VAR->SIMBallxpos>=90)&&(VAR->SIMBallxpos<96))  return 4;  
return 255;
}

uint8_t RecupeYPositionOnGrid(GROUPE *VAR){
if ((VAR->SIMBallypos>=8)&&(VAR->SIMBallypos<16)) return 0;
else if ((VAR->SIMBallypos>=16)&&(VAR->SIMBallypos<23))  return 1;
else if ((VAR->SIMBallypos>=23)&&(VAR->SIMBallypos<31))  return 2;
else if ((VAR->SIMBallypos>=31)&&(VAR->SIMBallypos<40))  return 3;
else if ((VAR->SIMBallypos>=40)&&(VAR->SIMBallypos<48))  return 4; 
else if ((VAR->SIMBallypos>=48)&&(VAR->SIMBallypos<55))  return 5;   
return 255;
}

uint8_t CheckCollisionWithTRACKBAR(GROUPE *VAR){
uint8_t TRACK=(VAR->TrackBary*8)+VAR->TrackBaryDecal;
if ((VAR->SIMBallxpos>6)||(VAR->SIMBallxpos<5)) {return 0;}
if (TRACK>VAR->SIMBallypos) {return 0;}
if ((TRACK+16)<VAR->SIMBallypos) {return 0;}

VAR->TrackAngleOut=(((VAR->SIMBallypos-TRACK)*200)/16)-100;//map((VAR->SIMBallypos-TRACK),0,16,-100,100);

return 1;

}

void WriteBallMove(GROUPE *VAR){
float CORECTIONY=(VAR->SIMBallSpeedy)+(VAR->TrackAngleOut/100.00);
if (CORECTIONY<-1) {CORECTIONY=-1;}
if (CORECTIONY>1) {CORECTIONY=1;}
VAR->Ballxpos=VAR->SIMBallxpos;
VAR->Ballypos=VAR->SIMBallypos;
VAR->BallSpeedx=VAR->SIMBallSpeedx;
VAR->BallSpeedy=CORECTIONY;
VAR->BALLyDecal=RecupeDecalageY(VAR->Ballypos-1);
VAR->Ypos=((VAR->Ballypos-1)/8);
}


void Tiny_Flip(uint8_t render0_picture1,GROUPE *VAR){
uint8_t y,x; 
for (y = 0; y < 8; y++){ 

for (x = 0; x < 128; x++){
if (render0_picture1==0) {
arduboy.SPItransfer(Block(x,y,VAR)|Ball(x,y,VAR)|TrackBar(x,y,VAR)|background(x,y)|PannelLive(x,y,VAR)|PannelLevel(x,y,VAR));//
}else if (render0_picture1==1) {
arduboy.SPItransfer(pgm_read_byte(&MAIN[x+(y*128)]));
}else if (render0_picture1==2) {
arduboy.SPItransfer(background(x,y));
}

}

}}

uint8_t PannelLevel(uint8_t X,uint8_t Y,GROUPE *VAR){
if ((Y<5)||(Y>6)||(X<117)||(X>123)) return 0x00;
#define VAl10 (VAR->LEVEL/10)
#define VAl01 (VAR->LEVEL-(VAl10*10))
if (Y==5) {return (pgm_read_byte(&DIGITAL[(X-117)+(VAl10*7)]));}
else if (Y==6) {return (pgm_read_byte(&DIGITAL[(X-117)+(VAl01*7)]));}
return 0x00;
}


uint8_t Block(uint8_t X,uint8_t Y,GROUPE *VAR){
uint8_t XValue=255;
if ((X>=67)&&(X<97)&&(Y>=1)&&(Y<=6)) {
if ((X>=67)&&(X<73)) XValue=0;
else if ((X>=73)&&(X<79)) XValue=1;
else if ((X>=79)&&(X<85)) XValue=2;
else if ((X>=85)&&(X<91)) XValue=3;
else if ((X>=91)&&(X<97)) XValue=4;
if (XValue==255) return 0x00;
if (VAR->BlocsGrid[(Y-1)][XValue]==255) return 0x00;
if (VAR->BlocsGrid[(Y-1)][XValue]==5) {return pgm_read_byte(&BLOCKREFLECT[((X-67)-(XValue*6))+(VAR->ANIMREFLECT*6)])|pgm_read_byte(&BLOCK[((X-67)-(XValue*6))+((VAR->BlocsGrid[(Y-1)][XValue])*6)]);}
  return pgm_read_byte(&BLOCK[((X-67)-(XValue*6))+((VAR->BlocsGrid[(Y-1)][XValue])*6)]);
  
  }  
return 0x00;
}



uint8_t RecupeDecalageY(uint8_t Valeur){
while(Valeur>7){Valeur=Valeur-8;}
return Valeur;
}


uint8_t Ball(uint8_t X,uint8_t Y,GROUPE *VAR){
#define BALLXPOS (VAR->Ballxpos-1)
 #define BALLYPOS (VAR->Ballypos-1)
 
 if (Y<VAR->Ypos) return 0x00;
 if (Y>(VAR->Ypos+1)) return 0x00;
 if ((X-uint8_t(BALLXPOS))<0) return 0x00;
 if (X<BALLXPOS) return 0x00;
  if (X>BALLXPOS+2) return 0x00;
  
if (VAR->BALLyDecal==0)  {
  if (Y==VAR->Ypos ) {return (pgm_read_byte(&BALL[(X-uint8_t(BALLXPOS))]));}
  }else{
uint8_t DECAL=RecupeDecalageY(BALLYPOS);
 
if (Y==VAR->Ypos) { return SplitSpriteDecalageY(DECAL,pgm_read_byte(&BALL[(X-uint8_t(BALLXPOS))]),1);}
if (Y==(VAR->Ypos)+1) { return SplitSpriteDecalageY(DECAL,pgm_read_byte(&BALL[(X-uint8_t(BALLXPOS))]),0);}
 

  }


return 0x00;
}



uint8_t SplitSpriteDecalageY(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN){
if (UPorDOWN) {
return Input<<decalage;
}else{
return Input>>(8-decalage); 
}}

uint8_t TrackBar(uint8_t X,uint8_t Y,GROUPE *VAR){
/*VAR->TrackBary
VAR->TrackBaryDecal*/
if (X>6) return 0;
if (X<3) return 0;
if (Y>=(3+VAR->TrackBary)) return 0;
if (Y<(VAR->TrackBary)) return 0;

if (VAR->TrackBaryDecal==0){if (Y!=VAR->TrackBary+2){ return (pgm_read_byte(&TRACKBAR[(X-3)+((Y-VAR->TrackBary)*4)]));
}}else{

if (Y==VAR->TrackBary) {return SplitSpriteDecalageY(VAR->TrackBaryDecal,pgm_read_byte(&TRACKBAR[(X-3)]),1);}
else if (Y==VAR->TrackBary+1) {return SplitSpriteDecalageY(VAR->TrackBaryDecal,pgm_read_byte(&TRACKBAR[(X-3)]),0)|SplitSpriteDecalageY(VAR->TrackBaryDecal,pgm_read_byte(&TRACKBAR[(X-3)+4]),1);}
else if ((Y==VAR->TrackBary+2)&&(VAR->TrackBaryDecal!=0)) {return SplitSpriteDecalageY(VAR->TrackBaryDecal,pgm_read_byte(&TRACKBAR[(X-3)+4]),0);}
  }

  return 0x00;
}


uint8_t PannelLive(uint8_t X,uint8_t Y,GROUPE *VAR){
if ((Y<1)||(Y>VAR->live)||(X>121)||(X<119)) return 0x00;

return (pgm_read_byte(&LIVEARKANOID[X-119]));
}

uint8_t SWIFT_TEXTURE=0;
uint8_t background(uint8_t X,uint8_t Y){ 
if (X==0) SWIFT_TEXTURE=0;
if (X<=105){
if (SWIFT_TEXTURE<14) {SWIFT_TEXTURE++;}else{SWIFT_TEXTURE=0;}
switch(Y){
  case 0:return (pgm_read_byte(&back_UP[X]));break;
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:return pgm_read_byte(&texture[SWIFT_TEXTURE]);break;
  case 7:return pgm_read_byte(&back_DOWN[X]);break;
  default:break;  
}

}else{
return (pgm_read_byte(&back_RIGHT[(X-106)+(Y*22)]));    
    }
return 0x00;
}


void LoadLevel(uint8_t Level,GROUPE *VAR){
uint8_t a,b;
for(b=0;b<5;b++){
for(a=0;a<6;a++){
VAR->BlocsGrid[a][b]=pgm_read_byte(&LEVEL[(Level*30)+b+(a*5)]);
}}}

void ResetVar(GROUPE *VAR){
uint8_t f=VAR->LEVEL;
while(1){if (f>4) {f=f-5;}else{break;}}
LoadLevel(f,VAR);
VAR->launch=0;
}

void ResetBall(GROUPE *VAR){
VAR->ANIMREFLECT=0;
VAR->TrackBary=2;//2
VAR->TrackBaryDecal=4;//4
VAR->Ballxpos=8;
VAR->SIMBallxpos=8;
VAR->Ballypos=32;
VAR->SIMBallypos=32;
VAR->BallSpeedx=1;
VAR->SIMBallSpeedx=1;
if (VAR->Frame>32) {
VAR->BallSpeedy=.41;
VAR->SIMBallSpeedy=.41;
}else{
VAR->BallSpeedy=.47;
VAR->SIMBallSpeedy=.47;  
}
VAR->launch=0;
}

//tiny gilbert
void ScrollUpdate(DriftSprite* DSprite){
if ((DSprite->MainPositionOnGridH)<10) {if ((scrool>0)&&(step4<=3)) {step4++;}if ((step4>3)&&(scrool>0)) {step4=0;scrool--;DSprite->MainPositionOnGridH=DSprite->MainPositionOnGridH+1;}}
if ((DSprite->MainPositionOnGridH)>18) {step4=step4-1;if (step4<0) {step4=3;scrool++;DSprite->MainPositionOnGridH=DSprite->MainPositionOnGridH-1;}}
}

void UpdateVerticalSlide(DriftSprite* DSprite){
if (DSprite->y8decalage==0) {
if (visible==1){
DSprite->DriftGrid[0][0]=5;
DSprite->DriftGrid[0][1]=6;}else{
DSprite->DriftGrid[0][0]=0;
DSprite->DriftGrid[0][1]=0;
}}else{
if (visible==1){
DSprite->DriftGrid[0][0]=5;
DSprite->DriftGrid[0][1]=6;
DSprite->DriftGrid[1][0]=55;
DSprite->DriftGrid[1][1]=66;}else{
DSprite->DriftGrid[0][0]=0;
DSprite->DriftGrid[0][1]=0;
DSprite->DriftGrid[1][0]=0;
DSprite->DriftGrid[1][1]=0;   
}}}

void GravityUpdate(DriftSprite* DSprite){
int8_t memy8decalage=DSprite->y8decalage;
int8_t memMainPositionOnGridV=DSprite->MainPositionOnGridV;
DSprite->y8decalage=DSprite->y8decalage+2;
if (DSprite->y8decalage>7) {
DSprite->y8decalage=0;
DSprite->MainPositionOnGridV++;
}
if (CollisionCheck(DSprite)>=1){
DSprite->MainPositionOnGridV=memMainPositionOnGridV;
DSprite->y8decalage=memy8decalage;
}}

void JumpProcedure(DriftSprite* DSprite){
int8_t memo2=0;
if ((Jump>0)){
memo2=DSprite->MainPositionOnGridV;
DSprite->y8decalage=DSprite->y8decalage-(Jump*2);
if (DSprite->y8decalage<0)   {DSprite->y8decalage=7; DSprite->MainPositionOnGridV--;Jump--;
if ((DSprite->MainPositionOnGridV<=0)||(jumpcancel==1)||(CollisionCheck(DSprite)>=1)) {
Jump=0;
jumpcancel=1; 
DSprite->y8decalage=0;
DSprite->MainPositionOnGridV=memo2;
Jump=0;jumpcancel=1;}
if (Jump==0) {jumpcancel=1;}
}}}  

int8_t CollisionCheck(DriftSprite* DSprite){
int8_t xscan=0,yscan=0;
 //varable de la grille
#define MXg DSprite->MainPositionOnGridH
#define MYg DSprite->MainPositionOnGridV
#define MXDRIFTg DSprite->x4decalage
#define MYDRIFTg DSprite->y8decalage
#define x1g (MXg*4)+MXDRIFTg
#define y1g (MYg*8)+MYDRIFTg
#define x2g (MXg*4)+MXDRIFTg+7
#define y2g (MYg*8)+MYDRIFTg
#define x3g (MXg*4)+MXDRIFTg
#define y3g (MYg*8)+MYDRIFTg+7
#define x4g (MXg*4)+MXDRIFTg+7
#define y4g (MYg*8)+MYDRIFTg+7
#define bx1g (MXg+xscan)*4
#define by1g (MYg+yscan)*8
#define bx2g ((MXg+xscan)*4)+3
#define by2g (MYg+yscan)*8
#define bx3g (MXg+xscan)*4
#define by3g ((MYg+yscan)*8)+7
#define bx4g ((MXg+xscan)*4)+3
#define by4g ((MYg+yscan)*8)+7
#define NoTested ((Map[MYg+yscan][MXg+xscan]!=8)&&(Map[MYg+yscan][MXg+xscan]!=0)&&(Map[MYg+yscan][MXg+xscan]!=13)&&(Map[MYg+yscan][MXg+xscan]!=14)&&(Map[MYg+yscan][MXg+xscan]!=11)&&(Map[MYg+yscan][MXg+xscan]!=5)&&(Map[MYg+yscan][MXg+xscan]!=6)&&(Map[MYg+yscan][MXg+xscan]!=55)&&(Map[MYg+yscan][MXg+xscan]!=66))
for (yscan=-1;yscan<3;yscan++){
for (xscan=-1;xscan<3;xscan++){
if (NoTested) { 
if ((x1g>bx2g)||(x2g<bx1g)||(y1g>by3g)||(y3g<by1g)) {}else{return 1;}
}}}
return 0;
}

void ResetVar(void){
ResetVarNextLevel();
Level=0;
live=7;
}

void ResetVarNextLevel(void){
scrool=0;
step4=0;
MainAnim=0;
LorR=1;
Jump=0;
jumpcancel=0;
VSlideOut=0;
for (uint8_t x=0;x<20;x++){
key[x][0]=0;
key[x][1]=0;}
keyS=0;
LevelMult=0;}

void NextLevel(void){
ResetVarNextLevel();
Level++; 
sound(2);sound(2);sound(2);sound(2);}

void TinyMainShift(DriftSprite* DSprite){
DSprite->DriftGrid[0][0]=5;
DSprite->DriftGrid[0][1]=6;
DSprite->DriftGrid[1][0]=5;
DSprite->DriftGrid[1][1]=6;}

void SpriteShiftInitialise(DriftSprite* DSprite){
DSprite->DriftGrid[0][0]=0; //bas gauche
DSprite->DriftGrid[0][1]=0; //bas droite
DSprite->DriftGrid[1][0]=0; //haut gauche
DSprite->DriftGrid[1][1]=0; //haut droite
DSprite->x4decalage=0;
DSprite->y8decalage=0;
DSprite->MainPositionOnGridH=11;
DSprite->MainPositionOnGridV=3;
}

void Tiny_Flip(DriftSprite* DSprite){
uint8_t nn,x,m,n,t,Start,decal;
uint8_t while1=1;
#define PrecessQuit nn++;if (nn>127) {while1=0;break;}

for (x=0;x<live-1;x++){
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite26[t]));}
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite27[t]));}
}
for (x=0;x<11+(3-(live-1));x++){
for(t=0;t<8;t++){arduboy.SPItransfer(0x00);}}
if ((pgm_read_byte(&KeyinLevel[Level])==keyS)&&(timer<=30)) {
for (x=0;x<4;x++){
arduboy.SPItransfer(pgm_read_byte(&sprite12[x]));}
for (x=0;x<3;x++){
for (t=0;t<4;t++){arduboy.SPItransfer(0x00);}
}
}else{
for (x=0;x<4;x++){
for (t=0;t<4;t++){arduboy.SPItransfer(0x00);}
}}  
for (m = 1; m < 8; m++)
{

n=0; 
nn=0;  
while1=1;
Start=4-step4; 
#define decalIN for(decal=0;decal<DSprite->x4decalage;decal++){arduboy.SPItransfer(0X00);PrecessQuit}// for main sprite scr pix2pix
#define decalOUT for(decal=0;decal<4-DSprite->x4decalage;decal++){arduboy.SPItransfer(0X00);PrecessQuit}// for main sprite scr pix2pix
while(while1){
if ((Map[m][n]==7)&&(while1!=0)) {
for(t=Start;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite7[t]));PrecessQuit}Start=0;
// main sprite 
}else if (((Map[m][n]==5)||(Map[m][n]==55))&&(while1!=0)) {
if (Map[m][n]==55) {VSlideOut=((100/VSlide[8-DSprite->y8decalage])/100);}else{VSlideOut=VSlide[DSprite->y8decalage];}
decalIN;
if (LorR==1){
if (MainAnim==0) {
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite20[t])*VSlideOut);PrecessQuit}Start=0;
}
if (MainAnim==1) { 
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite22[t])*VSlideOut);PrecessQuit}Start=0;
}
if (MainAnim==2) {
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite24[t])*VSlideOut);PrecessQuit}Start=0; 
} 
}else{  
if (MainAnim==0) {
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite26[t])*VSlideOut);PrecessQuit}Start=0;   
}
if (MainAnim==1) {
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite28[t])*VSlideOut);PrecessQuit}Start=0;   
}
if (MainAnim==2) {
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite30[t])*VSlideOut);PrecessQuit}Start=0;  
}}
}else if (((Map[m][n]==6)||(Map[m][n]==66))&&(while1!=0)) {
if (Map[m][n]==66) {VSlideOut=((100/VSlide[8-DSprite->y8decalage])/100);}else{VSlideOut=VSlide[DSprite->y8decalage];}
if (LorR==1){
if (MainAnim==0) {
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite21[t])*VSlideOut);PrecessQuit Start=DSprite->x4decalage;}
}
if (MainAnim==1) {
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite23[t])*VSlideOut);PrecessQuit Start=DSprite->x4decalage;}
}
if (MainAnim==2) {
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite25[t])*VSlideOut);PrecessQuit Start=DSprite->x4decalage;}
} }else{  
if (MainAnim==0) {
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite27[t])*VSlideOut);PrecessQuit Start=DSprite->x4decalage;}
}
if (MainAnim==1) {
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite29[t])*VSlideOut);PrecessQuit Start=DSprite->x4decalage;} 
}
if (MainAnim==2) {
for(t=0;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite31[t])*VSlideOut);PrecessQuit Start=DSprite->x4decalage;}
}}
//fin main sprite
}else if ((Map[m][n]==1)&&(while1!=0)) {
for(t=Start;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite1[t]));PrecessQuit}Start=0;
}else if ((Map[m][n]==2)&&(while1!=0)) {
for(t=Start;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite2[t]));PrecessQuit}Start=0;
}else if ((Map[m][n]==3)&&(while1!=0)) {
for(t=Start;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite3[t]));PrecessQuit}Start=0;
}else if ((Map[m][n]==4)&&(while1!=0)) {
for(t=Start;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite4[t]));PrecessQuit}Start=0;
}else if ((Map[m][n]==8)&&(while1!=0)) {
for(t=Start;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite8[t]));PrecessQuit}Start=0;
}else if ((Map[m][n]==15)&&(while1!=0)) {
for(t=Start;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite15[t]));PrecessQuit}Start=0;
}else if ((Map[m][n]==16)&&(while1!=0)) {
for(t=Start;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite16[t]));PrecessQuit}Start=0;
}
else if ((Map[m][n]==11)&&(while1!=0)) {
if (timer>30){
for(t=Start;t<4;t++){
arduboy.SPItransfer(pgm_read_byte(&sprite11[t]));PrecessQuit}Start=0;
}else{
for(t=Start;t<4;t++){
arduboy.SPItransfer(pgm_read_byte(&sprite12[t]));PrecessQuit}Start=0;
}}else if ((Map[m][n]==13)&&(while1!=0)) {
for(t=Start;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite13[t]));PrecessQuit}Start=0;
}else if ((Map[m][n]==14)&&(while1!=0)) {
for(t=Start;t<4;t++){arduboy.SPItransfer(pgm_read_byte(&sprite14[t]));PrecessQuit}Start=0;
}
else{if (while1!=0){
for(t=Start;t<4;t++){
arduboy.SPItransfer(0x00);PrecessQuit}Start=0;
}}
if (while1!=0) {n++; }
}

}}

void SSD1306_ssd1306_fillscreen(uint8_t BYTE){
for(uint16_t t=0;t<1024;t++){
arduboy.SPItransfer(BYTE);
}
}

/*void SSD1306_ssd1306_draw_bmp(uint8_t x,uint8_t y,uint8_t w,uint8_t h){
//SSD1306_ssd1306_draw_bmp(32, 4, 96, 8, start);
for(uint8_t y=0;y<8;y++){
for(uint8_t x=0;x<128;x++){
if ((x>31)&&(x<96)&&(y>3)&&(y<8)) { arduboy.SPItransfer(pgm_read_byte(&start[(x-32)+((64*(y-4)))]));}else{
 arduboy.SPItransfer(0xff);
}
}

}
}*/

//fin tiny gilbert
uint8_t CheckLevelEnded(PERSONAGEBomber *Sprite){
for (uint8_t t=1;t<=4;t++){
if (Sprite[t].dead==0) {return 0;}  
}
return 1;
}

void DeadSong(void){
for (uint8_t t=1;t<255;t++){
Sound(t,1);
Sound(255-t,2);   
}
delay(300);
}

void DestroyBloc(void){
uint8_t x=((BOMBXY[0]-8)/8);
uint8_t y=(BOMBXY[1]-1);
if (y!=0) BOOLWRITE0Bomber(x+((y-1)*15));
BOOLWRITE0Bomber(x+((y)*15));
if (y!=6) BOOLWRITE0Bomber(x+((y+1)*15));
if (x!=0) BOOLWRITE0Bomber((x-1)+((y)*15));
if (x!=14) BOOLWRITE0Bomber((x+1)+((y)*15));
}

uint8_t COLLISION11(uint8_t x1,uint8_t y1,uint8_t w1,uint8_t h1,uint8_t x2,uint8_t y2,uint8_t w2,uint8_t h2){
if ((x1+w1<x2)||(y1+h1<y2)||(x1>x2+w2)||(y1>y2+h2)) {return 0;}else{return 1;}
}

void DestroyEnemy(PERSONAGEBomber *Sprite){
#define xminf(I) (Sprite[I].x)
#define yminf(I) ((Sprite[I].y*8)+Sprite[I].Decalagey)
for (uint8_t t=0;t<=4;t++){
if (COLLISION11(xminf(t),yminf(t),7,7,BOMBXY[0]+1,(BOMBXY[1]*8)-6,6,21)) {
Sprite[t].y=0; 
Sprite[t].Decalagey=0;
if (Sprite[t].dead!=1)  {Sprite[t].dead=1;}
}}
for (uint8_t t=0;t<=4;t++){
if (COLLISION11(xminf(t),yminf(t),7,7,BOMBXY[0]-6,(BOMBXY[1]*8)+1,21,6)) {
Sprite[t].y=0; 
Sprite[t].Decalagey=0;
if (Sprite[t].dead!=1) {Sprite[t].dead=1;}
}}}



uint8_t CollisionTiny2Caracter(PERSONAGEBomber *Sprite){
uint8_t ReturnCollision=0;
#define xmax(I) (Sprite[I].x+6)
#define xmin(I) (Sprite[I].x)
#define ymax(I) ((Sprite[I].y*8)+Sprite[I].Decalagey+6)
#define ymin(I) ((Sprite[I].y*8)+Sprite[I].Decalagey)
if ((INGAMEBomber)) {    
for (uint8_t t=1;t<=4;t++){
if (Sprite[t].dead!=1) {
if ((xmax(0)<xmin(t))||(xmin(0)>xmax(t))||(ymax(0)<ymin(t))||(ymin(0)>ymax(t))) {}else{ 
ReturnCollision=1;
}}
}}return ReturnCollision;}


uint8_t RANDOM(void){
if (RNDcount<99) {RNDcount++;}else{RNDcount=0;} 
return pgm_read_byte(&RANDLIST[RNDcount]);
}

void RefreshCaracterBomber(PERSONAGEBomber *Sprite){
uint8_t memx,memy,memdecalagey;
for (uint8_t t=0;t<=4;t++){
  if (((t!=0)&&(SPEEDBOMBER==1))||(FrameBomber%2==0)){
memx=Sprite[t].x;
memy=Sprite[t].y;
memdecalagey=Sprite[t].Decalagey;
if (Sprite[t].DirectionV==1) {Sprite[t].x++;}
if (Sprite[t].DirectionV==0) {Sprite[t].x--;}
if (CheckCollisionWithBackBomber(t,1,Sprite)) {
if (t!=0) {Sprite[t].DirectionV=RANDOM();}else{ Sprite[t].DirectionV=2;}
Sprite[t].x=memx;
}
if (Sprite[t].DirectionH==1) {if (Sprite[t].Decalagey<7) {Sprite[t].Decalagey++;}else{Sprite[t].Decalagey=0;Sprite[t].y++;}}
if (Sprite[t].DirectionH==0) {if (Sprite[t].Decalagey>0) {Sprite[t].Decalagey--;}else{Sprite[t].Decalagey=7;Sprite[t].y--;}}
if (CheckCollisionWithBackBomber(t,0,Sprite)) {
if (t!=0) {Sprite[t].DirectionH=RANDOM();}else{Sprite[t].DirectionH=2;}
Sprite[t].y=memy;
Sprite[t].Decalagey=memdecalagey;
}
if (t==0) {
if (FrameBomber%2==0) {
if (Sprite[t].DirectionH==1) {Sprite[t].DirectionAnim=0;}
if (Sprite[t].DirectionH==0) {Sprite[t].DirectionAnim=(2*3);} 
if (Sprite[t].DirectionV==1) {Sprite[t].DirectionAnim=(3*3);}
if (Sprite[t].DirectionV==0) {Sprite[t].DirectionAnim=(1*3);}
}}else{
if ((FrameBomber==0)||(FrameBomber==12)) {
Sprite[t].DirectionAnim=0;
if (Sprite[t].DirectionH==1) {Sprite[t].DirectionAnim=0;}
if (Sprite[t].DirectionH==0) {Sprite[t].DirectionAnim=0;}
}}
if (t==0) {
if (FrameBomber%2==0) {
if (Sprite[0].switchanim==0) {
if ((Sprite[0].anim<2)&&((Sprite[t].DirectionH!=2)||(Sprite[t].DirectionV!=2))) {Sprite[0].anim++;}else{Sprite[0].switchanim=1;} 
}else{
if (Sprite[0].anim>0) {Sprite[0].anim--;}else{Sprite[0].switchanim=0;}  
}}}else{
if  (FrameBomber%6==0) {
if (Sprite[t].anim<2) {Sprite[t].anim++;}else{Sprite[t].anim=0;} 
}}
}// if t1
}}

uint8_t CheckCollisionWithBackBomber(uint8_t SpriteCheck,uint8_t HorVcheck,PERSONAGEBomber *Sprite){
uint8_t BacktoComp;
if (HorVcheck==1) {
BacktoComp=RecupeBacktoCompVBomber(SpriteCheck,Sprite); 
}else{
BacktoComp=RecupeBacktoCompHBomber(SpriteCheck,Sprite);}
BacktoComp=BacktoComp+CheckCollisionWithBlock(SpriteCheck,Sprite);
BacktoComp=BacktoComp+CheckCollisionWithBomb(SpriteCheck,Sprite);
if ((BacktoComp)!=0) {return 1;}else{return 0;}}
uint8_t CheckCollisionWithBomb(uint8_t SpriteCheck,PERSONAGEBomber *Sprite){
if (SpriteCheck==0) {return 0;}
return COLLISION11(Sprite[SpriteCheck].x,(Sprite[SpriteCheck].y*8)+Sprite[SpriteCheck].Decalagey,7,7,BOMBXY[0],(BOMBXY[1]*8),7,7);
}

uint8_t CheckCollisionWithBlock(uint8_t SpriteCheck,PERSONAGEBomber *Sprite){
#define A (Sprite[SpriteCheck].x)
#define AA ((A+7))
uint8_t B=((((Sprite[SpriteCheck].y-1)*8)+Sprite[SpriteCheck].Decalagey)/8);
uint8_t BB=(((((Sprite[SpriteCheck].y-1)*8)+Sprite[SpriteCheck].Decalagey)+7)/8);
uint8_t Rest=0;
uint8_t ReadBloc=pgm_read_byte(&BlocDetectBomber[A]);
if ((ReadBloc!=0xff)&&(ReadBloc!=0xfE)) {
Rest=Rest+BOOLREADBomber(ReadBloc+(B*15));
Rest=Rest+BOOLREADBomber(ReadBloc+(BB*15));
}
ReadBloc=pgm_read_byte(&BlocDetectBomber[AA]);
if ((ReadBloc!=0xff)&&(ReadBloc!=0xfE))  {
Rest=Rest+BOOLREADBomber(ReadBloc+(B*15));
Rest=Rest+BOOLREADBomber(ReadBloc+(BB*15));
}
return Rest;
}

uint8_t RecupeBacktoCompVBomber(uint8_t SpriteCheck,PERSONAGEBomber *Sprite){
uint8_t Y1=0b00000000;
uint8_t Y2=Y1;
#define SpriteWide 6
#define MAXV (Sprite[SpriteCheck].x+SpriteWide)
#define MINV (Sprite[SpriteCheck].x)
if (Sprite[SpriteCheck].DirectionV==1) {
Y1=pgm_read_byte(&backBomber[((Sprite[SpriteCheck].y)*128)+(MAXV)]);
Y2=pgm_read_byte(&backBomber[((Sprite[SpriteCheck].y+1)*128)+(MAXV)]);
}else if (Sprite[SpriteCheck].DirectionV==0) {
Y1=pgm_read_byte(&backBomber[((Sprite[SpriteCheck].y)*128)+(MINV)]);
Y2=pgm_read_byte(&backBomber[((Sprite[SpriteCheck].y+1)*128)+(MINV)]);
}else{Y1=0;Y2=0;}
//decortique
Y1=TrimBomber(0,Y1,Sprite[SpriteCheck].Decalagey);
Y2=TrimBomber(1,Y2,Sprite[SpriteCheck].Decalagey);
if (((Y1)!=0b00000000)||((Y2)!=0b00000000) ) {return 1;}else{return 0;}
}

uint8_t TrimBomber(uint8_t Y1orY2,uint8_t TrimValue,uint8_t Decalage){
uint8_t Comp;
if (Y1orY2==0) {
Comp=0b01111111<<Decalage;
return (TrimValue&Comp);
}else{
Comp=(0b01111111>>(8-Decalage));
return (TrimValue&Comp);
}}


uint8_t ScanHRecupeBomber(uint8_t UporDown,uint8_t Decalage){
if (UporDown==0){
return 0b01111111<<Decalage;}
else{
return 0b01111111>>(8-Decalage);
}}

uint8_t RecupeBacktoCompHBomber(uint8_t SpriteCheck,PERSONAGEBomber *Sprite){
uint8_t TempPGMByte;
if (Sprite[SpriteCheck].DirectionH==0) {
uint8_t RECUPE=(ScanHRecupeBomber(0,Sprite[SpriteCheck].Decalagey));
for(uint8_t t=0;t<=6;t++){
if ((((Sprite[SpriteCheck].y)*128)+(Sprite[SpriteCheck].x+t)>1023)||(((Sprite[SpriteCheck].y)*128)+(Sprite[SpriteCheck].x+t)<0)) {TempPGMByte=0x00;}else{
 TempPGMByte=(pgm_read_byte(&backBomber[((Sprite[SpriteCheck].y)*128)+(Sprite[SpriteCheck].x+t)])); 
}
#define CHECKCOLLISION ((RECUPE)&(TempPGMByte))
if  (CHECKCOLLISION!=0) {return 1;}
}}else if (Sprite[SpriteCheck].DirectionH==1) {
uint8_t tadd=0;
if (Sprite[SpriteCheck].Decalagey>2) { tadd=1;}else{tadd=0;}
uint8_t RECUPE=(ScanHRecupeBomber(tadd,Sprite[SpriteCheck].Decalagey));
for(uint8_t t=0;t<=6;t++){
if (((((Sprite[SpriteCheck].y+tadd)*128)+(Sprite[SpriteCheck].x+t))>1023)||((((Sprite[SpriteCheck].y+tadd)*128)+(Sprite[SpriteCheck].x+t))<0)) {TempPGMByte=0x00;}else{
TempPGMByte=pgm_read_byte(&backBomber[((Sprite[SpriteCheck].y+tadd)*128)+(Sprite[SpriteCheck].x+t)]);
}
#define CHECKCOLLISION2 ((RECUPE)&(TempPGMByte))
if  (CHECKCOLLISION2!=0) {return 1;}
}}return 0;}

void Tiny_FlipBomber(uint8_t render0_picture1,PERSONAGEBomber *Sprite){
uint8_t y,x; 
uint8_t Div2x=0;
for (y = 0; y < 8; y++){ 
for (x = 0; x < 128; x++){
if (render0_picture1==0) {
if (INGAMEBomber) {arduboy.SPItransfer(PrintLive(x,y)&((backgroundBomber(x,y)|SpriteWriteBomber(x,y,Sprite)|BlockBomb(x,y,Div2x)|BombBlitz(x,y)|Explose(x,y))));}else{
}}else if (render0_picture1==1){
arduboy.SPItransfer((pgm_read_byte(&BackBlitzBomber[x+(y*128)])));}Div2x=!Div2x;}

}}

uint8_t PrintLive(uint8_t x,uint8_t y){
uint8_t rest=0xFF;
if ((x>7)||(x<1)) return 0xFF;
switch (live){
  case 0:if (y==6) rest=0x00;
  case 1:if (y==5) rest=0x00;
  case 2:if (y==4) rest=0x00;
  case 3:
  default:break;
}
return rest;  
}

uint8_t Explose(uint8_t x,uint8_t y){
#define XPOSFIRE (BOMBXY[0]-8)
#define YPOSFIRE (BOMBXY[1]-1)
if (BOMBXY[2]==1) {   
if ((x>7)&&(y>0)){                         
if ((x>=XPOSFIRE)&&(x<XPOSFIRE+24)&&(y>=YPOSFIRE)&&(y<=YPOSFIRE+2)) {
return pgm_read_byte(&fireBomber[((x-(XPOSFIRE))+((y-(YPOSFIRE))*72))+(BOMBXY[3]*24)]);
}}}
return 0x00;}

uint8_t BombBlitz(uint8_t x,uint8_t y){
if ((x>=BOMBXY[0])&&(x<=BOMBXY[0]+7)&&(y==BOMBXY[1])) {
return pgm_read_byte(&bombBomber[(x-BOMBXY[0])+(BOMBXY[3]*8)]);
}else{return 0x00;}
}

uint8_t BlockBomb(uint8_t x,uint8_t y,uint8_t Divx){
if ((y>0)){
uint8_t BLOCVAL=pgm_read_byte(&BlocDetectBomber[x]);
if (BLOCVAL==0xFE) BLOCVAL=0x01;
if (BLOCVAL==0xFF) return 0x00;
if (BOOLREADBomber(BLOCVAL+((y-1)*15))){
if (Divx==0) {
return 0b10101010;  
}else{
return 0b01010101;}
}}
return 0x00;
}

void BOOLWRITE0Bomber(uint8_t BoolNumber){
uint8_t REST=BoolNumber;
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
BlocBombMem[DOTBOOLPOSITION]=BlocBombMem[DOTBOOLPOSITION]&SOUSTRAIRE;
}

uint8_t BOOLREADBomber(uint8_t Numero){
if (Numero>105) return 0;
uint8_t BYTECHECK=Numero;
uint8_t INT8CHOSE=0;
while(BYTECHECK>7){
BYTECHECK=BYTECHECK-8; 
INT8CHOSE++;}
uint8_t Var0=0;
switch (BYTECHECK){
  case 0:Var0=0b10000000;break;
  case 1:Var0=0b01000000;break;
  case 2:Var0=0b00100000;break;
  case 3:Var0=0b00010000;break;
  case 4:Var0=0b00001000;break;
  case 5:Var0=0b00000100;break;
  case 6:Var0=0b00000010;break;
  case 7:Var0=0b00000001;break;
  default:Var0=0b00000000;break;}
if ((BlocBombMem[INT8CHOSE]&Var0)!=0) {return 1;}else{return 0;}
}

uint8_t SplitSpriteDecalageYBomber(uint8_t decalage,uint8_t Input,uint8_t UPorDOWN){
if (UPorDOWN) {
return Input<<decalage;
}else{
return Input>>(8-decalage); 
}}

uint8_t SpriteWriteBomber(uint8_t x,uint8_t y,PERSONAGEBomber  *Sprite){
uint8_t var1=0;
uint8_t AddBin=0b00000000;
while(1){ 
if (Sprite[var1].y==y) {
if (Sprite[var1].dead!=1) AddBin=AddBin|SplitSpriteDecalageYBomber(Sprite[var1].Decalagey,return_if_sprite_presentBomber(x,Sprite,var1),1);
}else if (((Sprite[var1].y+1)==y)&&(Sprite[var1].Decalagey!=0)) {
if (Sprite[var1].dead!=1) AddBin=AddBin|SplitSpriteDecalageYBomber(Sprite[var1].Decalagey,return_if_sprite_presentBomber(x,Sprite,var1),0);
}
var1++;
if (var1==5) {break;}
}return AddBin;}

uint8_t return_if_sprite_presentBomber(uint8_t x,PERSONAGEBomber  *Sprite,uint8_t SpriteNumber){

if  ((x>=Sprite[SpriteNumber].x)&&(x<(Sprite[SpriteNumber].x+8))) { 

if ((INGAMEBomber==0)&&(SpriteNumber==0)) {  return 0;}     
return pgm_read_byte(&caractersBomber[((x-Sprite[SpriteNumber].x)+(8*(Sprite[SpriteNumber].type*12)))+(Sprite[SpriteNumber].anim*8)+(Sprite[SpriteNumber].DirectionAnim*8)]);
}return 0;}

uint8_t backgroundBomber(uint8_t x,uint8_t y){
return pgm_read_byte(&BackBlitzBomber[((y)*128)+((x))]);
}
