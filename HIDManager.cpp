#include "HIDManager.h"

HIDManager::HIDManager(USB *p) : HIDUniversal(p), 
  keyboard(),           // 静态初始化
  mouse(),             // 静态初始化
  totalDevices(0),
  currentDevice(-1),
  lastPollTime(0),
  pollCount(0) {
  
  // 初始化状态位标志
  status.keyboardConnected = false;
  status.mouseConnected = false;
  status._reserved = 0;
  
  // 内存优化的设备槽位初始化
  for (uint8_t i = 0; i < MAX_DEVICES; i++) {
    devices[i].vid = 0;
    devices[i].pid = 0;
    devices[i].deviceType = DEVICE_UNKNOWN;
    devices[i].active = false;
    devices[i].bufferSize = 0;
    devices[i].changed = false;
    devices[i].lastActivity = 0;
    devices[i].changeFlags = 0;
    memset(devices[i].buffer, 0, BUFFER_SIZE);
  }
}

HIDManager::~HIDManager() {
  // 静态对象不需要手动释放内存
}

void HIDManager::init() {
  // 初始化静态设备处理器
  keyboard.reset();   // 重置键盘设备状态
  mouse.reset();      // 重置鼠标设备状态
  
  lastPollTime = millis();
  pollCount = 0;
  
#if DEBUG_MODE
  Serial.println(F("HID instance ready (optimized)"));
#endif
}

uint8_t HIDManager::OnInitSuccessful() {
  Serial.print(F("HID: VID:0x"));
  Serial.print(HIDUniversal::VID, HEX);
  Serial.print(F(" PID:0x"));
  Serial.println(HIDUniversal::PID, HEX);
  
  return 0;
}

void HIDManager::ParseHIDData(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  if (len == 0 || buf == nullptr) return;
  
  // 简化调试输出
  Serial.print(F("Data len="));
  Serial.print(len);
  Serial.print(F(": "));
  for (uint8_t i = 0; i < len && i < 8; i++) {
    if (buf[i] < 0x10) Serial.print(F("0"));
    Serial.print(buf[i], HEX);
    Serial.print(F(" "));
  }
  Serial.println();
  
  // 优化：每个HIDManager实例只处理一个设备
  if (totalDevices == 0) {
    DeviceType detectedType = identifyDeviceType(len, buf);
    
    currentDevice = createDeviceSlot(HIDUniversal::VID, HIDUniversal::PID);
    if (currentDevice != -1) {
      devices[currentDevice].deviceType = detectedType;
      devices[currentDevice].bufferSize = len;
      
      if (detectedType == DEVICE_KEYBOARD) {
#if DEBUG_MODE
        Serial.println(F("=== KEYBOARD ==="));
#endif
        status.keyboardConnected = true;
        if (!keyboard.initialized) {
          keyboard.init();
        }
      } else if (detectedType == DEVICE_MOUSE) {
#if DEBUG_MODE
        Serial.println(F("=== MOUSE ==="));
#endif
        status.mouseConnected = true;
        if (!mouse.initialized) {
          mouse.init();
        }
      }
    }
  } else {
    currentDevice = 0;  // 单设备实例
  }
  
  // 处理设备数据
  if (currentDevice != -1) {
    processDeviceData(currentDevice, len, buf);
  }
}

int8_t HIDManager::findDeviceSlot(uint16_t vid, uint16_t pid) {
  for (uint8_t i = 0; i < MAX_DEVICES; i++) {
    if (devices[i].active && devices[i].vid == vid && devices[i].pid == pid) {
      return i;
    }
  }
  return -1;
}

int8_t HIDManager::createDeviceSlot(uint16_t vid, uint16_t pid) {
  for (uint8_t i = 0; i < MAX_DEVICES; i++) {
    if (!devices[i].active) {
      devices[i].vid = vid;
      devices[i].pid = pid;
      devices[i].active = true;
      devices[i].lastActivity = getRelativeTime();
      totalDevices++;
      return i;
    }
  }
  return -1;
}

DeviceType HIDManager::identifyDeviceType(uint8_t len, uint8_t *buf) {
  // 基于报告长度和内容模式识别设备类型
  if (len == 8) {
    // 键盘报告通常是8字节
    // 进一步检查：键盘报告的第2字节通常是保留位(0x00)
    return DEVICE_KEYBOARD;
  } else if (len >= 3 && len <= 4) {
    // 鼠标报告通常是3-4字节
    return DEVICE_MOUSE;
  } else if (len == 6) {
    // 某些键盘可能使用6字节报告
    return DEVICE_KEYBOARD;
  }
  
  Serial.print(F("Unknown device type with report length: "));
  Serial.println(len);
  return DEVICE_UNKNOWN;
}

void HIDManager::processDeviceData(int8_t deviceIndex, uint8_t len, uint8_t *buf) {
  if (deviceIndex < 0 || deviceIndex >= MAX_DEVICES) return;
  
  // 使用优化的变化检测
  if (!hasDataChanged(deviceIndex, len, buf)) {
    return;  // 数据未变化，直接返回
  }
  
  // 更新变化标志和缓冲区
  updateChangeFlags(deviceIndex, len, buf);
  
  DeviceSlot& device = devices[deviceIndex];
  
#if DEBUG_MODE
  // 优化的调试输出
  debugPrint("Device ");
  Serial.print(deviceIndex);
  debugPrint(" (");
  debugPrint(device.deviceType == DEVICE_KEYBOARD ? "KB" : 
            (device.deviceType == DEVICE_MOUSE ? "MS" : "UK"));
  debugPrint(") changed: ");
  for (uint8_t i = 0; i < len && i < BUFFER_SIZE; i++) {
    debugPrintHex(buf[i]);
    debugPrint(" ");
  }
  Serial.println();
#endif
  
  // 优化的设备处理器调用（去除指针检查）
  if (device.deviceType == DEVICE_KEYBOARD && status.keyboardConnected) {
    keyboard.parseKeyboardReport(len, buf);
  } else if (device.deviceType == DEVICE_MOUSE && status.mouseConnected) {
    mouse.parseMouseReport(len, buf);
  }
}

void HIDManager::checkDeviceStatus() {
  // 优化的设备超时检查
  uint16_t currentTime = getRelativeTime();
  
  for (uint8_t i = 0; i < MAX_DEVICES; i++) {
    if (devices[i].active) {
      // 使用16位时间差检查（处理溢出情况）
      uint16_t timeDiff = (uint16_t)(currentTime - devices[i].lastActivity);
      if (timeDiff > (DEVICE_TIMEOUT >> 6)) {  // 除以64转换为相对时间单位
#if DEBUG_MODE
        debugPrint("Device timeout: ");
        Serial.println(i);
#endif
        devices[i].active = false;
        totalDevices--;
        
        // 更新连接状态位标志
        if (devices[i].deviceType == DEVICE_KEYBOARD) {
          status.keyboardConnected = false;
        } else if (devices[i].deviceType == DEVICE_MOUSE) {
          status.mouseConnected = false;
        }
      }
    }
  }
}

// 智能轮询间隔计算
uint16_t HIDManager::getPollInterval() {
  if (totalDevices == 0) {
    return POLL_NONE;  // 无设备时使用最低频率
  }
  
  // 检查设备活动性
  uint16_t currentTime = getRelativeTime();
  for (uint8_t i = 0; i < MAX_DEVICES; i++) {
    if (devices[i].active) {
      uint16_t timeDiff = (uint16_t)(currentTime - devices[i].lastActivity);
      if (timeDiff < (1000 >> 6)) {  // 1秒内有活动
        return POLL_ACTIVE;
      }
    }
  }
  
  return POLL_IDLE;  // 设备空闲
}

// 调试输出优化函数
#if DEBUG_MODE
void HIDManager::debugPrint(const char* msg) {
  Serial.print(msg);
}

void HIDManager::debugPrintHex(uint8_t value) {
  if (value < 0x10) Serial.print(F("0"));
  Serial.print(value, HEX);
}
#endif

// 性能统计
void HIDManager::printMemoryUsage() {
#if DEBUG_MODE
  Serial.println(F("=== Memory Usage Report ==="));
  Serial.print(F("HIDManager size: "));
  Serial.print(sizeof(HIDManager));
  Serial.println(F(" bytes"));
  
  Serial.print(F("DeviceSlot size: "));
  Serial.print(sizeof(DeviceSlot));
  Serial.println(F(" bytes"));
  
  Serial.print(F("Total devices: "));
  Serial.println(totalDevices);
  
  Serial.print(F("Poll count: "));
  Serial.println(pollCount);
  
  unsigned long uptime = millis() - lastPollTime;
  if (uptime > 0) {
    Serial.print(F("Poll rate: "));
    Serial.print((pollCount * 1000) / uptime);
    Serial.println(F(" Hz"));
  }
  Serial.println(F("========================"));
#endif
}

// 优化的数据变化检测函数
bool HIDManager::hasDataChanged(int8_t deviceIndex, uint8_t len, uint8_t *buf) {
  if (deviceIndex < 0 || deviceIndex >= MAX_DEVICES) return false;
  
  // 使用位标志快速检查变化
  uint8_t newFlags = 0;
  for (uint8_t i = 0; i < len && i < BUFFER_SIZE; i++) {
    if (devices[deviceIndex].buffer[i] != buf[i]) {
      newFlags |= (1 << (i & 7));  // 设置对应位标志
    }
  }
  
  return newFlags != 0;
}

void HIDManager::updateChangeFlags(int8_t deviceIndex, uint8_t len, uint8_t *buf) {
  if (deviceIndex < 0 || deviceIndex >= MAX_DEVICES) return;
  
  devices[deviceIndex].changeFlags = 0;
  for (uint8_t i = 0; i < len && i < BUFFER_SIZE; i++) {
    if (devices[deviceIndex].buffer[i] != buf[i]) {
      devices[deviceIndex].changeFlags |= (1 << (i & 7));
      devices[deviceIndex].buffer[i] = buf[i];
    }
  }
  
  devices[deviceIndex].changed = (devices[deviceIndex].changeFlags != 0);
  if (devices[deviceIndex].changed) {
    devices[deviceIndex].lastActivity = getRelativeTime();
  }
}

void HIDManager::printConnectedDevices() {
  if (totalDevices == 0) {
    Serial.println(F("No device"));
    return;
  }
  
  // 简化输出 - 每个实例只显示自己的设备
  for (uint8_t i = 0; i < MAX_DEVICES; i++) {
    if (devices[i].active) {
      switch (devices[i].deviceType) {
        case DEVICE_KEYBOARD:
          Serial.print(F("Keyboard"));
          break;
        case DEVICE_MOUSE:
          Serial.print(F("Mouse"));
          break;
        default:
          Serial.print(F("Unknown"));
          break;
      }
      
      Serial.print(F(" (VID:0x"));
      Serial.print(devices[i].vid, HEX);
      Serial.print(F(" PID:0x"));
      Serial.print(devices[i].pid, HEX);
      Serial.println(F(")"));
      return;  // 每个实例只有一个设备
    }
  }
}