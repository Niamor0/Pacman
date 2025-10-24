#include "common.hpp"

// ---------- tiny draw helpers ----------
static void drawTextColor(const char* s,float x,float y,float r,float g,float b,void* font){
    glColor3f(r,g,b); glRasterPos2f(x,y); while(*s) glutBitmapCharacter(font,*s++);
}
static void drawQuad(float x0,float y0,float x1,float y1, RGBc c){
    glColor3f(c.r,c.g,c.b); glBegin(GL_QUADS);
    glVertex2f(x0,y0); glVertex2f(x1,y0); glVertex2f(x1,y1); glVertex2f(x0,y1); glEnd();
}
static void drawCircle(float cx,float cy,float r, RGBc c){
    glColor3f(c.r,c.g,c.b); glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx,cy);
    for(int i=0;i<=40;i++){ float a=i*0.157f; glVertex2f(cx+r*cos(a), cy+r*sin(a)); }
    glEnd();
}
static float cellCenterXLocal(int c){ return c + 0.5f; }
static float cellCenterYLocal(int r){ return (ROWS-1-r) + 0.5f; }

// ---------- sprites ----------
static void drawWatermelon(float cx,float cy){
    const float PI = 3.14159265f;
    const float R  = 0.35f;
    float Rw = R, Rw_in = R - 0.05f, Rw_white_in = Rw_in - 0.03f, Rf = Rw_white_in;

    glColor3f(1.0f, 0.15f, 0.20f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for(int i=0;i<=64;i++){ float a=(PI*i)/64.0f; glVertex2f(cx + Rf*cos(a), cy - Rf*sin(a)); }
    glEnd();

    glColor3f(0.98f, 0.98f, 0.98f);
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0;i<=64;i++){ float a=(PI*i)/64.0f;
        glVertex2f(cx + Rw_in*cos(a),       cy - Rw_in*sin(a));
        glVertex2f(cx + Rw_white_in*cos(a), cy - Rw_white_in*sin(a));
    } glEnd();

    glColor3f(0.10f, 0.70f, 0.20f);
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0;i<=64;i++){ float a=(PI*i)/64.0f;
        glVertex2f(cx + Rw*cos(a),     cy - Rw*sin(a));
        glVertex2f(cx + Rw_in*cos(a),  cy - Rw_in*sin(a));
    } glEnd();

    RGBc seedCol{0.05f,0.05f,0.05f};
    float sr=0.03f;
    drawCircle(cx - 0.16f, cy - 0.18f, sr, seedCol);
    drawCircle(cx + 0.00f, cy - 0.22f, sr, seedCol);
    drawCircle(cx + 0.16f, cy - 0.18f, sr, seedCol);
    drawCircle(cx - 0.08f, cy - 0.28f, sr, seedCol);
    drawCircle(cx + 0.08f, cy - 0.28f, sr, seedCol);
}
static void drawPacRightSpriteLocal(float r){
    const float TWO_PI = 6.283185307f;
    const float mouthDeg = 58.0f;
    const float mouth = mouthDeg * 3.14159265f/180;

    glColor3f(PAC_COL.r, PAC_COL.g, PAC_COL.b);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0.0f, 0.0f);
        for(int i=0;i<=100;i++){
            float a = mouth*0.5f + (TWO_PI - mouth) * (i/100.0f);
            glVertex2f(r*cosf(a), r*sinf(a));
        }
    glEnd();

    glColor3f(0.0f,0.0f,0.0f);
    glBegin(GL_TRIANGLES);
        float a0 = -mouth*0.5f, a1 = +mouth*0.5f;
        glVertex2f(0.0f,0.0f);
        glVertex2f(r*cosf(a0), r*sinf(a0));
        glVertex2f(r*cosf(a1), r*sinf(a1));
    glEnd();

    RGBc eyeW{1,1,1}, eyeB{0.10f,0.65f,1.0f};
    float ew = r*0.23f, ep = r*0.12f;
    drawCircle(r*0.20f, r*0.25f, ew, eyeW);
    drawCircle(r*0.27f, r*0.25f, ep, eyeB);

    RGBc lip{0.85f,0.10f,0.10f};
    drawCircle(r*0.88f, -r*0.08f, r*0.06f, lip);

    RGBc bow{0.90f,0.10f,0.10f};
    glColor3f(bow.r,bow.g,bow.b);
    glBegin(GL_TRIANGLES);
        glVertex2f(-r*0.15f, +r*0.60f);
        glVertex2f(-r*0.55f, +r*0.45f);
        glVertex2f(-r*0.35f, +r*0.80f);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertex2f(-r*0.05f, +r*0.62f);
        glVertex2f(+r*0.25f, +r*0.78f);
        glVertex2f(+r*0.05f, +r*0.40f);
    glEnd();
    drawCircle(+r*0.02f, +r*0.58f, r*0.10f, bow);
}
static void drawGhostSprite(float cx, float cy, float r, RGBc bodyCol){
    const float PI = 3.14159265f;
    float w = r*2.0f, h = r*2.2f, halfW = w*0.5f;

    glColor3f(bodyCol.r, bodyCol.g, bodyCol.b);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy + h*0.15f);
        for(int i=0;i<=64;i++){
            float a = PI * (i/64.0f);
            glVertex2f(cx + halfW*cosf(a), cy + h*0.15f + halfW*sinf(a));
        }
    glEnd();

    glBegin(GL_QUADS);
        glVertex2f(cx - halfW, cy - h*0.55f);
        glVertex2f(cx + halfW, cy - h*0.55f);
        glVertex2f(cx + halfW, cy + h*0.15f);
        glVertex2f(cx - halfW, cy + h*0.15f);
    glEnd();

    float bumpR = w/6.0f; float startX = cx - halfW + bumpR;
    for(int k=0;k<3;k++){
        float bx = startX + k*(2*bumpR);
        glBegin(GL_TRIANGLE_FAN);
            glVertex2f(bx, cy - h*0.55f);
            for(int i=0;i<=32;i++){
                float a = 3.14159265f + 3.14159265f*(i/32.0f);
                glVertex2f(bx + bumpR*cosf(a), cy - h*0.55f + bumpR*sinf(a));
            }
        glEnd();
    }

    RGBc eyeW{1,1,1}, eyeB{0.10f,0.65f,1.0f};
    float eyeR = r*0.28f, pupilR = r*0.16f;
    float lcx = cx - r*0.35f, rcx = cx + r*0.15f, ey = cy + r*0.25f;
    drawCircle(lcx, ey, eyeR, eyeW); drawCircle(rcx, ey, eyeR, eyeW);
    drawCircle(lcx - r*0.10f, ey, pupilR, eyeB);
    drawCircle(rcx - r*0.10f, ey, pupilR, eyeB);
}

// ---------- scene render ----------
static void renderMaze(){
    glClearColor(BG_COL.r,BG_COL.g,BG_COL.b,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    for(int r=0;r<ROWS;r++)
        for(int c=0;c<COLS;c++){
            float x=c,y=(ROWS-1-r);
            if(MAZE[r][c]==WALL||MAZE[r][c]==GATE) drawQuad(x+0.06f,y+0.06f,x+0.94f,y+0.94f,WALL_COL);
            else if(MAZE[r][c]==DOTCELL)           drawCircle(x+0.5f,y+0.5f,0.08f,DOT_COL);
        }
}
static void renderActors(){
    glPushMatrix();
    glTranslatef(pac.x, pac.y, 0.0f);
    float extraFlip = (deathActive ? 180.0f : 0.0f);
    glRotatef(extraFlip, 0.0f, 0.0f, 1.0f);
    drawPacRightSpriteLocal(pac.radius * 1.15f);
    glPopMatrix();

    RGBc gc[4]={BLINKY_COL,PINKY_COL,INKY_COL,CLYDE_COL};
    for(int i=0;i<4;i++){
        drawGhostSprite(ghosts[i].x, ghosts[i].y, ghosts[i].radius * 1.05f, gc[i]);
    }
}
static void renderSupers(){
    SuperPos buf[8];
    int n = getActiveSupers(buf, 8);
    for(int i=0;i<n;i++){
        float cx = cellCenterXLocal(buf[i].c);
        float cy = cellCenterYLocal(buf[i].r);
        drawWatermelon(cx, cy);
    }
}
static void renderHUD(){
    char buf[128];
    float secs=std::chrono::duration<float>(Clock::now()-tStart).count();
    std::snprintf(buf,sizeof(buf),"SCORE:%d  LIVES:%d  TIME:%.1fs",score,lives,secs);
    drawTextColor(buf,0.6f,ROWS+0.3f,HUD_COL.r,HUD_COL.g,HUD_COL.b,GLUT_BITMAP_9_BY_15);
    if(paused && !winGame && !gameOver) drawTextColor("PAUSED",8,10,1,1,1,GLUT_BITMAP_HELVETICA_18);
    if(winGame)  drawTextColor("YOU WIN!",8,10,1,1,0,GLUT_BITMAP_HELVETICA_18);
}

void renderGame(){
    renderMaze();
    renderActors();
    renderSupers();
    renderHUD();
    glutSwapBuffers();
}

void reshape(int w,int h){
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0,COLS,0,ROWS+1.2);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}
