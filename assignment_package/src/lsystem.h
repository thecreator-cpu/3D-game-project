#pragma once
#include <la.h>
#include <utils.h>
#include <unordered_map>
#include <scene/terrain.h>
#include <glm/gtx/transform.hpp>
#include <iostream> 
#include <stack> 
#include <mygl.h>

class Terrain;
class Lsystem;
class MyGL;

struct Turtle {
glm::vec3 pos;
glm::vec3 orient;
int width;
int height;
};

enum BlockType : unsigned char;
typedef void (Lsystem::*func)(void);
//typedef void (*func)(void);

class Lsystem
{
public:
    Lsystem();
    Lsystem(Turtle t);
    Lsystem(Turtle t, MyGL* mygl);
    Lsystem(Turtle t, bool delta, Terrain *terrain);
    Lsystem(Turtle t, int type, MyGL* mygl);


    //expand string with the rule i times
    std::string expand(std::string str, int i);
    //draw the string
    void draw(std::string str);

    void rotatePos ();
    void rotateNeg ();
    void storeTurtle ();
    void getTurtle ();
    void increaseWidth ();
    void storeTurtleTower ();
    void rotatePosTower();
    void rotateNegTower();

    void drawLine();
    void drawSeg(int loop);
    void drawCurve(int loop);

    //make all blocks above this block empty
    void cleanAbove(glm::vec3 pos);

    //make all blocks above this block water up to 128
    void waterAbove(glm::vec3 pos);

    //give position of intersection of the ray with block
    glm::vec3 marchBlock(glm::vec3 pos, glm::vec3 dir);

    //draws the cross section of the river with given width
    void riverCrosssection(glm::vec3 pos, int width);

    //creates river bank on left if dir is +1.
    //creates river bank on right if dir is -1.
    void riverBank(glm::vec3 pos, int dir);
    void riverBankPerpendicular(glm::vec3 pos, int dir);


    //towe code
    void drawTowers();
    void drawTower(glm::vec3 pos, int radius, int height);
    void drawTowerCrosssection(glm::vec3 pos, int radius, int height);
    void drawBridge();

    //global to locatl fuctions
    void setGlobalBlock(glm::vec3 pos, BlockType t);
    Terrain* getTerrain(glm::vec3 pos);


private:
    friend class Terrain;
    friend class MyGL;
    Turtle currTurtle;
    Terrain *terrain;
    std::map<char, std::string> rule;
    std::map<char, func> functions;
    std::stack<Turtle> turtleStack;
    float angle;
    int segLength;
    bool isDelta;

    MyGL* m_mygl;
};


//typedef Turtle (*func)(Turtle);



