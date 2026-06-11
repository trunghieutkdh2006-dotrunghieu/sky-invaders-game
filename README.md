Đối với máy macbook: main.cpp - vscode - terminal \
Kiểm tra xem có các file: ls

Câu lệnh: \
clang++ -std=c++17 main.cpp audio.cpp bullet.cpp enemy.cpp gamestate.cpp powerup.cpp render.cpp \
-o game \
-I/opt/homebrew/include \
-L/opt/homebrew/lib \
-lSDL2 \
-lSDL2_image \
-lSDL2_ttf \
-lSDL2_mixer
