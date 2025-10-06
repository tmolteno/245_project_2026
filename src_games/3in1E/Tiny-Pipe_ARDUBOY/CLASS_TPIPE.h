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

//DEFINE
#define NO_SPR 4

//VAR PUBLIC
uint8_t RND_COUNTER_TPIPE=0;


//Structure
typedef struct GamePlay_TPIPE{
uint8_t LIVES;
uint8_t FIRSTTIME;
uint8_t STATE;
uint8_t Level;
uint8_t TIMER_RENEW;
uint8_t LevelXspeed;
uint8_t TOTAL_TURTLE_LEVEL;
uint8_t SPEED_POP_TURTLE;
int8_t POWER;
uint8_t EARTQUAKE;
uint8_t EARTQUAKE_INVERT;
uint8_t NOMOVE_TIME;
uint8_t DIGIT1;
uint8_t DIGIT2;
}GamePlay_TPIPE;

//PROTOTYPE
uint8_t COLLISION_1VS1(int8_t x1,int8_t x2,int8_t y1,int8_t y2,int8_t sx1,int8_t sx2,int8_t sy1,int8_t sy2);
uint8_t FLOORS_VS_SPRITE(uint8_t Killed_,int8_t x1,int8_t x2,int8_t y1,int8_t y2);

class PASIVE_SPRITE_TPIPE{
private:
int8_t x;
int8_t y;
uint8_t frame;
uint8_t killed;
uint8_t active;
uint8_t width;
uint8_t height;
public:
void INIT(uint8_t Active_,int8_t X_,int8_t Y_){
x=X_;
y=Y_;
frame=0;
active=Active_;
killed=0;
width=7;
height=7;
}

uint8_t GET_K(void){return killed;}
void PUT_K(uint8_t K_){killed=K_;}
int8_t GET_X(void){return x;}
int8_t GET_Y(void){return y;}
uint8_t GET_W(void){return width;}
uint8_t GET_H(void){return height;}
uint8_t GET_F(void){return frame;}
void PUT_A(uint8_t F_){active=F_;}
uint8_t GET_A (void){return active;}
void PUT_F(uint8_t F_){frame=F_;}
void PUT_W(uint8_t Width_){width=Width_;}
void PUT_H(uint8_t height_){height=height_;}
void PUT_X(int8_t X_){x=X_;}
void PUT_Y(int8_t Y_){y=Y_;}
void ANIM(void){PUT_F((GET_F()<2)?(GET_F()+1):0);}
};

//Public Var
PASIVE_SPRITE_TPIPE Target;

class SPRITE_TPIPE:public PASIVE_SPRITE_TPIPE {
private:
int8_t xspeed;
int8_t xadd;
int8_t yspeed;
int8_t yadd;
uint8_t Anim_Direction;
uint8_t Cancel_Jump;
uint8_t NoMoveTimer;
public:
uint8_t GET_NOMOVETIMER(void){return NoMoveTimer;}
void PUT_NOMOVETIMER(uint8_t NoMoveTimer_){NoMoveTimer=NoMoveTimer_;}
int8_t GET_XSPEED(void){return xspeed;}
void PUT_XSPEED(int8_t XSPEED_){xspeed=XSPEED_;}
int8_t GET_YSPEED(void){return yspeed;}
void PUT_YSPEED(int8_t YSPEED_){yspeed=YSPEED_;}
uint8_t GET_AD(void){return Anim_Direction;}
void PUT_AD(uint8_t F_){Anim_Direction=F_;}

void DECEL(void){
if (xspeed<0){
xspeed++;
RunL();
}else if (xspeed>0){
xspeed--;
RunR(); 
}}

uint8_t GET_CJ(void){return Cancel_Jump;}
void PUT_CJ(uint8_t CJ_){Cancel_Jump=CJ_;}
void Reset_X_Speed(void){xspeed=0;xadd=0;}

void RunR(void){
xadd=abs(xspeed)+xadd;
if (xadd>10) {
xadd-=10;
if (GET_X()<114) {
PUT_X(GET_X()+1);
ANIM();
if (FLOORS_VS_SPRITE(GET_K(),GET_X()+1,GET_X()+GET_W()-1,GET_Y(),GET_Y()+GET_H())) {
PUT_X(GET_X()-1);
}}else{
PUT_X(8);;
}}}
  
void RunL(void){
xadd=abs(xspeed)+xadd;
if (xadd>10) {
xadd-=10;
if (GET_X()>8) {
PUT_X(GET_X()-1);
ANIM();
if (FLOORS_VS_SPRITE(GET_K(),GET_X()+1,GET_X()+GET_W()-1,GET_Y(),GET_Y()+GET_H())) {
PUT_X(GET_X()+1);
}}else{PUT_X(114);}}
}

void GRAVITY_REFRESH(void){
yspeed=(yspeed<30)?yspeed+2:30;
yadd=abs(yspeed)+yadd;}

void GRAVITY(uint8_t Main_){
if (GET_A()) {
GRAVITY_REFRESH();
if (yspeed>=0) {
while(yadd>10) {PUT_Y(GET_Y()+1);yadd-=10;}
while(1){
if (FLOORS_VS_SPRITE(GET_K(),GET_X()+1,GET_X()+GET_W()-1,GET_Y(),GET_Y()+GET_H())) {
PUT_Y(GET_Y()-1);
RESET_GRAVITY();
Cancel_Jump=(Cancel_Jump==1)?2:Cancel_Jump;
}else{if (GET_Y()>63) {PUT_A(0);}break;}}
}else{
while(yadd>10) {if (GET_Y()>-10) {PUT_Y(GET_Y()-1);yadd-=10;}else{RESET_GRAVITY();}}
if (FLOORS_VS_SPRITE(GET_K(),GET_X()+1,GET_X()+GET_W()-1,GET_Y(),GET_Y()+GET_H())) {
while(1){
if (FLOORS_VS_SPRITE(GET_K(),GET_X()+1,GET_X()+GET_W()-1,GET_Y(),GET_Y()+GET_H())) {
PUT_Y(GET_Y()+1);
RESET_GRAVITY();
}else{if ((Target.GET_A()==0)&&(Main_==1)){Target.INIT(1,GET_X(),GET_Y()-8);}break;}
}}}}}

void RESET_GRAVITY(void){
yadd=0;
yspeed=0;
}

void JUMP(void){
if (Cancel_Jump==0) {
Cancel_Jump=1; 
if (FLOORS_VS_SPRITE(GET_K(),GET_X()+1,GET_X()+GET_W()-1,GET_Y()+1,GET_Y()+GET_H()+1)) {
yspeed=-25;
yadd=10;
}}}

void RIGHTMOVE(void){
PUT_AD(0);
if (xspeed<0){
DECEL();
RunL();
}else{
xspeed=(xspeed<10)?xspeed+1:xspeed;
RunR();
}}
  
void LEFTMOVE(void){
PUT_AD(3);
  if (xspeed>0){
DECEL();
RunR();
}else{
xspeed=(xspeed>-10)?xspeed-1:xspeed;
RunL();
}}

void DIRECT_R_MOVE(void){
if (GET_X()>109){
if (GET_Y()>52){
PUT_X(24);PUT_Y(-3);
RESET_GRAVITY();
goto EnD_;}
}
PUT_AD(0);
RunR(); 
EnD_:;
}
  
void DIRECT_L_MOVE(void){
if (GET_X()<12){
if (GET_Y()>52){
PUT_X(97);PUT_Y(-3);
RESET_GRAVITY();
goto EnD_;}
}
PUT_AD(3);
RunL();  
EnD_:;
}

void AUTO_MOVE(void){
if (xspeed>0) {DIRECT_R_MOVE();}else{DIRECT_L_MOVE();}
}
};

uint8_t COLLISION_SIMPLIFIED(SPRITE_TPIPE *Main_,SPRITE_TPIPE *Other_){
return COLLISION_1VS1(Main_->GET_X(),Main_->GET_X()+Main_->GET_W(),Main_->GET_Y(),Main_->GET_Y()+Main_->GET_H(),
Other_->GET_X()+1,Other_->GET_X()+Other_->GET_W()-2,Other_->GET_Y()+5,Other_->GET_Y()+Other_->GET_H());
}

uint8_t COLLISION_1VS1(int8_t x1,int8_t x2,int8_t y1,int8_t y2,int8_t sx1,int8_t sx2,int8_t sy1,int8_t sy2){
if (x1>sx2) return 0;
if (x2<sx1) return 0;
if (y1>sy2) return 0;
if (y2<sy1) return 0;
return 1;
}

uint8_t FLOORS_VS_SPRITE(uint8_t Killed_,int8_t x1,int8_t x2,int8_t y1,int8_t y2){
if (!Killed_) {
if (COLLISION_1VS1(x1,x2,y1,y2,0,58,15,17)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,69,127,15,17)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,38,89,29,31)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,0,28,34,36)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,99,127,34,36)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,0,48,46,48)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,79,127,46,48)) return 1;
if (COLLISION_1VS1(x1,x2,y1,y2,0,127,61,61)) return 1;
}
return 0;
}

void RND_MIXER_TPIPE(void){
RND_COUNTER_TPIPE=(RND_COUNTER_TPIPE<29)?RND_COUNTER_TPIPE+1:0;
}

uint8_t PSEUDO_RND_TPIPE(void){
RND_MIXER_TPIPE();
return pgm_read_byte(&RND_TPIPE[RND_COUNTER_TPIPE]);
}
