#include "drawable.h"
#include <la.h>

// #include <iostream>

Drawable::Drawable(OpenGLContext* context)
    : bufIdx(), bufPos(), bufNor(), bufCol(), bufUV(), bufCos(), bufAnimateable(), bufInterleaf(), bufIdxTrans(), bufInterleafTrans(),
      idxBound(false), posBound(false), norBound(false), colBound(false), uvBound(false),
      cosBound(false), animateableBound(false),
      interleafBound(false), idxTransBound(false), interleafTransBound(false),
      context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroy()
{
    context->glDeleteBuffers(1, &bufIdx);
    context->glDeleteBuffers(1, &bufPos);
    context->glDeleteBuffers(1, &bufNor);
    context->glDeleteBuffers(1, &bufCol);
    context->glDeleteBuffers(1, &bufUV);
    context->glDeleteBuffers(1, &bufCos);
    context->glDeleteBuffers(1, &bufAnimateable);
    context->glDeleteBuffers(1, &bufInterleaf);
    context->glDeleteBuffers(1, &bufInterleafTrans);
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return count;
}

// Transparency addition
int Drawable::elemCountTrans()
{
    return countTrans;
}

void Drawable::generateIdx()
{
    idxBound = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdx);
}

void Drawable::generatePos()
{
    posBound = true;
    // Create a VBO on our GPU and store its handle in bufPos
    context->glGenBuffers(1, &bufPos);
}

void Drawable::generateNor()
{
    norBound = true;
    // Create a VBO on our GPU and store its handle in bufNor
    context->glGenBuffers(1, &bufNor);
}

void Drawable::generateCol()
{
    colBound = true;
    // Create a VBO on our GPU and store its handle in bufCol
    context->glGenBuffers(1, &bufCol);
}

// Texture & Normal addition
void Drawable::generateUV()
{
    uvBound = true;
    // Create a VBO on our GPU and store its handle in bufUV
    context->glGenBuffers(1, &bufUV);
}

// Blinn-Phong addition
void Drawable::generateCos()
{
    cosBound = true;
    // Create a VBO on our GPU and store its handle in bufCol
    context->glGenBuffers(1, &bufCos);
}

// Time addition
void Drawable::generateAnimateable()
{
    animateableBound = true;
    // Create a VBO on our GPU and store its handle in bufCol
    context->glGenBuffers(1, &bufAnimateable);
}

bool Drawable::bindIdx()
{
    if(idxBound) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    }
    return idxBound;
}

bool Drawable::bindPos()
{
    if(posBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    }
    return posBound;
}

bool Drawable::bindNor()
{
    if(norBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    }
    return norBound;
}

bool Drawable::bindCol()
{
    if(colBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    }
    return colBound;
}

// Texture & Normal addition
bool Drawable::bindUV()
{
    if(uvBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    }
    return uvBound;
}

// Blinn-Phong addition
bool Drawable::bindCos()
{
    if(cosBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufCos);
    }
    return cosBound;
}

// Time addition
bool Drawable::bindAnimateable()
{
    if(animateableBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufAnimateable);
    }
    return animateableBound;
}


///////////////////Interleaf Member Functions Definitions////////////////

void Drawable::generateInterleaf()
{
    interleafBound = true;
    // Create a VBO on our GPU and store its handle in bufInterleaf
    context->glGenBuffers(1, &bufInterleaf);
}

bool Drawable::bindInterleaf()
{
    if (interleafBound) {
        // Once you have the handle, give the handle a type
        context->glBindBuffer(GL_ARRAY_BUFFER, bufInterleaf);
        // std::cout << "here" << std::endl;
    }
    return interleafBound;
}

// Transparency addition
void Drawable::generateIdxTrans()
{
    idxTransBound = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdxTrans);
}

bool Drawable::bindIdxTrans()
{
    if(idxTransBound) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxTrans);
    }
    return idxTransBound;
}

void Drawable::generateInterleafTrans()
{
    interleafTransBound = true;
    // Create a VBO on our GPU and store its handle in bufInterleaf
    context->glGenBuffers(1, &bufInterleafTrans);
}

bool Drawable::bindInterleafTrans()
{
    if (interleafTransBound) {
        // Once you have the handle, give the handle a type
        context->glBindBuffer(GL_ARRAY_BUFFER, bufInterleafTrans);
        // std::cout << "here" << std::endl;
    }
    return interleafTransBound;
}
