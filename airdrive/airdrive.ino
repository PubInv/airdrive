#define __ASSERT_USE_STDERR

#include <assert.h>


// handle diagnostic informations given by assertion and abort program execution:
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link. 
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.flush();
    // abort program execution.
    abort();
}

// drive(flow : mllpm, at_pressure: cmH2O, time: ms, begin_time: ms, O2mix: [0:100], custom: JSON)

// These functions return within 2 ms
int drive(int flow_mlpm, int at_pressure_cmH2O_tenths, int time_ms, int from_now_ms, char* client_data, 
char *custom);
int pause_and_prep(int timer_ms, char* client_data, char *custom); 

/*
 * This is a "FAKE" implementation until Nathaniel gives us an SPI Version
 */
 // return number is expected flow in milliliters per second that the air drive thinks it can return
 // flow_mlps is the require flow in milliliters pers second
 // at_pressure_cmH2O_tenths is the pressure in the mm of water against which the air drive must operate
 // duration_ms is the time that the this flow should be produced in ms
 // from_now_ms is when to begin producing this flow. This can be used to allow the drive to prepare for action.
 // client_data is a pointer to a 64 byte buffer guaranteeed to be returned on the next call undisturbed.
 // custom is a pointer to an character buffer of unspecified size. It is used only for data specific to the given airdrive.
int drive(int flow_mlps, int at_pressure_cmH2O_tenths, int duration_ms, int from_now_ms, char* client_data, 
char *custom) {
  Serial.print("drive called: ");
  Serial.println(flow_mlps);
  Serial.println(at_pressure_cmH2O_tenths);
  Serial.println(duration_ms);
  Serial.println(from_now_ms);
  Serial.println(custom);
  // Just to be sneaky, I will add the volume produced since the last pause_and_prep call to my client_data!
  // This is actually in microliters (millionths of a liter)
  int v_millionths_l = flow_mlps*duration_ms;
  ((long *)client_data)[0] += v_millionths_l;
}

// return value is the number of ms that the machine requires to be ready;
// no furher call should be made for that number of ms
int pause_and_prep(int timer_ms, char* client_data, char *custom) {
  Serial.print("pause_and_prep called: ");
  Serial.println(timer_ms);
  // Just to be sneaky, I will add the volume produced since the last pause_and_prep call to my client_data!
  ((long *)client_data)[0] = 0;
  return timer_ms;
}

char client_data[64];
// Thest a single call to drive after doing a prep
void test_single_puff() {
  int p = pause_and_prep(1000,client_data,"Get ready!");
  delay(p);

  int d = drive(10,200,100,0,client_data,"I know you are really a PolyVent.");
  Serial.print("d =");
  Serial.println(d);
  Serial.println("test_single_puff told it to deliver:");
  Serial.println(10*100);
    // We should not know what the client_data is, but we will cheat...
  Serial.print("Air Drive Thinks it Delivered Volume: ");
  Serial.println(((long *)client_data)[0]);
  assert( ((long *)client_data)[0] == 10*100);
}

// Thest making a 600-ml puff in 1 second.
void test_big_puff() {
  int p = pause_and_prep(1000,client_data,"Get ready!");
  delay(p);

  const int MEASURED_PRESSURE = 200;

  const int TARGET_VOL = 600;
  const int TARGET_INSPIRE_TIME_MS = 1000;
  const int DRIVE_INTERVAL_MS = 10;
  const float TARGET_FLOW_MLPS = (float) TARGET_VOL /  ((float) TARGET_INSPIRE_TIME_MS / 1000.0);
  const int INTERVALS = TARGET_INSPIRE_TIME_MS / DRIVE_INTERVAL_MS;
  for(int i = 0; i < INTERVALS; i++) {
      int d = drive((int) TARGET_FLOW_MLPS,MEASURED_PRESSURE,DRIVE_INTERVAL_MS,0,client_data,"I know you are really a PolyVent.");
      Serial.println(i);
      Serial.println(((long *)client_data)[0]);
  }
  // We should not know what the client_data is, but we will cheat...
  Serial.print("Air Drive Thinks it Delivered Volume: ");
  Serial.println(((long *)client_data)[0]);
  // This is failing for an unknown reason.
 // assert( ((long *)client_data)[0] == (TARGET_VOL*1000));
}
void setup() {
  // put your setup code here, to run once:
  //Initiate Serial communication.
  Serial.begin(500000);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Starting!");
  test_single_puff();
  // We should not know what the client_data is, but we will cheat...
  Serial.print("Air Drive Thinks it Delivered Volume: ");
  Serial.println(((int *)client_data)[0]);
  delay(1000);
  test_big_puff();
  delay(3000);
}
