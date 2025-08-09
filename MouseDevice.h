#ifndef __MOUSEDEVICE_h__
#define __MOUSEDEVICE_h__

#include <Arduino.h>

// 鼠标HID报告结构 (标准4字节格式)
struct MouseReport {
  uint8_t buttons;  // 按键状态: Bit0-左键, Bit1-右键, Bit2-中键
  int8_t x;         // X轴相对移动 (-127 到 +127)
  int8_t y;         // Y轴相对移动 (-127 到 +127)
  int8_t wheel;     // 滚轮移动 (-127 到 +127)
};

// 鼠标按键位定义
#define MOUSE_LEFT_BUTTON 0x01
#define MOUSE_RIGHT_BUTTON 0x02
#define MOUSE_MIDDLE_BUTTON 0x04

class MouseDevice {
public:
  MouseDevice();

  // 初始化鼠标设备
  void init();

  // 重置设备状态（内存优化版本）
  void reset();

  // 解析鼠标HID报告
  void parseMouseReport(uint8_t len, uint8_t* data);

  // 获取当前绝对坐标
  void getCurrentPosition(int16_t* x, int16_t* y);

  // 公共访问初始化状态
  bool initialized;

private:
  // 检测按键变化
  void detectButtonChanges();

  // 检测鼠标移动
  void detectMovement();

  // 检测滚轮滚动
  void detectWheelMovement();

  // 输出鼠标按键事件
  void printButtonEvent(uint8_t button, bool pressed);

  // 输出鼠标移动事件
  void printMoveEvent(int16_t x, int16_t y);

  // 输出滚轮事件
  void printWheelEvent(int8_t wheel);

  // 获取按键名称
  const char* getButtonName(uint8_t button);

  // 当前和上一次的鼠标报告
  MouseReport currentReport;
  MouseReport previousReport;

  // 绝对坐标跟踪
  int16_t absoluteX;
  int16_t absoluteY;
};

#endif  //__MOUSEDEVICE_h__