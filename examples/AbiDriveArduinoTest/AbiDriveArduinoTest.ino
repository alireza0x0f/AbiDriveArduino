
#include <SoftwareSerial.h>
#include <AbiDriveArduino.h>

// Printing with stream operator
template<class T> inline Print& operator <<(Print &obj,     T arg) { obj.print(arg);    return obj; }
template<>        inline Print& operator <<(Print &obj, float arg) { obj.print(arg, 4); return obj; }

// Serial to the AbiDrive
SoftwareSerial abidrive_serial(8, 9); //RX (AbiDrive TX), TX (AbiDrive RX)
// Note: you must also connect GND on AbiDrive to GND on Arduino!

// AbiDrive object
AbiDriveArduino abidrive(abidrive_serial);

void setup() {
  // AbiDrive uses 115200 baud
  abidrive_serial.begin(115200);

  // Serial to PC
  Serial.begin(115200);
  while (!Serial) ; // wait for Arduino Serial Monitor to open

  Serial.println("AbiDriveArduino");
  Serial.println("Setting parameters...");

  // In this example we set the same parameters to both motors.
  // You can of course set them different if you want.
  // See the documentation or play around in abidrivetool to see the available parameters
  for (int axis = 0; axis < 2; ++axis) {
    abidrive_serial << "w axis" << axis << ".controller.config.vel_limit " << 22000.0f << '\n';
    abidrive_serial << "w axis" << axis << ".motor.config.current_lim " << 11.0f << '\n';
    // This ends up writing something like "w axis0.motor.config.current_lim 10.0\n"
  }

  Serial.println("Ready!");
  Serial.println("Send the character '0' or '1' to calibrate respective motor (you must do this before you can command movement)");
  Serial.println("Send the character 's' to exectue test move");
  Serial.println("Send the character 'b' to read bus voltage");
  Serial.println("Send the character 'p' to read motor positions in a 10s loop");
}

void loop() {

  if (Serial.available()) {
    char c = Serial.read();

    // Run calibration sequence
    if (c == '0' || c == '1') {
      int motornum = c-'0';
      int requested_state;

      requested_state = AbiDriveArduino::AXIS_STATE_MOTOR_CALIBRATION;
      Serial << "Axis" << c << ": Requesting state " << requested_state << '\n';
      abidrive.run_state(motornum, requested_state, true);

      requested_state = AbiDriveArduino::AXIS_STATE_ENCODER_OFFSET_CALIBRATION;
      Serial << "Axis" << c << ": Requesting state " << requested_state << '\n';
      abidrive.run_state(motornum, requested_state, true);

      requested_state = AbiDriveArduino::AXIS_STATE_CLOSED_LOOP_CONTROL;
      Serial << "Axis" << c << ": Requesting state " << requested_state << '\n';
      abidrive.run_state(motornum, requested_state, false); // don't wait
    }

    // Sinusoidal test move
    if (c == 's') {
      Serial.println("Executing test move");
      for (float ph = 0.0f; ph < 6.28318530718f; ph += 0.01f) {
        float pos_m0 = 20000.0f * cos(ph);
        float pos_m1 = 20000.0f * sin(ph);
        abidrive.SetPosition(0, pos_m0);
        abidrive.SetPosition(1, pos_m1);
        delay(5);
      }
    }

    // Read bus voltage
    if (c == 'b') {
      abidrive_serial << "r vbus_voltage\n";
      Serial << "Vbus voltage: " << abidrive.readFloat() << '\n';
    }

    // print motor positions in a 10s loop
    if (c == 'p') {
      static const unsigned long duration = 10000;
      unsigned long start = millis();
      while(millis() - start < duration) {
        for (int motor = 0; motor < 2; ++motor) {
          abidrive_serial << "r axis" << motor << ".encoder.pos_estimate\n";
          Serial << abidrive.readFloat() << '\t';
        }
        Serial << '\n';
      }
    }
  }
}
