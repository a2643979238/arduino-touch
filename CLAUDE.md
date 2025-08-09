# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

这是一个基于Arduino的USB HID设备管理系统，支持通过USB Hub动态管理多个HID设备（键盘和鼠标）。系统实现了热插拔功能、设备自动识别、键盘按键事件（包括修饰符）和鼠标事件（移动坐标、按键点击）的实时检测，并以人类可读格式输出事件信息。

## 核心架构

### 硬件抽象层
- **USB管理**：使用USB Host Shield库管理USB连接和Hub通信
- **HID处理**：通过HIDUniversal库处理键盘和鼠标的HID报告数据

### 主要组件
- **KBUnderHub.ino**：主程序，初始化USB系统并运行HID管理器
- **HIDManager类**：智能数据路由管理器，继承自HIDUniversal
  - 单一USB接口处理多设备数据流
  - 基于HID报告长度和内容的智能设备识别
  - 动态设备槽位分配（最多2个设备：1键盘+1鼠标）
  - 数据路由到相应的设备处理器
  - 设备活动监控和超时检测
- **KeyboardDevice类**：专门处理键盘HID报告解析和按键状态跟踪
  - 支持按键按下/抬起检测
  - 修饰符识别（Ctrl、Shift、Alt、Win的左右区分）
  - 完整的按键扫描码到字符映射
- **MouseDevice类**：专门处理鼠标HID报告解析和事件检测
  - 鼠标移动坐标跟踪（绝对坐标计算）
  - 左右中键点击检测
  - 滚轮滚动检测

### 数据流（智能路由架构）
1. USB.Task()轮询USB设备状态，检测USB HID复合设备连接
2. 设备连接时：HIDManager.OnInitSuccessful() → 识别为USB HID复合设备
3. HID数据接收：HIDManager.ParseHIDData() → 接收混合的键盘/鼠标数据
4. 智能设备识别：
   - 8字节数据包 → 键盘设备
   - 3-4字节数据包 → 鼠标设备
5. 动态设备注册：首次检测到新设备类型时自动注册到设备槽位
6. 数据路由：根据设备类型将HID数据分发给KeyboardDevice或MouseDevice
7. 事件解析：各设备类解析HID报告，检测状态变化
8. 输出：以人类可读格式输出事件到串口
9. 设备断开检测：基于数据活动超时检测（5秒无活动视为断开）

## 开发环境

这是一个Arduino项目，需要：
- Arduino IDE或PlatformIO
- USB Host Shield库
- 支持USB Host的Arduino兼容开发板

## 串口调试

项目使用115200波特率进行串口通信，输出人类可读格式：

### 键盘事件输出示例
```
Keyboard device initialized
Keyboard: Key 'A' pressed (LShift)
Keyboard: Key 'A' released
Keyboard: Left Ctrl pressed
Keyboard: Left Ctrl released
```

### 鼠标事件输出示例
```
Mouse device initialized
Mouse: Moved to (120, 85)
Mouse: Left button pressed
Mouse: Left button released
Mouse: Right button pressed
Mouse: Wheel up 1
```

## 关键技术特性

- **热插拔支持**：动态检测设备连接和断开，无需重启系统
- **多设备管理**：同时支持最多4个HID设备连接
- **设备自动识别**：基于HID报告长度自动识别键盘(8字节)或鼠标(3-4字节)
- **状态跟踪**：精确检测按键/按钮的按下和抬起事件
- **坐标计算**：鼠标绝对坐标实时跟踪
- **修饰符支持**：完整的键盘修饰符识别（左右Ctrl/Shift/Alt/Win）
- **资源管理**：设备断开时自动清理资源，避免内存泄漏
- **错误处理**：设备连接失败和数据解析异常的处理

## HID报告格式

- **键盘报告**：8字节标准格式 [修饰符][保留][按键1-6]
- **鼠标报告**：3-4字节格式 [按键状态][X移动][Y移动][滚轮]

## 扩展考虑

系统采用模块化设计，可以轻松扩展：
- 添加更多HID设备类型（如游戏手柄）
- 实现设备热插拔检测
- 增加配置文件支持
- 添加无线通信模块