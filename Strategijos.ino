bool pusiu_state = false;
unsigned long paiesku_buves = 0;
const long sukimosi_intervalas = 200;

bool push_state = false;
unsigned long judejimo_buves = 0;
int laukimo_laikas = 3000;
int judejimo_laikas = 100;

int intervalas_3kg = laukimo_laikas;

void paieska() {
  paiesku_buves = millis();

  while (true) {
    readSensors();
    if (millis() - paiesku_buves >= sukimosi_intervalas) {
      paiesku_buves = millis();
      if (pusiu_state == false) {
        pusiu_state = true;
        motor(80, -40);
      } else {
        pusiu_state = false;
        motor(-40, 80);
      }
    }
    if (RUN_STATE != 0) {
      break;
    }
  }
}

void Mega_sumo_taktika() {
  motor(SPEED_NORMAL, SPEED_NORMAL);
  delay(judejimo_laikas);
  motor(0, 0);
  delay(1);
  judejimo_buves = millis();

  bool reikia_atgal = false;
  int judejimo_skaicius = 1;

  while (true) {

    readSensors();

    if (RUN_STATE != 0) {
      break;
    }
    if (millis() - judejimo_buves >= intervalas_3kg) {
      judejimo_buves = millis();
      if (push_state == false) {
        push_state = true;
        intervalas_3kg = judejimo_laikas;
        if (judejimo_skaicius == 2) {
          motor(-SPEED_NORMAL, -SPEED_NORMAL);
          judejimo_skaicius = 0;
        }
        else {
          motor(SPEED_NORMAL, SPEED_NORMAL);
          judejimo_skaicius++;
        }
      }
      else {
        push_state = false;
        motor(0, 0);
        intervalas_3kg = laukimo_laikas;
      }
    }
  }

}

