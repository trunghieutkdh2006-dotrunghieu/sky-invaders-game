#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include <string>
#include "types.h"

using namespace std;

// ===== BIẾN TOÀN CỤC =====

// Font chữ dùng để vẽ text (được load trong main.cpp)
extern TTF_Font *font;

// Kích thước màn hình (được lấy từ SDL_GetRendererOutputSize)
extern int W;
extern int H;

// ===== BACKGROUND =====

// Texture của background (ảnh nền cuộn)
extern SDL_Texture *backgroundTexture;

// Tải background từ file "assets/background.png"
// @param renderer - SDL_Renderer dùng để tạo texture
// @return true nếu load thành công, false nếu thất bại
bool loadBackground(SDL_Renderer *renderer);

// Vẽ background cuộn theo chiều dọc
// @param r - SDL_Renderer dùng để vẽ
// @param scrollY - Vị trí cuộn hiện tại (càng lớn càng xuống dưới)
// Tạo hiệu ứng 2 layer cuộn liên tục
void drawBackground(SDL_Renderer *r, int scrollY);

// Giải phóng background texture khi kết thúc game
void destroyBackground();

// ===== TEXT =====

// Vẽ text bình thường (màu xanh cyan)
// @param r - SDL_Renderer
// @param text - Nội dung cần vẽ
// @param x, y - Tọa độ góc trên bên trái
void drawText(SDL_Renderer *r, string text, int x, int y);

// Vẽ text theo phong cách neon (phát sáng)
// @param r - SDL_Renderer
// @param text - Nội dung cần vẽ
// @param x, y - Tọa độ góc trên bên trái
// Tạo hiệu ứng glow: vẽ 3 lớp mờ xung quanh + 1 lớp chính giữa
void drawTextNeon(SDL_Renderer *r, string text, int x, int y);

// Vẽ text căn giữa theo chiều ngang
// @param r - SDL_Renderer
// @param text - Nội dung cần vẽ
// @param y - Tọa độ y (x tự động tính ở giữa màn hình)
void drawTextCenter(SDL_Renderer *r, string text, int y);

// ===== BUTTON =====

// Vẽ nút bấm
// @param r - SDL_Renderer
// @param b - Button cần vẽ (gồm rect, text, hover)
// Nếu hover = true: màu sáng hơn
// Nếu hover = false: màu tối hơn
void drawButton(SDL_Renderer *r, Button &b);

// Cập nhật trạng thái hover của nút dựa vào vị trí chuột
// @param b - Button cần cập nhật
// Gọi hàm này mỗi frame trong menu hoặc pause
void updateButton(Button &b);

// ===== HP BAR =====

// Vẽ thanh máu của player
// @param r - SDL_Renderer
// @param hp - Máu hiện tại
// @param maxHp - Máu tối đa
// Màu sắc thay đổi theo tỷ lệ máu:
//   > 60%: màu xanh lá
//   30-60%: màu vàng
//   < 30%: màu đỏ
void drawHealthBar(SDL_Renderer *r, int hp, int maxHp);

// Vẽ thanh máu của enemy
// @param r - SDL_Renderer
// @param en - Enemy cần vẽ thanh máu
// Thanh máu hiển thị phía trên đầu enemy
// Màu vàng cam, có hiệu ứng glow nhẹ
void drawEnemyHP(SDL_Renderer *r, Enemy &en);