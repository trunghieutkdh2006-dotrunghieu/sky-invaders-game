#include <SDL2/SDL.h>
#pragma once
#include <string>
using namespace std;

// ============================================================
// ENUMS (Các trạng thái và kiểu di chuyển)
// ============================================================

// Trạng thái của game
enum GameState
{
    MENU,        // Màn hình chính
    PLAYING,     // Đang chơi
    PAUSED,      // Tạm dừng
    INSTRUCTION, // Màn hình hướng dẫn
    LEADERBOARD, // Bảng xếp hạng (chưa dùng)
    GAMEOVER,    // Kết thúc game
};

// Chế độ chơi
enum GameMode
{
    HARDCORE,   // Chế độ khó (chết là mất hết)
    ENDLESS,    // Chế độ vô tận (chưa dùng)
    TIME_ATTACK // Chế độ tính giờ (chưa dùng)
};

// Kiểu di chuyển của enemy
enum MoveType
{
    MOVE_ORBIT,         // Di chuyển theo quỹ đạo tròn
    MOVE_ZIGZAG,        // Di chuyển ngoằn ngoèo
    MOVE_SINE,          // Di chuyển theo hình sin
    MOVE_SPIRAL,        // Di chuyển theo đường xoắn ốc
    MOVE_DASH,          // Lao thẳng về phía player
    MOVE_RANDOM,        // Di chuyển ngẫu nhiên
    MOVE_SWARM,         // Di chuyển theo bầy đàn
    MOVE_FLEE,          // Chạy trốn khỏi player
    MOVE_PURSUE,        // Đuổi theo player (dự đoán vị trí)
    MOVE_WAVE,          // Di chuyển theo sóng
    MOVE_TELEPORT,      // Dich chuyển tức thời
    MOVE_BOUNCE,        // Nảy khi chạm tường
    MOVE_CIRCLE_PLAYER, // Xoay quanh player
    MOVE_SNAKE,         // Di chuyển như rắn
    MOVE_BOSS_PHASE     // Boss thay đổi cách di chuyển theo máu
};

// Trạng thái của enemy khi dash
enum EnemyState
{
    PREPARE, // Chuẩn bị dash
    DASH,    // Đang dash
    RECOVER  // Hồi phục sau dash
};

// Trạng thái dash của player
enum DashState
{
    DASH_PREPARE, // Chuẩn bị dash
    DASHING,      // Đang dash
    DASH_RECOVER  // Hồi phục sau dash
};

// ============================================================
// STRUCTS (Các cấu trúc dữ liệu)
// ============================================================

// ----- ĐẠN -----
struct Bullet
{
    float x, y;   // Vị trí hiện tại
    float vx, vy; // Vận tốc (px/frame)
    bool enemy;   // true = đạn của enemy, false = đạn của player
    int life = 1; // Thời gian sống (càng lớn càng lâu)
};

// ----- VẬT PHẨM SỨC MẠNH -----
struct PowerUp
{
    float x, y;   // Vị trí
    int type;     // 0: Rapid Fire, 1: Shield, 2: Stronger Bullets, 3: Laser
    int lifetime; // Thời gian tồn tại (frame)
};

// ----- KẺ THÙ -----
struct Enemy
{
    // Vị trí và di chuyển
    float x, y;             // Tọa độ hiện tại
    float centerX, centerY; // Tâm (dùng cho di chuyển orbit)
    float radius;           // Bán kính quỹ đạo (orbit)
    float angle;            // Góc hiện tại (orbit, spiral, snake)
    float speed;            // Tốc độ di chuyển
    float vx, vy;           // Vận tốc (dash, bounce)

    // Trạng thái
    int timer;    // Bộ đếm thời gian (dash, teleport)
    int state;    // Trạng thái hiện tại (PREPARE, DASH, RECOVER)
    int moveType; // Kiểu di chuyển (MoveType)

    // Máu
    float hp;        // Máu hiện tại
    float maxHP;     // Máu tối đa
    float hpDisplay; // Máu hiển thị (hiệu ứng mượt)

    // Trạng thái chết
    bool dead;      // Đã chết hoàn toàn
    bool dying;     // Đang chết (hiệu ứng)
    int deathFrame; // Số frame đã chết (hiệu ứng)

    // Tấn công
    int shootTimer;       // Bộ đếm thời gian bắn
    int teleportTimer;    // Bộ đếm dịch chuyển
    int shootCooldown;    // Cooldown giữa các lần bắn
    int maxShootCooldown; // Cooldown tối đa

    // Phân loại
    bool boss; // true = boss, false = thường
    int type;  // Loại enemy (0-6)
};

// ----- HIỆU ỨNG HẠT -----
struct Particle
{
    float x, y, vx, vy; // Vị trí và vận tốc
    int life;           // Thời gian sống còn lại
};

// ----- CHỮ BAY LÊN -----
struct FloatingText
{
    float x, y;  // Vị trí
    string text; // Nội dung
    int life;    // Thời gian sống (giảm dần rồi biến mất)
};

// ----- NÚT BẤM -----
struct Button
{
    SDL_Rect rect; // Hình chữ nhật (x, y, w, h)
    string text;   // Chữ hiển thị trên nút
    bool hover;    // true = chuột đang di chuột qua
};

// ----- LASER THẲNG -----
struct Laser
{
    float x, y;   // Điểm bắt đầu
    float dx, dy; // Hướng (vector đơn vị)
    int life;     // Thời gian sống
    int level;    // Cấp độ (càng cao càng dày, càng mạnh)
};

// ----- VẬT PHẨM HỒI MÁU (TRÁI TIM) -----
struct HealthDrop
{
    float x, y;     // Vị trí
    float vy;       // Vận tốc rơi
    int value;      // Lượng máu hồi (thường = 1)
    bool active;    // true = còn tồn tại, false = đã dùng
    bool onGround;  // true = đã chạm đất
    int life;       // Thời gian sống còn lại
    int groundTime; // Thời gian đã nằm trên đất
};

// ----- ĐỘ KHÓ -----
struct Difficulty
{
    float enemySpeed;  // Tốc độ enemy
    float fireRate;    // Tần suất bắn (càng nhỏ càng nhanh)
    int enemyCount;    // Số lượng enemy
    float dodgePower;  // Khả năng né đạn
    float bulletSpeed; // Tốc độ đạn
};

// ----- NGƯỜI CHƠI -----
struct Player
{
    float x, y; // Vị trí

    float speed = 5.0f; // Tốc độ di chuyển

    // Dash
    bool dashing = false; // Đang dash?
    int dashTime = 0;     // Thời gian dash còn lại
    int dashCooldown = 0; // Cooldown dash

    int hp = 5; // Máu

    // Vũ khí
    int fireCooldown = 0; // Cooldown bắn
    int fireRate = 10;    // Tốc độ bắn
};

// ----- LASER VÒNG CUNG (CHO BOSS) -----
struct ArcLaser
{
    float x, y;       // Vị trí bắt đầu
    float startAngle; // Góc bắt đầu (radian)
    float endAngle;   // Góc kết thúc (radian)
    float radius;     // Bán kính vòng cung
    int life;         // Thời gian sống
    int level;        // Cấp độ (ảnh hưởng sát thương)
    int direction;    // Hướng quay (1: phải, -1: trái)
};