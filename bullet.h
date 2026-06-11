#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "types.h"
using namespace std;

// ===== BULLET (ĐẠN CƠ BẢN) =====

// Cập nhật vị trí và xóa đạn hết thời gian sống
// @param bullets - Vector chứa tất cả đạn (của player và enemy)
void updateBullets(vector<Bullet> &bullets);

// ===== LASER (TIA THẲNG) =====

// Bắn tia laser thẳng từ enemy về phía player
// @param lasers - Vector chứa các tia laser
// @param en - Enemy bắn laser
// @param px, py - Vị trí player (để xác định hướng bắn)
// @param level - Cấp độ hiện tại (ảnh hưởng đến sát thương)
void shootLaser(vector<Laser> &lasers, Enemy &en, float px, float py, int level);

// Vẽ tất cả tia laser lên màn hình
// @param r - SDL_Renderer dùng để vẽ
// @param lasers - Vector chứa các tia laser cần vẽ
void renderLasers(SDL_Renderer *r, vector<Laser> &lasers);

// Cập nhật tia laser: giảm thời gian sống, kiểm tra va chạm với player
// @param lasers - Vector chứa các tia laser
// @param px, py - Vị trí player
// @param playerHP - Máu player (giảm nếu trúng laser)
// @param damageCooldown - Thời gian miễn nhiễm sau khi trúng đòn
// @param state - Trạng thái game (chuyển sang GAMEOVER nếu hết máu)
void updateLasers(vector<Laser> &lasers, float px, float py,
                  int &playerHP, int &damageCooldown, GameState &state);

// ===== ARC LASER (TIA VÒNG CUNG) =====
// (Dành riêng cho boss từ level 3 trở lên)

// Bắn tia laser vòng cung (hình quạt) từ enemy
// @param arcLasers - Vector chứa các tia vòng cung
// @param en - Enemy bắn (thường là boss)
// @param px, py - Vị trí player (xác định hướng bắn)
// @param level - Cấp độ (càng cao càng nhiều tia)
void shootArcLaser(vector<ArcLaser> &arcLasers, Enemy &en, float px, float py, int level);

// Cập nhật tia vòng cung: giảm thời gian sống, kiểm tra va chạm
// @param arcLasers - Vector chứa các tia vòng cung
// @param px, py - Vị trí player
// @param playerHP - Máu player (giảm nếu trúng)
// @param damageCooldown - Thời gian miễn nhiễm
// @param state - Trạng thái game
void updateArcLasers(vector<ArcLaser> &arcLasers, float px, float py,
                     int &playerHP, int &damageCooldown, GameState &state);

// Vẽ tất cả tia laser vòng cung lên màn hình
// @param r - SDL_Renderer dùng để vẽ
// @param arcLasers - Vector chứa các tia vòng cung cần vẽ
void renderArcLasers(SDL_Renderer *r, vector<ArcLaser> &arcLasers);