#include <Servo.h>

// พินของ Joystick
const int VRX_LEFT = A0;   // X axis of left joystick
const int VRY_LEFT = A1;   // Y axis of left joystick
const int VRX_RIGHT = A2;  // X axis of right joystick
const int VRY_RIGHT = A3;  // Y axis of right joystick

// พินของเซอร์โว
const int baseServoPin = 6;
const int shoulderServoPin = 9;
const int elbowServoPin = 10;
const int gripperServoPin = 11;

Servo baseServo;
Servo shoulderServo;
Servo elbowServo;
Servo gripperServo;

// ตำแหน่งของเซอร์โว
int basePosition = 90;
int shoulderPosition = 90;
int elbowPosition = 90;
int gripperPosition = 90;

// ค่าเริ่มต้นของ joystick
int joyMiddle = 512;
int joyDeadZone = 50;

// ตัวแปรสำหรับการควบคุมความเร็ว
unsigned long lastMoveTime = 0;
unsigned long moveInterval = 15; // ระยะเวลาระหว่างการเคลื่อนที่ (มิลลิวินาที)

// ตัวแปรสำหรับการควบคุมการจับและปล่อย
const int GRIPPER_OPEN = 180;
const int GRIPPER_CLOSE = 0;

void setup() {
  Serial.begin(115200);
  
  baseServo.attach(baseServoPin, 500, 2400);
  shoulderServo.attach(shoulderServoPin, 500, 2400);
  elbowServo.attach(elbowServoPin, 500, 2400);
  gripperServo.attach(gripperServoPin, 500, 2400);

  Serial.println("Arduino initialized");

  // ตั้งค่าตำแหน่งเริ่มต้น
  controlServo("Base", basePosition);
  controlServo("Shoulder", shoulderPosition);
  controlServo("Elbow", elbowPosition);
  controlServo("Gripper", gripperPosition);
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastMoveTime >= moveInterval) {
    // อ่านค่า Joystick
    int leftX = analogRead(VRX_LEFT);
    int leftY = analogRead(VRY_LEFT);
    int rightX = analogRead(VRX_RIGHT);
    int rightY = analogRead(VRY_RIGHT);

    // ควบคุม Base (Left X)
    if (abs(leftX - joyMiddle) > joyDeadZone) {
      basePosition += map(leftX, 0, 1023, -2, 2);
      basePosition = constrain(basePosition, 0, 180);
      controlServo("Base", basePosition);
    }

    // ควบคุม Shoulder (Left Y)
    if (abs(leftY - joyMiddle) > joyDeadZone) {
      shoulderPosition += map(leftY, 0, 1023, 2, -2); // กลับทิศทาง
      shoulderPosition = constrain(shoulderPosition, 0, 180);
      controlServo("Shoulder", shoulderPosition);
    }

    // ควบคุม Elbow (Right Y)
    if (abs(rightY - joyMiddle) > joyDeadZone) {
      elbowPosition += map(rightY, 0, 1023, 2, -2); // กลับทิศทาง
      elbowPosition = constrain(elbowPosition, 0, 180);
      controlServo("Elbow", elbowPosition);
    }

    // ควบคุม Gripper ด้วยแกน X ของ joystick ขวา
    if (abs(rightX - joyMiddle) > joyDeadZone) {
      gripperPosition += map(rightX, 0, 1023, -2, 2);
      gripperPosition = constrain(gripperPosition, GRIPPER_CLOSE, GRIPPER_OPEN);
      controlServo("Gripper", gripperPosition);
    }

    lastMoveTime = currentTime;
  }


  // รับคำสั่งจาก ESP8266
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    Serial.print("Received from ESP8266: ");
    Serial.println(command);
    executeCommand(command);
  }
}

void controlServo(String servoName, int angle) {
  Servo* targetServo;
  int* targetPosition;

  if (servoName == "Base") {
    targetServo = &baseServo;
    targetPosition = &basePosition;
  } else if (servoName == "Shoulder") {
    targetServo = &shoulderServo;
    targetPosition = &shoulderPosition;
  } else if (servoName == "Elbow") {
    targetServo = &elbowServo;
    targetPosition = &elbowPosition;
  } else if (servoName == "Gripper") {
    targetServo = &gripperServo;
    targetPosition = &gripperPosition;
  } else {
    return; // ไม่พบ servo ที่ต้องการ
  }

  // จำกัดค่าองศาระหว่าง 0-180
  angle = constrain(angle, 0, 180); 
  
  targetServo->write(angle);
  *targetPosition = angle;

  // ส่งข้อมูลกลับไปยัง ESP8266
  Serial.println(servoName + "," + String(angle));
}

void executeCommand(String command) {
  int commaIndex = command.indexOf(',');
  if (commaIndex != -1) {
    String servoName = command.substring(0, commaIndex);
    int angle = command.substring(commaIndex + 1).toInt();
    controlServo(servoName, angle);
  }
}