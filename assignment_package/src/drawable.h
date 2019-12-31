#pragma once

#include <openglcontext.h>
#include <la.h>

//This defines a class which can be rendered by our shader program.
//Make any geometry a subclass of ShaderProgram::Drawable in order to render it with the ShaderProgram class.
class Drawable
{
protected:
    int count;     // The number of indices stored in bufIdx.
    GLuint bufIdx; // A Vertex Buffer Object that we will use to store triangle indices (GLuints)
    GLuint bufPos; // A Vertex Buffer Object that we will use to store mesh vertices (vec4s)
    GLuint bufNor; // A Vertex Buffer Object that we will use to store mesh normals (vec4s)
    GLuint bufCol; // Can be used to pass per-vertex color information to the shader, but is currently unused.
                   // Instead, we use a uniform vec4 in the shader to set an overall color for the geometry
    // Texture & Normal addition
    GLuint bufUV;  // A Vertex Buffer Object that we will use to store triangle UVs (vec2s)
    // Blinn-Phong addition
    GLuint bufCos;
    // Time addition
    GLuint bufAnimateable;
    // a pointer to a vertex buffer object that i will use to store
    // tehe mesh vertices, normals, and colors
    GLuint bufInterleaf;

    // Transparency addition
    int countTrans;
    GLuint bufIdxTrans;
    GLuint bufInterleafTrans;

    bool idxBound; // Set to TRUE by generateIdx(), returned by bindIdx().
    bool posBound;
    bool norBound;
    bool colBound;
    // Texture & Normal addition
    bool uvBound;
    // Blinn-Phong addition
    bool cosBound;
    // Time addition
    bool animateableBound;
    // for interleaved vbo information
    bool interleafBound;

    // Transparency addition
    bool idxTransBound;
    bool interleafTransBound;

    OpenGLContext* context; // Since Qt's OpenGL support is done through classes like QOpenGLFunctions_3_2_Core,
                          // we need to pass our OpenGL context to the Drawable in order to call GL functions
                          // from within this class.


public:
    Drawable(OpenGLContext* context);
    virtual ~Drawable();

    virtual void create() = 0; // To be implemented by subclasses. Populates the VBOs of the Drawable.
    void destroy(); // Frees the VBOs of the Drawable.

    // Getter functions for various GL data
    virtual GLenum drawMode();
    int elemCount();
    int elemCountTrans();

    // Call these functions when you want to call glGenBuffers on the buffers stored in the Drawable
    // These will properly set the values of idxBound etc. which need to be checked in ShaderProgram::draw()
    void generateIdx();
    void generatePos();
    void generateNor();
    void generateCol();
    // Texture & Normal addition
    void generateUV();
    // Blinn-Phong addition
    void generateCos();
    // Time addition
    void generateAnimateable();
    // for interleaved vbo information
    void generateInterleaf();

    // Transparency addition
    void generateIdxTrans();
    void generateInterleafTrans();

    bool bindIdx();
    bool bindPos();
    bool bindNor();
    bool bindCol();
    // Texture & Normal addition
    bool bindUV();
    // Blinn-Phong addition
    bool bindCos();
    // Time addition
    bool bindAnimateable();
    // for interleaved vbo information
    bool bindInterleaf();

    // Transparency addition
    bool bindIdxTrans();
    bool bindInterleafTrans();
};
