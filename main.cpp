#include "raylib.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr int screenWidth = 800;
constexpr int screenHeight = 450;
constexpr int winningScore = 5;
constexpr float paddleWidth = 14.0f;
constexpr float paddleHeight = 90.0f;
constexpr float paddleInset = 30.0f;
constexpr float ballRadius = 9.0f;
constexpr Color backgroundColor = {10, 14, 30, 255};
constexpr Color panelColor = {20, 27, 49, 255};
constexpr Color cyanColor = {72, 229, 255, 255};
constexpr Color pinkColor = {255, 91, 155, 255};
constexpr Color yellowColor = {255, 220, 105, 255};
constexpr Color mutedColor = {150, 166, 195, 255};

enum class GameState {
    Menu,
    Playing,
    Finished
};

struct Paddle {
    Rectangle bounds;
    float speed;
};

struct Ball {
    Vector2 position;
    Vector2 velocity;
    float speed;
};

void CenterText(const char* text, int y, int fontSize, Color color) {
    DrawText(text, (screenWidth - MeasureText(text, fontSize)) / 2, y, fontSize, color);
}

void DrawBackdrop(float time) {
    ClearBackground(backgroundColor);
    DrawCircleGradient(92, 90, 150, Fade(cyanColor, 0.09f), Fade(backgroundColor, 0.0f));
    DrawCircleGradient(screenWidth - 72, screenHeight - 42, 180, Fade(pinkColor, 0.08f),
                       Fade(backgroundColor, 0.0f));

    for (int x = 20; x < screenWidth; x += 40) {
        for (int y = 20; y < screenHeight; y += 40) {
            DrawCircle(x, y, 1.0f, Fade(mutedColor, 0.12f));
        }
    }

    DrawRectangle(0, 0, screenWidth, 4, Fade(cyanColor, 0.8f));
    DrawRectangle(0, screenHeight - 4, screenWidth, 4, Fade(pinkColor, 0.8f));
    DrawCircle(70, 370, 3.0f + std::sin(time * 2.0f) * 1.0f, Fade(cyanColor, 0.75f));
    DrawCircle(730, 85, 3.0f + std::sin(time * 2.0f + 1.0f) * 1.0f, Fade(pinkColor, 0.75f));
}

void ResetBall(Ball& ball, int direction) {
    ball.position = {screenWidth / 2.0f, screenHeight / 2.0f};
    ball.speed = 360.0f;
    ball.velocity = {ball.speed * static_cast<float>(direction), 0.0f};
}

void ResetMatch(Paddle& player, Paddle& computer, Ball& ball, int& playerScore, int& computerScore) {
    player.bounds = {paddleInset, (screenHeight - paddleHeight) / 2.0f, paddleWidth, paddleHeight};
    computer.bounds = {screenWidth - paddleInset - paddleWidth, (screenHeight - paddleHeight) / 2.0f,
                       paddleWidth, paddleHeight};
    player.speed = 420.0f;
    computer.speed = 320.0f;
    playerScore = 0;
    computerScore = 0;
    ResetBall(ball, 1);
}

void MovePaddle(Paddle& paddle, float direction, float deltaTime) {
    paddle.bounds.y += direction * paddle.speed * deltaTime;
    paddle.bounds.y = std::clamp(paddle.bounds.y, 0.0f, screenHeight - paddleHeight);
}

void BounceFromPaddle(Ball& ball, const Paddle& paddle, int direction) {
    const float paddleCenter = paddle.bounds.y + paddleHeight / 2.0f;
    const float hitOffset = (ball.position.y - paddleCenter) / (paddleHeight / 2.0f);
    const float angle = hitOffset * 1.05f;

    ball.speed = std::min(ball.speed + 20.0f, 600.0f);
    ball.velocity = {std::cos(angle) * ball.speed * static_cast<float>(direction),
                     std::sin(angle) * ball.speed};
    ball.position.x = direction > 0
        ? paddle.bounds.x + paddleWidth + ballRadius
        : paddle.bounds.x - ballRadius;
}

void DrawGame(const Paddle& player, const Paddle& computer, const Ball& ball,
              int playerScore, int computerScore, float time) {
    DrawBackdrop(time);

    for (int y = 0; y < screenHeight; y += 30) {
        DrawRectangle(screenWidth / 2 - 2, y, 4, 16, Fade(mutedColor, 0.35f));
    }

    DrawRectangleRounded({player.bounds.x - 3, player.bounds.y - 3, paddleWidth + 6, paddleHeight + 6},
                         0.45f, 8, Fade(cyanColor, 0.22f));
    DrawRectangleRounded({computer.bounds.x - 3, computer.bounds.y - 3, paddleWidth + 6, paddleHeight + 6},
                         0.45f, 8, Fade(pinkColor, 0.22f));
    DrawRectangleRounded(player.bounds, 0.45f, 8, cyanColor);
    DrawRectangleRounded(computer.bounds, 0.45f, 8, pinkColor);
    DrawCircleV(ball.position, ballRadius + 7.0f, Fade(yellowColor, 0.18f));
    DrawCircleV(ball.position, ballRadius, yellowColor);

    DrawText(TextFormat("%i", playerScore), screenWidth / 2 - 80, 30, 40, cyanColor);
    DrawText(TextFormat("%i", computerScore), screenWidth / 2 + 55, 30, 40, pinkColor);
    DrawText("PLAYER 1  |  W / S", 30, 15, 16, mutedColor);
    DrawText("CPU", screenWidth - 66, 15, 16, mutedColor);
    DrawText("FIRST TO 5", screenWidth / 2 - MeasureText("FIRST TO 5", 14) / 2, 88, 14, mutedColor);
}
}

int main() {
    InitWindow(screenWidth, screenHeight, "Pong");
    SetTargetFPS(60);

    Paddle player{};
    Paddle computer{};
    Ball ball{};
    int playerScore = 0;
    int computerScore = 0;
    GameState state = GameState::Menu;
    ResetMatch(player, computer, ball, playerScore, computerScore);

    while (!WindowShouldClose()) {
        const float deltaTime = std::min(GetFrameTime(), 0.05f);

        if (state == GameState::Menu) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                ResetMatch(player, computer, ball, playerScore, computerScore);
                state = GameState::Playing;
            }
        } else if (state == GameState::Playing) {
            float playerDirection = 0.0f;
            if (IsKeyDown(KEY_W)) playerDirection -= 1.0f;
            if (IsKeyDown(KEY_S)) playerDirection += 1.0f;
            MovePaddle(player, playerDirection, deltaTime);

            float computerDirection = 0.0f;
            if (ball.velocity.x > 0.0f) {
                const float computerCenter = computer.bounds.y + paddleHeight / 2.0f;
                if (ball.position.y < computerCenter - 10.0f) computerDirection = -1.0f;
                if (ball.position.y > computerCenter + 10.0f) computerDirection = 1.0f;
            } else {
                const float computerCenter = computer.bounds.y + paddleHeight / 2.0f;
                if (computerCenter < screenHeight / 2.0f - 5.0f) computerDirection = 1.0f;
                if (computerCenter > screenHeight / 2.0f + 5.0f) computerDirection = -1.0f;
            }
            MovePaddle(computer, computerDirection, deltaTime);

            ball.position.x += ball.velocity.x * deltaTime;
            ball.position.y += ball.velocity.y * deltaTime;

            if (ball.position.y - ballRadius <= 0.0f && ball.velocity.y < 0.0f) {
                ball.position.y = ballRadius;
                ball.velocity.y *= -1.0f;
            } else if (ball.position.y + ballRadius >= screenHeight && ball.velocity.y > 0.0f) {
                ball.position.y = screenHeight - ballRadius;
                ball.velocity.y *= -1.0f;
            }

            if (ball.velocity.x < 0.0f &&
                CheckCollisionCircleRec(ball.position, ballRadius, player.bounds)) {
                BounceFromPaddle(ball, player, 1);
            } else if (ball.velocity.x > 0.0f &&
                       CheckCollisionCircleRec(ball.position, ballRadius, computer.bounds)) {
                BounceFromPaddle(ball, computer, -1);
            }

            if (ball.position.x + ballRadius < 0.0f) {
                ++computerScore;
                if (computerScore == winningScore) {
                    state = GameState::Finished;
                } else {
                    ResetBall(ball, -1);
                }
            } else if (ball.position.x - ballRadius > screenWidth) {
                ++playerScore;
                if (playerScore == winningScore) {
                    state = GameState::Finished;
                } else {
                    ResetBall(ball, 1);
                }
            }
        } else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_R)) {
            ResetMatch(player, computer, ball, playerScore, computerScore);
            state = GameState::Playing;
        }

        BeginDrawing();
        if (state == GameState::Menu) {
            DrawBackdrop(static_cast<float>(GetTime()));
            DrawRectangleRounded({145, 62, 510, 330}, 0.08f, 12, Fade(panelColor, 0.91f));
            DrawRectangleRoundedLines({145, 62, 510, 330}, 0.08f, 12, Fade(mutedColor, 0.28f));
            CenterText("NEON PONG", 105, 48, WHITE);
            CenterText("A CLASSIC ARCADE DUEL", 163, 16, cyanColor);
            DrawRectangleRounded({235, 210, 330, 48}, 0.35f, 8, Fade(backgroundColor, 0.8f));
            DrawCircle(265, 234, 6, cyanColor);
            DrawCircle(535, 234, 6, pinkColor);
            CenterText("W / S  MOVE YOUR PADDLE", 223, 18, WHITE);
            CenterText("Challenge the computer. First to 5 wins.", 280, 18, mutedColor);
            DrawRectangleRounded({285, 328, 230, 42}, 0.35f, 8, Fade(cyanColor, 0.16f));
            DrawRectangleRoundedLines({285, 328, 230, 42}, 0.35f, 8, Fade(cyanColor, 0.75f));
            CenterText("PRESS ENTER TO PLAY", 340, 16, WHITE);
        } else {
            DrawGame(player, computer, ball, playerScore, computerScore, static_cast<float>(GetTime()));
            if (state == GameState::Finished) {
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(backgroundColor, 0.78f));
                DrawRectangleRounded({170, 112, 460, 230}, 0.08f, 12, Fade(panelColor, 0.97f));
                DrawRectangleRoundedLines({170, 112, 460, 230}, 0.08f, 12, Fade(mutedColor, 0.35f));
                CenterText(playerScore == winningScore ? "YOU WIN!" : "CPU WINS!", 153, 38,
                           playerScore == winningScore ? cyanColor : pinkColor);
                CenterText(TextFormat("%i  -  %i", playerScore, computerScore), 211, 30, WHITE);
                CenterText("PRESS ENTER OR R TO PLAY AGAIN", 278, 17, mutedColor);
            }
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
