#include "MouseDevice.h"

MouseDevice::MouseDevice() {
  initialized = false;
  memset(&currentReport, 0, sizeof(MouseReport));
  memset(&previousReport, 0, sizeof(MouseReport));
  absoluteX = 0;
  absoluteY = 0;
}

void MouseDevice::init() {
  initialized = true;
  absoluteX = 0;
  absoluteY = 0;
}

void MouseDevice::reset() {
  initialized = false;
  memset(&currentReport, 0, sizeof(MouseReport));
  memset(&previousReport, 0, sizeof(MouseReport));
  absoluteX = 0;
  absoluteY = 0;
}

void MouseDevice::parseMouseReport(uint8_t len, uint8_t* data) {
  if (!initialized || len < 3 || data == nullptr) return;

  // 保存上一次的报告
  memcpy(&previousReport, &currentReport, sizeof(MouseReport));

  // 解析当前报告
  currentReport.buttons = data[0];
  currentReport.x = (int8_t)data[1];  // 强制转换为有符号
  currentReport.y = (int8_t)data[2];  // 强制转换为有符号

  if (len >= 4) {
    currentReport.wheel = (int8_t)data[3];  // 滚轮数据
  } else {
    currentReport.wheel = 0;
  }

  // 检测并输出变化
  detectButtonChanges();
  detectMovement();
  detectWheelMovement();
}

void MouseDevice::detectButtonChanges() {
  uint8_t changedButtons = currentReport.buttons ^ previousReport.buttons;

  // 检测左键变化
  if (changedButtons & MOUSE_LEFT_BUTTON) {
    bool pressed = (currentReport.buttons & MOUSE_LEFT_BUTTON) != 0;
    printButtonEvent(MOUSE_LEFT_BUTTON, pressed);
  }

  // 检测右键变化
  if (changedButtons & MOUSE_RIGHT_BUTTON) {
    bool pressed = (currentReport.buttons & MOUSE_RIGHT_BUTTON) != 0;
    printButtonEvent(MOUSE_RIGHT_BUTTON, pressed);
  }

  // 检测中键变化
  if (changedButtons & MOUSE_MIDDLE_BUTTON) {
    bool pressed = (currentReport.buttons & MOUSE_MIDDLE_BUTTON) != 0;
    printButtonEvent(MOUSE_MIDDLE_BUTTON, pressed);
  }
}

void MouseDevice::detectMovement() {
  // 检测鼠标移动
  if (currentReport.x != 0 || currentReport.y != 0) {
    // 更新绝对坐标
    absoluteX += currentReport.x;
    absoluteY += currentReport.y;

    // 防止坐标溢出
    if (absoluteX < -32767) absoluteX = -32767;
    if (absoluteX > 32767) absoluteX = 32767;
    if (absoluteY < -32767) absoluteY = -32767;
    if (absoluteY > 32767) absoluteY = 32767;

    printMoveEvent(absoluteX, absoluteY);
  }
}

void MouseDevice::detectWheelMovement() {
  // 检测滚轮滚动
  if (currentReport.wheel != 0) {
    printWheelEvent(currentReport.wheel);
  }
}

void MouseDevice::printButtonEvent(uint8_t button, bool pressed) {
  Serial.print(F("Mouse: "));
  Serial.print(getButtonName(button));
  Serial.print(F(" button "));
  Serial.println(pressed ? F("pressed") : F("released"));
}

void MouseDevice::printMoveEvent(int16_t x, int16_t y) {
  Serial.print(F("Mouse: Moved to ("));
  Serial.print(x);
  Serial.print(F(", "));
  Serial.print(y);
  Serial.println(F(")"));
}

void MouseDevice::printWheelEvent(int8_t wheel) {
  Serial.print(F("Mouse: Wheel "));
  if (wheel > 0) {
    Serial.print(F("up "));
    Serial.println(wheel);
  } else {
    Serial.print(F("down "));
    Serial.println(-wheel);
  }
}

const char* MouseDevice::getButtonName(uint8_t button) {
  switch (button) {
    case MOUSE_LEFT_BUTTON:
      return "Left";
    case MOUSE_RIGHT_BUTTON:
      return "Right";
    case MOUSE_MIDDLE_BUTTON:
      return "Middle";
    default:
      return "Unknown";
  }
}

void MouseDevice::getCurrentPosition(int16_t* x, int16_t* y) {
  if (x) *x = absoluteX;
  if (y) *y = absoluteY;
}