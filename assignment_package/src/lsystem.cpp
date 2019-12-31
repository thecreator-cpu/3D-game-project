#include "lsystem.h"

Lsystem::Lsystem()
{  
    functions.insert(std::pair<char,func>('I', &Lsystem::increaseWidth));

}

Lsystem::Lsystem(Turtle t): currTurtle(t)
{
}

Lsystem::Lsystem(Turtle t, MyGL* mygl):
    currTurtle(t),
    m_mygl(mygl){
}

//tower constructor
Lsystem::Lsystem(Turtle t, int type, MyGL* mygl):Lsystem(){
    currTurtle = t;
    m_mygl = mygl;
    if (type == 0){
        functions.insert(std::pair<char,func>('[',  &Lsystem::storeTurtleTower));
        functions.insert(std::pair<char,func>(']', &Lsystem::getTurtle));
        functions.insert(std::pair<char,func>('F', &Lsystem::drawBridge));
        functions.insert(std::pair<char,func>('+',  &Lsystem::rotatePosTower));
        functions.insert(std::pair<char,func>('-', &Lsystem::rotateNegTower));


        rule.insert(std::pair<char,std::string>('B',"+FF"));
        rule.insert(std::pair<char,std::string>('F',"B++B+++BFFF"));
        angle = 13.0 * PI / 180.0;
        segLength = 20;

    }
}

Lsystem::Lsystem(Turtle t, bool deltaRiver,Terrain *ter):Lsystem()
{
    isDelta = deltaRiver;

    currTurtle = t;
    terrain = ter;
    if(deltaRiver){

        /*
        rule.insert(std::pair<char,std::string>('R',"+F+F-F+F"));
        rule.insert(std::pair<char,std::string>('L',"-FF"));
        rule.insert(std::pair<char,std::string>('F',"[R]L"));
        angle = PI/12.0;
        segLength = 10;*/

        rule.insert(std::pair<char,std::string>('R',"+F+F-F+F+F+F+F"));
        rule.insert(std::pair<char,std::string>('L',"-FF-FF"));
        rule.insert(std::pair<char,std::string>('F',"[R]L"));
        angle = PI/12.0;
        segLength = 12;



    } else{
        //rule.insert(std::pair<char,std::string>('R',"+F+F+F+F+F+F+F++F"));
        //rule.insert(std::pair<char,std::string>('L',"-F-F-F-F-F-F-F-F-F-F-F-F"));
        rule.insert(std::pair<char,std::string>('R',"+F+F++F+F+F"));
        rule.insert(std::pair<char,std::string>('L',"-F-F-F-F-F-F"));
        rule.insert(std::pair<char,std::string>('F',"RFLF"));

        angle = PI/20.0;
        segLength = 3;


    }
    
    functions.insert(std::pair<char,func>('[',  &Lsystem::storeTurtle));
    functions.insert(std::pair<char,func>(']', &Lsystem::getTurtle));
    functions.insert(std::pair<char,func>('F', &Lsystem::drawLine));
    functions.insert(std::pair<char,func>('+',  &Lsystem::rotatePos));
    functions.insert(std::pair<char,func>('-', &Lsystem::rotateNeg));
}


void Lsystem::increaseWidth (){
    currTurtle.width += 1;
}

std::string Lsystem::expand(std::string s, int loops) {
    std::string str = s;
    for(int i = 0; i < loops; i++){

        std::string temp = "";
        for (unsigned j=0; j<str.length(); ++j)
         {
            char key = str.at(j);

            //add the char itself if there's no rule for it
            if(rule.find( key ) == rule.end()){
                temp += key;
            } else{
                temp += rule.at(key);
            }

            //increase size only in loop 2
            if(!isDelta && i==2){
               temp += 'I';
            }

            if(!isDelta && i==1){

               temp += "[F]";
            }

         }
        str = temp;
    }
    return str;
}

void Lsystem::draw(std::string str) {

    for (unsigned i=0; i<str.length(); ++i)
     {

        char key = str.at(i);

        //don't do anything if the character doesn't have a function
        if(functions.find( key ) == functions.end()){

            continue;
        }

        //get the function corresponding to the char
        func f = Lsystem::functions.at(key);
        (this->*f)();
        // std::cout << " (this->*f)();"<< std::endl;
     }

}

void Lsystem::rotatePos (){
    glm::vec3 yaxis =  glm::vec3(0,1,0);
    glm::vec4 vec = glm::vec4(currTurtle.orient,1.0);

    //rotate orientation vector
    vec = glm::rotate(angle, yaxis) * vec;
    currTurtle.orient = glm::vec3(vec);


}

void Lsystem::rotateNeg (){

    glm::vec3 yaxis =  glm::vec3(0,1,0);
    glm::vec4 vec = glm::vec4(currTurtle.orient,1.0);

    //rotate orientation vector
    vec = glm::rotate(-angle, yaxis) * vec;

    currTurtle.orient = glm::vec3(vec);
}

void Lsystem::rotatePosTower(){

    glm::vec3 yaxis =  glm::vec3(0,1,0);
    glm::vec4 vec = glm::vec4(currTurtle.orient,1.0);

    //rotate orientation vector
    vec = glm::rotate(angle, yaxis) * vec;
    currTurtle.orient = glm::vec3(vec);

    //draw tower
    int height = 150 +(float(rand() % 100) /100.0) * 50;

    drawTower(currTurtle.pos, 10, height);

    currTurtle.pos[1] = 140 + (float(rand() % 100) /100.0) * (height-140);
}

void Lsystem::rotateNegTower(){

}

void Lsystem::storeTurtle (){
    currTurtle.width -= 1;
    turtleStack.push(currTurtle);
}

void Lsystem::storeTurtleTower (){
    turtleStack.push(currTurtle);

}

void Lsystem::getTurtle (){
    currTurtle = turtleStack.top();
    currTurtle.width += 1;
    turtleStack.pop();
}

void Lsystem::drawLine (){
    drawSeg(segLength);
}

void Lsystem::drawCurve(int loop){
   // currTurtle
   // return currTurtle;
}

void Lsystem::drawSeg(int loop){
    glm::vec3 o = currTurtle.pos;
    glm::vec3 dir = currTurtle.orient;

    glm::vec3 origin = o + dir * 0.001f;

    //cell corner coordinate
    glm::vec3 cell = glm::vec3(glm::floor(origin[0]),
                                glm::floor(origin[1]),
                                glm::floor(origin[2]));

    //sign of direction
    glm::vec3 sign = glm::vec3();

    //fills sign
    for(int i = 0; i<=2; i++){
        if (dir[i] > 0){
            sign[i] = 1;
        } else{
            sign[i] = 0;
        }
    }

    //calculate the first want.
    glm::vec3 want = cell + sign;

    float t = 0;

    //min_i indicates whether x,y,or z are just incremented
    int min_i = -1;

    int w = 0;
    while(w < loop) {
        w++;
        //break when out of bound
       if(cell[0] >= 64 || cell[0] < 0 ||
          cell[2] >= 64 || cell[2] < 0){
                return;
        }


       //draw river with width as currTurtle.width
       riverCrosssection(cell,currTurtle.width);

        //see chich side hits first
        float minT = std::numeric_limits<int>::max();
        for(int i = 0; i <= 2; i++) {
            if (dir[i] != 0.f) {
                float t_temp = (float(want[i]) - origin[i]) / float(dir[i]);
                if(t_temp < minT){
                    minT = t_temp;
                    min_i = i;
                }
            }
        }

        //compute cell position
        cell = glm::floor(origin + (float)fabs(minT) * dir);

        //subtract one if ray from the positive direction
        if(dir[min_i] < 0){
            cell[min_i] = cell[min_i] - 1;
        }

        //calculate new want
        want = cell + sign;
        t = minT;
    }

    //set turtle position
    currTurtle.pos = cell;
}

void Lsystem::cleanAbove(glm::vec3 pos){
    for(int i = pos[1]+1; i < 255; i++){
        terrain->setBlockAt(pos[0],i, pos[2], EMPTY);
    }
}

void Lsystem::waterAbove(glm::vec3 pos){
    for(int i = pos[1]; i <= 128; i++){
        //replace with water
        terrain->setBlockAt(pos[0],i, pos[2], WATER);
    }
    for(int i = 129; i < 256; i++){
        //replace with empty
        terrain->setBlockAt(pos[0],i, pos[2], EMPTY);
    }
}

void Lsystem::riverBank(glm::vec3 pos, int dir){
    for(int i = 0; i < 3; i++){
        int max = i*i;
        //move x 1 unit left or right
        pos[0] += dir;
        //set y to the maximum desired height
        pos[1] = 128 + max+1;
        cleanAbove(pos);

    }

}
void Lsystem::riverBankPerpendicular(glm::vec3 origin, int dir){

    glm::vec3 yaxis =  glm::vec3(0,1,0);
    glm::vec4 vec = glm::vec4(currTurtle.orient,1.0);
    //rotate orientation vector of river

    if(dir == -1){
        vec = glm::rotate(-3.14f/2.f, yaxis) * vec;
    } else{
        vec = glm::rotate(3.14f/2.f, yaxis) * vec;
    }

    glm::vec3 pos = origin;
    for(int i = 0; i < 3; i++){
        int max = i*i;
        pos = marchBlock(pos,  glm::vec3(vec));
        //set y to the maximum desired height
        pos[1] = 128 + max+1;
        cleanAbove(pos);

    }

}


void Lsystem::riverCrosssection(glm::vec3 cell, int width){
    glm::vec3 pos;

    glm::vec3 end = cell;
    end[0] = end[0] + currTurtle.width -1;

    glm::vec3 riverCell;
    //make the bottom of the river ragged
    int halfWidth = width / 2;
    for(int i = 0; i <= halfWidth; i++){
        int depth = std::max(width,(i+1)*(i+1));
        int height = 128 - depth;

        riverCell = cell;
        riverCell[0] += i;
        riverCell[1] = height;
        waterAbove(riverCell);

        riverCell = end;
        riverCell[0] -= i;
        riverCell[1] = height;
        waterAbove(riverCell);

    }

    riverBankPerpendicular(cell,-1);
    riverBankPerpendicular(end,1);
}

glm::vec3 Lsystem::marchBlock(glm::vec3 position, glm::vec3 dir){
    glm::vec3 origin = position + dir * 0.001f;
    //cell corner coordinate
    glm::vec3 cell = glm::vec3(glm::floor(origin[0]),
                                glm::floor(origin[1]),
                                glm::floor(origin[2]));

    //sign of direction
    glm::vec3 sign = glm::vec3();

    //fills sign
    for(int i = 0; i<=2; i++){
        if (dir[i] > 0){
            sign[i] = 1;
        } else{
            sign[i] = 0;
        }
    }

    //calculate the first want.
    glm::vec3 want = cell + sign;

    float t = 0;

    //min_i indicates whether x,y,or z are just incremented
    int min_i = -1;


        //see which side hits first
        float minT = std::numeric_limits<int>::max();
        for(int i = 0; i <= 2; i++) {
            if (dir[i] != 0.f) {
                float t_temp = (float(want[i]) - origin[i]) / float(dir[i]);
                if(t_temp < minT){
                    minT = t_temp;
                    min_i = i;
                }
            }
        }

    return position + (minT)*dir;
}

void Lsystem::drawTowers(){

}
void Lsystem::drawTower(glm::vec3 pos, int radius, int height){
    for(int x = 128; x < height; x++){
        drawTowerCrosssection(pos,radius, x);
    }
}

void Lsystem::setGlobalBlock(glm::vec3 pos, BlockType t){

    Terrain* terrain = getTerrain(pos);
    pos = m_mygl->globalToTerrain(pos);

    terrain->setBlockAt(pos[0],pos[1],pos[2], t);

}

Terrain* Lsystem::getTerrain(glm::vec3 pos){
    int terrainX = glm::floor(pos[0]/ 64.f);
    int terrainZ = glm::floor(pos[2]/ 64.f);

    std::pair<int, int> key = std::pair<int, int>(terrainX, terrainZ);
    Terrain* terrain = nullptr;
    //make new terrain if terrain doesn't exist
    if(m_mygl->m_terrains.find(key) == m_mygl->m_terrains.end()){
        //uPtr<Terrain> terrain_ptr = mkU<Terrain>(m_mygl, terrainX, terrainZ);
        //terrain_ptr->CreateTerrain();
        m_mygl->m_terrains.insert(std::pair<std::pair<int,int>,uPtr<Terrain>>
                                 (key,mkU<Terrain>(m_mygl, terrainX, terrainZ)));
        terrain = m_mygl->m_terrains.at(key).get();
        terrain->multiThreading = false;
        terrain->CreateTerrain();
        m_mygl->m_terrainToBeCreated.push(terrain);
    }

    terrain = m_mygl->m_terrains.at(key).get();
   // terrain->CreateTerrain();
    return terrain;
}

void Lsystem::drawTowerCrosssection(glm::vec3 pos, int radius, int height){
    float center = Terrain::fbm(pos[0],pos[2]);
    pos[1] = height;
    //alter max based on the height
    float max = 0.05;
    int rad = Terrain::fbm(height/10.0,height/10.0)*radius;


    for(int x = pos[0] - radius; x <= pos[0] + radius; x++) {
        for(int z = pos[2] - radius; z <= pos[2] + radius; z++) {
            //if(abs(Terrain::fbm(x,z) - center) < max){
            if(glm::distance(glm::vec3(x,height,z),pos) < rad){
                glm::vec3 cell = glm::vec3(x, height, z);

                setGlobalBlock(cell, STONE);
                //std::cout<<"set cell tower"<< glm::to_string(cell)<<std::endl;
            }


        }
    }

}

void Lsystem::drawBridge(){
    glm::vec3 o = currTurtle.pos;
    glm::vec3 dir = currTurtle.orient;

    //randomize y direction
    //dir[1] = float((rand() % 100) /100.0) * 0.3f;

    glm::vec3 origin = o + dir * 0.001f;

    //random from 140 to origin[1]


    //cell corner coordinate
    glm::vec3 cell = glm::vec3(glm::floor(origin[0]),
                                glm::floor(origin[1]),
                                glm::floor(origin[2]));

    //sign of direction
    glm::vec3 sign = glm::vec3();

    //fills sign
    for(int i = 0; i<=2; i++){
        if (dir[i] > 0){
            sign[i] = 1;
        } else{
            sign[i] = 0;
        }
    }

    //calculate the first want.
    glm::vec3 want = cell + sign;

    float t = 0;

    //min_i indicates whether x,y,or z are just incremented
    //set to -1 ?
    int min_i = 1;

    int w = 0;
    while(w < segLength) {
        w++;

        //cell stores the cell that the ray currently is in

        for(int i = 0; i < 4; i++){
            glm::vec3 pos = cell;
            pos[2] = pos[2] + i;
            setGlobalBlock(pos, STONE);
        }
        //std::cout<<"set cell bridge"<< glm::to_string(cell)<<std::endl;

        //see chich side hits first
        float minT = std::numeric_limits<int>::max();
        for(int i = 0; i <= 2; i++) {
            if (dir[i] != 0.f) {
                float t_temp = (float(want[i]) - origin[i]) / float(dir[i]);
                if(t_temp < minT){
                    minT = t_temp;
                    min_i = i;
                }
            }
        }

        //compute cell position
        cell = glm::floor(origin + (float)fabs(minT) * dir);

        //subtract one if ray from the positive direction
        if(dir[min_i] < 0){
            cell[min_i] = cell[min_i] - 1;
        }


        //calculate new want
        want = cell + sign;
        t = minT;
    }

    //set turtle position
    currTurtle.pos = cell;
}
