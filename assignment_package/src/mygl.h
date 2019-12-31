#pragma once

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/cube.h>
#include <scene/worldaxes.h>
#include "camera.h"
#include <scene/quad.h>
#include <scene/terrain.h>
#include <player.h>
#include <texture.h>
#include <postprocessshader.h>
#include <queue>
#include <QMutex>

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <smartpointerhelp.h>

#include <lsystem.h>

class Terrain;
class Lsystem;

class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    uPtr<Quad> mp_PPQuad;
    uPtr<Quad> mp_geomSkyQuad;
    uPtr<Quad> mp_geomQuad;
    uPtr<Cube> mp_geomCube;// The instance of a unit cube we can use to render any cube. Should NOT be used in final version of your project.
    uPtr<WorldAxes> mp_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).

    // the shader programs and post-processor shaders are GPU programs that
    // represent their respective vertex and fragment shaders
    uPtr<ShaderProgram> mp_progLambert;// A shader program that uses lambertian reflection
    uPtr<ShaderProgram> mp_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    uPtr<PostProcessShader> mp_progPPNoOp;
    uPtr<PostProcessShader> mp_progPPSwimming;
    uPtr<ShaderProgram> mp_progSky; // A screen-space shader for creating the sky background

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    // A collection of handles to the five frame buffers we've given
    // ourselves to perform render passes. The 0th frame buffer is always
    // written to by the render pass that uses the currently bound surface shader.
    GLuint m_frameBuffer;
    // A collection of handles to the textures used by the frame buffers.
    // m_frameBuffers[i] writes to m_renderedTextures[i].
    GLuint m_renderedTexture;
    // A collection of handles to the depth buffers used by our frame buffers.
    // m_frameBuffers[i] writes to m_depthRenderBuffers[i].
    GLuint m_depthRenderBuffer;

    uPtr<Camera> mp_camera;
    uPtr<Player> mp_player;
    std::vector<uPtr<Player>> mp_otherEntities;
    std::map<std::pair<int, int>, uPtr<Terrain>> m_terrains;
    std::queue<Terrain*> m_terrainToBeCreated;
    Terrain* mp_currTerrain; // current terrain

    // Texture & normal addition 7
    uPtr<Texture> mp_texture;
    uPtr<Texture> mp_normal;

    /// Timer linked to timerUpdate(). Fires approx. 60 times per second
    QTimer timer;
    int64_t prevTime;
    int64_t startTime;

    void MoveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.
    void addBlock();
    void delBlock();
    //checks if new terrain shall be created
    void newTerrainCheck();
    //creates new terrain
    void newTerrain(glm::vec3 newPos);
    void createCompletedTerrain();
    glm::vec3 globalToTerrain(glm::vec3 pos);

    // Texture & normal addition 5
    // creates texture & normal
    void createTexNor();
    // Texture & normal addition 12
    // loads texture & normal
    void loadTexNor();
    // Texture & normal addition 8
    // binds texture & normal
    void bindTexNor();

    // applies the color filter to the normally rendered scene
    void performPostprocessRenderPass();
    // sets up the frame buffer handles
    void createRenderBuffers();

    QMutex* setBlockMutex;
    QMutex* doneThreadsMutex;

    void drawTowers();

public:
    friend class Lsystem;
    explicit MyGL(QWidget *parent = 0);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void GLDrawScene();

    // ORIGIN O NEEDS TO BE IN WORLD COORDINATES
    glm::vec2 distToBlock(glm::vec3 o, glm::vec3 dir, float max);



protected:
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);

private slots:
    /// Slot that gets called ~60 times per second
    void timerUpdate();
};


