#include <M5Stack.h>
#include "Free_Fonts.h"

#define MIC_Unit 36
#define MAX_LEN 320
#define X_OFFSET 0
#define Y_OFFSET 100
#define X_SCALE 1

volatile int16_t val_buf[MAX_LEN] = {0}; // 描画用バッファ
volatile int16_t write_index = 0;       // 書き込みインデックス
volatile int16_t data_count = 0;        // 有効データの個数
volatile int micValue = 0;              // 最新のAD値

// タイマー割り込みで呼び出される関数
void IRAM_ATTR onTimer() {
  micValue = analogRead(MIC_Unit);

  // データをバッファに書き込み
  val_buf[write_index] = map((int16_t)(micValue * X_SCALE), 1800, 4095, 0, 100);

  // 次の書き込み位置を計算
  write_index = (write_index + 1) % MAX_LEN;

  // データ数を更新（MAX_LENを超えないように）
  if (data_count < MAX_LEN) {
    data_count++;
  }
}

// 描画関数
static void draw_waveform() {
  if (data_count <= 1) return; // 有効なデータが不足している場合はスキップ

  int read_index = write_index; // 現在の書き込み位置を起点に読み出し開始
  for (int i = 0; i < data_count - 1; i++) {
    int next_index = (read_index + 1) % MAX_LEN;

    // データを描画
    M5.Lcd.drawLine(i + X_OFFSET, val_buf[read_index] + Y_OFFSET, 
                    i + 1 + X_OFFSET, val_buf[next_index] + Y_OFFSET, TFT_GREEN);

    read_index = next_index; // 次のデータへ
  }
}

hw_timer_t *timer = NULL;

void setup() {
  M5.begin();
  M5.Lcd.setFreeFont(FSS12);
  M5.Lcd.setTextDatum(TC_DATUM);
  M5.Lcd.drawString("MIC Unit", 160, 0, GFXFF);

  dacWrite(25, 0);

  // タイマーの設定
  timer = timerBegin(0, 80, true);         // タイマー0を設定 (80分周 -> 1µs)
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000, true);      // 1msごとに割り込み
  timerAlarmEnable(timer);                 // タイマーを有効化
}

void loop() {
  M5.Lcd.fillScreen(TFT_BLACK); // 描画領域をクリア
  
  // AD値を描画
  M5.Lcd.setTextDatum(TC_DATUM);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("AD Value: " + String(micValue), 160, 10, GFXFF);

  draw_waveform();              // 描画処理
  delay(20);                    // 少し遅延を入れる（スムーズな描画のため）
}
