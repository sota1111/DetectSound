#include "WaveformDrawer.h"

WaveformDrawer::WaveformDrawer() : write_index(0), data_count(0) {
    memset(val_buf, 0, sizeof(val_buf));
}

void WaveformDrawer::updateBuffer(int micValue) {
    val_buf[write_index] = map(micValue * X_SCALE, 1800, 4095, 0, 100);
    write_index = (write_index + 1) % MAX_LEN;
    if (data_count < MAX_LEN) {
        data_count++;
    }
}

void WaveformDrawer::drawWaveform() {
    if (data_count <= 1) return;

    int read_index = write_index;
    for (int i = 0; i < data_count - 1; i++) {
        int next_index = (read_index + 1) % MAX_LEN;
        M5.Lcd.drawLine(i + X_OFFSET, val_buf[read_index] + Y_OFFSET, 
                        i + 1 + X_OFFSET, val_buf[next_index] + Y_OFFSET, TFT_GREEN);
        read_index = next_index;
    }

    int constantY = map(NOISE_CONSTANT_VALUE, 1800, 4095, 0, 100) + Y_OFFSET;
    M5.Lcd.drawLine(X_OFFSET, constantY, X_OFFSET + MAX_LEN, constantY, TFT_RED);
}
