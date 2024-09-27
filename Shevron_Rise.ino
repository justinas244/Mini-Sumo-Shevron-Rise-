#include <IRremote.h>
#include <EEPROM.h>
int RECV_PIN = 15; // įvesties pinas, kuriuo priimami IR signalai
IRrecv irrecv(RECV_PIN);
decode_results results;

const byte program_led = 17;  //RX LED
const byte sensor_led = 30;  //TX LED

//////////////////// JSUMO pultelio kodai //////////////////////////
int kanalArray[11][4] = {
    { 2559, 2438, 2439, 2440 }, // Kanalo parinkimas, Ready, Start, Stop kodai
    { 2687, 2566, 2567, 2568 },
    { 2815, 2694, 2695, 2696 },
    { 2943, 2822, 2823, 2824 },
    { 3071, 2950, 2951, 2952 },
    { 3199, 3078, 3079, 3080 },
    { 3327, 3206, 3207, 3208 },
    { 3455, 3334, 3335, 3336 },
    { 3583, 3462, 3463, 3464 },
    { 3711, 3590, 3591, 3592 },
    { 3839, 3718, 3719, 3720 }
};


int pasirinktas_kanalas = 0;
bool paleista = false;
bool sustabdyta = false;

////////////////////////////////////////////////////////////////////

//LINE begin
const byte LINE_LEFT = A3;
const byte LINE_RIGHT = A2;
//LINE end


//FRONT SENSOR begin
const byte LEFT = 5;
const byte FRONT_LEFT = 4;
const byte FRONT_RIGHT = 3;
const byte RIGHT = 2;
//FRONT SENSOR end

//START MODULE begin
const byte START_MODULE = 14;
//START MODULE end

//SENSORS SUMS
byte line_sum = 0;
byte sharp_sum = 0;

//MOTOR CONTROL VARIABLES
double INTEGRAL = 0;
int V = 0;

//MOTORS begin
int motor_R = 10;    //pwm
int motor_L = 9;   //pwm

int motor_r = 16;  //dir
int motor_l = 8;  // dir
//MOTORS end

//VALDYMO KINTAMIEJI begin
const byte SPEED_STOP = 0;
 byte SPEED_NORMAL = 80;//80
 byte Default_speed = SPEED_NORMAL;//80
const byte SPEED_ATTACK2 = 180;//180
const byte SPEED_ATTACK = 255;//255

const byte SPEED_GO_BACK_LINE = 255;//255
const byte SPEED_GO_BACK_LINE_TURN = 230;//200
const byte SPEED_GO_BACK_LINE_TURN2 = 230;//200

/// LAIKAI ///////////////////////
const unsigned short TIME_FOR_GO_BACK = 80; //80
const unsigned short TIME_FOR_TURNING = 120;
const unsigned short TIME_FOR_TURNING_BOTH = 120;
unsigned short TURNING_TIME = TIME_FOR_TURNING;
unsigned long LINE_TIME = 2;

//STATES
byte RUN_STATE = 0;
byte LINE_AVOID_STATE = 0;
byte LAST_SEEN = 0;
byte LAST_STATE = 0;
byte STRATEGY_STATE = 0;

//STATE TIMERS
unsigned long LAST_SEEN_TIME = 0;
unsigned long STRATEGY_START_TIME = 0;
unsigned long TIME_FOR_STRATEGY = 0;

unsigned long TIME_FOR_ATACK_SET = 0;
unsigned long TIME = 0;

bool abu_mate = false;
int selectedProgram = -1;

unsigned long valdymo[][3] = {  /// pultelio programu kodai hex formatu
  {0x530DB67C, 0x17B4A228, 0xA90},     // Programu pultelio Start komanda
  {0xCD76C8CB, 0x7BA067FF, 0x5D0}      // Programu pultelio Start komanda
};

unsigned long programos_check[3] = {0x54E680D6, 0x903F952A, 0xA70}; 

unsigned long sensoriu_isjungimas[3] = {0xA74647DD, 0xFDAC0152, 0xDD0}; 
unsigned long sensoriu_ijungimas[3] = {0xD41A2FD5, 0x8243CF09, 0xFD0}; 

byte naudoti_pulta = 0;
bool naudoti_sensorius = true;


byte sukimo_state = 0;    // busena kuri nusako kad suktusi bet skaitytu pilnai sensorius(jei ijungti jie)
unsigned long greicio_perjungimo_interval = 5000;
unsigned long bejudesio = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LINE_LEFT, INPUT);
  pinMode(LINE_RIGHT, INPUT);

  pinMode(LEFT, INPUT_PULLUP);
  pinMode(FRONT_LEFT, INPUT_PULLUP);
  pinMode(FRONT_RIGHT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);

  pinMode(START_MODULE, INPUT);
  pinMode(program_led,OUTPUT);
  pinMode(sensor_led,OUTPUT);

    pinMode(motor_R,OUTPUT);
    pinMode(motor_L,OUTPUT);
    pinMode(motor_r,OUTPUT);
    pinMode(motor_l,OUTPUT);
    
    irrecv.enableIRIn(); // įjungti priėmimą
    digitalWrite(program_led,HIGH);
    digitalWrite(sensor_led,HIGH);
    delay(1000);

     pasirinktas_kanalas = EEPROM.read(0);
    
    while(true){
     jsumo_pultelis();
     pultelio_programos();
     if(digitalRead(START_MODULE)){ naudoti_pulta = 0; break; }
     sensor_check();
     
     if (irrecv.decode(&results)) {
        unsigned long kodas = results.value;
        int jsumo_kodas = IrReceiver.decodedIRData.decodedRawData;
        
        Serial.println(kodas, HEX);
        if(kodas == valdymo[0][0] || kodas == valdymo[0][1] || kodas == valdymo[0][2] || jsumo_kodas == kanalArray[pasirinktas_kanalas][2]){
          pinMode(START_MODULE, OUTPUT);
          //delay(5000);
          delay(1);
          digitalWrite(START_MODULE, HIGH);
          naudoti_pulta = 1;
          break;
        }
        
        delay(1);
      }
      
    }
    
  motor(0,0);
  delay(2);

  //STRATEGY_STATE = 0;
  STRATEGY_START_TIME = millis();
  bejudesio = millis();
}

void loop() {
  if (millis() - LINE_TIME > TURNING_TIME) LINE_AVOID_STATE = 0;
  if (millis() - STRATEGY_START_TIME > TIME_FOR_STRATEGY) { STRATEGY_STATE = 0; sukimo_state = 0;}
  
  if (millis() - bejudesio >= greicio_perjungimo_interval){
    SPEED_NORMAL = 255;}
  else{
      SPEED_NORMAL  = Default_speed;
  }

   readSensors();
   switch (naudoti_pulta){
     case 1:
     stop_skaitymas();
     break;
   }
  
  switch (RUN_STATE) {
    case 0: switch(LINE_AVOID_STATE) {
              case 0: switch(STRATEGY_STATE) {
                        case 0: 
                                switch(sukimo_state){
                                   case  0:  motor(SPEED_NORMAL,  SPEED_NORMAL); break;
                                   case  1: motor(SPEED_ATTACK,  -SPEED_ATTACK); break;
                                   case  2: motor(-SPEED_ATTACK,  SPEED_ATTACK); break;
                                }
                             break;
                        
                         case 1: //TIME_FOR_STRATEGY = 35;
                                 motor(-SPEED_ATTACK, SPEED_ATTACK);
                                 delay(65);//70
                                 STRATEGY_STATE = 21;
                                //break;
                                
                        case 21: TIME_FOR_STRATEGY = 900;
                                 motor(SPEED_ATTACK, 115);
                                 break;
                                
                        case 2:  TIME_FOR_STRATEGY = 250;
                                 motor(SPEED_ATTACK, SPEED_ATTACK);
                                 break;
                                
                        case 3: //TIME_FOR_STRATEGY = 35;
                                motor(SPEED_ATTACK, -SPEED_ATTACK);
                                delay(65);//70
                                STRATEGY_STATE = 31;
                                //break;
                                
                       case 31: TIME_FOR_STRATEGY = 900;
                                 motor(110,SPEED_ATTACK);
                                 break;
                                 
                       case 4: motor(-SPEED_ATTACK, SPEED_ATTACK);
                               delay(45);
                               break;

                       case 5: motor(-SPEED_ATTACK, SPEED_ATTACK);
                               delay(50);
                               motor(0,0);
                               delay(1);
                               paieska();
                               break;
                               
                       case 6: motor(SPEED_ATTACK, -SPEED_ATTACK);
                               delay(45);
                               break;
                               
                       case 7: motor(-SPEED_ATTACK, SPEED_ATTACK);
                               delay(40);
                               STRATEGY_STATE = 71;
                               
                       case 71: motor(SPEED_ATTACK, SPEED_ATTACK);
                                delay(180);
                                motor(SPEED_ATTACK, -SPEED_ATTACK);
                                delay(120);
                                STRATEGY_STATE = 72;
                           
                       case 72: motor(SPEED_ATTACK, SPEED_ATTACK);
                                delay(200);
                                break;
                                
                       case 8: Mega_sumo_taktika();
                               break;

                       case 9: motor(SPEED_ATTACK, -SPEED_ATTACK);
                               delay(40);
                               STRATEGY_STATE = 91;
                               
                       case 91: motor(SPEED_ATTACK, SPEED_ATTACK);
                                delay(180);
                                motor(-SPEED_ATTACK, SPEED_ATTACK);
                                delay(120);
                                STRATEGY_STATE = 92;
                           
                       case 92: motor(SPEED_ATTACK, SPEED_ATTACK);
                                delay(200);
                                break;

                       case 10: motor(SPEED_ATTACK, SPEED_ATTACK);
                                delay(400);
                                break; 
                                          
                      } break;
              case 1: motor(SPEED_GO_BACK_LINE_TURN, -SPEED_GO_BACK_LINE_TURN2);  bejudesio=millis();  break;
              case 2: motor(-SPEED_GO_BACK_LINE_TURN2, SPEED_GO_BACK_LINE_TURN);  bejudesio=millis(); break;
              case 3: switch(LAST_SEEN) {
                        case 0: motor(SPEED_GO_BACK_LINE_TURN, -SPEED_GO_BACK_LINE_TURN2); bejudesio=millis();  break;
                        case 1: motor(-SPEED_GO_BACK_LINE_TURN2, SPEED_GO_BACK_LINE_TURN); bejudesio=millis();  break;
                        case 2: motor(SPEED_GO_BACK_LINE_TURN, -SPEED_GO_BACK_LINE_TURN2); bejudesio=millis();  break;
                      } break;   
            } 
            INTEGRAL = 0;
            break;
                
    case 1: motor(-SPEED_GO_BACK_LINE, -SPEED_GO_BACK_LINE);
            delay(TIME_FOR_GO_BACK);
            LINE_AVOID_STATE = 1;
            TIME_FOR_STRATEGY = 0;
            TURNING_TIME = TIME_FOR_TURNING;
            LINE_TIME = millis();
            sukimo_state = 0;
            bejudesio=millis();
            break;
            
    case 2: motor(-SPEED_GO_BACK_LINE, -SPEED_GO_BACK_LINE);
            delay(TIME_FOR_GO_BACK);
            LINE_AVOID_STATE = 2;
            TIME_FOR_STRATEGY = 0;
            TURNING_TIME = TIME_FOR_TURNING;
            LINE_TIME = millis();
            sukimo_state = 0;
            bejudesio=millis();
            break;
            
    case 3: motor(-SPEED_GO_BACK_LINE, -SPEED_GO_BACK_LINE);
            delay(TIME_FOR_GO_BACK);
            LINE_AVOID_STATE = 3;
            TURNING_TIME = TIME_FOR_TURNING_BOTH;
            TIME_FOR_STRATEGY = 0;
            LINE_TIME = millis();
            sukimo_state = 0;
            bejudesio=millis();
            break;
            
   case 4: TIME_FOR_STRATEGY = 500;
           motor(-SPEED_ATTACK,SPEED_ATTACK );
           sukimo_state = 2;
           bejudesio=millis();
           break;
           
  case 5:  TIME_FOR_STRATEGY = 10;
           motor(-SPEED_ATTACK,SPEED_ATTACK );
           sukimo_state = 2;
           break;

  case 6: motor(SPEED_ATTACK,SPEED_ATTACK);
          sukimo_state = 0;
          //bejudesio=millis();
          break;
          
  case 7: TIME_FOR_STRATEGY = 10;
          motor(SPEED_ATTACK,-SPEED_ATTACK );
          sukimo_state = 1;
          break;
          
  case 8: TIME_FOR_STRATEGY = 500;
          motor(SPEED_ATTACK,-SPEED_ATTACK );
          sukimo_state = 1;
          bejudesio=millis();
          break;
          
  }
   
}

void readSensors() {
  line_sum = (PINF >> 2) & 0x0C;
  sharp_sum = (PIND & 0b10011) | (PINC & 0b1000000);
 //Serial.println(sharp_sum, BIN);
   
   switch(line_sum){
     case 0b1000: RUN_STATE = 3; break;
     case 0b0100: RUN_STATE = 1; break;
     case 0b1100: RUN_STATE = 2; break;
     case 0b0000: 
     switch(naudoti_sensorius){
        case 1:
            switch(sharp_sum){
               //case 0b1010011: RUN_STATE = 0; break;   //  niekas nemato
               case 0b0010011: RUN_STATE = 4; abu_mate = false; break;   //  tik kaire mato
               case 0b1000011: if(abu_mate == false){RUN_STATE = 5;} else{RUN_STATE = 6;} break;   //  tik kaire  priekis mato
               
               case 0b0000011: RUN_STATE = 4; abu_mate = false; break;   //  tik kaire ir priekis
               case 0b1000010: RUN_STATE = 6; abu_mate = true;  break;   //  abu priekiniai mato
               case 0b1010000: RUN_STATE = 7; abu_mate = false; break;    //  tik desine ir desine priekis  mato
               
               case 0b1010010: if(abu_mate == false){RUN_STATE = 7;} else{RUN_STATE = 6;} break; //  tik desine  priekis mato
               case 0b1010001: RUN_STATE = 8; abu_mate = false; break;    //  tik desine  mato
      
               case 0b0010010: RUN_STATE = 4; break;   // Mato desine priekis ir kaire sonas. Turim reikalu pries veliavos naudojima
               case 0b1000001: RUN_STATE = 8; break;   // Mato kaire priekis ir desine sonas. Turim reikalu pries veliavos naudojima
      
               case 0b0000010: RUN_STATE = 6; abu_mate = true;  break;   //  abu priekiniai mato ir kaire taip pat
               case 0b1000000: RUN_STATE = 6; abu_mate = true;  break;   //  abu priekiniai mato ir kdesine taip pat
      
               default:  RUN_STATE = 0;   break; // kai niekas nemato arba mato nenormaliai
            }   
        break;
        case 0: RUN_STATE = 0;   break;
       }
   }

  delay(1);

}

void motor(int left, int right) {
  right = right*(-1);
  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);

  if (right >= 0) {
    digitalWrite(motor_r, LOW);
    analogWrite(motor_R, right);
  }
  else {
    digitalWrite(motor_r, HIGH);
    analogWrite(motor_R, 255+right);
  }

  if (left >= 0) {
    digitalWrite(motor_l, LOW);
    analogWrite(motor_L, left);
  }
  else {
    digitalWrite(motor_l, HIGH);
    analogWrite(motor_L, 255+left);
  }
}

void sensor_check(){
    line_sum = (PINF >> 2) & 0x0C;
    sharp_sum = (PIND & 0b10011) | (PINC & 0b1000000);
    
  if( line_sum != 0b0000 ||  sharp_sum != 0b1010011){
    digitalWrite(sensor_led, LOW);
  }
  else{
    digitalWrite(sensor_led, HIGH);
  }
}

