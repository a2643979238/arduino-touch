#ifndef __HUBKBs_h__
#define __HUBKBs_h__
#include <hiduniversal.h>

// 当前支持的键盘数量
#define KBNUM 3

class KBSET : public HIDUniversal {
public:
  KBSET(USB *p)
    : HIDUniversal(p){};
  bool connected() {
    return HIDUniversal::isReady();
  };
  // 本次收到数据的键盘编号
  uint8_t Current;
  // 存放第x个键盘是否有改变发生的标记
  bool Changed[KBNUM];
  // 存放第x个键盘当前收到的缓冲区长度
  uint8_t BufferSize[KBNUM];
  // 存放第x个键盘收到的缓冲区数据
  uint8_t Buffer[KBNUM][64];
  // 存放第x个键盘的PID 和 VID
  uint16_t PID[KBNUM];
  uint16_t VID[KBNUM];
private:
  void ParseHIDData(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);  // Called by the HIDUniversal library
  uint8_t BufferSizeOld[KBNUM];
  uint8_t BufferOld[KBNUM][64];
  // 当前收到的键盘数据总数
  uint8_t TotolKB;
  uint8_t OnInitSuccessful() {  // Called by the HIDUniversal library on success
    KBSET::TotolKB = 0;
    for (uint8_t j = 0; j < KBNUM; j++) {
      for (uint8_t i = 0; i < 64; i++) {
        KBSET::BufferOld[j][i] = 0xFF;
      }
    }
    return 0;
  };
};

#endif  //__HUBKBs_h__
