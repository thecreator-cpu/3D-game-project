#include "player.h"
#include <iostream>

Player::Player() : Player(nullptr) {

}

Player::Player(Camera* cam) : Player(glm::vec3(25, 254, 25), cam)
{

}

Player::Player(glm::vec3 pos, Camera* cam) : pos(pos), vel(glm::vec3(0, 0, 0)),
    maxVel(0.15f), baseAcc(0.05f), acc(0.05f), cam(cam), w(false), a(false), s(false), d(false),
    e(false), q(false), spacebar(false), jump(false), left_mouse(false), right_mouse(false),
    fly_mode(false), aerial(false), swimming(false)
{}

void Player::keyPressEvent(QKeyEvent *e) {

    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }

    if (e->key() == Qt::Key_W && e->key() == Qt::Key_A) {
        w = true;
        a = true;
    } else if (e->key() == Qt::Key_A && e->key() == Qt::Key_S) {
        a = true;
        s = true;
    } else if (e->key() == Qt::Key_S && e->key() == Qt::Key_D) {
        s = true;
        d = true;
    } else if (e->key() == Qt::Key_D && e->key() == Qt::Key_W) {
        d = true;
        w = true;
    } else if (e->key() == Qt::Key_W) {
        w = true;
    } else if (e->key() == Qt::Key_S) {
        s = true;
    } else if (e->key() == Qt::Key_D) {
        d = true;
    } else if (e->key() == Qt::Key_A) {
        a = true;
    } else if (e->key() == Qt::Key_Right) { // original camera controls
        cam->RotateAboutUp(-amount);
    } else if (e->key() == Qt::Key_Left) {
        cam->RotateAboutUp(amount);
    } else if (e->key() == Qt::Key_Up) {
        cam->RotateAboutRight(-amount);
    } else if (e->key() == Qt::Key_Down) {
        cam->RotateAboutRight(amount);
    } else if (e->key() == Qt::Key_1) {
        cam->fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        cam->fovy -= amount;
    }
    cam->RecomputeAttributes();

    // keys that differ between fly mode and normal
    if (!fly_mode) {
        if (e->key() == Qt::Key_F) {
            fly_mode = true;
        } else if (e->key() == Qt::Key_Space) {
            spacebar = true;
            if (!aerial || swimming) {
                jump = true;
            } else {
                jump = false;
            }
        }
    } else {
        if (e->key() == Qt::Key_E) {
            this->e = true;
        } else if (e->key() == Qt::Key_Q) {
            q = true;
        } else if (e->key() == Qt::Key_F) {
            fly_mode = false;
            this->e = false;
            q = false;
        }
    }
}

void Player::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_W && e->key() == Qt::Key_A) {
        w = false;
        a = false;
    } else if (e->key() == Qt::Key_A && e->key() == Qt::Key_S) {
        a = false;
        s = false;
    } else if (e->key() == Qt::Key_S && e->key() == Qt::Key_D) {
        s = false;
        d = false;
    } else if (e->key() == Qt::Key_D && e->key() == Qt::Key_W) {
        d = false;
        w = false;
    } else if (e->key() == Qt::Key_W) {
        w = false;
    } else if (e->key() == Qt::Key_S) {
        s = false;
    } else if (e->key() == Qt::Key_D) {
        d = false;
    } else if (e->key() == Qt::Key_A) {
        a = false;
    } else if (e->key() == Qt::Key_E) {
        this->e = false;
    } else if (e->key() == Qt::Key_Q) {
        q = false;
    } else if (e->key() == Qt::Key_Space) {
        spacebar = false;
        jump = false;
    }
}

void Player::mouseMoveEvent(QMouseEvent *e) {
    x = e->x();
    y = e->y();
}

void Player::mousePressEvent(QMouseEvent *e) {

}

void Player::readUserInput() {
    glm::mat3 removeY = {1, 0, 0,
                         0, 0, 0,
                         0, 0, 1};
    // camera rotations with mouse
    //cam->RotateAboutUp(-1.f * (x - x_center) / 10.f);
    //cam->RotateAboutRight(-1.f * glm::clamp((y - y_center) / 10.f, -89.f, 89.f));

    // decrease the acceleration when swimming
    float origAcc = acc;
    if (swimming)
    {
        acc *= (2.f / 3.f);
    }

    // add negative velocity for aerial state
    // make the vertical velocity mor negative if you are in aerial mode
    // to imitate the speeding affect of falling
    // this velocity is also capped by 3 * maxVel so that the player has a
    // terminal velocity
    if (aerial) {
        if ((vel.y < 0 && glm::abs(glm::length(vel)) < maxVel * 3) || vel.y >= 0)
        {
            if (swimming)
            {
                vel += 0.5f * acc * glm::vec3(0.f, -1.f, 0.f);
            } else {
                vel += 1.5f * acc * glm::vec3(0.f, -1.f, 0.f);
            }
        }
    }
    // add positive velocity if jumping
    // instantaneously increase the vertical velocity by a large
    // amount to imitate the sudden increase in velocity in the
    // vertical direction when jumping
    if (jump) {
        if (swimming)
        {
            // swimming upwards at a constant rate
            vel[1] = 0.1f;
            // jump is not set to false here because spacebar
            // activates jump, but when the swimming boolean is set
            // to true, then having both swimming and jump set to true
            // represents swimming upwards
        } else {
            vel += 10.f * acc * glm::vec3(0.f, 1.f, 0.f);
            jump = false;
        }
    }
    // check key presses
    if (w && a) {
        glm::vec3 direction = cam->look - cam->right;
        direction = glm::normalize(removeY * direction);
        if (glm::length(vel) < maxVel) {
            vel += acc * direction;
        }
    } else if (w && d) {
        glm::vec3 direction = cam->look + cam->right;
        direction = glm::normalize(removeY * direction);
        if (glm::length(vel) < maxVel) {
            vel += acc * direction;
        }
    } else if (a && s) {
        glm::vec3 direction = -1.f * cam->look - cam->right;
        direction = glm::normalize(removeY * direction);
        if (glm::length(vel) < maxVel) {
            vel += acc * direction;
        }
    } else if (s && d) {
        glm::vec3 direction = -1.f * cam->look + cam->right;
        direction = glm::normalize(removeY * direction);
        if (glm::length(vel) < maxVel) {
            vel += acc * direction;
        }
    } else if (w) {
        if (glm::length(vel) < maxVel) {
            vel += acc * glm::normalize(removeY * cam->look);
        }
    } else if (a) {
        if (glm::length(vel) < maxVel) {
            vel -= acc * glm::normalize(removeY * cam->right);
        }
    } else if (s) {
        if (glm::length(vel) < maxVel) {
            vel -= acc * glm::normalize(removeY * cam->look);
        }
    } else if (d) {
        if (glm::length(vel) < maxVel) {
            vel += acc * glm::normalize(removeY * cam->right);
        }
    } else if (e) {
        if (glm::length(vel) < maxVel) {
            vel += acc * glm::vec3(0.f, 1.f, 0.f);
        }
    } else if (q) {
        if (glm::length(vel) < maxVel) {
            vel += acc * glm::vec3(0.f, -1.f, 0.f);
        }
    } else {
        // if no keys are pressed, then slow down the player's
        // velocities
        if (vel != glm::vec3(0.f)) {
            if (aerial) { // do not slow down y since it is handled above
                glm::vec3 vel_xz = glm::vec3(vel[0], 0.f, vel[2]);
                if (glm::length(vel_xz) > 2.f * acc) {
                    vel_xz -= 2.f * acc * glm::normalize(vel_xz);
                    vel[0] = vel_xz[0];
                    vel[2] = vel_xz[2];
                } else {
                    // set the velocities to zero at a certain thrshold in
                    // order to avoid going backwards while attempting to slow
                    // down the player
                    vel[0] = 0.f;
                    vel[2] = 0.f;
                }
            } else { // slow down x, y, z
                if (glm::length(vel) > 2.f * acc) {
                    vel -= 2.f * acc * glm::normalize(vel);
                } else {
                    vel = glm::vec3(0.f);
                }
            }
        }
    }

    if (swimming)
    {
        vel *= (2.f / 3.f);
        acc = origAcc;
    }
}
