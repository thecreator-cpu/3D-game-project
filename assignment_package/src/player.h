#ifndef PLAYER_H
#define PLAYER_H

#include <la.h>
#include <camera.h>
#include <QKeyEvent>

class Player
{
public:
    friend class MyGL;
    Player();
    Player(Camera* cam);
    Player(glm::vec3 pos, Camera* cam);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void readUserInput();
private:
    glm::vec3 pos; // world position
    glm::vec3 vel; // velocity
    float maxVel;  // maximum velocity
    float baseAcc;
    float acc; // acceleration for maneuvers
    Camera* cam;

    // store key/mouse states
    bool w;
    bool a;
    bool s;
    bool d;
    bool e;
    bool q;
    bool spacebar;
    bool jump; // in the process of a jump?
    int x_center;
    int y_center;
    int x;
    int y;
    bool left_mouse;
    bool right_mouse;

    // states
    bool fly_mode;
    bool aerial;
    bool swimming;
};

#endif // PLAYER_H
