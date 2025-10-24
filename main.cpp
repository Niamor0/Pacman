#include "common.hpp"

static void display() { renderGame(); }

int main(int argc, char** argv) {
    srand((unsigned)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(760, 820);
    glutCreateWindow("PAC-MAN — GLUT (R=Restart, P=Pause, Esc=Quit)");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKey);
    glutKeyboardFunc(keyDown);

    startNewGame();
    glutTimerFunc(16, [](int){ step(); }, 0);
    glutMainLoop();
    return 0;
}
