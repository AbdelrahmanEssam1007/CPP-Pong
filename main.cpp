#include <iostream>
#include <memory>
#include <raylib.h>

using std::cout, std::endl, std::cin, std::string;

int p1Score = 0;
int p2Score = 0;


class Ball {
 public:
  float x, y;
  float radius;
  int xSpeed, ySpeed;

  Ball() {
    radius = 20.0f;
    x = static_cast<float>(GetScreenWidth()) / 2.0f;
    y = static_cast<float>(GetScreenHeight()) / 2.0f;
    xSpeed = 7;
    ySpeed = 7;
  }

  void Draw() const {
    DrawCircle(static_cast<int>(x), static_cast<int>(y), radius, WHITE);
  }

  void Reset() {
    x = static_cast<float>(GetScreenWidth()) / 2.0f;
    y = static_cast<float>(GetScreenHeight()) / 2.0f;
    constexpr int Direction[2] = {1, -1};
    xSpeed *= Direction[GetRandomValue(0, 1)];
    ySpeed *= Direction[GetRandomValue(0, 1)];
  }

  void Move() {
    x += static_cast<float>(xSpeed);
    y += static_cast<float>(ySpeed);

    if (y + radius >= static_cast<float>(GetScreenHeight()) || y - radius <= 0.0f) {
      ySpeed *= -1;
    }

    if (x - radius <= 0.0f) {
      p2Score++;
      Reset();
    }

    if (x + radius >= static_cast<float>(GetScreenWidth())) {
      p1Score++;
      Reset();
    }
  }
};

class Paddle {
 public:
  virtual ~Paddle() = default;
  float x, y;
  int width, height;
  int speed;

  Paddle(const float startX, const float startY) {
    width = 20;
    height = 100;
    x = startX;
    y = startY;
    speed = 9;
  }

  virtual void Move() {
  }

  void Draw() const {
    DrawRectangle(static_cast<int>(x), static_cast<int>(y), width, height, WHITE);
  }
};

// Player 1 Paddle (Arrow Key Movement)
class PlayerPaddle final : public Paddle {
 public:
  PlayerPaddle(const float startX, const float startY) : Paddle(startX, startY) {
  }

  void Move() override {
    if (IsKeyDown(KEY_UP) && y > 0.0f) {
      y -= static_cast<float>(speed);
    }
    if (IsKeyDown(KEY_DOWN) && y + static_cast<float>(height) < static_cast<float>(GetScreenHeight())) {
      y += static_cast<float>(speed);
    }
  }
};

// Player 2 Paddle (W/S Movement)
class p2Paddle final : public Paddle {
 public:
  p2Paddle(const float startX, const float startY) : Paddle(startX, startY) {
  }

  void Move() override {
    if (IsKeyDown(KEY_W) && y > 0.0f) {
      y -= static_cast<float>(speed);
    }
    if (IsKeyDown(KEY_S) && y + static_cast<float>(height) < static_cast<float>(GetScreenHeight())) {
      y += static_cast<float>(speed);
    }
  }
};

class CPUPaddle final : public Paddle {
 public:
  CPUPaddle(const float startX, const float startY) : Paddle(startX, startY) {
  }

  void Move(const int ball_y) {
    if (y + static_cast<float>(height) / 2.0f < static_cast<float>(ball_y) &&
        y + static_cast<float>(height) < static_cast<float>(GetScreenHeight())) {
      y += static_cast<float>(speed) * 0.65f;
    }
    if (y + static_cast<float>(height) / 2.0f > static_cast<float>(ball_y) && y > 0.0f) {
      y -= static_cast<float>(speed) * 0.65f;
    }
  }
};

enum GameState { MENU, PLAYING };
void DecideWinner(GameState& gameState) {
  if (p1Score == 5 || p2Score == 5) {
    while (!IsKeyPressed(KEY_R)) {
      if (IsKeyPressed(KEY_ESCAPE) || WindowShouldClose()) {
        exit(0);
      }
      BeginDrawing();
      ClearBackground(BLACK);

      const char* winnerText = (p1Score == 5) ? "Player 1 Wins!" : (p2Score == 5) ? "Player 2 Wins!" : "";

      if (*winnerText != '\0') {
        DrawText(winnerText, GetScreenWidth() / 2 - MeasureText(winnerText, 40) / 2, GetScreenHeight() / 2 - 40, 40,
                 WHITE);
      }

      DrawText("Press R to restart", GetScreenWidth() / 2 - MeasureText("Press R to restart", 20) / 2,
               GetScreenHeight() / 2 + 40, 20, WHITE);

      EndDrawing();
    }

    gameState = MENU;  // Change game state to MENU
    p1Score = 0;  // Reset scores
    p2Score = 0;
  }
}

int main() {
  constexpr int screenWidth = 900;
  constexpr int screenHeight = 600;
  InitWindow(screenWidth, screenHeight, "Pong");
  SetTargetFPS(60);

  GameState gameState = MENU;
  int gameMode = 0;
  bool isPaused = false;  // Flag to control pause state

  Ball ball;
  PlayerPaddle pad(static_cast<float>(screenWidth) - 40.0f, static_cast<float>(screenHeight) / 2.0f - 50.0f);
  std::unique_ptr<Paddle> opponent = nullptr;

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    if (gameState == MENU) {
      struct MenuOption {
        const char* text;
        int y;
        int fontSize;
        Color color;
      };

      MenuOption options[] = {{"PONG", 50, 80, WHITE},
                              {"1 - Play against CPU", 200, 30, WHITE},
                              {"2 - 2 Player Mode", 250, 30, WHITE},
                              {"Press 1 or 2 to select", 350, 20, GRAY}};

      for (const auto& [text, y, fontSize, color] : options) {
        const int textWidth = MeasureText(text, fontSize);
        DrawText(text, (screenWidth - textWidth) / 2, y, fontSize, color);
      }

      if (IsKeyPressed(KEY_ONE)) {
        gameMode = 1;
        opponent = std::make_unique<CPUPaddle>(20.0f, static_cast<float>(screenHeight) / 2.0f - 50.0f);
        gameState = PLAYING;
      }
      if (IsKeyPressed(KEY_TWO)) {
        gameMode = 2;
        opponent = std::make_unique<p2Paddle>(20.0f, static_cast<float>(screenHeight) / 2.0f - 50.0f);
        gameState = PLAYING;
      }
    }
    else if (gameState == PLAYING) {
      DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);

      // Movement
      ball.Move();
      pad.Move();
      if (gameMode == 1) {
        if (auto* cpuPaddle = dynamic_cast<CPUPaddle*>(opponent.get())) {
          cpuPaddle->Move(static_cast<int>(ball.y));
        }
      }
      else {
        opponent->Move();
      }

      // Collision
      if (CheckCollisionCircleRec(
            Vector2{ball.x, ball.y}, ball.radius,
            Rectangle{pad.x, pad.y, static_cast<float>(pad.width), static_cast<float>(pad.height)})) {
        ball.x = pad.x - ball.radius;  // Move ball back to prevent multiple collisions
        ball.xSpeed *= -1;
      }

      if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius,
                                  Rectangle{opponent->x, opponent->y, static_cast<float>(opponent->width),
                                            static_cast<float>(opponent->height)})) {
        ball.x = opponent->x + static_cast<float>(opponent->width) +
          ball.radius;  // Move ball back to prevent multiple collisions
        ball.xSpeed *= -1;
      }

      // Drawing
      ball.Draw();
      pad.Draw();
      opponent->Draw();
      DrawText(TextFormat("%i", p1Score), screenWidth / 4 - 50, 20, 80, WHITE);
      DrawText(TextFormat("%i", p2Score), screenWidth / 4 * 3 - 50, 20, 80, WHITE);

      DecideWinner(gameState);
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
