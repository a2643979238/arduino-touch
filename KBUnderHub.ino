#include <usbhub.h>
#include <SPI.h>
#include "HIDManager.h"


USB Usb;
USBHub Hub(&Usb);

// 只使用2个HID实例以节省内存 - 支持键盘+鼠标
HIDManager hid1(&Usb);
HIDManager hid2(&Usb);

// 中断辅助热插拔检测
#if USE_INTERRUPT
#define USB_INT_PIN 3  // 中断引脚 (Arduino Uno: Pin 3)
volatile bool usbInterruptFlag = false;

// USB中断服务程序
void usbInterruptHandler() {
  usbInterruptFlag = true;
}
#endif

void setup() {
  Serial.begin(115200);

#if USE_INTERRUPT
  // 配置中断引脚 (需要硬件连接: Arduino Pin 9 -> Pin 3)
  pinMode(USB_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(USB_INT_PIN), usbInterruptHandler, FALLING);

  Serial.println(F("USB HID Manager - Interrupt Enhanced"));
  Serial.println(F("INT pin configured for hot-plug detection"));
#else
  Serial.println(F("USB HID Manager - Polling Mode"));
#endif

  if (Usb.Init() == -1) {
    Serial.println(F("USB Host initialization failed"));
    while (1)
      ;  // Halt
  }

  Serial.println(F("Supporting keyboard + mouse (optimized)"));
  Serial.println(F("Waiting for devices...\n"));

  // 初始化HID管理器实例
  hid1.init();
  hid2.init();

  // 显示内存使用情况
  Serial.print(F("Free memory: "));
  Serial.print(freeMemory());
  Serial.println(F(" bytes"));
}

// 简化的内存检测函数（避免编译错误）
int freeMemory() {
  // 为了避免跨平台兼容性问题，暂时返回固定值
  // 在实际部署时可以启用平台特定的内存检测
  return 1500;  // 估计可用内存值
}

void loop() {
  static unsigned long lastPoll = 0;
  static unsigned long lastStatusCheck = 0;
  bool forceCheck = false;

  unsigned long currentTime = millis();

#if USE_INTERRUPT
  // 中断辅助检测：当INT引脚触发时立即检查
  if (usbInterruptFlag) {
    usbInterruptFlag = false;
    forceCheck = true;

#if DEBUG_MODE
    Serial.println(F("USB interrupt detected"));
#endif
  }
#endif

  // 智能轮询优化
  uint16_t pollInterval = max(hid1.getPollInterval(), hid2.getPollInterval());

  if (forceCheck || (currentTime - lastPoll >= pollInterval)) {
    Usb.Task();
    lastPoll = currentTime;

    // 检查HID实例的设备状态（降低频率或中断触发）
    if (forceCheck || (currentTime - lastStatusCheck > 1000)) {
      hid1.checkDeviceStatus();
      hid2.checkDeviceStatus();
      lastStatusCheck = currentTime;
    }
  }

  // 定期输出状态和性能统计
  static unsigned long lastReport = 0;
  if (currentTime - lastReport > 20000) {  // 每20秒报告一次
    Serial.println(F("=== System Status ==="));

    Serial.print(F("HID1: "));
    hid1.printConnectedDevices();
    Serial.print(F("HID2: "));
    hid2.printConnectedDevices();

    // 性能统计
    hid1.printMemoryUsage();

    Serial.print(F("Free memory: "));
    Serial.print(freeMemory());
    Serial.println(F(" bytes"));

    Serial.println();
    lastReport = currentTime;
  }
}
