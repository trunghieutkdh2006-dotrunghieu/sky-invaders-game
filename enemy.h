#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "types.h"
using namespace std;

// ===== BIẾN TOÀN CỤC =====
extern int W, H; // Chiều rộng và chiều cao màn hình

// ===== ĐỘ KHÓ THEO LEVEL =====
// Tính toán các thông số độ khó dựa trên cấp độ hiện tại
// @param level - Cấp độ hiện tại
// @return Difficulty - Cấu trúc chứa: enemySpeed, fireRate, enemyCount, dodgePower, bulletSpeed
Difficulty getDifficulty(int level);

// ===== MÁU CỦA ENEMY =====
// Tính máu tối đa của enemy theo cấp độ
// @param level - Cấp độ hiện tại
// @return float - Lượng máu của enemy (công thức: 1 + level)
float enemyHP(int level);

// ===== TẠO ENEMY =====
// Spawn enemy mới cho mỗi level
// @param enemies - Vector chứa danh sách enemy
// @param lv - Cấp độ hiện tại (quyết định số lượng và độ khó)
// @param W, H - Kích thước màn hình (giới hạn vị trí spawn)
void spawn(vector<Enemy> &enemies, int lv, int W, int H);

// ===== CẬP NHẬT ENEMY =====
// Hàm quan trọng nhất: xử lý tất cả logic của enemy mỗi frame
// @param enemies - Vector chứa danh sách enemy
// @param bullets - Vector đạn (để enemy bắn và kiểm tra trúng đạn)
// @param particles - Vector hiệu ứng hạt (nổ, khói)
// @param texts - Vector chữ bay lên (HEADSHOT, LEVEL UP,...)
// @param healthDrops - Vector vật phẩm hồi máu
// @param powerUps - Vector vật phẩm sức mạnh
// @param lasers - Vector laser thẳng
// @param arcLasers - Vector laser vòng cung (cho boss)
// @param px, py - Vị trí player (để enemy di chuyển và bắn theo)
// @param level - Cấp độ hiện tại (ảnh hưởng đến tốc độ bắn, máu, sát thương)
// @param bulletPower - Sức mạnh đạn của player (từ powerup)
// @param formationTime - Thời gian formation (hiệu ứng)
// @param score - Điểm số (cộng khi tiêu diệt enemy)
// @param hitStopFrames - Số frame dừng màn hình khi trúng đòn
// @param W, H - Kích thước màn hình (giới hạn di chuyển)
void updateEnemies(
    vector<Enemy> &enemies,
    vector<Bullet> &bullets,
    vector<Particle> &particles,
    vector<FloatingText> &texts,
    vector<HealthDrop> &healthDrops,
    vector<PowerUp> &powerUps,
    vector<Laser> &lasers,
    vector<ArcLaser> &arcLasers,
    float px,
    float py,
    int level,
    int bulletPower,
    float formationTime,
    int &score,
    int &hitStopFrames,
    int W,
    int H);