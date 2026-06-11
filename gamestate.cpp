#include "gamestate.h"
#include <cmath>
#include <vector>

// ============================================================
// BIẾN TOÀN CỤC (được khai báo trong gamestate.h)
// ============================================================

// Trạng thái hiện tại của game
// Khởi tạo ở MENU (màn hình chính)
GameState state = MENU;

// Trạng thái sẽ chuyển đến sau hiệu ứng fade
GameState nextState = MENU;

// Độ mờ của màn hình (0 = trong suốt, 1 = đen hoàn toàn)
float fade = 0.0f;

// Cờ báo hiệu đang thực hiện hiệu ứng chuyển màn
bool fading = false;

// Hướng fade: false = fade OUT (tối dần), true = fade IN (sáng dần)
bool fadeIn = false;

// Số frame còn lại của hiệu ứng hitstop (dừng màn hình)
int hitStopFrames = 0;

// Cường độ rung màn hình khi hitstop (càng lớn càng rung mạnh)
float hitStopStrength = 1.0f;

// ============================================================
// BẮT ĐẦU CHUYỂN MÀN (TRANSITION)
// ============================================================
// Hàm này được gọi khi muốn chuyển từ màn hình này sang màn hình khác
// Ví dụ: từ MENU sang PLAYING, từ PLAYING sang GAMEOVER
void startTransition(GameState target)
{
    fading = true;      // Bắt đầu hiệu ứng chuyển màn
    fadeIn = false;     // Bắt đầu bằng fade OUT (màn hình tối dần)
    nextState = target; // Lưu trạng thái đích
}

// ============================================================
// KÍCH HOẠT HIỆU ỨNG HITSTOP
// ============================================================
// Hitstop là hiệu ứng game tạm dừng trong vài frame
// Tạo cảm giác "đánh trúng đích" mạnh mẽ
// Thường dùng khi: enemy chết, player trúng đòn, boss xuất hiện
void triggerHitStop(int frames)
{
    hitStopFrames = frames; // Đặt số frame dừng
}

// ============================================================
// CẬP NHẬT HIỆU ỨNG FADE MỖI FRAME
// ============================================================
// Hàm này phải được gọi trong game loop mỗi frame
// Quy trình:
//   1. Nếu không có chuyển màn (fading = false) -> thoát
//   2. Nếu đang fade OUT (fadeIn = false): tăng dần fade (tối dần)
//      - Khi fade đạt 1.0: chuyển state sang nextState, bắt đầu fade IN
//   3. Nếu đang fade IN (fadeIn = true): giảm dần fade (sáng dần)
//      - Khi fade về 0: kết thúc chuyển màn (fading = false)
void updateFade()
{
    // Nếu không có chuyển màn thì thoát
    if (!fading)
        return;

    // === PHA 1: FADE OUT (MÀN HÌNH TỐI DẦN) ===
    if (!fadeIn)
    {
        // Tăng độ mờ lên 0.05 mỗi frame
        fade += 0.05f;

        // Khi đã đen hoàn toàn (fade >= 1.0)
        if (fade >= 1.0f)
        {
            fade = 1.0f;       // Đảm bảo chính xác là 1.0
            fadeIn = true;     // Chuyển sang pha fade IN
            state = nextState; // Chuyển trạng thái game
        }
    }
    // === PHA 2: FADE IN (MÀN HÌNH SÁNG DẦN) ===
    else
    {
        // Giảm độ mờ xuống 0.05 mỗi frame
        fade -= 0.05f;

        // Khi đã sáng hoàn toàn (fade <= 0)
        if (fade <= 0.0f)
        {
            fade = 0.0f;    // Đảm bảo chính xác là 0
            fading = false; // Kết thúc chuyển màn
        }
    }
}

// ============================================================
// CẬP NHẬT DI CHUYỂN PLAYER (HÀM DỰ PHÒNG, KHÔNG DÙNG CHÍNH)
// ============================================================
// Hàm này có thể dùng để điều khiển player trong các chế độ khác
// Hiện tại không được sử dụng trong main.cpp (dùng xử lý trực tiếp)
void updatePlayer(Player &p)
{
    // Lấy tốc độ hiện tại
    float currentSpeed = p.speed;

    // Xử lý dash (tăng tốc)
    if (p.dashing && p.dashTime > 0)
    {
        currentSpeed *= 2.5f; // Dash: tăng gấp 2.5 lần tốc độ
        p.dashTime--;         // Giảm thời gian dash
    }
    else
    {
        p.dashing = false; // Kết thúc dash
    }

    // Giảm cooldown dash
    if (p.dashCooldown > 0)
        p.dashCooldown--;

    // Lấy trạng thái bàn phím
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    // Di chuyển theo các phím WASD
    if (keystate[SDL_SCANCODE_W]) // Lên
        p.y -= currentSpeed;

    if (keystate[SDL_SCANCODE_S]) // Xuống
        p.y += currentSpeed;

    if (keystate[SDL_SCANCODE_A]) // Trái
        p.x -= currentSpeed;

    if (keystate[SDL_SCANCODE_D]) // Phải
        p.x += currentSpeed;
}

// ============================================================
// KÍCH HOẠT DASH CHO PLAYER
// ============================================================
// Hàm này được gọi khi nhấn phím Shift
void triggerDash(Player &p)
{
    // Chỉ dash khi đã hết cooldown
    if (p.dashCooldown == 0)
    {
        p.dashing = true;    // Bật trạng thái dash
        p.dashTime = 12;     // Dash kéo dài 12 frame (~0.2 giây)
        p.dashCooldown = 60; // Cooldown 60 frame (1 giây)
    }
}

// ============================================================
// BẮN ĐẠN THEO DẠNG CHÙM (SPREAD SHOT)
// ============================================================
// Bắn nhiều viên đạn tỏa tròn xung quanh hướng chính
// Các tham số:
//   bullets    - Vector đạn (nơi thêm đạn mới)
//   px, py     - Vị trí bắn (thường là vị trí player)
//   dirX, dirY - Hướng bắn chính (vector)
//   count      - Số lượng đạn trong chùm
//   spreadDeg  - Góc trải rộng (độ)
void shootSpread(vector<Bullet> &bullets,
                 float px, float py,
                 float dirX, float dirY,
                 int count,
                 float spreadDeg)
{
    // Góc cơ bản từ vector hướng (atan2: góc radian)
    float baseAngle = atan2(dirY, dirX);

    // Đổi góc trải rộng từ độ sang radian
    float spread = spreadDeg * M_PI / 180.0f;

    // Tạo từng viên đạn
    for (int i = 0; i < count; i++)
    {
        // Tính tỷ lệ từ 0 đến 1
        // Nếu count = 1, t = 0.5 (ở giữa)
        // Nếu count > 1, i=0 -> 0, i=count-1 -> 1
        float t = (count == 1) ? 0.5f : (float)i / (count - 1);

        // Góc của viên đạn hiện tại = góc cơ bản + (t-0.5)*độ trải
        // t=0.5: góc giữa (hướng chính)
        // t=0: góc lệch -spread/2
        // t=1: góc lệch +spread/2
        float angle = baseAngle + (t - 0.5f) * spread;

        // Tạo viên đạn
        Bullet b;
        b.x = px;                  // Vị trí X
        b.y = py;                  // Vị trí Y
        b.vx = cos(angle) * 12.0f; // Vận tốc X
        b.vy = sin(angle) * 12.0f; // Vận tốc Y
        b.life = 60;               // Sống 60 frame (1 giây)
        b.enemy = false;           // Đạn của player

        // Thêm vào vector đạn
        bullets.push_back(b);
    }
}