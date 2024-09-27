
void stop_skaitymas() {
  jsumo_stop();
  if (irrecv.decode(&results)) {
    unsigned long kodass = results.value;
    int jsumo_kodas = IrReceiver.decodedIRData.decodedRawData;
    if(kodass == valdymo[1][0] || kodass == valdymo[1][1] || kodass == valdymo[1][2] || jsumo_kodas == kanalArray[pasirinktas_kanalas][3]) {
      digitalWrite(START_MODULE, LOW);
      delay(5000);
      while(true){}
    }
    
    irrecv.resume(); // persikrauti įvedimo buferį
  }
}
void jsumo_stop(){
  
  if (IrReceiver.decode()) {
   int jsumo_kodas = IrReceiver.decodedIRData.decodedRawData;
   if(jsumo_kodas == kanalArray[pasirinktas_kanalas][3]) {
      digitalWrite(START_MODULE, LOW);
      delay(5000);
      while(true){}
    }
  }
    
    irrecv.resume();
  
}


unsigned long programCodes[][3] = {  /// pultelio programu kodai hex formatu
  {0xC2D091BF, 0x56EF334B, 0x10 },  // 1 programa
  {0x528A5222, 0x17313DCE, 0x810},  // 2 programa
  {0xE4400B14, 0x50216988, 0x410},  // 3 programa
  {0x321579D4, 0x6D6E8E28, 0xC10},  // 4 programa
  {0x33157B67, 0x6E6E8FBB, 0x210},  // 5 programa
  {0xA41CE90D, 0xDF75FD61, 0xA10},  // 6 programa
  {0xFE084450, 0x69E9A2C4, 0x610},  // 7 programa
  {0x1DDBEF8C, 0x593503E0, 0xE10},  // 8 programa
  {0x1ADBEAD5, 0x5634FF29, 0x110},  // 9 programa
  {0x55EF31B6, 0xC1D0902A, 0x910},  // 10 programa
  // čia galite įtraukti kitų programų kodus
};

void pultelio_programos() {

  if (irrecv.decode(&results)) {
    unsigned long codeValue = results.value;

    // Tikriname, ar gautas kodas atitinka kuria nors programą
    for (int i = 0; i < sizeof(programCodes) / sizeof(programCodes[0]); ++i) {
      if (codeValue == programCodes[i][0] || codeValue == programCodes[i][1] || codeValue == programCodes[i][2]) {
        selectedProgram = i;
         //STRATEGY_STATE =  selectedProgram + 1;
        break;
      }
    }


    if (selectedProgram != -1) {
      // Jei rastas atitinkantis kodas, atliekame veiksmus pagal pasirinktą programą
      digitalWrite(program_led,LOW);
      switch (selectedProgram) {
        case 0:
          Serial.println("Program 1 veiksmai");
          STRATEGY_STATE = 1;
          break;
        case 1:
          Serial.println("Program 2 veiksmai");
          STRATEGY_STATE = 2;
          break;
        case 2:
          Serial.println("Program 3 veiksmai");
          STRATEGY_STATE = 3;
          break;
        case 3:
          Serial.println("Program 4 veiksmai");
          STRATEGY_STATE = 4;
          break;
        case 4:
          Serial.println("Program 5 veiksmai");
          STRATEGY_STATE = 5;
          break;
        case 5:
          Serial.println("Program 6 veiksmai");
          STRATEGY_STATE = 6;
          break;
        case 6:
          Serial.println("Program 7 veiksmai");
           STRATEGY_STATE = 7;
          break;
        case 7:
          Serial.println("Program 8 veiksmai");
          STRATEGY_STATE = 8;
          break;
        case 8:
          Serial.println("Program 9 veiksmai");
          STRATEGY_STATE = 9;
          break;
        case 9:
          Serial.println("Program 10 veiksmai");
          STRATEGY_STATE = 10;
          break;
      }
    } else {
      Serial.println("Pradinis vaziavimas");
      STRATEGY_STATE = 0;
      digitalWrite(program_led,HIGH);
    }
    if(codeValue == sensoriu_isjungimas[0] || codeValue == sensoriu_isjungimas[1] || codeValue == sensoriu_isjungimas[2]){
      Serial.print("Sensoriu naudojimas : NIEKO NEMATO ");
      naudoti_sensorius = false;
    }
    
    if(codeValue == sensoriu_ijungimas[0] || codeValue == sensoriu_ijungimas[1] || codeValue == sensoriu_ijungimas[2]){
      Serial.print("Sensoriu naudojimas : VISI NEMATO ");
      naudoti_sensorius = true;
    }

    if(codeValue == programos_check[0] || codeValue == programos_check[1] || codeValue == programos_check[2]){
      Serial.print("Pasirinkta strategija: ");
      Serial.println(STRATEGY_STATE);
      if( STRATEGY_STATE != 0){
        for(int i = 0; i<STRATEGY_STATE; i++){
          digitalWrite(program_led,HIGH);
          if(digitalRead(START_MODULE)){ break; }
          delay(500);
          if(digitalRead(START_MODULE)){ break; }
          digitalWrite(program_led,LOW);
          delay(500);
         if(digitalRead(START_MODULE)){ break; }
        }
        //digitalWrite(program_led,LOW);
      }
    }
   
    irrecv.resume(); // persikrauti įvedimo buferį
  }

}

void jsumo_pultelis (){
  
   if (IrReceiver.decode()) {
    
    int kodas = IrReceiver.decodedIRData.decodedRawData;
    Serial.println(kodas);
    for(int i=0; i<=10; i++){
      if(kodas == kanalArray[i][0]){
        pasirinktas_kanalas = i;
        //EEPROM.write(0,pasirinktas_kanalas);
        EEPROM.update(0,pasirinktas_kanalas);

        for(int i=0; i<=5; i++){
        digitalWrite(program_led,HIGH);
        delay(200);
        digitalWrite(program_led,LOW);
        delay(200);
        }
      }
    }
    
    irrecv.resume();
  }
}


