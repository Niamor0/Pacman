#include "common.hpp"

// ======= COLORS (single definition here) =======
const RGBc BG_COL{0.02f,0.02f,0.02f};
const RGBc WALL_COL{0.95f,0.25f,0.25f};
const RGBc DOT_COL{0.95f,0.95f,0.95f};
const RGBc HUD_COL{1.00f,1.00f,1.00f};
const RGBc PAC_COL{1.00f,0.95f,0.00f};
const RGBc BLINKY_COL{1.00f,0.00f,0.00f};
const RGBc PINKY_COL{1.00f,0.55f,0.90f};
const RGBc INKY_COL{0.00f,1.00f,1.00f};
const RGBc CLYDE_COL{1.00f,0.60f,0.00f};

// ======= GLOBAL STATE =======
Actor pac {9.5f,15.5f,0,0,0.33f};
Actor ghosts[4] = {
    {9.5f,10.5f,0,0,0.33f},
    {7.5f,10.5f,0,0,0.33f},
    {11.5f,10.5f,0,0,0.33f},
    {9.5f, 9.5f,0,0,0.33f}
};

int   pelletsTotal=0, pelletsEaten=0, score=0, lives=3;
bool  paused=false, gameOver=false, winGame=false;
bool  deathActive = false;

Clock::time_point tStart = Clock::now();
static auto tLast       = Clock::now();
static auto tDeathStart = Clock::now();
static const float DEATH_DURATION = 1.0f;

int MAZE[ROWS][COLS];
static const int MAZE_TEMPLATE[ROWS][COLS] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1},
  {1,0,1,1,1,0,0,1,0,1,1,0,1,0,0,1,1,0,1},
  {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
  {1,0,1,0,1,1,0,1,1,1,1,1,1,0,1,0,1,0,1},
  {1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
  {1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1},
  {1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1},
  {1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1},
  {1,0,0,0,1,0,0,0,1,2,1,0,0,0,1,0,0,0,1},
  {1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1},
  {1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
  {1,0,1,0,1,1,0,1,1,1,1,1,1,0,1,0,1,0,1},
  {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
  {1,0,1,1,1,0,0,1,0,1,1,0,1,0,0,1,1,0,1},
  {1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

// ---------- helpers ----------
static inline int  yToRow(float y){ return ROWS-1-(int)std::floor(y); }
static inline int  xToCol(float x){ return (int)std::floor(x); }
static inline bool blockedForPac(int r,int c){
    return (r<0||r>=ROWS||c<0||c>=COLS||MAZE[r][c]==WALL||MAZE[r][c]==GATE);
}
static inline bool blockedForGhostCell(int r,int c){
    return (r<0||r>=ROWS||c<0||c>=COLS||MAZE[r][c]==WALL);
}
static inline float clampf(float v,float lo,float hi){ return v<lo?lo:(v>hi?hi:v); }
static inline float cellCenterX(int c){ return c + 0.5f; }
static inline float cellCenterY(int r){ return (ROWS-1-r) + 0.5f; }
static inline bool atCellCenter(float x,float y,int r,int c,float eps=0.06f){
    return std::fabs(x - cellCenterX(c)) < eps && std::fabs(y - cellCenterY(r)) < eps;
}

// ---------- Super food ----------
struct SuperFood { bool active; int r,c; };
static const int MAX_SUPERS = 3;
static SuperFood supers[MAX_SUPERS];
static const float SUPER_SPAWN_INTERVAL = 10.0f;
static float lastSuperSpawnAt = 0.0f;

// expose read-only counts for renderer
int getActiveSupers(SuperPos out[], int maxN){
    int n=0;
    for(int i=0;i<MAX_SUPERS && n<maxN;i++){
        if(!supers[i].active) continue;
        out[n++] = SuperPos{ supers[i].r, supers[i].c };
    }
    return n;
}

// ---------- Ghost dir ----------
static int gDx[4]={0}, gDy[4]={0};

// ---------- pellets ----------
static void countDots(){
    pelletsTotal=0;
    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) if(MAZE[r][c]==DOTCELL) pelletsTotal++;
}
static void copyMazeFromTemplate(){
    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) MAZE[r][c]=MAZE_TEMPLATE[r][c];
}
static void initGhostDirsRandom(){
    for(int i=0;i<4;i++){
        int d=rand()%4;
        int dx[4]={1,-1,0,0};
        int dy[4]={0,0,1,-1};
        gDx[i]=dx[d]; gDy[i]=dy[d];
    }
}
static void resetSupers(){
    for(int i=0;i<MAX_SUPERS;i++) supers[i] = (SuperFood){false,0,0};
    lastSuperSpawnAt = 0.0f;
}
static void resetActors(){
    pac = {9.5f,15.5f,0,0,0.33f};
    ghosts[0] = {9.5f,10.5f,0,0,0.33f};
    ghosts[1] = {7.5f,10.5f,0,0,0.33f};
    ghosts[2] = {11.5f,10.5f,0,0,0.33f};
    ghosts[3] = {9.5f, 9.5f,0,0,0.33f};
}

// ---------- super spawn/eat ----------
static int countActiveSupersInternal(){ int n=0; for(int i=0;i<MAX_SUPERS;i++) if(supers[i].active) n++; return n; }
static bool isValidSpawnCell(int r,int c){
    if(r<0||r>=ROWS||c<0||c>=COLS) return false;
    if(MAZE[r][c]==WALL || MAZE[r][c]==GATE) return false;
    int rp=yToRow(pac.y), cp=xToCol(pac.x);
    if(r==rp && c==cp) return false;
    for(int i=0;i<4;i++){
        int rg=yToRow(ghosts[i].y), cg=xToCol(ghosts[i].x);
        if(r==rg && c==cg) return false;
    }
    for(int i=0;i<MAX_SUPERS;i++){
        if(supers[i].active && supers[i].r==r && supers[i].c==c) return false;
    }
    return true;
}
static void spawnOneSuper(){
    for(int tries=0; tries<200; ++tries){
        int r = rand()%ROWS, c = rand()%COLS;
        if(!isValidSpawnCell(r,c)) continue;
        for(int i=0;i<MAX_SUPERS;i++){
            if(!supers[i].active){ supers[i] = (SuperFood){true,r,c}; return; }
        }
        return;
    }
}
static void maybeSpawnSupers(float elapsed){
    if(elapsed - lastSuperSpawnAt >= SUPER_SPAWN_INTERVAL){
        lastSuperSpawnAt = elapsed;
        if(countActiveSupersInternal() < MAX_SUPERS) spawnOneSuper();
    }
}
static void checkEatSuper(){
    for(int i=0;i<MAX_SUPERS;i++){
        if(!supers[i].active) continue;
        float cx = cellCenterX(supers[i].c);
        float cy = cellCenterY(supers[i].r);
        float dx = pac.x - cx, dy = pac.y - cy;
        float rr = (pac.radius + 0.30f);
        if(dx*dx + dy*dy <= rr*rr){ supers[i].active = false; score += 100; }
    }
}

// ---------- pellets/scores ----------
static void eatPellet(){
    int r=yToRow(pac.y), c=xToCol(pac.x);
    if(MAZE[r][c]==DOTCELL){MAZE[r][c]=EMPTY;pelletsEaten++;score+=10;}
    if(pelletsEaten==pelletsTotal){winGame=true;paused=true;}
}

// ---------- death flow ----------
static void triggerDeath(){
    if(deathActive || gameOver || winGame) return;
    deathActive = true; tDeathStart = Clock::now();
    pac.vx = pac.vy = 0.0f;
}
static void finalizeDeath(){
    lives -= 1;
    if(lives <= 0){ gameOver = true; paused = true; deathActive = false; return; }
    resetActors(); initGhostDirsRandom(); deathActive = false;
}

// ---------- movement/AI ----------
static void worldToNextCell(int r,int c,int dx,int dy,int& nr,int& nc){
    nr = r; nc = c;
    if(dx>0)  nc = c+1;
    if(dx<0)  nc = c-1;
    if(dy>0)  nr = r-1;
    if(dy<0)  nr = r+1;
}
static bool ghostCanGo(int r,int c,int dx,int dy){
    int nr,nc; worldToNextCell(r,c,dx,dy,nr,nc); return !blockedForGhostCell(nr,nc);
}
static void chooseGhostDirWithChase(int i,int r,int c){
    int curDx=gDx[i], curDy=gDy[i];
    struct D{int dx,dy;}; D dirs[4]={{1,0},{-1,0},{0,1},{0,-1}};
    float bestScore = 1e9f; int bestDx=0,bestDy=0;
    for(auto d:dirs){
        if(d.dx==-curDx && d.dy==-curDy) continue;
        if(!ghostCanGo(r,c,d.dx,d.dy)) continue;
        int nr,nc; worldToNextCell(r,c,d.dx,d.dy,nr,nc);
        float nx=cellCenterX(nc), ny=cellCenterY(nr);
        float score = std::fabs(nx - pac.x) + std::fabs(ny - pac.y) + (rand()%100)*0.001f;
        if(score<bestScore){ bestScore=score; bestDx=d.dx; bestDy=d.dy; }
    }
    if(bestDx||bestDy){ gDx[i]=bestDx; gDy[i]=bestDy; return; }
    for(auto d:dirs){ if(ghostCanGo(r,c,d.dx,d.dy)){ gDx[i]=d.dx; gDy[i]=d.dy; return; } }
    gDx[i]=0; gDy[i]=0;
}
static void updatePac(float dt){
    float nx=pac.x+pac.vx*PAC_SPEED*dt, ny=pac.y+pac.vy*PAC_SPEED*dt;
    if(!blockedForPac(yToRow(pac.y),xToCol(nx))) pac.x=clampf(nx,0.5f,COLS-0.5f);
    if(!blockedForPac(yToRow(ny),xToCol(pac.x))) pac.y=clampf(ny,0.5f,ROWS-0.5f);
    eatPellet(); checkEatSuper();
}
static void updateGhosts(float dt){
    float elapsed=std::chrono::duration<float>(Clock::now()-tStart).count();
    float gs=GHOST_SPEED0+(int(elapsed)/STEP_EVERY_S)*GHOST_STEP;
    const float GHOST_MAX = PAC_SPEED - 0.4f;
    if(gs > GHOST_MAX) gs = GHOST_MAX;

    for(int i=0;i<4;i++){
        int r=yToRow(ghosts[i].y), c=xToCol(ghosts[i].x);
        if(gDx[i]!=0) ghosts[i].y = cellCenterY(r);
        if(gDy[i]!=0) ghosts[i].x = cellCenterX(c);
        if(atCellCenter(ghosts[i].x, ghosts[i].y, r, c)){
            chooseGhostDirWithChase(i,r,c);
            int nr,nc; worldToNextCell(r,c,gDx[i],gDy[i],nr,nc);
            if(blockedForGhostCell(nr,nc)){ gDx[i]=gDy[i]=0; }
        }
        ghosts[i].x = clampf(ghosts[i].x + gDx[i]*gs*dt, 0.5f, COLS-0.5f);
        ghosts[i].y = clampf(ghosts[i].y + gDy[i]*gs*dt, 0.5f, ROWS-0.5f);
        float dx=ghosts[i].x-pac.x, dy=ghosts[i].y-pac.y;
        float rr = (ghosts[i].radius+pac.radius-0.04f);
        if(dx*dx+dy*dy < rr*rr){ triggerDeath(); return; }
    }
}

// ---------- API ----------
void startNewGame(){
    paused=false; gameOver=false; winGame=false; deathActive=false;
    score=0; pelletsEaten=0; lives=3;
    copyMazeFromTemplate(); countDots();
    resetActors(); initGhostDirsRandom(); resetSupers();
    tStart=Clock::now(); tLast=tStart;
}
void step(){
    auto now=Clock::now();
    float dt=std::chrono::duration<float>(now-tLast).count();
    tLast=now;

    if(deathActive){
        float tSince = std::chrono::duration<float>(now - tDeathStart).count();
        if(tSince >= DEATH_DURATION) finalizeDeath();
        glutPostRedisplay(); glutTimerFunc(16,[](int){step();},0); return;
    }
    if(paused){ glutPostRedisplay(); glutTimerFunc(16,[](int){step();},0); return; }

    float elapsed=std::chrono::duration<float>(now - tStart).count();
    maybeSpawnSupers(elapsed);

    updatePac(dt);
    updateGhosts(dt);

    glutPostRedisplay();
    glutTimerFunc(16,[](int){step();},0);
}
void specialKey(int key,int,int){
    if(gameOver||winGame||deathActive) return;
    if(key==GLUT_KEY_UP){   pac.vx=0;  pac.vy=+1; }
    if(key==GLUT_KEY_DOWN){ pac.vx=0;  pac.vy=-1; }
    if(key==GLUT_KEY_LEFT){ pac.vx=-1; pac.vy=0;  }
    if(key==GLUT_KEY_RIGHT){pac.vx=+1; pac.vy=0;  }
}
void keyDown(unsigned char k,int,int){
    if(k==27) exit(0);
    if(k=='p'||k=='P'){ if(!deathActive && !gameOver && !winGame) { paused=!paused; glutPostRedisplay(); } }
    if(k=='r'||k=='R'){ startNewGame(); }
}
