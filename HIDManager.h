#ifndef __HIDMANAGER_h__
#define __HIDMANAGER_h__

#include <hiduniversal.h>
#include "KeyboardDevice.h"
#include "MouseDevice.h"

// 性能优化配置
#define MAX_DEVICES 1
#define USE_INTERRUPT 1  // 中断模式开关
#define BUFFER_SIZE 8    // 统一缓冲区大小

// 轮询频率配置 (毫秒)
#define POLL_ACTIVE 1        // 设备活跃时轮询间隔
#define POLL_IDLE 10         // 设备空闲时轮询间隔
#define POLL_NONE 100        // 无设备时轮询间隔
#define DEVICE_TIMEOUT 5000  // 设备超时时间(ms)

// 设备类型枚举
enum DeviceType {
  DEVICE_UNKNOWN = 0,
  DEVICE_KEYBOARD = 1,
  DEVICE_MOUSE = 2
};

// 设备信息结构（内存优化版本 - 从30字节优化到16字节）
struct DeviceSlot {
  uint16_t vid;
  uint16_t pid;
  DeviceType deviceType : 4;  // 4位枚举，节省内存
  bool active : 1;
  bool changed : 1;
  uint8_t bufferSize : 6;       // 6位足够存储缓冲区大小
  uint8_t buffer[BUFFER_SIZE];  // 单一缓冲区
  uint16_t lastActivity;        // 相对时间戳，节省2字节
  uint8_t changeFlags;          // 位标志记录变化，替代bufferOld
};

// HID设备管理器 - 内存优化版本
class HIDManager : public HIDUniversal {
public:
  HIDManager(USB *p);
  ~HIDManager();

  // 初始化设备管理器
  void init();

  // 检查是否已连接设备
  bool isConnected() {
    return HIDUniversal::isReady();
  }

  // 智能设备状态检查（频率自适应）
  void checkDeviceStatus();

  // 获取当前轮询间隔
  uint16_t getPollInterval();

  // 检查是否有设备连接
  inline bool hasDevices() {
    return totalDevices > 0;
  }

  // 获取连接的设备信息
  void printConnectedDevices();

  // 性能统计
  void printMemoryUsage();

private:
  // HIDUniversal回调函数
  void ParseHIDData(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) override;
  uint8_t OnInitSuccessful() override;

  // 设备管理优化版本
  int8_t findDeviceSlot(uint16_t vid, uint16_t pid);
  int8_t createDeviceSlot(uint16_t vid, uint16_t pid);
  DeviceType identifyDeviceType(uint8_t len, uint8_t *buf);
  void processDeviceData(int8_t deviceIndex, uint8_t len, uint8_t *buf);

  // 内存优化函数
  bool hasDataChanged(int8_t deviceIndex, uint8_t len, uint8_t *buf);
  void updateChangeFlags(int8_t deviceIndex, uint8_t len, uint8_t *buf);
  inline uint16_t getRelativeTime() {
    return (uint16_t)(millis() & 0xFFFF);
  }


  // 设备槽位（内存优化）
  DeviceSlot devices[MAX_DEVICES];
  uint8_t totalDevices;
  int8_t currentDevice;

  // 设备处理器（静态分配）
  KeyboardDevice keyboard;  // 不再使用指针
  MouseDevice mouse;        // 不再使用指针

  // 连接状态位标志（节省内存）
  struct {
    bool keyboardConnected : 1;
    bool mouseConnected : 1;
    bool _reserved : 6;  // 保留位
  } status;
};


#endif  //__HIDMANAGER_h__