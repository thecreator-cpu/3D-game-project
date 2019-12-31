#pragma once
#include <QList>
#include <la.h>

#include "drawable.h"
#include "smartpointerhelp.h"
#include <unordered_map>
#include <shaderprogram.h>
#include <array>
#include <lsystem.h>
#include <QRunnable>
#include <QMutex>

class Lsystem;
class MyGL;
class Terrain;

struct Turtle;
// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY,
    GRASS, DIRT, STONE, BEDROCK, LAVA, LEAF, WOOD, // opaque
    SAND, SALT, PLATEAU, BIRCH, BIRCHLEAF, // custom
    WATER, ICE // transparent
};



class Terrain
{
private:
    // Chunk is an inner class of the Terrain class
    class Chunk : public Drawable
    {
    public :
        Chunk();
        Chunk(OpenGLContext*);
        virtual ~Chunk();

        void operator = (Chunk&);
        BlockType getBlockCopy(int, int, int) const;
        BlockType& getBlockRef(int, int, int);
        void create() override;
        virtual GLenum drawMode();

        // this is necessary, otherwise the private data members
        // of the Chunk class are inaccessible in the Terrain class
        friend class Terrain;



    private :
        // rows are continguous
        // starts from bottom, front row and ends in top, back row
        // next row encountered in the array is the row below the
        // previous
        // index 0, 0, 0 is the closest, left-most, bottom block
        // BlockType m_blocks[65536];
        std::array<BlockType, 65536> m_blocks;
        // 0 : x - 1,    1 : z - 1,       2 : x + 1,    3 : z + 1
        // 0 : left of,  1 : in front of, 2 : right of, 3 : behind
        // Chunk* neighbors[4];
        std::array<Chunk*, 4> neighbors;
    };


    class WorkerThreadFBM : public QRunnable
    {
    public :
        int id;
        Terrain* terrainTgt;
        // the 4 chunks that this thread is responsible for generating
        std::pair<int, int> responsibilities[4];
        std::array<bool, 4>& doneThreads;
        QMutex* setBlockMutex;
        QMutex* doneThreadsMutex;

        WorkerThreadFBM(int, Terrain*, std::array<bool, 4>&);
        ~WorkerThreadFBM();
        void run() override;

        float interpNoise2D(float x, float y);
        float fbm(float x, float y);
        float rand(glm::vec2 n);

        friend class Terrain;
    };


public:
    int posX, posZ;
    glm::ivec3 dimensions;
    // 32-bit x-coordinate is stored in the upper 32 bits
    // 32-bit z-coordinate is stored in the loweer 32 bits
    // the indexing of the chunks is 0 through 3 in the
    // x and z directions as opposed to 0 through 48 with
    // increments of 16
    // index 0, 0 is the closest, left-most chunk
    std::unordered_map<int64_t, uPtr<Chunk>> m_chunks;
    // the inner chunks made inside of the class need a pointer to a drawable
    // object. thus i need the outer class, Terrain, to get the OpenGLContext
    // for me
    std::array<bool, 4> doneThreads;
    OpenGLContext* gc;
    // Biome addition
    int sealevel;

    Terrain();
    Terrain(OpenGLContext*, int, int);

    void initChunks();
    void createChunks();
    void destroyChunks();
    void drawChunks(ShaderProgram*);
    void CreateTestScene();

    BlockType getBlockAt(int x, int y, int z) const;   // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t); // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.

    // Raycast--Anne
    glm::vec2 distToBlock(glm::vec3 origin, glm::vec3 dir, float max);

    // Anne's functions
    void CreateTerrain();
    static float interpNoise2D(float x, float y);
    static float fbm(float x, float y);
    static float rand(glm::vec2 n);
    void makeRiver();
    void makeDeltaRiver();

    // Biome addition
    //float falloff(glm::vec2 p, glm::vec2 corner);
    //float perlin(float x, float y);
    glm::vec2 rand2(glm::vec2 n);
    glm::vec2 rand3(glm::vec2 n);
    float surflet(glm::vec2 p, glm::vec2 gridPoint, bool elevation);
    float perlinNoise(glm::vec2 uv, bool elevation); // boolean true uses rand2(), false uses rand3()
    float saltflats(glm::vec2 uv);
    float plateau(glm::vec2 uv);
    float forest(glm::vec2 uv);
    float island(glm::vec2 uv);
    bool setTree(int x, int z); // returns whether to set a tree at a (x, z) position
    void createTree(int x, int y, int z); // pass in (x, y, z) of block above terrain to create tree
    void drawIsland(glm::vec3 pos, float radius, int height); // pos height of top, center
    void drawIslandRecurse(glm::vec3 pos, float radius, int height);

    // Transparency addition
    // used in Chunk::Create() to draw hull of opaque blocks
    static bool isEmptyOrTransparent(BlockType b);

    bool multiThreading;
    bool isDone();
    QMutex* setBlockMutex;
    QMutex* doneThreadsMutex;

    friend class MyGL;
};
