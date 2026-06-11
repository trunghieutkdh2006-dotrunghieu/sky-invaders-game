#pragma once
#include <SDL2/SDL_mixer.h>

extern bool muted;
extern Mix_Music *backgroundMusic;

void toggleMute();
void playBGMIfStopped();
/*
#pragma once
#include <SDL2/SDL_mixer.h>

// ===== BIẾN TOÀN CỤC =====

// Trạng thái tắt/bật âm thanh (true = tắt, false = bật)
extern bool muted;

// Con trỏ đến nhạc nền (background music)
extern Mix_Music* backgroundMusic;

// ===== HÀM =====

// Hàm bật/tắt âm thanh (mute/unmute)
// Khi muted = true: tắt nhạc nền và tất cả âm thanh
// Khi muted = false: bật lại nhạc nền và âm thanh
void toggleMute();

// Hàm phát nhạc nền nếu đang không phát
// Chỉ phát khi:
//   1. Không ở chế độ mute
//   2. Nhạc đang không phát
//   3. File nhạc đã được load thành công
void playBGMIfStopped();
*/