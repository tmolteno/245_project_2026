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
#include "spritebank_TBIKE.h"

//prototype
void ResetSprite(void);
void intro_sound(void);
void End_Line_Win_sound(void);
void restore_start_line(void);
void Next_Level(void);
void RESET_FOR_NEW_GAME(void);
void loop_TBIKE();
void Pause1(void);
uint8_t GAME_PLAY(void);
void TIME_TRACK(void);
void PLANTAGE(void);
void Higher_adj(uint8_t test);
void dinamic_adj(void);
void GRAVITY_ADJ(void);
void JUMP_ADJ(void);
void CheckCollision(void);
uint8_t  Trouver_Sprite_Collisionner(void);
void analise_minutieuse(void);
void ADD_LIVE(uint8_t SPR);
void CLIMB_ADJ3(void);
void Break_Gravity(void);
void CLIMB_ADJ0(void);
void CLIMB_ADJ1(void);
void CLIMB_ADJ2(void);
uint8_t RECUPE_X_SPRITE(uint8_t Type_Sprite);
uint8_t RECUPE_Y_SPRITE(uint8_t Type_Sprite);
uint8_t CHECK_SPEED_ADJ(float REC_INT);
void TRACK_RUN_ADJ(void);
uint8_t RECUPE_MAP_BYTE(const uint8_t *Level);
uint8_t SPLIT_MAP_BYTE(uint8_t BYTE,uint8_t L0_R1);
void RefreshPosSprite(void);
void INCREMENTE_SCROLL(void);
uint8_t CREATE_NEWSPRITE(void);
uint8_t  BACKGROUND(uint8_t xPASS,uint8_t yPASS);
uint8_t TOPBACK(uint8_t xPASS,uint8_t yPASS);
uint8_t FOUL1(void);
uint8_t GRADIN23(uint8_t yPASS);
uint8_t TIRE4(void);
uint8_t ROAD567(uint8_t yPASS);
uint8_t TABLEAU8(uint8_t xPASS);
uint8_t Return_live(uint8_t xPASS);
uint8_t Return_time(uint8_t xPASS);
uint8_t Return_Progress(uint8_t xPASS);
void AdjustVarScroll(void);
uint8_t Recupe(uint8_t xPASS,uint8_t yPASS);
uint8_t BLITZ_SPRITE_MAP(uint8_t xPASS,uint8_t yPASS);
uint8_t BIGSTEP_SPRITE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS);
uint8_t MINISTEP_SPRITE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS);
uint8_t Plantage(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS);
uint8_t HUILE_SPRITE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS);
uint8_t LINE_SPRITE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS);
uint8_t START_SPRITE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS);
uint8_t SPEED_SPRITE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS);
uint8_t NEW_LIVE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS);
uint8_t BIKE_SPRITE(uint8_t xPASS,uint8_t yPASS);
void Tiny_Flip_TBIKE(uint8_t MODE);

#define NUMofSPRITE 2

uint8_t ReWind=0;
uint8_t End_game=0;
int8_t Live_TBIKE=3;
uint8_t Progress_bar_Interval;
uint8_t Progress_bar_Interval_timer;
uint8_t Progress_bar_value;
uint8_t Time_bar_Interval;
uint8_t Time_bar_Interval_timer;
uint8_t Time_bar_value;
uint8_t CLIMB_ACTIVATE=0;
float Higher_jump=0;
float ADD_PILL=0;
float Jump_dinamic_duration;
uint8_t PLANCHER;
uint8_t PLANCHER_add;
int8_t TRANSITION_TRACK=0;
uint8_t ENDMAP=0;
uint8_t LINE_Y;
uint8_t DECALAGE;
uint8_t DIV1=0;
float ACCEL=0;
uint8_t trackrun=2;
uint8_t Trackrun_progress=2;
float gravity_expo=0;
uint8_t BIKE_POSy=33;
uint8_t TRIG_OK=0;
uint16_t MAP_POS=0;
uint8_t animBike=1;
uint8_t RENEW_SPRITE_TBIKE=0;
uint8_t NoSprite=0;
uint8_t FOUL_BLITZ=0;
uint8_t VAR_SCROLL1=0;
uint8_t VAR_SCROLL2=0;
uint8_t VAR_SCROLL3=0;
uint8_t Not_Move=0;
uint8_t Not_Turn=0;
uint8_t Wheel_up=1;
uint8_t FREEAIR=0;
uint8_t BypassWheelupreset=0;
uint8_t Latch1=0;
uint8_t PLANTAGE_0=0;
uint8_t Pause=0;
Sprite_TBIKE sprite_TBIKE[NUMofSPRITE];
const uint8_t *INTROPIC;

void ResetSprite(void){
for (uint8_t t=0;t<NUMofSPRITE;t++){
sprite_TBIKE[t].ACTIVE=0;
sprite_TBIKE[t].TypeofSprite=0;
sprite_TBIKE[t].xPOS=0;
sprite_TBIKE[t].yPOS=0;
sprite_TBIKE[t].Y_START=0;
sprite_TBIKE[t].Y_END=0;
}}


void intro_sound(void){
Sound(100,255); 
_delay_ms(400); 
Sound(100,255); 
_delay_ms(400); 
Sound(100,255); 
_delay_ms(400); 
Sound(155,255); 
Sound(155,255);
Sound(155,255); 
Sound(155,255); 

}

void End_Line_Win_sound(void){
for(uint8_t t=0;t<5;t++){
  Sound (100,100);
  Sound (1,100);}
}

void restore_start_line(void){
sprite_TBIKE[0].ACTIVE=1;
sprite_TBIKE[0].TypeofSprite=5;  
sprite_TBIKE[0].xPOS=36;  
sprite_TBIKE[0].yPOS=32;
}

void Next_Level(void){
ENDMAP=0;
CLIMB_ACTIVATE=0;
Higher_jump=0;
ADD_PILL=0;
TRANSITION_TRACK=0;
DIV1=0;
ACCEL=0;
trackrun=2;
Trackrun_progress=2;
gravity_expo=0;
BIKE_POSy=33;
TRIG_OK=0;
animBike=1;
RENEW_SPRITE_TBIKE=0;
NoSprite=0;
FOUL_BLITZ=0;
VAR_SCROLL1=0;
VAR_SCROLL2=0;
VAR_SCROLL3=0;
Not_Move=0;
Not_Turn=0;
Wheel_up=1;
FREEAIR=0;
BypassWheelupreset=0;
Latch1=0;
PLANTAGE_0=0;
Pause=0;
if (ReWind==1) {ReWind=0;MAP_POS=0;}
if (MAP_POS!=0) {MAP_POS++;}
Progress_bar_Interval=pgm_read_byte(&Level0[MAP_POS]);
Progress_bar_Interval_timer=0;
Progress_bar_value=0;
MAP_POS++;
Time_bar_Interval=pgm_read_byte(&Level0[MAP_POS]);
Time_bar_Interval_timer=0;
Time_bar_value=0;
MAP_POS++;
INTROPIC=&NEXTRACE[0];
}

void RESET_FOR_NEW_GAME(void){
End_game=0;
Live_TBIKE=3;
Jump_dinamic_duration=0;
PLANCHER=0;
PLANCHER_add=0;
LINE_Y=0;
DECALAGE=0;
MAP_POS=0;
INTROPIC=&START_GAME[0];
}

void loop_TBIKE() {
New_Games:;
RESET_FOR_NEW_GAME();
while(1){
Tiny_Flip_TBIKE(2);
if ((BUTTON_DOWN)) {INTROPIC=&START_RACE[0];Sound(200,20);Sound(60,20);goto NEXT_LEVEL;}
}
NEXT_LEVEL:;
Tiny_Flip_TBIKE(2);
_delay_ms(2000);
ResetSprite();
Next_Level();
NEW_START:;
if ((BUTTON_DOWN)) {goto NEW_START;}
uint8_t t=0;
restore_start_line();
Tiny_Flip_TBIKE(3);
intro_sound();
if ((BUTTON_DOWN)) {goto NEW_START;}
animBike=6;
while(1){
if (PLANTAGE_0==0){
if (TRIG_OK==0) {
if ((arduboy.pressed(LEFT_BUTTON))&&(ACCEL>1)) {BypassWheelupreset=1;if ((Wheel_up<5)&&(Latch1==0)){Wheel_up++;}}
else if ((arduboy.pressed(RIGHT_BUTTON))&&(ACCEL>1)) {BypassWheelupreset=1;if ((Wheel_up>0)&&(Latch1==0)){Wheel_up--;if ((Wheel_up==0)&&(FREEAIR==0)) {Wheel_up=1;} }}

if ((Not_Move==0)&&(Not_Turn==0)){
if ((arduboy.pressed(DOWN_BUTTON))) {if (Trackrun_progress<3){Trackrun_progress++;TRIG_OK=1;}}
else if ((arduboy.pressed(UP_BUTTON))) {if (Trackrun_progress>0){Trackrun_progress--;TRIG_OK=2; }}
}
if ((TRIG_OK==0)&&(Wheel_up==1)&&(t>0)) {animBike = (animBike==1) ? 6 : 1;}
}
if (((BUTTON_DOWN)&&(ENDMAP!=1))&&((FREEAIR==0)||(Wheel_up<=2))&&(End_game==0) ) {
  if (ACCEL<8){ACCEL=ACCEL+0.10;}
  }else{
    if (ACCEL>1){ACCEL=ACCEL-0.10;}BypassWheelupreset=0;
   if ((Wheel_up>1)&&(Latch1==0)&&(FREEAIR==0)){Wheel_up--;}
    }
if (BypassWheelupreset==0){
if ((Wheel_up>1)&&(Latch1==0)&&(FREEAIR==0)){Wheel_up--;}
}else{BypassWheelupreset=0;}
}else{PLANTAGE();}
dinamic_adj();
for (t=0;t<CHECK_SPEED_ADJ(ACCEL);t++){
  INCREMENTE_SCROLL();if (DIV1==3) { Sound(1,1);TRACK_RUN_ADJ();
  if (Wheel_up!=1) {animBike=Wheel_up;}
  DIV1=0;}else{DIV1++;}}
  if (Pause==1) {if (Live_TBIKE>-1) {Live_TBIKE--;}Pause=0;}
Latch1++;
if (Latch1==4){ Latch1=0;}
if (((FREEAIR==0)&&((Wheel_up==0)||(Wheel_up==5)))&&(PLANTAGE_0!=1)) {PLANTAGE_0=1;}
while((currentMillis-MemMillis)<38){
currentMillis=millis();
}
MemMillis=currentMillis;
Tiny_Flip_TBIKE(1);
TIME_TRACK();
if (ENDMAP==1) {if (ACCEL<=1) {End_Line_Win_sound();Pause1();End_game=0;goto NEXT_LEVEL;}}
if (End_game==1) {if (ACCEL<=1) {Pause1();goto New_Games;}}
GAME_PLAY();
}}
void Pause1(void){
_delay_ms(2000);
}


uint8_t GAME_PLAY(void){
if (Live_TBIKE==-1) {End_game=1;}
return 0;
}

void TIME_TRACK(void){
if (Time_bar_Interval_timer<Time_bar_Interval) {Time_bar_Interval_timer++;
}else{
  Time_bar_Interval_timer=0;
  if (Time_bar_value<34) {Time_bar_value++;}else{End_game=1;}
  }
}

void PLANTAGE(void){
if (ACCEL>1.4){ACCEL=ACCEL-0.10;}else{PLANTAGE_0=0;Wheel_up=1;Pause=1;goto EED;}
if (Wheel_up>0) {Wheel_up--;}else{Wheel_up=5;}
EED:;
}

void Higher_adj(uint8_t test){
switch(test){

  case(0):
  case(1):
  case(2):Higher_jump=0.24;break;
  case(3):
  case(4):
  case(5):Higher_jump=0.18;break;
  case(6):
  case(7):
  case(8):Higher_jump=0.046;break;
  default:  break;
}}

void dinamic_adj(void){
if (Jump_dinamic_duration!=0){
Not_Move=1;JUMP_ADJ();
}else{GRAVITY_ADJ();
CLIMB_ACTIVATE=0;
Not_Move=0;
}}

void GRAVITY_ADJ(void){
PLANCHER=pgm_read_byte(&STEP_BIKE_TRACKRUN[trackrun]);
if (trackrun==Trackrun_progress) {
if (BIKE_POSy<(PLANCHER-PLANCHER_add)) {BIKE_POSy=BIKE_POSy+gravity_expo;gravity_expo=gravity_expo+0.2;FREEAIR=1;}else{FREEAIR=0;Not_Turn=0;BIKE_POSy=PLANCHER-PLANCHER_add;gravity_expo=0;}
}
}

void JUMP_ADJ(void){
 if (Jump_dinamic_duration>=.24){ADD_PILL=ADD_PILL+Jump_dinamic_duration; Jump_dinamic_duration=Jump_dinamic_duration-Higher_jump;}else{ADD_PILL=0;Jump_dinamic_duration=0;}
 if (ADD_PILL>=1) {ADD_PILL=ADD_PILL-1;if (BIKE_POSy>0) {BIKE_POSy=BIKE_POSy-1;}}
}

void CheckCollision(void){
if (Trouver_Sprite_Collisionner()!=0) {
analise_minutieuse();Not_Move=1;}
}

uint8_t  Trouver_Sprite_Collisionner(void){
#define xBike 30
#define yBike BIKE_POSy+12
for(uint8_t t=0;t<NUMofSPRITE;t++){
if (sprite_TBIKE[t].ACTIVE!=0) {
if ((sprite_TBIKE[t].xPOS>xBike)||((sprite_TBIKE[t].xPOS+RECUPE_X_SPRITE(sprite_TBIKE[NoSprite].TypeofSprite))<xBike)||(trackrun>sprite_TBIKE[t].Y_END)||(trackrun<sprite_TBIKE[t].Y_START)) {goto TIEND;}
NoSprite=t;
goto Fin; 
TIEND:;
}}
return 0;
Fin:;
return 1;
}

void analise_minutieuse(void){
switch(sprite_TBIKE[NoSprite].TypeofSprite){
   case(1):CLIMB_ADJ0();break;
   case(2):CLIMB_ADJ1();break;
   case(3):if ((ACCEL>2)&&(FREEAIR==0)){ACCEL=ACCEL-0.20;}break;
   case(4):ENDMAP=1;Sound(200,4);break;
   case(5):break;
   case(6):CLIMB_ADJ2();break;
   case(7):CLIMB_ADJ3();break;
   case(8):if (FREEAIR==0) {ADD_LIVE(NoSprite);}break;
   default:break;
}}

void ADD_LIVE(uint8_t SPR){
sprite_TBIKE[SPR].ACTIVE=0;
if (Live_TBIKE<3) {Live_TBIKE++;}
Sound(60,4);
Sound(200,4);
Sound(120,4);
}

void CLIMB_ADJ3(void){
#define POS_JUMP (30-sprite_TBIKE[NoSprite].xPOS)
if ((POS_JUMP==0)&&(FREEAIR==0)) {PLANTAGE_0=1;}
}

void Break_Gravity(void){
if (POS_JUMP==0) {
gravity_expo=0;
if ((FREEAIR==0)&&(Wheel_up==1)) {if (ACCEL>5) {ACCEL=ACCEL-2;}}} 
}

void CLIMB_ADJ0(void){
#define POS_JUMP (30-sprite_TBIKE[NoSprite].xPOS)
Break_Gravity();
Wheel_up=3;
  if ((POS_JUMP)<11) {Not_Turn=1;CLIMB_ACTIVATE=1;PLANCHER_add++;}
  else if((POS_JUMP)>15) {PLANCHER_add--;}
}

void CLIMB_ADJ1(void){
#define POS_JUMP (30-sprite_TBIKE[NoSprite].xPOS)
Break_Gravity();
Wheel_up=3;
  if ((POS_JUMP)<5) {Not_Turn=1;CLIMB_ACTIVATE=1;PLANCHER_add++;}
  else if((POS_JUMP)>9) {PLANCHER_add--;}
}

void CLIMB_ADJ2(void){
#define POS_JUMP (30-sprite_TBIKE[NoSprite].xPOS)
Break_Gravity();
Wheel_up=3;
  if ((POS_JUMP)<9) {Not_Turn=1;CLIMB_ACTIVATE=1;PLANCHER_add++;}
  else{PLANCHER_add=0;}
}

uint8_t RECUPE_X_SPRITE(uint8_t Type_Sprite){
return pgm_read_byte(&DIM_SPRITE[((Type_Sprite-1)*2)]);
}

uint8_t RECUPE_Y_SPRITE(uint8_t Type_Sprite){
return pgm_read_byte(&DIM_SPRITE[(((Type_Sprite-1)*2)+1)]);
}

uint8_t CHECK_SPEED_ADJ(float REC_INT){
uint8_t ret=0;
while(1){
if (REC_INT>1) {ret++;REC_INT=REC_INT-1;}else{ Higher_adj(ret);return ret;}
}}

void TRACK_RUN_ADJ(void){
if (trackrun!=Trackrun_progress) {
 if ((trackrun<Trackrun_progress)&&(TRANSITION_TRACK==0)){TRANSITION_TRACK=5;}
 if ((trackrun>Trackrun_progress)&&(TRANSITION_TRACK==0)){TRANSITION_TRACK=-5;}
  
 if (TRANSITION_TRACK>0){TRANSITION_TRACK--;animBike=8;BIKE_POSy++;}
 else if (TRANSITION_TRACK<0){TRANSITION_TRACK++;animBike=7;BIKE_POSy--;}
if (TRANSITION_TRACK==0) {TRIG_OK=0;trackrun=Trackrun_progress;}}
}

uint8_t RECUPE_MAP_BYTE(const uint8_t *Level){
return pgm_read_byte(&Level[MAP_POS]);
}

uint8_t SPLIT_MAP_BYTE(uint8_t BYTE,uint8_t L0_R1){
  switch(L0_R1){
    case(0):return (BYTE>>6);break;
    case(1):return (BYTE>>4)&0b00000011;break;
    case(2):return BYTE&0b00001111;break;
    default:break;
  }
  return 0;
}

void RefreshPosSprite(void){
for(uint8_t t=0;t<NUMofSPRITE;t++){
if (sprite_TBIKE[t].ACTIVE!=0) {
if (sprite_TBIKE[t].xPOS<=-26) {sprite_TBIKE[t].ACTIVE=0;}else{sprite_TBIKE[t].xPOS--;}
}}
CheckCollision();
}

void INCREMENTE_SCROLL(void){
if (VAR_SCROLL3==0) {VAR_SCROLL3=1;
if (VAR_SCROLL2==0) {VAR_SCROLL2=1;
if (VAR_SCROLL1==0) {VAR_SCROLL1=1;}else{VAR_SCROLL1=0;}}else{VAR_SCROLL2=0;}}else{VAR_SCROLL3=0;}
if (VAR_SCROLL3==1) {VAR_SCROLL3=2;if (Progress_bar_Interval_timer<Progress_bar_Interval) {Progress_bar_Interval_timer++;}else{Progress_bar_Interval_timer=0;if (Progress_bar_value<32) {Progress_bar_value++;}}
if (RENEW_SPRITE_TBIKE<64) {RENEW_SPRITE_TBIKE++;}else{CREATE_NEWSPRITE();}
  if (Jump_dinamic_duration!=0){Not_Move=1;JUMP_ADJ();}  if (CLIMB_ACTIVATE) {CLIMB_ACTIVATE=0;Jump_dinamic_duration=1;}
RefreshPosSprite();if (DScroll3<9) {DScroll3++;}else{DScroll3=0;}}
if (VAR_SCROLL2==1) {VAR_SCROLL2=2;if (DScroll2<14) {DScroll2++;}else{DScroll2=0;}}
if (VAR_SCROLL1==1) {VAR_SCROLL1=2;if (DScroll1<7) {DScroll1++;}else{DScroll1=0;}}
}

uint8_t CREATE_NEWSPRITE(void){
uint8_t type=SPLIT_MAP_BYTE(RECUPE_MAP_BYTE(Level0),2);
uint8_t recupe_y_pos0=SPLIT_MAP_BYTE(RECUPE_MAP_BYTE(Level0),0);
uint8_t recupe_y_pos=(recupe_y_pos0*5);
for(uint8_t t=0;t<NUMofSPRITE;t++){
if (sprite_TBIKE[t].ACTIVE==0) {
sprite_TBIKE[t].xPOS=127;
sprite_TBIKE[t].ACTIVE=1;
sprite_TBIKE[t].Y_START=recupe_y_pos0;
sprite_TBIKE[t].Y_END=SPLIT_MAP_BYTE(RECUPE_MAP_BYTE(Level0),1);
 switch(type){
case(0):sprite_TBIKE[t].TypeofSprite=1;sprite_TBIKE[t].yPOS=20;break;
case(1):sprite_TBIKE[t].TypeofSprite=2;sprite_TBIKE[t].yPOS=26;break;
case(2):sprite_TBIKE[t].TypeofSprite=3;sprite_TBIKE[t].yPOS=recupe_y_pos+32;break;
case(3):sprite_TBIKE[t].TypeofSprite=4;sprite_TBIKE[t].yPOS=32;break; 
case(4):sprite_TBIKE[t].TypeofSprite=5;sprite_TBIKE[t].yPOS=32;break; 
case(5):sprite_TBIKE[t].TypeofSprite=6;sprite_TBIKE[t].yPOS=recupe_y_pos+22;break;
case(6):sprite_TBIKE[t].TypeofSprite=7;sprite_TBIKE[t].yPOS=recupe_y_pos+34;break;
case(7):sprite_TBIKE[t].TypeofSprite=8;sprite_TBIKE[t].yPOS=recupe_y_pos+30;break;
  default:sprite_TBIKE[t].TypeofSprite=16;break;
 }
RENEW_SPRITE_TBIKE=0;
if (type==14) {ReWind=1;}
if ((type!=15)&&(ReWind!=1)) {MAP_POS++;}
return 0;
}} 
return 0;
}

uint8_t  BACKGROUND(uint8_t xPASS,uint8_t yPASS){
switch(yPASS){
  case(0):return FOUL1();break;
  case(1):return GRADIN23(yPASS);break;
  case(2):return GRADIN23(yPASS);break;
  case(3):return TIRE4();break;
  case(4):return ROAD567(yPASS);break;
  case(5):return ROAD567(yPASS);break;
  case(6):return ROAD567(yPASS);break;
  case(7):return (0xff-TABLEAU8(xPASS));break;
  default:return 0x00;break;
}}

uint8_t TOPBACK(uint8_t xPASS,uint8_t yPASS){
return pgm_read_byte(&TOP_BACK[xPASS+(yPASS*128)]);
}

uint8_t FOUL1(void){
if (BScroll1<7) {BScroll1++;}else{BScroll1=0;}
return pgm_read_byte(&FOUL[BScroll1]);  
}
uint8_t GRADIN23(uint8_t yPASS){
uint8_t MUL=0;
if (yPASS==2) {return 0;MUL=8;}
if (BScroll1<7) {BScroll1++;}else{BScroll1=0;}
return pgm_read_byte(&GRADIN[BScroll1+MUL]);  
}

uint8_t TIRE4(void){
if (BScroll2<14) {BScroll2++;}else{BScroll2=0;}
return pgm_read_byte(&TIRE[BScroll2]);  
}

uint8_t ROAD567(uint8_t yPASS){
uint8_t MUL=0;
MUL=(yPASS==5)?10:MUL;
MUL=(yPASS==6)?20:MUL;
if (BScroll3<9) {BScroll3++;}else{BScroll3=0;}
return pgm_read_byte(&ROAD[BScroll3+MUL]);  
}

uint8_t TABLEAU8(uint8_t xPASS){
return ((0xff-pgm_read_byte(&DISPLAY8[xPASS]))|Return_live(xPASS)|Return_time(xPASS)|(Return_Progress(xPASS)));  
}

uint8_t Return_live(uint8_t xPASS){
 uint8_t Start_black=26;
 switch(Live_TBIKE){
 case -1:
 case 0:Start_black=4;break;
 case 1:Start_black=11;break;
 case 2:Start_black=19;break;
 case 3:Start_black=26;break;
 default:break;
 }
 if (xPASS<Start_black) {return 0x00;}
 if (xPASS>26) {return 0x00;}
 return 0b01111110;
}

uint8_t Return_time(uint8_t xPASS){
if ((xPASS>=(40+(35-Time_bar_value)))&&(xPASS<=75)) {return 0b00111100;}
return 0x00;   
}

uint8_t Return_Progress(uint8_t xPASS){
if (xPASS==(Progress_bar_value+90)) {return 0b00011000;}
return 0x00;   
}

void AdjustVarScroll(void){
BScroll1=(DScroll1);
BScroll2=(DScroll2);
BScroll3=(DScroll3);
}

uint8_t Sprite2PAINTinBLACK;

uint8_t Recupe(uint8_t xPASS,uint8_t yPASS){
Sprite2PAINTinBLACK=254;
return (BACKGROUND(xPASS,yPASS)
|BIKE_SPRITE(xPASS,yPASS)
|BLITZ_SPRITE_MAP(xPASS,yPASS));
}

uint8_t BLITZ_SPRITE_MAP(uint8_t xPASS,uint8_t yPASS){
uint8_t SPRITERECUPE=0;
if ((yPASS<2)||(yPASS>6)) {goto Fin;}
for(uint8_t t=0;t<NUMofSPRITE;t++){
if (sprite_TBIKE[t].ACTIVE!=0) {
switch(sprite_TBIKE[t].TypeofSprite){
case(1):SPRITERECUPE=BIGSTEP_SPRITE(t,xPASS,yPASS);break;
case(2):SPRITERECUPE=MINISTEP_SPRITE(t,xPASS,yPASS);break;
case(3):SPRITERECUPE=HUILE_SPRITE(t,xPASS,yPASS);break;
case(4):SPRITERECUPE=LINE_SPRITE(t,xPASS,yPASS);break;
case(5):SPRITERECUPE=START_SPRITE(t,xPASS,yPASS);break;
case(6):SPRITERECUPE=SPEED_SPRITE(t,xPASS,yPASS);break;
case(7):SPRITERECUPE=Plantage(t,xPASS,yPASS);break;
case(8):SPRITERECUPE=NEW_LIVE(t,xPASS,yPASS);break;
default:SPRITERECUPE=0x00;break;
}
if ((SPRITERECUPE!=0)||(Sprite2PAINTinBLACK!=254)) {Sprite2PAINTinBLACK=t;goto Fin;}
}} 
Fin:;
return SPRITERECUPE;
}

uint8_t BIGSTEP_SPRITE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS){
return blitzSprite(sprite_TBIKE[numSprite].xPOS,sprite_TBIKE[numSprite].yPOS,xPASS,yPASS,0,BigStepA);
}

uint8_t MINISTEP_SPRITE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS){
return blitzSprite(sprite_TBIKE[numSprite].xPOS,sprite_TBIKE[numSprite].yPOS,xPASS,yPASS,0,MinijumpA);
}

uint8_t Plantage(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS){
return blitzSprite(sprite_TBIKE[numSprite].xPOS,sprite_TBIKE[numSprite].yPOS,xPASS,yPASS,0,plantage);
}

uint8_t HUILE_SPRITE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS){
return blitzSprite(sprite_TBIKE[numSprite].xPOS,sprite_TBIKE[numSprite].yPOS,xPASS,yPASS,0,huile);
}

uint8_t LINE_SPRITE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS){
return blitzSprite(sprite_TBIKE[numSprite].xPOS,sprite_TBIKE[numSprite].yPOS,xPASS,yPASS,0,Line);
}

uint8_t START_SPRITE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS){
return blitzSprite(sprite_TBIKE[numSprite].xPOS,sprite_TBIKE[numSprite].yPOS,xPASS,yPASS,0,Start);
}

uint8_t SPEED_SPRITE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS){
return blitzSprite(sprite_TBIKE[numSprite].xPOS,sprite_TBIKE[numSprite].yPOS,xPASS,yPASS,0,Speed);
}

uint8_t NEW_LIVE(uint8_t numSprite,uint8_t xPASS,uint8_t yPASS){
return blitzSprite(sprite_TBIKE[numSprite].xPOS,sprite_TBIKE[numSprite].yPOS,xPASS,yPASS,0,New_Live);
}

uint8_t BIKE_SPRITE(uint8_t xPASS,uint8_t yPASS){
return blitzSprite(24,BIKE_POSy,xPASS,yPASS,animBike,bike1);
}


void Tiny_Flip_TBIKE(uint8_t MODE){
uint8_t y,x; 
for (y = 0; y < 8; y++){ 
AdjustVarScroll();   
for (x = 0; x < 128; x++){
  switch(MODE){
   case 0:arduboy.SPItransfer(Recupe(x,y));break;
   case 1:arduboy.SPItransfer(Recupe(x,y));break;
   case 2:arduboy.SPItransfer(blitzSprite(38,30,x,y,0,INTROPIC));break;
   case 3:arduboy.SPItransfer(Recupe(x,y));break;
   default:break;
}}}}

