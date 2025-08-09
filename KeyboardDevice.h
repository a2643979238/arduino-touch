#ifndef __KEYBOARDDEVICE_h__
#define __KEYBOARDDEVICE_h__

#include <Arduino.h>

// 键盘HID报告结构 (标准8字节格式)
struct KeyboardReport {
  uint8_t modifiers;  // 修饰符: Bit0-Ctrl, Bit1-Shift, Bit2-Alt, Bit3-Win (左), Bit4-7为右侧对应键
  uint8_t reserved;   // 保留字节
  uint8_t keys[6];    // 同时按下的按键扫描码
};

// 修饰符位定义
#define MOD_LEFT_CTRL 0x01
#define MOD_LEFT_SHIFT 0x02
#define MOD_LEFT_ALT 0x04
#define MOD_LEFT_WIN 0x08
#define MOD_RIGHT_CTRL 0x10
#define MOD_RIGHT_SHIFT 0x20
#define MOD_RIGHT_ALT 0x40
#define MOD_RIGHT_WIN 0x80

class KeyboardDevice {
public:
  KeyboardDevice();

  // 初始化键盘设备
  void init();
  
  // 重置设备状态（内存优化版本）
  void reset();

  // 解析键盘HID报告
  void parseKeyboardReport(uint8_t len, uint8_t* data);
  
  // 公共访问初始化状态
  bool initialized;

private:
  // 检测按键变化
  void detectKeyChanges();

  // 解析修饰符
  void parseModifiers(uint8_t currentMod, uint8_t previousMod);

  // 输出按键事件
  void printKeyEvent(uint8_t keyCode, bool pressed, uint8_t modifiers);

  // 获取按键名称
  const char* getKeyName(uint8_t keyCode);

  // 获取修饰符字符串
  String getModifierString(uint8_t modifiers);

  // 当前和上一次的键盘报告
  KeyboardReport currentReport;
  KeyboardReport previousReport;
};

#endif  //__KEYBOARDDEVICE_h__