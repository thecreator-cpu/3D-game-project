#include <scene/terrain.h>

#include <scene/cube.h>
#include <iostream>
#include <QThreadPool>


Terrain::Terrain(OpenGLContext* gc, int posX, int posZ)
    : dimensions(64, 256, 64), gc(gc),
      posX(posX), posZ(posZ), multiThreading(false),
      setBlockMutex(new QMutex()), doneThreadsMutex(new QMutex())
{
    initChunks();
    doneThreads.fill(false);
}

Terrain::Terrain() : dimensions(64, 256, 64), posX(0), posZ(0)
{
    initChunks();
}

// function for printing out the binary representation of an int64_t
void bin(int64_t n)
{
    /* step 1 */
    if (n > 1)
        bin(n/2);

    /* step 2 */
    std::cout << n % 2;
}

// the inputs x, y, and z should always be inbetween 0 and 64
// anything outside of this range is undefined behavior
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if (x < 0 || y < 0 || z < 0) {
        /*
        std::cout << "attempting to use getBlockAt(x, y, z) to access negative "
                  << "indices; however, local coordinates of blocks are always "
                  << "positive relative to their repsective terrain objects." << std::endl;
        */
        return EMPTY;
    }

    if (x >= 64 || y >= 256 || z >= 64)
    {
        std::cout << "in getBlockAt(x, y, z) the x, y, and z values are not in local coordinates: "
                  << x << ", " << y << ", " << z << std::endl;
    }

    x = x % 64;
    z = z % 64;

    int cX = int(x / 16);
    int cZ = int(z / 16);
    // ints are typically 32 bits wide
    int64_t key = 0;
    key = key | cX;
    key = key << 32;
    key = key | cZ;

    try {
        return m_chunks.at(key)->getBlockCopy(x % 16, y, z % 16);
    } catch (const std::exception& e) {
        std::cout << "a standard exception was caught when trying to access the"
                  << " m_chunks object in getBlockAt() with: ";
        bin(key);
        std::cout << std::endl << "the message is: " << e.what() << std::endl;
    }
}

// the inputs x, y, and z should always be inbetween 0 and 64
// anything outside of this range is undefined behavior
void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if (x < 0 || y < 0 || z < 0) {
        // std::cout << "attempting to use setBlockAt(x, y, z) to access negative "
        //           << "indices; however, local coordinates of blocks are always "
        //           << "positive relative to their repsective terrain objects." << std::endl;
        return;
    }

    if (x >= 64|| y >= 256|| z >= 64)

    {
        return;
        // std::cout << "in setBlockAt(x, y, z) the x and z values are not in local coordinates "
        //           << x << ", " << y << ", " << z << std::endl;
    }

    // TODO: Make this work with your new block storage!
    // m_blocks[x][y][z] = t;
    x = x % 64;
    z = z % 64;

    int cX = int(x / 16);
    int cZ = int(z / 16);
    // ints are typically 32 bits wide
    int64_t key = 0;
    key = key | cX;
    key = key << 32;
    key = key | cZ;
    // this will throw an error if the key-map pair does not exist
    try {
        m_chunks.at(key)->getBlockRef(x % 16, y, z % 16) = t;
    } catch (const std::exception& e) {
        std::cout << "a standard exception was caught when trying to access the"
                  << " m_chunks object in setBlockAt() with key: ";
        bin(key);
        std::cout << std::endl << "the message is: " << e.what() << std::endl;
    }
    return;
}

// populate the chunk map
// and setup the neighbor pointers
// this currently does not setup neighbor pointers
// across terrain objects
void Terrain::initChunks()
{
    // clear the map and then assign its content
    m_chunks.clear();
    // intialize map
    for (int x = 0; x < 4; x++) {
        for (int z = 0; z < 4; z++) {
            // upper bits of the key is x and the lower bits will be z
            int64_t key = 0;
            key = key | x;
            key = key << 32;
            key = key | z;
            m_chunks[key] = mkU<Chunk>(gc);
            // bin(key);
            // std::cout << " ";
        }
        // std::cout << std::endl;
    }

    // setup neighbor pointers
    for (int x = 0; x < 4; x++) {
        for (int z = 0; z < 4; z++) {
            int64_t key = 0;
            key = key | x;
            key = key << 32;
            key = key | z;
            int64_t tempKey = 0;
            // left neighbor
            if (x == 0) {
                m_chunks.at(key)->neighbors.at(0) = nullptr;
            } else {
                tempKey = 0;
                tempKey = tempKey | (x - 1);
                tempKey = tempKey << 32;
                tempKey = tempKey | z;
                m_chunks.at(key)->neighbors.at(0) = m_chunks.at(tempKey).get();
            }
            // front neighbor
            if (z == 0) {
                m_chunks.at(key)->neighbors.at(1) = nullptr;
            } else {
                tempKey = 0;
                tempKey = tempKey | x;
                tempKey = tempKey << 32;
                tempKey = tempKey | (z - 1);
                m_chunks.at(key)->neighbors.at(1) = m_chunks.at(tempKey).get();
            }
            // right neighbor
            if (x == 3) {
                m_chunks.at(key)->neighbors.at(2) = nullptr;
            } else {
                tempKey = 0;
                tempKey = tempKey | (x + 1);
                tempKey = tempKey << 32;
                tempKey = tempKey | z;
                m_chunks.at(key)->neighbors.at(2) = m_chunks.at(tempKey).get();
            }
            // back neighbor
            if (z == 3) {
                m_chunks.at(key)->neighbors.at(3) = nullptr;
            } else {
                tempKey = 0;
                tempKey = tempKey | x;
                tempKey = tempKey << 32;
                tempKey = tempKey | (z + 1);
                m_chunks.at(key)->neighbors.at(3) = m_chunks.at(tempKey).get();
            }
        }
    }
}

void Terrain::createChunks()
{
    for (int x = 0; x < 4; x++) {
        for (int z = 0; z < 4; z++) {
            // upper bits of the key is x and the lower bits will be z
            int64_t key = 0;
            key = key | x;
            key = key << 32;
            key = key | z;
            try {
                m_chunks.at(key)->create();
            } catch (const std::exception& e) {
                std::cout << "a standard exception was caught when trying to access the"
                          << " m_chunks object in createChunks() with: ";
                bin(key);
                std::cout << std::endl << "the message is: " << e.what() << std::endl;
            }
        }
    }
}

void Terrain::destroyChunks()
{
    for (int x = 0; x < 4; x++) {
        for (int z = 0; z < 4; z++) {
            // upper bits of the key is x and the lower bits will be z
            int64_t key = 0;
            key = key | x;
            key = key << 32;
            key = key | z;
            try {
                m_chunks.at(key)->destroy();
            } catch (const std::exception& e) {
                std::cout << "a standard exception was caught when trying to access the"
                          << " m_chunks object in destroyChunks() with: ";
                bin(key);
                std::cout << std::endl << "the message is: " << e.what() << std::endl;
            }
        }
    }
}

void Terrain::drawChunks(ShaderProgram* sp)
{
    if (this->isDone())
    {
        for (int x = 0; x < 4; x++) {
            for (int z = 0; z < 4; z++) {
                // upper bits of the key is x and the lower bits will be z
                int64_t key = 0;
                key = key | x;
                key = key << 32;
                key = key | z;
                sp->setModelMatrix(glm::translate(glm::mat4(), glm::vec3((x * 16) + (posX * 64), 0, (z * 16) + (posZ * 64))));
                sp->draw(*m_chunks.at(key));
                // bin(key);
                // std::cout << " ";
            }
            // std::cout << std::endl;
        }
    }
}

void Terrain::CreateTestScene()
{
    /*
    // populate the chunk map
    initChunks();
    // Create the basic terrain floor
    for (int x = 0; x < 64; ++x)
    {
        for (int z = 0; z < 64; ++z)
        {
            for (int y = 127; y < 256; ++y)
            {
                if (y <= 128)
                {
                    if ((x + z) % 2 == 0)
                    {
                        m_blocks[x][y][z] = STONE;
                        setBlockAt(x, y, z, STONE);
                        // std::cout << getBlockAt(x, y, z) << std::endl;
                    }
                    else
                    {
                        m_blocks[x][y][z] = DIRT;
                        setBlockAt(x, y, z, DIRT);
                        // std::cout << getBlockAt(x, y, z) << std::endl;
                    }
                }
                else
                {
                    m_blocks[x][y][z] = EMPTY;
                    setBlockAt(x, y, z, EMPTY);
                    // std::cout << getBlockAt(x, y, z) << std::endl;
                }
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x)
    {
        m_blocks[x][129][0] = GRASS;
        m_blocks[x][130][0] = GRASS;
        m_blocks[x][129][63] = GRASS;
        m_blocks[0][130][x] = GRASS;
        setBlockAt(x, 129, 0, GRASS);
        setBlockAt(x, 130, 0, GRASS);
        setBlockAt(x, 129, 63, GRASS);
        setBlockAt(0, 130, x, GRASS);
    }
    for(int y = 129; y < 140; ++y)
    {
        m_blocks[32][y][32] = GRASS;
        setBlockAt(32, y, 32, GRASS);
    }
    */
}

bool Terrain::isDone()
{
    return (doneThreads.at(0) && doneThreads.at(1) && doneThreads.at(2) && doneThreads.at(3))
            || !multiThreading;
}

//////////////////////Anne's Functions////////////////////////////////

// Using CreateTerrain() under biomes addition
/*
void Terrain::CreateTerrain(){

    //fill y = 1 to 128 with stone
    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            for(int y = 0; y < 256; ++y)
            {
                if (y <= 128)
                {
                    setBlockAt(x,y,z,STONE);
                    //m_blocks[x][y][z] = STONE;
                }else{
                    setBlockAt(x,y,z,EMPTY);
                    //m_blocks[x][y][z] = EMPTY;
                }
            }
         }
    }

    if (multiThreading)
    {
        for (int i = 0; i < 4; i++)
        {
            WorkerThreadFBM* wt = new WorkerThreadFBM(i, this, doneThreads);
            for (int j = 0; j < 4; j++)
            {
                wt->responsibilities[j] = std::pair<int, int>(i, j);
            }
            QThreadPool::globalInstance()->start(wt);
        }
        return;
    }


    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            int max = 170;
            int ymax = 129 + (max-129) * fbm((posX*64+x)/64.f, (posZ*64+z)/64.f);
            for(int y = 129; y < 256; y++){
                if( y < ymax){
                    setBlockAt(x,y,z,DIRT);
                } else if( y > ymax){
                    setBlockAt(x,y,z,EMPTY);
                } else{
                    //make the top grass
                     setBlockAt(x,ymax,z,GRASS);
                }
            }
        }
    }
}
*/

float Terrain::fbm(float x, float y) {
    float total = 0;
    float persistence = 0.5f;
    int octaves = 8;

    for(int i = 1; i <= octaves; i++) {
        float freq = pow(2.f, i);
        float amp = pow(persistence, i);

        total += interpNoise2D(x * freq,
                               y * freq) * amp;
    }

    return total;
}

float Terrain::interpNoise2D(float x, float y) {
    float intX = floor(x);
    float fractX = glm::fract(x);
    float intY = floor(y);
    float fractY = glm::fract(y);

    float v1 = rand(glm::vec2(intX, intY));
    float v2 = rand(glm::vec2(intX + 1, intY));
    float v3 = rand(glm::vec2(intX, intY + 1));
    float v4 = rand(glm::vec2(intX + 1, intY + 1));

    float i1 = glm::mix(v1, v2, fractX);
    float i2 = glm::mix(v3, v4, fractX);
    return glm::mix(i1, i2, fractY);
}

float Terrain::rand(glm::vec2 n) {
    return (glm::fract(sin(glm::dot(n, glm::vec2(12.9898, 4.1414))) * 43758.5453));
}

///////////////////WorkerThreadFBM Inner Class/////////////////////

Terrain::WorkerThreadFBM::WorkerThreadFBM(int i, Terrain* t, std::array<bool, 4>& a)
    : id(i), terrainTgt(t), doneThreads(a),
      setBlockMutex(t->setBlockMutex), doneThreadsMutex(t->doneThreadsMutex)
{}

Terrain::WorkerThreadFBM::~WorkerThreadFBM()
{}

void Terrain::WorkerThreadFBM::run()
{
    doneThreadsMutex->lock();
    for (int i = 0; i < 4; i++)
    {
        for (int xx = 0; xx < 16; xx++)
        {
            for (int zz = 0; zz < 16; zz++)
            {
                int x = xx + responsibilities[i].first * 16;
                int z = zz + responsibilities[i].second * 16;
                int max = 140;

                int ymax = 129 + (max - 129) * fbm((terrainTgt->posX * 64 + x) / 64.f, (terrainTgt->posZ * 64 + z) / 64.f);

                // x = x % 64;
                // z = z % 64;
                // setBlockMutex->lock();
                for(int y = 129; y < 256; y++){
                    if( y < ymax){
                        terrainTgt->setBlockAt(x,y,z,DIRT);
                    } else if( y > ymax){
                        terrainTgt->setBlockAt(x,y,z,EMPTY);
                    } else{
                        //make the top grass
                         terrainTgt->setBlockAt(x,ymax,z,GRASS);
                    }
                }
                // setBlockMutex->unlock();
            }
        }
        int64_t key = 0;
        key = key | responsibilities[i].first;
        key = key << 32;
        key = key | responsibilities[i].second;
        //doneThreadsMutex->lock();
        terrainTgt->doneThreads.at(this->id) = true;
        //doneThreadsMutex->unlock();
    }
    doneThreadsMutex->unlock();
}

float Terrain::WorkerThreadFBM::fbm(float x, float y) {
    float total = 0;
    float persistence = 0.5f;
    int octaves = 8;

    for(int i = 1; i <= octaves; i++) {
        float freq = pow(2.f, i);
        float amp = pow(persistence, i);

        total += interpNoise2D(x * freq,
                               y * freq) * amp;
    }
    return total;
}

float Terrain::WorkerThreadFBM::interpNoise2D(float x, float y) {
    float intX = floor(x);
    float fractX = glm::fract(x);
    float intY = floor(y);
    float fractY = glm::fract(y);

    float v1 = rand(glm::vec2(intX, intY));
    float v2 = rand(glm::vec2(intX + 1, intY));
    float v3 = rand(glm::vec2(intX, intY + 1));
    float v4 = rand(glm::vec2(intX + 1, intY + 1));

    float i1 = glm::mix(v1, v2, fractX);
    float i2 = glm::mix(v3, v4, fractX);
    return glm::mix(i1, i2, fractY);
}

float Terrain::WorkerThreadFBM::rand(glm::vec2 n) {
    return (glm::fract(sin(glm::dot(n, glm::vec2(12.9898, 4.1414))) * 43758.5453));
}

///////////////////////Chunk Inner Class////////////////////////
Terrain::Chunk::Chunk()
    : Drawable(nullptr)
{
    m_blocks.fill(EMPTY);
    neighbors.fill(nullptr);
}

Terrain::Chunk::Chunk(OpenGLContext* gc)
    : Drawable(gc)
{
    m_blocks.fill(EMPTY);
    neighbors.fill(nullptr);
}

void Terrain::Chunk::operator =(Chunk& that)
{
    for (int i = 0; i < 65536; i++)
    {
        this->m_blocks.at(i) = that.m_blocks.at(i);
    }

    for (int i = 0; i < 4; i++)
    {
        this->neighbors.at(i) = that.neighbors.at(i);
    }

    return;
}

// Raycast--Anne
glm::vec2 Terrain::distToBlock(glm::vec3 o, glm::vec3 dir, float max){

    o = glm::vec3(fmod(o[0], 64.f), o[1], fmod(o[2], 64.f));
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
    while(t < max && w < 10) {
        w++;

        //std::cout << glm::to_string(want) << std::endl;

        //break when non-empty block is hit
        if(getBlockAt(int(cell[0]), int(cell[1]), int(cell[2])) != EMPTY) {
            /*
            int direction;
            direction = min_i + 1;

            //
            if(dir[min_i] > 0) {
                direction = -direction;
            }*/

            return glm::vec2(t, float(min_i));
        }

        float minT = std::numeric_limits<int>::max();
        for(int i = 0; i <= 2; i++) {
            if (dir[i] != 0.f) {
                float t_temp = (float(want[i]) - origin[i]) / float(dir[i]);

                //std::cout << i << " t_temp: " << t_temp << std::endl;

                if(t_temp < minT){
                    minT = t_temp;
                    min_i = i;
                }

                //std::cout << i << " min_i: " << min_i << std::endl;
            }
        }

        //increment in the direction of the min
         //want[min_i] = want[min_i] + sign[min_i];

        cell = glm::floor(origin + minT * dir);

        //subtract one if ray from the positive direction
        if(dir[min_i] < 0){
            cell[min_i] = cell[min_i] - 1;
        }
        want = cell + sign;


         t = minT;
    }

    //no block found
    return glm::vec2(-1,-1);
}

BlockType Terrain::Chunk::getBlockCopy(int x, int y, int z) const
{
    x %= 16;
    z %= 16;
    if (x + (y * 16) + (z * 4096) >= 65536) {
        std::cout << x << " " << y << " " << z << std::endl;
    }
    // return m_blocks.at(z + (y * 256) + (x * 4096));
    return m_blocks.at(x + (y * 16) + (z * 4096));
}

BlockType& Terrain::Chunk::getBlockRef(int x, int y, int z)
{
    x %= 16;
    z %= 16;
    if (x + (y * 16) + (z * 4096) >= 65536) {
        std::cout << x << " " << y << " " << z << std::endl;
    }
    return m_blocks.at(x + (y * 16) + (z * 4096));
}

void Terrain::Chunk::create()
{
    std::vector<GLuint> idx;
    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> nor;
    std::vector<glm::vec4> col;
    // Texture & Normal addition
    std::vector<glm::vec2> uv;
    // Blinn-Phong addition
    std::vector<float> cos;
    // Time addition
    std::vector<float> animateable;
    std::vector<float> interleaf;
    int drawnFaces = 0;

    // Transparency addition
    bool transparent;
    std::vector<GLuint> idxTrans;
    std::vector<glm::vec4> posTrans;
    std::vector<glm::vec4> norTrans;
    std::vector<glm::vec4> colTrans;
    std::vector<glm::vec2> uvTrans;
    std::vector<float> cosTrans;
    std::vector<float> animateableTrans;
    std::vector<float> interleafTrans;
    int drawnFacesTrans = 0;

    // iterate through all of the blocks in the chunk and determine
    // which faces of the blocks should be drawn such that only
    // a hull of the terrain is sent to the gpu to be drawn
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 256; y++) {
            for (int z = 0; z < 16; z++) {
                if (this->getBlockCopy(x, y, z) != EMPTY)
                {
                    glm::vec4 blockCol = glm::vec4(0, 0, 0, 1);

                    // Texture & Normal addition
                    glm::vec2 blockUV_top = glm::vec2(0, 0);
                    glm::vec2 blockUV_bottom = glm::vec2(0, 0);
                    glm::vec2 blockUV_side = glm::vec2(0, 0);

                    // Blinn-Phong addition
                    float blockCos = 0;
                    // Time addition
                    float blockAnimateable = false;

                    if (this->getBlockCopy(x, y, z) == GRASS) {
                        transparent = false;
                        blockCos = 1.5;
                        blockAnimateable = false;
                        blockUV_top = glm::vec2(8.f/16.f, 13.f/16.f);
                        blockUV_bottom = glm::vec2(2.f/16.f, 15.f/16.f);
                        blockUV_side = glm::vec2(3.f/16.f, 15.f/16.f);
                    } else if (this->getBlockCopy(x, y, z) == DIRT) {
                        transparent = false;
                        blockCos = 1.5;
                        blockAnimateable = false;
                        blockUV_top = glm::vec2(2.f/16.f, 15.f/16.f);
                        blockUV_bottom = glm::vec2(2.f/16.f, 15.f/16.f);
                        blockUV_side = glm::vec2(2.f/16.f, 15.f/16.f);
                        //blockCol = glm::vec4(0.6, 0.25, 0, 1);
                    } else if (this->getBlockCopy(x, y, z) == STONE) {
                        transparent = false;
                        blockCos = 5;
                        blockAnimateable = false;
                        blockUV_top = glm::vec2(1.f/16.f, 15.f/16.f);
                        blockUV_bottom = glm::vec2(1.f/16.f, 15.f/16.f);
                        blockUV_side = glm::vec2(1.f/16.f, 15.f/16.f);
                        //blockCol = glm::vec4(0.5, 0.5, 0.5, 1);
                    } else if (this->getBlockCopy(x, y, z) == BEDROCK) {
                        transparent = false;
                        blockCos = 1.5;
                        blockAnimateable = false;
                        blockUV_top = glm::vec2(1.f/16.f, 14.f/16.f);
                        blockUV_bottom = glm::vec2(1.f/16.f, 14.f/16.f);
                        blockUV_side = glm::vec2(1.f/16.f, 14.f/16.f);
                    } else if (this->getBlockCopy(x, y, z) == LAVA) {
                        transparent = false;
                        blockCos = 1.5;
                        blockAnimateable = true;
                        blockUV_top = glm::vec2(14.f/16.f, 0.f/16.f);
                        blockUV_bottom = glm::vec2(14.f/16.f, 0.f/16.f);
                        blockUV_side = glm::vec2(14.f/16.f, 0.f/16.f);
                    } else if (this->getBlockCopy(x, y, z) == LEAF) {
                        transparent = false;
                        blockCos = 1.5;
                        blockAnimateable = false;
                        blockUV_top = glm::vec2(5.f/16.f, 12.f/16.f);
                        blockUV_bottom = glm::vec2(5.f/16.f, 12.f/16.f);
                        blockUV_side = glm::vec2(5.f/16.f, 12.f/16.f);
                    } else if (this->getBlockCopy(x, y, z) == WOOD) {
                        transparent = false;
                        blockCos = 1;
                        blockAnimateable = false;
                        blockUV_top = glm::vec2(5.f/16.f, 14.f/16.f);
                        blockUV_bottom = glm::vec2(5.f/16.f, 14.f/16.f);
                        blockUV_side = glm::vec2(4.f/16.f, 14.f/16.f);
                    } else if (this->getBlockCopy(x, y, z) == SAND) {
                        transparent = false;
                        blockCos = 5;
                        blockAnimateable = false;
                        blockUV_top = glm::vec2(2.f/16.f, 14.f/16.f);
                        blockUV_bottom = glm::vec2(2.f/16.f, 14.f/16.f);
                        blockUV_side = glm::vec2(2.f/16.f, 14.f/16.f);
                    } else if (this->getBlockCopy(x, y, z) == SALT) {
                        transparent = false;
                        blockCos = 5;
                        blockAnimateable = false;
                        blockUV_top = glm::vec2(8.f/16.f, 5.f/16.f);
                        blockUV_bottom = glm::vec2(8.f/16.f, 5.f/16.f);
                        blockUV_side = glm::vec2(8.f/16.f, 5.f/16.f);
                    } else if (this->getBlockCopy(x, y, z) == PLATEAU) {
                        transparent = false;
                        blockCos = 1.5;
                        blockAnimateable = false;
                        blockUV_top = glm::vec2(10.f/16.f, 5.f/16.f);
                        blockUV_bottom = glm::vec2(10.f/16.f, 5.f/16.f);
                        blockUV_side = glm::vec2(9.f/16.f, 5.f/16.f);
                        //blockCol = glm::vec4(0.5, 0.5, 0.5, 1);
                    } else if (this->getBlockCopy(x, y, z) == BIRCH) {
                        transparent = false;
                        blockCos = 1;
                        blockAnimateable = false;
                        blockUV_top = glm::vec2(5.f/16.f, 14.f/16.f);
                        blockUV_bottom = glm::vec2(5.f/16.f, 14.f/16.f);
                        blockUV_side = glm::vec2(11.f/16.f, 5.f/16.f);
                        //blockCol = glm::vec4(0.5, 0.5, 0.5, 1);
                    } else if (this->getBlockCopy(x, y, z) == BIRCHLEAF) {
                        transparent = false;
                        blockCos = 1.5;
                        blockAnimateable = false;
                        blockUV_top = glm::vec2(4.f/16.f, 12.f/16.f);
                        blockUV_bottom = glm::vec2(4.f/16.f, 12.f/16.f);
                        blockUV_side = glm::vec2(4.f/16.f, 12.f/16.f);
                        blockCol = glm::mix(glm::vec4(252.f / 255.f, 78.f / 255.f, 3.f / 255.f, 1),
                                            glm::vec4(252.f / 255.f, 240.f / 255.f, 3.f / 255.f, 1),
                                            (rand(glm::vec2(x, y)) + rand(glm::vec2(y, z))) / 2.f);
                    } else if (this->getBlockCopy(x, y, z) == WATER) {
                        transparent = true;
                        blockCos = 15;
                        blockAnimateable = true;
                        blockUV_top = glm::vec2(14.f/16.f, 2.f/16.f);
                        blockUV_bottom = glm::vec2(14.f/16.f, 2.f/16.f);
                        blockUV_side = glm::vec2(14.f/16.f, 2.f/16.f);
                    } else if (this->getBlockCopy(x, y, z) == ICE) {
                        transparent = true;
                        blockCos = 15;
                        blockAnimateable = false;
                        blockUV_top = glm::vec2(3.f/16.f, 11.f/16.f);
                        blockUV_bottom = glm::vec2(3.f/16.f, 11.f/16.f);
                        blockUV_side = glm::vec2(3.f/16.f, 11.f/16.f);
                    }
                    if (!transparent) {
                        // generate front face data if it should be drawn
                        if ((z == 0 && ((neighbors[1] && isEmptyOrTransparent(neighbors[1]->getBlockCopy(x, y, 15))) || ! neighbors[1]))
                                || (z != 0 && isEmptyOrTransparent(this->getBlockCopy(x, y, z - 1))))
                        {
                            pos.push_back(glm::vec4(x, y, z, 1));
                            pos.push_back(glm::vec4(x + 1, y, z, 1));
                            pos.push_back(glm::vec4(x + 1, y + 1, z, 1));
                            pos.push_back(glm::vec4(x, y + 1, z, 1));
                            nor.push_back(glm::vec4(0, 0, -1, 1));
                            nor.push_back(glm::vec4(0, 0, -1, 1));
                            nor.push_back(glm::vec4(0, 0, -1, 1));
                            nor.push_back(glm::vec4(0, 0, -1, 1));
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            idx.push_back(drawnFaces * 4);
                            idx.push_back((drawnFaces * 4) + 1);
                            idx.push_back((drawnFaces * 4) + 2);
                            idx.push_back(drawnFaces * 4);
                            idx.push_back((drawnFaces * 4) + 2);
                            idx.push_back((drawnFaces * 4) + 3);
                            // Texture & Normal addition
                            uv.push_back(glm::vec2(blockUV_side[0], blockUV_side[1]));
                            uv.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1]));
                            uv.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1] + 1.f/16.f));
                            uv.push_back(glm::vec2(blockUV_side[0], blockUV_side[1] + 1.f/16.f));
                            // Blinn Phong addition
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            // Time addition
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            drawnFaces++;
                        }
                        // generate right face data if it should be drawn
                        if ((x == 15 && ((neighbors[2] && isEmptyOrTransparent(neighbors[2]->getBlockCopy(0, y, z))) || !neighbors[2]))
                                 || (x != 15 && isEmptyOrTransparent(this->getBlockCopy(x + 1, y, z)))) {
                            pos.push_back(glm::vec4(x + 1, y, z, 1));
                            pos.push_back(glm::vec4(x + 1, y, z + 1, 1));
                            pos.push_back(glm::vec4(x + 1, y + 1, z + 1, 1));
                            pos.push_back(glm::vec4(x + 1, y + 1, z, 1));
                            nor.push_back(glm::vec4(1, 0, 0, 1));
                            nor.push_back(glm::vec4(1, 0, 0, 1));
                            nor.push_back(glm::vec4(1, 0, 0, 1));
                            nor.push_back(glm::vec4(1, 0, 0, 1));
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            idx.push_back(drawnFaces * 4);
                            idx.push_back((drawnFaces * 4) + 1);
                            idx.push_back((drawnFaces * 4) + 2);
                            idx.push_back(drawnFaces * 4);
                            idx.push_back((drawnFaces * 4) + 2);
                            idx.push_back((drawnFaces * 4) + 3);
                            // Texture & Normal addition
                            uv.push_back(glm::vec2(blockUV_side[0], blockUV_side[1]));
                            uv.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1]));
                            uv.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1] + 1.f/16.f));
                            uv.push_back(glm::vec2(blockUV_side[0], blockUV_side[1] + 1.f/16.f));
                            // Blinn Phong addition
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            // Time addition
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            drawnFaces++;
                        }
                        // generate back face if it should be drawn
                        if ((z == 15 && ((neighbors[3] && isEmptyOrTransparent(neighbors[3]->getBlockCopy(x, y, 0))) || !neighbors[3]))
                                || (z != 15 && isEmptyOrTransparent(this->getBlockCopy(x, y, z + 1))))
                        {
                            pos.push_back(glm::vec4(x + 1, y, z + 1, 1));
                            pos.push_back(glm::vec4(x, y, z + 1, 1));
                            pos.push_back(glm::vec4(x, y + 1, z + 1, 1));
                            pos.push_back(glm::vec4(x + 1, y + 1, z + 1, 1));
                            nor.push_back(glm::vec4(0, 0, 1, 1));
                            nor.push_back(glm::vec4(0, 0, 1, 1));
                            nor.push_back(glm::vec4(0, 0, 1, 1));
                            nor.push_back(glm::vec4(0, 0, 1, 1));
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            idx.push_back(drawnFaces * 4);
                            idx.push_back((drawnFaces * 4) + 1);
                            idx.push_back((drawnFaces * 4) + 2);
                            idx.push_back(drawnFaces * 4);
                            idx.push_back((drawnFaces * 4) + 2);
                            idx.push_back((drawnFaces * 4) + 3);
                            // Texture & Normal addition
                            uv.push_back(glm::vec2(blockUV_side[0], blockUV_side[1]));
                            uv.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1]));
                            uv.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1] + 1.f/16.f));
                            uv.push_back(glm::vec2(blockUV_side[0], blockUV_side[1] + 1.f/16.f));
                            // Blinn Phong addition
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            // Time addition
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            drawnFaces++;
                        }
                        // generate left face if it should be drawn
                        if ((x == 0 && ((neighbors[0] && isEmptyOrTransparent(neighbors[0]->getBlockCopy(15, y, z))) || !neighbors[0]))
                                || (x != 0 && isEmptyOrTransparent(this->getBlockCopy(x - 1, y, z))))
                        {
                            pos.push_back(glm::vec4(x, y, z + 1, 1));
                            pos.push_back(glm::vec4(x, y, z, 1));
                            pos.push_back(glm::vec4(x, y + 1, z, 1));
                            pos.push_back(glm::vec4(x, y + 1, z + 1, 1));
                            nor.push_back(glm::vec4(-1, 0, 0, 1));
                            nor.push_back(glm::vec4(-1, 0, 0, 1));
                            nor.push_back(glm::vec4(-1, 0, 0, 1));
                            nor.push_back(glm::vec4(-1, 0, 0, 1));
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            idx.push_back(drawnFaces * 4);
                            idx.push_back((drawnFaces * 4) + 1);
                            idx.push_back((drawnFaces * 4) + 2);
                            idx.push_back(drawnFaces * 4);
                            idx.push_back((drawnFaces * 4) + 2);
                            idx.push_back((drawnFaces * 4) + 3);
                            // Texture & Normal addition
                            uv.push_back(glm::vec2(blockUV_side[0], blockUV_side[1]));
                            uv.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1]));
                            uv.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1] + 1.f/16.f));
                            uv.push_back(glm::vec2(blockUV_side[0], blockUV_side[1] + 1.f/16.f));
                            // Blinn Phong addition
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            // Time addition
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            drawnFaces++;
                        }
                        // generate top face if it should be drawn
                        if (y == 255 || (y != 255 && isEmptyOrTransparent(this->getBlockCopy(x, y + 1, z))))
                        {
                            pos.push_back(glm::vec4(x, y + 1, z, 1));
                            pos.push_back(glm::vec4(x + 1, y + 1, z, 1));
                            pos.push_back(glm::vec4(x + 1, y + 1, z + 1, 1));
                            pos.push_back(glm::vec4(x, y + 1, z + 1, 1));
                            nor.push_back(glm::vec4(0, 1, 0, 1));
                            nor.push_back(glm::vec4(0, 1, 0, 1));
                            nor.push_back(glm::vec4(0, 1, 0, 1));
                            nor.push_back(glm::vec4(0, 1, 0, 1));
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            idx.push_back(drawnFaces * 4);
                            idx.push_back((drawnFaces * 4) + 1);
                            idx.push_back((drawnFaces * 4) + 2);
                            idx.push_back(drawnFaces * 4);
                            idx.push_back((drawnFaces * 4) + 2);
                            idx.push_back((drawnFaces * 4) + 3);
                            // Texture & Normal addition
                            uv.push_back(glm::vec2(blockUV_top[0], blockUV_top[1]));
                            uv.push_back(glm::vec2(blockUV_top[0] + 1.f/16.f, blockUV_top[1]));
                            uv.push_back(glm::vec2(blockUV_top[0] + 1.f/16.f, blockUV_top[1] + 1.f/16.f));
                            uv.push_back(glm::vec2(blockUV_top[0], blockUV_top[1] + 1.f/16.f));
                            // Blinn Phong addition
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            // Time addition
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            drawnFaces++;
                        }
                        // generate bottom face if it should be drawn
                        if (y == 0 || (y != 0 && isEmptyOrTransparent(this->getBlockCopy(x, y - 1, z))))
                        {
                            pos.push_back(glm::vec4(x, y, z + 1, 1));
                            pos.push_back(glm::vec4(x + 1, y, z + 1, 1));
                            pos.push_back(glm::vec4(x + 1, y, z, 1));
                            pos.push_back(glm::vec4(x, y, z, 1));
                            nor.push_back(glm::vec4(0, -1, 0, 1));
                            nor.push_back(glm::vec4(0, -1, 0, 1));
                            nor.push_back(glm::vec4(0, -1, 0, 1));
                            nor.push_back(glm::vec4(0, -1, 0, 1));
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            col.push_back(blockCol);
                            idx.push_back(drawnFaces * 4);
                            idx.push_back((drawnFaces * 4) + 1);
                            idx.push_back((drawnFaces * 4) + 2);
                            idx.push_back(drawnFaces * 4);
                            idx.push_back((drawnFaces * 4) + 2);
                            idx.push_back((drawnFaces * 4) + 3);
                            // Texture & Normal addition
                            uv.push_back(glm::vec2(blockUV_bottom[0], blockUV_bottom[1]));
                            uv.push_back(glm::vec2(blockUV_bottom[0] + 1.f/16.f, blockUV_bottom[1]));
                            uv.push_back(glm::vec2(blockUV_bottom[0] + 1.f/16.f, blockUV_bottom[1] + 1.f/16.f));
                            uv.push_back(glm::vec2(blockUV_bottom[0], blockUV_bottom[1] + 1.f/16.f));
                            // Blinn Phong addition
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            cos.push_back(blockCos);
                            // Time addition
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            animateable.push_back(blockAnimateable);
                            drawnFaces++;
                        }
                    } else { // Transparency addition
                        // generate front face data if it should be drawn
                        if ((z == 0 && ((neighbors[1] && neighbors[1]->getBlockCopy(x, y, 15) == EMPTY) || ! neighbors[1]))
                                || (z != 0 && this->getBlockCopy(x, y, z - 1) == EMPTY))
                        {
                            posTrans.push_back(glm::vec4(x, y, z, 1));
                            posTrans.push_back(glm::vec4(x + 1, y, z, 1));
                            posTrans.push_back(glm::vec4(x + 1, y + 1, z, 1));
                            posTrans.push_back(glm::vec4(x, y + 1, z, 1));
                            norTrans.push_back(glm::vec4(0, 0, -1, 1));
                            norTrans.push_back(glm::vec4(0, 0, -1, 1));
                            norTrans.push_back(glm::vec4(0, 0, -1, 1));
                            norTrans.push_back(glm::vec4(0, 0, -1, 1));
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            idxTrans.push_back(drawnFacesTrans * 4);
                            idxTrans.push_back((drawnFacesTrans * 4) + 1);
                            idxTrans.push_back((drawnFacesTrans * 4) + 2);
                            idxTrans.push_back(drawnFacesTrans * 4);
                            idxTrans.push_back((drawnFacesTrans * 4) + 2);
                            idxTrans.push_back((drawnFacesTrans * 4) + 3);
                            // Texture & Normal addition
                            uvTrans.push_back(glm::vec2(blockUV_side[0], blockUV_side[1]));
                            uvTrans.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1]));
                            uvTrans.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1] + 1.f/16.f));
                            uvTrans.push_back(glm::vec2(blockUV_side[0], blockUV_side[1] + 1.f/16.f));
                            // Blinn Phong addition
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            // Time addition
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            drawnFacesTrans++;
                        }
                        // generate right face data if it should be drawn
                        if ((x == 15 && ((neighbors[2] && neighbors[2]->getBlockCopy(0, y, z) == EMPTY) || !neighbors[2]))
                                 || (x != 15 && this->getBlockCopy(x + 1, y, z) == EMPTY)) {
                            posTrans.push_back(glm::vec4(x + 1, y, z, 1));
                            posTrans.push_back(glm::vec4(x + 1, y, z + 1, 1));
                            posTrans.push_back(glm::vec4(x + 1, y + 1, z + 1, 1));
                            posTrans.push_back(glm::vec4(x + 1, y + 1, z, 1));
                            norTrans.push_back(glm::vec4(1, 0, 0, 1));
                            norTrans.push_back(glm::vec4(1, 0, 0, 1));
                            norTrans.push_back(glm::vec4(1, 0, 0, 1));
                            norTrans.push_back(glm::vec4(1, 0, 0, 1));
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            idxTrans.push_back(drawnFacesTrans * 4);
                            idxTrans.push_back((drawnFacesTrans * 4) + 1);
                            idxTrans.push_back((drawnFacesTrans * 4) + 2);
                            idxTrans.push_back(drawnFacesTrans * 4);
                            idxTrans.push_back((drawnFacesTrans * 4) + 2);
                            idxTrans.push_back((drawnFacesTrans * 4) + 3);
                            // Texture & Normal addition
                            uvTrans.push_back(glm::vec2(blockUV_side[0], blockUV_side[1]));
                            uvTrans.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1]));
                            uvTrans.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1] + 1.f/16.f));
                            uvTrans.push_back(glm::vec2(blockUV_side[0], blockUV_side[1] + 1.f/16.f));
                            // Blinn Phong addition
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            // Time addition
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            drawnFacesTrans++;
                        }
                        // generate back face if it should be drawn
                        if ((z == 15 && ((neighbors[3] && neighbors[3]->getBlockCopy(x, y, 0) == EMPTY) || !neighbors[3]))
                                || (z != 15 && this->getBlockCopy(x, y, z + 1) == EMPTY))
                        {
                            posTrans.push_back(glm::vec4(x + 1, y, z + 1, 1));
                            posTrans.push_back(glm::vec4(x, y, z + 1, 1));
                            posTrans.push_back(glm::vec4(x, y + 1, z + 1, 1));
                            posTrans.push_back(glm::vec4(x + 1, y + 1, z + 1, 1));
                            norTrans.push_back(glm::vec4(0, 0, 1, 1));
                            norTrans.push_back(glm::vec4(0, 0, 1, 1));
                            norTrans.push_back(glm::vec4(0, 0, 1, 1));
                            norTrans.push_back(glm::vec4(0, 0, 1, 1));
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            idxTrans.push_back(drawnFacesTrans * 4);
                            idxTrans.push_back((drawnFacesTrans * 4) + 1);
                            idxTrans.push_back((drawnFacesTrans * 4) + 2);
                            idxTrans.push_back(drawnFacesTrans * 4);
                            idxTrans.push_back((drawnFacesTrans * 4) + 2);
                            idxTrans.push_back((drawnFacesTrans * 4) + 3);
                            // Texture & Normal addition
                            uvTrans.push_back(glm::vec2(blockUV_side[0], blockUV_side[1]));
                            uvTrans.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1]));
                            uvTrans.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1] + 1.f/16.f));
                            uvTrans.push_back(glm::vec2(blockUV_side[0], blockUV_side[1] + 1.f/16.f));
                            // Blinn Phong addition
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            // Time addition
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            drawnFacesTrans++;
                        }
                        // generate left face if it should be drawn
                        if ((x == 0 && ((neighbors[0] && neighbors[0]->getBlockCopy(15, y, z) == EMPTY) || !neighbors[0]))
                                || (x != 0 && this->getBlockCopy(x - 1, y, z) == EMPTY))
                        {
                            posTrans.push_back(glm::vec4(x, y, z + 1, 1));
                            posTrans.push_back(glm::vec4(x, y, z, 1));
                            posTrans.push_back(glm::vec4(x, y + 1, z, 1));
                            posTrans.push_back(glm::vec4(x, y + 1, z + 1, 1));
                            norTrans.push_back(glm::vec4(-1, 0, 0, 1));
                            norTrans.push_back(glm::vec4(-1, 0, 0, 1));
                            norTrans.push_back(glm::vec4(-1, 0, 0, 1));
                            norTrans.push_back(glm::vec4(-1, 0, 0, 1));
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            idxTrans.push_back(drawnFacesTrans * 4);
                            idxTrans.push_back((drawnFacesTrans * 4) + 1);
                            idxTrans.push_back((drawnFacesTrans * 4) + 2);
                            idxTrans.push_back(drawnFacesTrans * 4);
                            idxTrans.push_back((drawnFacesTrans * 4) + 2);
                            idxTrans.push_back((drawnFacesTrans * 4) + 3);
                            // Texture & Normal addition
                            uvTrans.push_back(glm::vec2(blockUV_side[0], blockUV_side[1]));
                            uvTrans.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1]));
                            uvTrans.push_back(glm::vec2(blockUV_side[0] + 1.f/16.f, blockUV_side[1] + 1.f/16.f));
                            uvTrans.push_back(glm::vec2(blockUV_side[0], blockUV_side[1] + 1.f/16.f));
                            // Blinn Phong addition
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            // Time addition
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            drawnFacesTrans++;
                        }
                        // generate top face if it should be drawn
                        if (y == 255 || (y != 255 && this->getBlockCopy(x, y + 1, z) == EMPTY))
                        {
                            posTrans.push_back(glm::vec4(x, y + 1, z, 1));
                            posTrans.push_back(glm::vec4(x + 1, y + 1, z, 1));
                            posTrans.push_back(glm::vec4(x + 1, y + 1, z + 1, 1));
                            posTrans.push_back(glm::vec4(x, y + 1, z + 1, 1));
                            norTrans.push_back(glm::vec4(0, 1, 0, 1));
                            norTrans.push_back(glm::vec4(0, 1, 0, 1));
                            norTrans.push_back(glm::vec4(0, 1, 0, 1));
                            norTrans.push_back(glm::vec4(0, 1, 0, 1));
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            idxTrans.push_back(drawnFacesTrans * 4);
                            idxTrans.push_back((drawnFacesTrans * 4) + 1);
                            idxTrans.push_back((drawnFacesTrans * 4) + 2);
                            idxTrans.push_back(drawnFacesTrans * 4);
                            idxTrans.push_back((drawnFacesTrans * 4) + 2);
                            idxTrans.push_back((drawnFacesTrans * 4) + 3);
                            // Texture & Normal addition
                            uvTrans.push_back(glm::vec2(blockUV_top[0], blockUV_top[1]));
                            uvTrans.push_back(glm::vec2(blockUV_top[0] + 1.f/16.f, blockUV_top[1]));
                            uvTrans.push_back(glm::vec2(blockUV_top[0] + 1.f/16.f, blockUV_top[1] + 1.f/16.f));
                            uvTrans.push_back(glm::vec2(blockUV_top[0], blockUV_top[1] + 1.f/16.f));
                            // Blinn Phong addition
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            // Time addition
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            drawnFacesTrans++;
                        }
                        // generate bottom face if it should be drawn
                        if (y == 0 || (y != 0 && this->getBlockCopy(x, y - 1, z) == EMPTY))
                        {
                            posTrans.push_back(glm::vec4(x, y, z + 1, 1));
                            posTrans.push_back(glm::vec4(x + 1, y, z + 1, 1));
                            posTrans.push_back(glm::vec4(x + 1, y, z, 1));
                            posTrans.push_back(glm::vec4(x, y, z, 1));
                            norTrans.push_back(glm::vec4(0, -1, 0, 1));
                            norTrans.push_back(glm::vec4(0, -1, 0, 1));
                            norTrans.push_back(glm::vec4(0, -1, 0, 1));
                            norTrans.push_back(glm::vec4(0, -1, 0, 1));
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            colTrans.push_back(blockCol);
                            idxTrans.push_back(drawnFacesTrans * 4);
                            idxTrans.push_back((drawnFacesTrans * 4) + 1);
                            idxTrans.push_back((drawnFacesTrans * 4) + 2);
                            idxTrans.push_back(drawnFacesTrans * 4);
                            idxTrans.push_back((drawnFacesTrans * 4) + 2);
                            idxTrans.push_back((drawnFacesTrans * 4) + 3);
                            // Texture & Normal addition
                            uvTrans.push_back(glm::vec2(blockUV_bottom[0], blockUV_bottom[1]));
                            uvTrans.push_back(glm::vec2(blockUV_bottom[0] + 1.f/16.f, blockUV_bottom[1]));
                            uvTrans.push_back(glm::vec2(blockUV_bottom[0] + 1.f/16.f, blockUV_bottom[1] + 1.f/16.f));
                            uvTrans.push_back(glm::vec2(blockUV_bottom[0], blockUV_bottom[1] + 1.f/16.f));
                            // Blinn Phong addition
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            cosTrans.push_back(blockCos);
                            // Time addition
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            animateableTrans.push_back(blockAnimateable);
                            drawnFacesTrans++;
                        }
                    }

                }
            }
        }
    }

    // combine all of the position, normal, color, and uv data into a
    // single interleaved vector
    for (int i = 0; i < drawnFaces * 4; i++)
    {
        interleaf.push_back(pos.at(i)[0]); // pos 0-3
        interleaf.push_back(pos.at(i)[1]);
        interleaf.push_back(pos.at(i)[2]);
        interleaf.push_back(pos.at(i)[3]);
        interleaf.push_back(nor.at(i)[0]); // nor 4-7
        interleaf.push_back(nor.at(i)[1]);
        interleaf.push_back(nor.at(i)[2]);
        interleaf.push_back(nor.at(i)[3]);
        interleaf.push_back(col.at(i)[0]); // col 8-11
        interleaf.push_back(col.at(i)[1]);
        interleaf.push_back(col.at(i)[2]);
        interleaf.push_back(col.at(i)[3]);
        // Texture & Normal addition
        interleaf.push_back(uv.at(i)[0]);  // uv 12-13
        interleaf.push_back(uv.at(i)[1]);
        // Blinn-Phong addition
        interleaf.push_back(cos.at(i));    // cos 14
        // Time addition
        interleaf.push_back(animateable.at(i)); // animateable 15
    }

    // Transparency addition
    for (int i = 0; i < drawnFacesTrans * 4; i++)
    {
        interleafTrans.push_back(posTrans.at(i)[0]); // pos 0-3
        interleafTrans.push_back(posTrans.at(i)[1]);
        interleafTrans.push_back(posTrans.at(i)[2]);
        interleafTrans.push_back(posTrans.at(i)[3]);
        interleafTrans.push_back(norTrans.at(i)[0]); // nor 4-7
        interleafTrans.push_back(norTrans.at(i)[1]);
        interleafTrans.push_back(norTrans.at(i)[2]);
        interleafTrans.push_back(norTrans.at(i)[3]);
        interleafTrans.push_back(colTrans.at(i)[0]); // col 8-11
        interleafTrans.push_back(colTrans.at(i)[1]);
        interleafTrans.push_back(colTrans.at(i)[2]);
        interleafTrans.push_back(colTrans.at(i)[3]);
        interleafTrans.push_back(uvTrans.at(i)[0]);  // uv 12-13
        interleafTrans.push_back(uvTrans.at(i)[1]);
        interleafTrans.push_back(cosTrans.at(i));    // cos 14
        interleafTrans.push_back(animateableTrans.at(i)); // animateable 15
    }

    count = idx.size();
    countTrans = idxTrans.size();

    // if you uncomment this block, you have to
    // uncomment the lines inside of the conditional statement
    // for d.bindInterleaf() and then comment out the line
    // directly above these lines
    /*
    for (int i = 0; i < count; i++)
    {
        idx.at(i) *= 3;
    }
    */

    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    // Transparency addition
    generateIdxTrans();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxTrans);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxTrans.size() * sizeof(GLuint), idxTrans.data(), GL_STATIC_DRAW);

    // 3 separate vbo implementation
    /*
    generatePos();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateNor();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    context->glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);

    generateCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);

    // Texture & Normal addition
    generateUV();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    context->glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(glm::vec2), uv.data(), GL_STATIC_DRAW);
    */


    // single interleaved vbo implementation
    generateInterleaf();
    // bindInterleaf();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufInterleaf);
    context->glBufferData(GL_ARRAY_BUFFER, interleaf.size() * sizeof(float), interleaf.data(), GL_STATIC_DRAW);

    // Transparency addition
    generateInterleafTrans();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufInterleafTrans);
    context->glBufferData(GL_ARRAY_BUFFER, interleafTrans.size() * sizeof(float), interleafTrans.data(), GL_STATIC_DRAW);
}

// Transparency addition
// ADD TRANSPARENT BLOCKS HERE
bool Terrain::isEmptyOrTransparent(BlockType b) {
    if (b == WATER || b == ICE || b == EMPTY) {
        return true;
    }
    return false;
}

GLenum Terrain::Chunk::drawMode()
{
    return GL_TRIANGLES;
}

Terrain::Chunk::~Chunk()
{}

//river code
void Terrain:: makeRiver(){
    Turtle turtle;
    turtle.pos = glm::vec3(10,128,0);
    turtle.orient = glm::vec3(0,0,1);
    turtle.width = 3;
    Lsystem lsys = Lsystem (turtle, false, this);

    std::string str= "F";
    str = lsys.expand(str, 4);
    lsys.draw(str);

    destroyChunks();
    createChunks();
}

void Terrain:: makeDeltaRiver(){

    Turtle turtle;
    turtle.pos = glm::vec3(32,128,0);
    turtle.orient = glm::vec3(0,0,1);
    turtle.width = 3;
    Lsystem lsys = Lsystem (turtle, true, this);

    std::string str= "F";
    str = lsys.expand(str, 4);
    lsys.draw(str);

    destroyChunks();
    createChunks();

}

// ----------------------------biome additions---------------------------

glm::vec2 Terrain::rand2(glm::vec2 n) {
    return (glm::fract(glm::sin(glm::vec2(glm::dot(n * glm::vec2(34.81, 76.12) + glm::vec2(53.31, 431.12), glm::vec2(12.9898, 4.1414)),
                                     glm::dot(n + glm::vec2(64.325, 468.431), glm::vec2(421.342, 32.4513)))) * 43758.5453f));
}

glm::vec2 Terrain::rand3(glm::vec2 n) {
    return (glm::fract(glm::sin(glm::vec2(glm::dot(n * glm::vec2(42.95, 53.23)  + glm::vec2(21.34, 231.27), glm::vec2(54.1342, 94.223)),
                                     glm::dot(n + glm::vec2(34.466, 632.24), glm::vec2(5432.23, 643.42)))) * 5432.0631f));
}

/*
float Terrain::perlin(float x, float y) {
    glm::vec2 grid = glm::vec2(floorf(x), floorf(y));
    glm::vec2 p = glm::fract(glm::vec2(x, y));
    // corners to point
    glm::vec2 v1 = glm::normalize(p);
    glm::vec2 v2 = glm::normalize(p - glm::vec2(1, 0));
    glm::vec2 v3 = glm::normalize(p - glm::vec2(1, 1));
    glm::vec2 v4 = glm::normalize(p - glm::vec2(0, 1));
    // random vecs at corners
    glm::vec2 r1 = glm::normalize(rand2(glm::vec2(floorf(x), floorf(y))));
    glm::vec2 r2 = glm::normalize(rand2(glm::vec2(floorf(x) + 1, floorf(y))));
    glm::vec2 r3 = glm::normalize(rand2(glm::vec2(floorf(x) + 1, floorf(y) + 1)));
    glm::vec2 r4 = glm::normalize(rand2(glm::vec2(floorf(x), floorf(y) + 1)));
    // dot products
    float dot1 = glm::dot(v1, r1);
    float dot2 = glm::dot(v2, r2);
    float dot3 = glm::dot(v3, r3);
    float dot4 = glm::dot(v4, r4);
    // distance
    float d1 = falloff(p, glm::vec2(0, 0));
    float d2 = falloff(p, glm::vec2(1, 0));
    float d3 = falloff(p, glm::vec2(1, 1));
    float d4 = falloff(p, glm::vec2(0, 1));
    return  dot1 * d1 +
            dot2 * d2 +
            dot3 * d3 +
            dot4 * d4;
}


float Terrain::falloff(glm::vec2 p, glm::vec2 corner) {
    float t = glm::distance(p, corner); // Linear
    t = 6.f * pow(t, 5) - 15.f * pow(t, 4) + 10.f * pow(t, 3); // Quintic, 6t5 + 15t4 - 10t3
    return 1 - t; // Want more weight the closer we are
}
*/

float Terrain::surflet(glm::vec2 p, glm::vec2 gridPoint, bool typeBumpiness) {
    // Compute the distance between p and the grid point along each axis, and warp it with a
    // quintic function so we can smooth our cells
    glm::vec2 diff = glm::abs(p - gridPoint);
    glm::vec2 t = glm::vec2(1.f) - (6.f * glm::pow(diff, glm::vec2(5.f, 5.f))
            - 15.f * glm::pow(diff, glm::vec2(4.f, 4.f))
            + 10.f * glm::pow(diff, glm::vec2(3.f, 3.f)));
    // Get the random vector for the grid point (assume we wrote a function random2)
    glm::vec2 gradient;
    if (typeBumpiness) {
        gradient = rand2(gridPoint);
    } else {
        gradient = rand3(gridPoint);
    }
    // Get the vector from the grid point to P
    glm::vec2 diff2 = p - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = glm::dot(diff2, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * t.x * t.y;
}

float Terrain::perlinNoise(glm::vec2 uv, bool elevation) {
    float surfletSum = 0.f;
    // Iterate over the four integer corners surrounding uv
    for(int dx = 0; dx <= 1; ++dx) {
        for(int dy = 0; dy <= 1; ++dy) {
            surfletSum += surflet(uv, glm::floor(uv) + glm::vec2(dx, dy), elevation);
        }
    }
    //std::cout << surfletSum << std::endl;
    return surfletSum;
}

void Terrain::CreateTerrain(){

    //fill y = 1 to 128 with stone
    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            for(int y = 0; y < 256; ++y)
            {
                if(y <= 128)
                {
                    setBlockAt(x,y,z,STONE);
                    //m_blocks[x][y][z] = STONE;
                }else{
                    setBlockAt(x,y,z,EMPTY);
                    //m_blocks[x][y][z] = EMPTY;
                }
            }

         }
    }

    /*
    if (multiThreading)
    {
        for (int i = 0; i < 4; i++)
        {
            WorkerThreadFBM* wt = new WorkerThreadFBM(i, this, doneThreads);
            for (int j = 0; j < 4; j++)
            {
                wt->responsibilities[j] = std::pair<int, int>(i, j);
            }
            QThreadPool::globalInstance()->start(wt);
        }
        return;
    }
    */

    // set desired sea level
    sealevel = 150;

    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            //int max = 200;
            glm::vec2 p = glm::vec2((posX*64+x)/64.f, (posZ*64+z)/64.f);
            // perlin returns value between [-1, 1], must re-map to [0, 1]
            //int ymax = 129 + (max-129) * fbm((posX*64+x)/64.f, (posZ*64+z)/64.f);
            //int ymax = 129 + (max-129) * (perlin(p[0], p[1]) / 2.f + 0.5f);
            //int ymax = 129 + (max-129) * (perlinNoise(p / 4.f, true) / 2.f + 0.5f);
            float bumpiness = fbm(p[0] / 12.f, p[1] / 12.f);
            float moisture = perlinNoise(p / 16.f, true) / 2.f + 0.5f;
            //float moisture = perlinNoise(p / 25.f + glm::vec2(0.5f, 0.5f), false) / 2.f + 0.5f;

            // testing
            //bumpiness = 1.f;
            //moisture = 1.f;

            float i1 = glm::mix(saltflats(p), forest(p), glm::smoothstep(0.47f, 0.53f, moisture));
            float i2 = glm::mix(plateau(p), island(p), glm::smoothstep(0.47f, 0.53f, moisture));
            float ymax = glm::mix(i1, i2, glm::smoothstep(0.47f, 0.53f, bumpiness));

            if (bumpiness < 0.5f) {
                if (moisture < 0.5f) { // Salt Flat
                    if (ymax < sealevel) ymax = sealevel;
                    for(int y = 129; y <= ymax; y++){
                        setBlockAt(x, y, z, SALT);
                    }
                } else { // Forest
                    if (ymax < sealevel) ymax = sealevel;
                    for(int y = 129; y < floorf(ymax); y++){
                        setBlockAt(x, y, z, DIRT);
                    }
                    setBlockAt(x, floorf(ymax), z, GRASS);
                    if (setTree(x, z)) {
                        createTree(x, floorf(ymax) + 1, z);
                    }
                }

            } else if (bumpiness > 0.5f ) {
                if (moisture < 0.5f) { // Desert Plateau
                    if (ymax < sealevel) ymax = sealevel;
                    for(int y = 129; y <= ymax; y++){
                        setBlockAt(x, y, z, PLATEAU);
                    }
                } else { // Islands
                    for(int y = 129; y <= glm::max((int)floorf(ymax), sealevel); y++){
                        if( y <= ymax){
                            setBlockAt(x, y, z, SAND);
                        } else if (y <= sealevel) {
                            setBlockAt(x, y, z, WATER);;
                        }
                    }
                }
            }
        }
    }
    // do not draw islands in the initial 3x3 terrains
    if (!(posX >= -1 && posX <= 1 && posZ >= -1 && posZ <= 1)) {
        // 75% chance of generating floating island
        if (rand(glm::vec2(posX, posZ)) > 0.25f) {
            glm::vec2 pos = 30.f * rand2(glm::vec2(posX, posZ));
            int height = floorf(40.f * glm::smoothstep(0.2f, 0.8f, (perlinNoise(pos, false) / 2.f + 0.5f)));
            float radius = 5.f * (perlinNoise(pos, true) / 2.f + 0.5f);
            drawIsland(glm::vec3(17 + floorf(pos[0]), 195 + height, 17 + floorf(pos[1])), 10 + floorf(radius), 195 + height);
        }
    }
    //drawIsland(glm::vec3(32, 238, 32), 10, 238);

}

float Terrain::saltflats(glm::vec2 uv) {
    return sealevel;
}

float Terrain::plateau(glm::vec2 uv) {
    int max = sealevel + 25;
    float x = perlinNoise(uv, true) / 2.f + 0.5f;
    float ymax = sealevel + (max-sealevel) * ((x <= 0.53f) * glm::smoothstep(0.50f, 0.55f, x)
                                        + (x > 0.53f) *  (0.8f + 0.2f * glm::smoothstep(0.50f, 0.55f, x)));
    return ymax;
}

float Terrain::forest(glm::vec2 uv) {
    int max = sealevel + 15;
    float ymax = sealevel + (max-sealevel) * glm::smoothstep(0.35f, 0.65f, (perlinNoise(uv, true) / 2.f + 0.5f));
    return ymax;
}

float Terrain::island(glm::vec2 uv) {
    int max = sealevel + 15;
    float ymax = 129 + (max-129) * glm::smoothstep(0.35f, 0.65f, (perlinNoise(uv, true) / 2.f + 0.5f));
    return ymax;
}

bool Terrain::setTree(int x, int z) {
    int gridsize = 5;
    // e.g., if gridsize = 3, creates 3 by 3 grid where only bottom left 2 by 2 grid can have a tree
    if (x % gridsize == gridsize - 1 || z % gridsize == gridsize - 1) {
        return false;
    }
    // e.g., if gridsize = 3, use bomttom left corner of 3 by 3 grid for rand
    // then map range [0, 1] to [0, 2] to choose location in 2 by 2 grid
    glm::vec2 location = rand2(glm::vec2(x - x % gridsize, z - z % gridsize)) * (gridsize - 1.f);
    if (x % gridsize == floorf(location[0]) && z % gridsize == floorf(location[1])) {
        return true;
    }
    return false;
}

void Terrain::createTree(int x, int y, int z) {
    int leavesheight = 10;
    int minheight = 15;
    int maxheight = 25;
    int height = minheight + floorf(rand3(glm::vec2(x, z))[0] * (maxheight - minheight));
    for (int i = y; i < y + height; i++) {
        setBlockAt(x, i, z, BIRCH);

        if (i > y + leavesheight) {
            for (int dx = -1; dx <= 1; dx++) {
                for (int dz = -1; dz <= 1; dz++) {
                    if (dx == 0 && dz == 0) {
                        // do nothing
                    } else if ((rand(glm::vec2(z + dz, i)) + rand(glm::vec2(x + dx, z + dz))) / 2.f > 0.5f) {
                        setBlockAt(x + dx, i, z + dz, BIRCHLEAF);
                        //std::cout << "(" << x + dx << ", " << i << ", " << z + dz << ")" << std::endl;
                    }
                }
            }
        }
    }
    setBlockAt(x, y + height, z, BIRCHLEAF);
}

void Terrain::drawIsland(glm::vec3 pos, float radius, int height){
    if (radius == 1) {
        setBlockAt(pos[0], height, pos[2], STONE);
        return;
    }
    pos[1] = height;

    for(int x = floorf(pos[0] - radius); x <= ceilf(pos[0] + radius); x++) {
        for(int z = floorf(pos[2] - radius); z <= ceilf(pos[2] + radius); z++) {
            // introduce randomness to shape
            float rad = radius + (rand(glm::vec2(x, z)) > 0.5f);
            if(glm::distance(glm::vec3(x, height, z), pos) < rad){
                setBlockAt(x, height, z, STONE);
                sealevel = pos[1] + 1;
                int max = sealevel + 4;
                glm::vec2 uv = glm::vec2(x / 16.f, z / 16.f) + rand2(glm::vec2(pos[0], pos[2]));
                float ymax = sealevel + (max-sealevel) * glm::smoothstep(0.35f, 0.65f, (perlinNoise(uv, true) / 2.f + 0.5f));
                for (int y = pos[1] + 1; y < ymax; y++) {
                    setBlockAt(x, y, z, DIRT);
                    //std::cout << ymax << std::endl;
                }
                setBlockAt(x, ymax, z, GRASS);
            }
        }
    }
    // generate waterfalls
    int numWaterfalls = ceilf(4 * rand(glm::vec2(pos[0], pos[2])));
    for (int i = 0; i < numWaterfalls; i++) {
        glm::vec2 location = 2.f * radius * rand2(glm::vec2(i, pos[0]));
        int xcoord = floorf(pos[0] - radius) + floorf(location[0]);
        int zcoord = floorf(pos[2] - radius) + floorf(location[1]);
        if(glm::distance(glm::vec3(xcoord, height, zcoord), pos) < radius){
            int waterHeight = height;
            while (waterHeight > 170 || getBlockAt(xcoord, waterHeight, zcoord) == EMPTY
                   || getBlockAt(xcoord, waterHeight, zcoord) == BIRCHLEAF) {
                setBlockAt(xcoord, waterHeight, zcoord, WATER);
                waterHeight--;
            }
            for (int dx = -1; dx <= 1; dx++) {
                for (int dz = -1; dz <= 1; dz++) {
                    setBlockAt(xcoord + dx, waterHeight, zcoord + dz, WATER);
                }
            }
        }
    }
    // recurse so each level is smaller
    drawIslandRecurse(pos, radius - 0.5f, height - 1);
}

void Terrain::drawIslandRecurse(glm::vec3 pos, float radius, int height) {
    if (radius == 1) {
        setBlockAt(pos[0], height, pos[2], STONE);
        return;
    }
    pos[1] = height;

    for(int x = floorf(pos[0] - radius); x <= ceilf(pos[0] + radius); x++) {
        for(int z = floorf(pos[2] - radius); z <= ceilf(pos[2] + radius); z++) {
            // introduce randomness to shape
            float rad = radius + (rand(glm::vec2(x, z)) > 0.5f);
            if(glm::distance(glm::vec3(x, height, z), pos) < rad){
                setBlockAt(x, height, z, STONE);
            }
        }
    }
    // recurse so each level is smaller
    drawIslandRecurse(pos, radius - 0.5f, height - 1);
}
