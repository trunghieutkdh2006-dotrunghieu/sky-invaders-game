#pragma once
#include <vector>
#include "types.h"
using namespace std;

// ===== BIẾN TOÀN CỤC =====
extern int W, H; // Kích thước màn hình (dùng để spawn vật phẩm ở vị trí ngẫu nhiên)

// ===== VẬT PHẨM SỨC MẠNH (POWERUP) =====

// Tạo vật phẩm sức mạnh ngẫu nhiên
// @param powerUps - Vector chứa các vật phẩm
// Các loại vật phẩm:
//   0: Tăng tốc độ bắn (Rapid Fire)
//   1: Kích hoạt khiên bảo vệ (Shield)
//   2: Tăng sức mạnh đạn (Stronger Bullets)
//   3: Kích hoạt laser (Laser)
void spawnPowerUp(vector<PowerUp> &powerUps);

// Tạo hiệu ứng nổ (particle) tại vị trí enemy chết
// @param p - Vector chứa các hạt (particle)
// @param x, y - Vị trí enemy chết
// Tạo 50 hạt bắn ra theo các góc ngẫu nhiên
void spawnExplosion(vector<Particle> &p, float x, float y);

// Áp dụng hiệu ứng của vật phẩm lên player
// @param playerHP - Máu player (chỉ áp dụng với vật phẩm hồi máu)
// @param fireRate - Tốc độ bắn (tăng khi nhặt vật phẩm loại 0)
// @param bulletPower - Sức mạnh đạn (tăng khi nhặt vật phẩm loại 2)
// @param hasShield - Trạng thái khiên (bật khi nhặt vật phẩm loại 1)
// @param hasLaser - Trạng thái laser (bật khi nhặt vật phẩm loại 3)
// @param type - Loại vật phẩm (0, 1, 2, 3)
void applyPowerUp(int &playerHP, float &fireRate, int &bulletPower,
                  bool &hasShield, bool &hasLaser, int type);

// Kiểm tra va chạm giữa player và vật phẩm
// @param px, py - Vị trí player
// @param powerUps - Vector chứa các vật phẩm
// @param playerHP - Máu player (tham chiếu)
// @param fireRate - Tốc độ bắn (tham chiếu)
// @param bulletPower - Sức mạnh đạn (tham chiếu)
// @param hasShield - Trạng thái khiên (tham chiếu)
// @param hasLaser - Trạng thái laser (tham chiếu)
// Nếu player đến gần vật phẩm (khoảng cách < 20px) thì áp dụng và xóa vật phẩm
void checkPowerUpCollision(float px, float py, vector<PowerUp> &powerUps,
                           int &playerHP, float &fireRate, int &bulletPower,
                           bool &hasShield, bool &hasLaser);

// ===== VẬT PHẨM HỒI MÁU (HEALTH DROP) =====

// Cập nhật vật phẩm hồi máu: rơi xuống đất, di chuyển về phía player, hồi máu khi chạm
// @param healthDrops - Vector chứa các vật phẩm hồi máu
// @param px, py - Vị trí player (để kéo về phía player)
// @param playerHP - Máu player (tăng khi nhặt vật phẩm)
// @param H - Chiều cao màn hình (để xác định mặt đất)
// Logic:
//   - Vật phẩm rơi xuống với gia tốc trọng trường (0.2)
//   - Khi chạm đất (y >= H-50) thì nằm yên
//   - Nếu player đến gần (< 120px) thì vật phẩm bị kéo về phía player
//   - Nếu chạm player (dist < 25) thì hồi máu (+1, tối đa 10)
//   - Vật phẩm biến mất sau 180 frame trên mặt đất
void updateHealthDrops(vector<HealthDrop> &healthDrops, float px, float py,
                       int &playerHP, int H);