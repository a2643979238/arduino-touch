#include "KeyboardDevice.h"

KeyboardDevice::KeyboardDevice() {
  initialized = false;
  memset(&currentReport, 0, sizeof(KeyboardReport));
  memset(&previousReport, 0, sizeof(KeyboardReport));
}

void KeyboardDevice::init() {
  initialized = true;
  Serial.println(F("Keyboard device initialized"));
}

void KeyboardDevice::reset() {
  initialized = false;
  memset(&currentReport, 0, sizeof(KeyboardReport));
  memset(&previousReport, 0, sizeof(KeyboardReport));
}

void KeyboardDevice::parseKeyboardReport(uint8_t len, uint8_t* data) {
  if (!initialized || len < 8 || data == nullptr) return;

  // 保存上一次的报告
  memcpy(&previousReport, &currentReport, sizeof(KeyboardReport));

  // 解析当前报告
  currentReport.modifiers = data[0];
  currentReport.reserved = data[1];
  for (int i = 0; i < 6; i++) {
    currentReport.keys[i] = data[i + 2];
  }

  // 检测并输出变化
  detectKeyChanges();
}

void KeyboardDevice::detectKeyChanges() {
  // 检测修饰符变化
  if (currentReport.modifiers != previousReport.modifiers) {
    parseModifiers(currentReport.modifiers, previousReport.modifiers);
  }

  // 检测按键按下 (在当前报告中但不在上一次报告中)
  for (int i = 0; i < 6; i++) {
    uint8_t currentKey = currentReport.keys[i];
    if (currentKey != 0) {  // 有效按键
      bool wasPressed = false;
      // 检查这个键是否在上一次报告中
      for (int j = 0; j < 6; j++) {
        if (previousReport.keys[j] == currentKey) {
          wasPressed = true;
          break;
        }
      }
      if (!wasPressed) {
        // 新按下的键
        printKeyEvent(currentKey, true, currentReport.modifiers);
      }
    }
  }

  // 检测按键抬起 (在上一次报告中但不在当前报告中)
  for (int i = 0; i < 6; i++) {
    uint8_t previousKey = previousReport.keys[i];
    if (previousKey != 0) {  // 有效按键
      bool stillPressed = false;
      // 检查这个键是否仍在当前报告中
      for (int j = 0; j < 6; j++) {
        if (currentReport.keys[j] == previousKey) {
          stillPressed = true;
          break;
        }
      }
      if (!stillPressed) {
        // 抬起的键
        printKeyEvent(previousKey, false, previousReport.modifiers);
      }
    }
  }
}

void KeyboardDevice::parseModifiers(uint8_t currentMod, uint8_t previousMod) {
  uint8_t changed = currentMod ^ previousMod;  // 找出变化的位

  if (changed & MOD_LEFT_CTRL) {
    Serial.print(F("Keyboard: Left Ctrl "));
    Serial.println((currentMod & MOD_LEFT_CTRL) ? F("pressed") : F("released"));
  }
  if (changed & MOD_LEFT_SHIFT) {
    Serial.print(F("Keyboard: Left Shift "));
    Serial.println((currentMod & MOD_LEFT_SHIFT) ? F("pressed") : F("released"));
  }
  if (changed & MOD_LEFT_ALT) {
    Serial.print(F("Keyboard: Left Alt "));
    Serial.println((currentMod & MOD_LEFT_ALT) ? F("pressed") : F("released"));
  }
  if (changed & MOD_LEFT_WIN) {
    Serial.print(F("Keyboard: Left Win "));
    Serial.println((currentMod & MOD_LEFT_WIN) ? F("pressed") : F("released"));
  }
  if (changed & MOD_RIGHT_CTRL) {
    Serial.print(F("Keyboard: Right Ctrl "));
    Serial.println((currentMod & MOD_RIGHT_CTRL) ? F("pressed") : F("released"));
  }
  if (changed & MOD_RIGHT_SHIFT) {
    Serial.print(F("Keyboard: Right Shift "));
    Serial.println((currentMod & MOD_RIGHT_SHIFT) ? F("pressed") : F("released"));
  }
  if (changed & MOD_RIGHT_ALT) {
    Serial.print(F("Keyboard: Right Alt "));
    Serial.println((currentMod & MOD_RIGHT_ALT) ? F("pressed") : F("released"));
  }
  if (changed & MOD_RIGHT_WIN) {
    Serial.print(F("Keyboard: Right Win "));
    Serial.println((currentMod & MOD_RIGHT_WIN) ? F("pressed") : F("released"));
  }
}

void KeyboardDevice::printKeyEvent(uint8_t keyCode, bool pressed, uint8_t modifiers) {
  Serial.print(F("Keyboard: Key '"));
  Serial.print(getKeyName(keyCode));
  Serial.print(F("' "));
  Serial.print(pressed ? F("pressed") : F("released"));

  String modStr = getModifierString(modifiers);
  if (modStr.length() > 0) {
    Serial.print(F(" ("));
    Serial.print(modStr);
    Serial.print(F(")"));
  }
  Serial.println();
}

const char* KeyboardDevice::getKeyName(uint8_t keyCode) {
  // USB HID键盘扫描码到字符的映射
  switch (keyCode) {
    case 0x04: return "A";
    case 0x05: return "B";
    case 0x06: return "C";
    case 0x07: return "D";
    case 0x08: return "E";
    case 0x09: return "F";
    case 0x0A: return "G";
    case 0x0B: return "H";
    case 0x0C: return "I";
    case 0x0D: return "J";
    case 0x0E: return "K";
    case 0x0F: return "L";
    case 0x10: return "M";
    case 0x11: return "N";
    case 0x12: return "O";
    case 0x13: return "P";
    case 0x14: return "Q";
    case 0x15: return "R";
    case 0x16: return "S";
    case 0x17: return "T";
    case 0x18: return "U";
    case 0x19: return "V";
    case 0x1A: return "W";
    case 0x1B: return "X";
    case 0x1C: return "Y";
    case 0x1D: return "Z";
    case 0x1E: return "1";
    case 0x1F: return "2";
    case 0x20: return "3";
    case 0x21: return "4";
    case 0x22: return "5";
    case 0x23: return "6";
    case 0x24: return "7";
    case 0x25: return "8";
    case 0x26: return "9";
    case 0x27: return "0";
    case 0x28: return "ENTER";
    case 0x29: return "ESC";
    case 0x2A: return "BACKSPACE";
    case 0x2B: return "TAB";
    case 0x2C: return "SPACE";
    case 0x2D: return "-";
    case 0x2E: return "=";
    case 0x2F: return "[";
    case 0x30: return "]";
    case 0x31: return "\\";
    case 0x33: return ";";
    case 0x34: return "'";
    case 0x35: return "`";
    case 0x36: return ",";
    case 0x37: return ".";
    case 0x38: return "/";
    case 0x39: return "CAPS";
    case 0x3A: return "F1";
    case 0x3B: return "F2";
    case 0x3C: return "F3";
    case 0x3D: return "F4";
    case 0x3E: return "F5";
    case 0x3F: return "F6";
    case 0x40: return "F7";
    case 0x41: return "F8";
    case 0x42: return "F9";
    case 0x43: return "F10";
    case 0x44: return "F11";
    case 0x45: return "F12";
    case 0x4F: return "RIGHT";
    case 0x50: return "LEFT";
    case 0x51: return "DOWN";
    case 0x52: return "UP";
    default:
      static char unknownKey[8];
      sprintf(unknownKey, "0x%02X", keyCode);
      return unknownKey;
  }
}

String KeyboardDevice::getModifierString(uint8_t modifiers) {
  String result = "";

  if (modifiers & MOD_LEFT_CTRL) {
    if (result.length() > 0) result += "+";
    result += "LCtrl";
  }
  if (modifiers & MOD_RIGHT_CTRL) {
    if (result.length() > 0) result += "+";
    result += "RCtrl";
  }
  if (modifiers & MOD_LEFT_SHIFT) {
    if (result.length() > 0) result += "+";
    result += "LShift";
  }
  if (modifiers & MOD_RIGHT_SHIFT) {
    if (result.length() > 0) result += "+";
    result += "RShift";
  }
  if (modifiers & MOD_LEFT_ALT) {
    if (result.length() > 0) result += "+";
    result += "LAlt";
  }
  if (modifiers & MOD_RIGHT_ALT) {
    if (result.length() > 0) result += "+";
    result += "RAlt";
  }
  if (modifiers & MOD_LEFT_WIN) {
    if (result.length() > 0) result += "+";
    result += "LWin";
  }
  if (modifiers & MOD_RIGHT_WIN) {
    if (result.length() > 0) result += "+";
    result += "RWin";
  }

  return result;
}