#include "audio.h"

// ===== BIẾN TOÀN CỤC =====

// Trạng thái tắt/bật âm thanh
// false = âm thanh bật (mặc định)
// true = âm thanh tắt
bool muted = false;

// ===== HÀM =====

// Hàm bật/tắt âm thanh (mute/unmute)
// Cách dùng: Gọi khi người chơi nhấn nút MUTE
void toggleMute()
{
    // Đảo ngược trạng thái muted
    muted = !muted;

    if (muted)
    {
        // === TẮT ÂM THANH ===

        // Tắt nhạc nền (volume = 0)
        Mix_VolumeMusic(0);

        // Tắt tất cả âm thanh hiệu ứng (channel -1 = tất cả channel)
        Mix_Volume(-1, 0);

        // Tạm dừng nhạc nền (để khi bật lại có thể tiếp tục)
        Mix_PauseMusic();
    }
    else
    {
        // === BẬT ÂM THANH ===

        // Bật nhạc nền với âm lượng tối đa
        Mix_VolumeMusic(MIX_MAX_VOLUME);

        // Bật tất cả âm thanh hiệu ứng với âm lượng tối đa
        Mix_Volume(-1, MIX_MAX_VOLUME);

        // Tiếp tục phát nhạc nền từ vị trí đã dừng
        Mix_ResumeMusic();
    }
}

// Hàm phát nhạc nền nếu đang không phát
// Cách dùng: Gọi trong game loop mỗi frame
// Chỉ phát nhạc khi:
//   1. Không ở chế độ mute
//   2. Nhạc đang không phát
//   3. File nhạc đã được load thành công (backgroundMusic != nullptr)
void playBGMIfStopped()
{
    // Kiểm tra điều kiện trước khi phát nhạc
    if (!muted &&                   // Không ở chế độ tắt tiếng
        Mix_PlayingMusic() == 0 &&  // Nhạc chưa được phát hoặc đã dừng
        backgroundMusic != nullptr) // File nhạc đã load thành công
    {
        // Phát nhạc nền, lặp vô hạn (-1)
        Mix_PlayMusic(backgroundMusic, -1);
    }
}