#pragma once
#include "types.h"
#include <vector>
using std::vector;

// ===== BIẾN TOÀN CỤC =====

// Trạng thái hiện tại của game (MENU, PLAYING, PAUSED, INSTRUCTION, GAMEOVER)
extern GameState state;

// Trạng thái tiếp theo (dùng cho hiệu ứng chuyển màn)
extern GameState nextState;

// Độ mờ của hiệu ứng fade (0.0 = trong suốt, 1.0 = đen hoàn toàn)
extern float fade;

// Cờ báo hiệu đang thực hiện chuyển màn (true = đang fade)
extern bool fading;

// Cờ báo hiệu đang fade IN (hiện màn mới) hay fade OUT (tắt màn cũ)
// true = đang hiện màn mới, false = đang tắt màn cũ
extern bool fadeIn;

// Số frame còn lại của hiệu ứng hitstop (dừng màn hình khi trúng đòn)
extern int hitStopFrames;

// Cường độ của hiệu ứng hitstop (càng lớn càng rung mạnh)
extern float hitStopStrength;

// ===== HÀM =====

// Bắt đầu hiệu ứng chuyển màn sang trạng thái target
// @param target - Trạng thái đích (MENU, PLAYING, GAMEOVER, ...)
// Cách dùng: startTransition(PLAYING) để chuyển từ MENU sang PLAYING
void startTransition(GameState target);

// Kích hoạt hiệu ứng hitstop (dừng màn hình trong frames frame)
// @param frames - Số frame dừng (thường là 5-10 frame)
// Cách dùng: triggerHitStop(8) khi enemy chết hoặc player trúng đòn
void triggerHitStop(int frames);

// Cập nhật hiệu ứng fade mỗi frame
// - Nếu đang fade OUT: tăng fade lên đến 1.0, sau đó chuyển state và fade IN
// - Nếu đang fade IN: giảm fade về 0.0
// - Gọi hàm này mỗi frame trong vòng lặp game
void updateFade();