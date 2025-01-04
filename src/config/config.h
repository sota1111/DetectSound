#ifndef CONFIG_H
#define CONFIG_H

// 騒音検出
// 基準値1: X分間の騒音レベルがY dBの時に通知
#define NOISE_ALERT_DURATION_MINUTES_X    // X分間
#define NOISE_ALERT_THRESHOLD_DB_Y        // Y dB

// 基準値2: XX dB以上の瞬間的な騒音がYY分間にZZ回以上観測された場合通知
#define INSTANT_NOISE_THRESHOLD_DB_XX     // XX dB
#define OBSERVATION_DURATION_MINUTES_YY   // YY分間
#define NOISE_EVENT_COUNT_THRESHOLD_ZZ    // ZZ回以上

#define NOISE_CONSTANT_VALUE (2500)

#define TIME_IRQ (100) //us
#define TIME_WAVE (20000) //us
//#define RECORD_1s_LEN 1000*1000/TIME_IRQ
// 10000を超えるデータはDRAMが足りない。PSRAMを使う必要がある。
#define RECORD_BEFORE_LEN (5000)
#define RECORD_AFTER_LEN (5000)
#define RECORD_MAX_LEN (RECORD_BEFORE_LEN + RECORD_AFTER_LEN)

// デバイス管理
#define TIME_UPDATE_MAX_VALUE (1000)
#define TIMEOUT_WIFI_CONNECT (30000)
#define JST (3600L * 9)

// サーバー関連
#define BASE_URL "https://6ansren87i.execute-api.ap-northeast-1.amazonaws.com/Prod/"

// ファイル関連
#define APARTMENT_NAME "APARTMENT_A"
#define ROOM_NAME "ROOM_A"

// 表示関連
#define FONT_SIZE 2

#endif