#include "KeyboardSets.h"

void KBSET::ParseHIDData(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  /*
    if (len && buf)  {
      Notify(PSTR("\r\n"), 0x80);
      for (uint8_t i = 0; i < len; i++) {
        D_PrintHex<uint8_t > (buf[i], 0x80);
        Notify(PSTR(" "), 0x80);
      }
    }
  */

  // 在 KBSET 中查找记录
  KBSET::Current = 0xFF;
  for (uint8_t i = 0; i < KBNUM; i++) {
    //
    if ((KBSET::PID[i] == HIDUniversal::PID) && (KBSET::VID[i] == HIDUniversal::VID)) {
      KBSET::Current = i;
    }
  }
  // 如果查找不到
  if (KBSET::Current == 0xFF) {
    KBSET::Current = KBSET::TotolKB;
    KBSET::PID[KBSET::Current] = HIDUniversal::PID;
    KBSET::VID[KBSET::Current] = HIDUniversal::VID;
    KBSET::BufferSize[KBSET::Current] = len;
    KBSET::TotolKB++;
  }

  // 检查本次数据和上次数据是否有差别
  if (memcmp(BufferOld[KBSET::Current], buf, len) == 0) {
    // 没有差别
    KBSET::Changed[KBSET::Current] = false;
  } else {
    memcpy(Buffer[KBSET::Current], buf, len);
    // 有差别
    KBSET::Changed[KBSET::Current] = true;
    memcpy(BufferOld[KBSET::Current], buf, len);
    Serial.print("VID:");
    Serial.print(KBSET::VID[KBSET::Current], HEX);
    Serial.print(" PID:");
    Serial.println(KBSET::PID[KBSET::Current], HEX);
    for (uint8_t i = 0; i < KBSET::BufferSize[KBSET::Current]; i++) {
      Serial.print((byte)(KBSET::Buffer[KBSET::Current][i]), HEX);
      Serial.print(" ");
    }
    Serial.println("");
  }
}
