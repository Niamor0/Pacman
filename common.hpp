#pragma once
#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <algorithm>
#include <ctime>
#include <cstring>

using Clock = std::chrono::steady_clock;

// ================== CONFIG ==================
static const int   COLS = 19;
static const int   ROWS = 21;
static const float PAC_SPEED    = 4.2f;
static const float GHOST_SPEED0 = 3.0f;
static const float GHOST_STEP   = 0.35f;
static const int   STEP_EVERY_S = 15;

// ================== BASIC TYPES ==================
struct RGBc { float r,g,b; };
struct Actor { float x, y, vx, vy, radius; };

enum Cell { WALL=1, DOTCELL=0, EMPTY=-1, GATE=2 };

// ================== COLORS (defined in logic.cpp) ==================
extern const RGBc BG_COL;
extern const RGBc WALL_COL;
extern const RGBc DOT_COL;
extern const RGBc HUD_COL;
extern const RGBc PAC_COL;
extern const RGBc BLINKY_COL;
extern const RGBc PINKY_COL;
extern const RGBc INKY_COL;
extern const RGBc CLYDE_COL;

// ================== SHARED GAME STATE (defined in logic.cpp) ==================
extern Actor pac;
extern Actor ghosts[4];
extern int   MAZE[ROWS][COLS];
extern int   pelletsTotal, pelletsEaten, score, lives;
extern bool  paused, gameOver, winGame;
extern bool  deathActive;
extern Clock::time_point tStart;

// ============= Read-only accessor for SuperFood positions (for renderer) =============
struct SuperPos { int r, c; };
// Writes up to maxN active super positions into out[]; returns count written.
int getActiveSupers(SuperPos out[], int maxN);

// ================== RENDER (friend owns in render.cpp) ==================
void renderGame();
void reshape(int w,int h);

// ================== LOGIC (you own in logic.cpp) ==================
void startNewGame();
void step();
void specialKey(int key,int,int);
void keyDown(unsigned char k,int,int);
