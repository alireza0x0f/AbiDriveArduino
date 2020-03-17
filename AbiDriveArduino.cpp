
#include "Arduino.h"
#include "AbiDriveArduino.h"

static const int kMotorOffsetFloat = 2;
static const int kMotorStrideFloat = 28;
static const int kMotorOffsetInt32 = 0;
static const int kMotorStrideInt32 = 4;
static const int kMotorOffsetBool = 0;
static const int kMotorStrideBool = 4;
static const int kMotorOffsetUint16 = 0;
static const int kMotorStrideUint16 = 2;

// Print with stream operator
template<class T> inline Print& operator <<(Print &obj,     T arg) { obj.print(arg);    return obj; }
template<>        inline Print& operator <<(Print &obj, float arg) { obj.print(arg, 4); return obj; }

AbiDriveArduino::AbiDriveArduino(Stream& serial)
    : serial_(serial) {}

void AbiDriveArduino::SetPosition(int motor_number, float position) {
    SetPosition(motor_number, position, 0.0f, 0.0f);
}

void AbiDriveArduino::SetPosition(int motor_number, float position, float velocity_feedforward) {
    SetPosition(motor_number, position, velocity_feedforward, 0.0f);
}

void AbiDriveArduino::SetPosition(int motor_number, float position, float velocity_feedforward, float current_feedforward) {
    serial_ << "p " << motor_number  << " " << position << " " << velocity_feedforward << " " << current_feedforward << "\n";
}

void AbiDriveArduino::SetVelocity(int motor_number, float velocity) {
    SetVelocity(motor_number, velocity, 0.0f);
}

void AbiDriveArduino::SetVelocity(int motor_number, float velocity, float current_feedforward) {
    serial_ << "v " << motor_number  << " " << velocity << " " << current_feedforward << "\n";
}

void AbiDriveArduino::SetCurrent(int motor_number, float current) {
    serial_ << "c " << motor_number << " " << current << "\n";
}

void AbiDriveArduino::TrapezoidalMove(int motor_number, float position){
    serial_ << "t " << motor_number << " " << position << "\n";
}

float AbiDriveArduino::readFloat() {
    return readString().toFloat();
}

float AbiDriveArduino::GetVelocity(int motor_number){
	serial_<< "r axis" << motor_number << ".encoder.vel_estimate\n";
	return AbiDriveArduino::readFloat();
}

int32_t AbiDriveArduino::readInt() {
    return readString().toInt();
}

bool AbiDriveArduino::run_state(int axis, int requested_state, bool wait) {
    int timeout_ctr = 100;
    serial_ << "w axis" << axis << ".requested_state " << requested_state << '\n';
    if (wait) {
        do {
            delay(100);
            serial_ << "r axis" << axis << ".current_state\n";
        } while (readInt() != AXIS_STATE_IDLE && --timeout_ctr > 0);
    }

    return timeout_ctr > 0;
}

String AbiDriveArduino::readString() {
    String str = "";
    static const unsigned long timeout = 1000;
    unsigned long timeout_start = millis();
    for (;;) {
        while (!serial_.available()) {
            if (millis() - timeout_start >= timeout) {
                return str;
            }
        }
        char c = serial_.read();
        if (c == '\n')
            break;
        str += c;
    }
    return str;
}
