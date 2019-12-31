#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>

#include <QThreadPool>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      mp_PPQuad(mkU<Quad>(this)), mp_geomSkyQuad(mkU<Quad>(this)), mp_geomQuad(mkU<Quad>(this)),
      mp_geomCube(mkU<Cube>(this)), mp_worldAxes(mkU<WorldAxes>(this)),
      mp_progLambert(mkU<ShaderProgram>(this)), mp_progFlat(mkU<ShaderProgram>(this)),
      mp_progPPNoOp(mkU<PostProcessShader>(this)), mp_progPPSwimming(mkU<PostProcessShader>(this)),
      mp_progSky(mkU<ShaderProgram>(this)),
      mp_camera(mkU<Camera>()), mp_player(mkU<Player>(glm::vec3(25, 254, 25), mp_camera.get())),
      prevTime(QDateTime::currentMSecsSinceEpoch()), startTime(QDateTime::currentMSecsSinceEpoch())
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    // Tell the timer to redraw 60 times per second
    timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible

}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);

    mp_PPQuad->destroy();
    mp_geomSkyQuad->destroy();
    mp_geomQuad->destroy();
    mp_geomCube->destroy();
    for (auto const& x : m_terrains)
    {
        (x.second)->destroyChunks();
    }
}


void MyGL::MoveMouseToCenter()
{
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    // Transparency addition
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    createRenderBuffers();

    //Create the instance of Cube, WorldAxes, and Quad
    mp_geomCube->create();
    mp_worldAxes->create();
    // set the frustrum depth to a number very close to the
    // near clipping plane since the default z-depth of a quad
    // is very close to the far-cliping plane
    mp_PPQuad->z = 0.00001f;
    mp_PPQuad->create();
    // create the quad that the sky will be drawn on
    // by default the z-depth is set to be very close to the
    // far-clipping plane
    mp_geomSkyQuad->create();
    mp_geomQuad->create();

    // Texture & normal addition 10
    createTexNor();
    loadTexNor();

    // Create and set up the diffuse shader
    mp_progLambert->create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    mp_progFlat->create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    // create post-processing shaders
    mp_progPPNoOp->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/noOp.frag.glsl");
    mp_progPPSwimming->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/swimming.frag.glsl");
    // Create and set up the sky shaders
    mp_progSky->create(":/glsl/sky.vert.glsl", ":/glsl/sky.frag.glsl");

    // Set a color with which to draw geometry since you won't have one
    // This makes your geometry render green.
    mp_progLambert->setGeometryColor(glm::vec4(0,1,0,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    // vao.bind();
    glBindVertexArray(vao);

    // move mouse to center
    MoveMouseToCenter();

    // m_terrains.insert(std::pair<std::pair<int, int>(0, 0), mkU<Terrain>(this, 0, 0)>);
    m_terrains.insert(std::pair<std::pair<int,int>,uPtr<Terrain>>(std::pair<int,int>(0,0),mkU<Terrain>(this, 0, 0)));
    m_terrains.insert(std::pair<std::pair<int,int>,uPtr<Terrain>>(std::pair<int,int>(0,1),mkU<Terrain>(this, 0, 1)));
    ///////////////// replace this with Anne's function////////////////
    // Anne's function will need to call initChunks() for her function
    // to work
    for (auto const& x : m_terrains)
    {
        (x.second)->multiThreading = false;
        /*(x.second)->setBlockMutex = setBlockMutex;
        (x.second)->doneThreadsMutex = doneThreadsMutex;*/
        (x.second)->CreateTerrain();
        m_terrainToBeCreated.push(x.second.get());
        // (x.second)->destroyChunks();
        // (x.second)->createChunks();
    }

    /*
     * RIVER CODE -- UNCOMMENT LATER
    //river1
    mp_currTerrain = m_terrains.at(std::pair<int, int>(0, 1)).get();
    mp_currTerrain -> makeRiver();

    //delta river
    mp_currTerrain = m_terrains.at(std::pair<int, int>(0, 0)).get();
    mp_currTerrain -> makeDeltaRiver();
    */

    //towers
    drawTowers();

    for (auto const& x : m_terrains)
    {
        (x.second)->destroyChunks();
        (x.second)->createChunks();
    }

}

void MyGL::drawTowers(){

    Turtle t = Turtle();
    Lsystem l = Lsystem(t, this);

    t = Turtle();
    t.orient = glm::vec3(1,0.1,1);
    t.pos = glm::vec3(1,170,1);
    l = Lsystem(t, int(0), this);

    std::string str= "F";
    str = l.expand(str, 3);
    std::cout<<"expanded: "<< str<<std::endl;
    l.draw(str);
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    // *mp_camera = Camera(w, h, glm::vec3(mp_currTerrain->dimensions.x, mp_currTerrain->dimensions.y * 0.75, mp_currTerrain->dimensions.z),
    //                    glm::vec3(mp_currTerrain->dimensions.x / 2, mp_currTerrain->dimensions.y / 2, mp_currTerrain->dimensions.z / 2), glm::vec3(0,1,0));
    *mp_camera = Camera(w, h, glm::vec3(64, 256 * 0.75, 64),
                          glm::vec3(64 / 2, 256 / 2, 64 / 2), glm::vec3(0,1,0));
    glm::mat4 viewproj = mp_camera->getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);

    mp_progPPNoOp->setDimensions(glm::ivec2(w, h));
    mp_progPPSwimming->setDimensions(glm::ivec2(w, h));

    // sky shader setup
    mp_progSky->setViewProjMatrix(glm::inverse(viewproj));
    mp_progSky->useMe();
    // assigns values to the locations in the gpu stored in the 1st
    // argruments and assigns corresponding values consecutively
    this->glUniform2i(mp_progSky->unifDimensions, width(), height());
    this->glUniform3f(mp_progSky->unifEye, mp_camera->eye.x, mp_camera->eye.y, mp_camera->eye.z);

    // set screen center
    mp_player->x_center = width() / 2;
    mp_player->y_center = height() / 2;

    printGLErrorLog();
}


// MyGL's constructor links timerUpdate() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to use timerUpdate
void MyGL::timerUpdate()
{
    // checks if player is near the edge and add new terrain
    newTerrainCheck();

    // calculate elapsed time
    float elapsedTime = QDateTime::currentMSecsSinceEpoch() - prevTime;
    prevTime = QDateTime::currentMSecsSinceEpoch();

    // find current terrain
    //newPos is the terrain near player
    glm::vec3 terrainPos = glm::vec3(floor(mp_player->pos[0]/64.f),
                                     floor(mp_player->pos[1]),
                                     floor(mp_player->pos[2]/64.f));

    // std::cout << "terrain idx: " << glm::to_string(terrainPos) << std::endl;

    try {
        mp_currTerrain = m_terrains.at(std::pair<int, int>(terrainPos[0], terrainPos[2])).get();
    } catch (const std::exception& e) {
        std::cout << "a standard exception was caught when trying to access the"
                  << " m_terrains map inside timerUpdate() with " << terrainPos[0] << ", "
                  << terrainPos[2] << std::endl;
        std::cout << "the message is: " << e.what() << std::endl;
    }

    // compute local pos in terrain
    glm::vec3 pos = globalToTerrain(mp_player->pos);
    // std::cout << glm::to_string(pos) << std::endl;

    // check if aerial
    if (mp_player->fly_mode) {
        mp_player->aerial = false;
    } else {
        if (glm::fract(pos[1]) > 0.01f) {
            mp_player->aerial = true;
        } else {
            glm::vec3 pos1 = globalToTerrain(glm::vec3(pos[0] - 0.5f, pos[1] - 1.f, pos[2] - 0.5f));
            glm::vec3 pos2 = globalToTerrain(glm::vec3(pos[0] + 0.5f, pos[1] - 1.f, pos[2] - 0.5f));
            glm::vec3 pos3 = globalToTerrain(glm::vec3(pos[0] + 0.5f, pos[1] - 1.f, pos[2] + 0.5f));
            glm::vec3 pos4 = globalToTerrain(glm::vec3(pos[0] - 0.5f, pos[1] - 1.f, pos[2] + 0.5f));
            // four corners of the base
            bool aerial1 = (mp_currTerrain->getBlockAt((int) glm::floor(pos1[0]),
                       (int) glm::round(pos1[1]), (int) glm::floor(pos1[2])) == EMPTY);
            bool aerial2 = (mp_currTerrain->getBlockAt((int) glm::floor(pos2[0]),
                         (int) glm::round(pos2[1]), (int) glm::floor(pos2[2])) == EMPTY);
            bool aerial3 = (mp_currTerrain->getBlockAt((int) glm::floor(pos3[0]),
                         (int) glm::round(pos3[1]), (int) glm::floor(pos3[2])) == EMPTY);
            bool aerial4 = (mp_currTerrain->getBlockAt((int) glm::floor(pos4[0]),
                         (int) glm::round(pos4[1]), (int) glm::floor(pos4[2])) == EMPTY);
            mp_player->aerial = aerial1 && aerial2 && aerial3 && aerial4;
        }

    }

    // check if swimming
    BlockType currTopBlock = mp_currTerrain->getBlockAt((int) glm::floor(pos[0]),
            (int) glm::floor(pos[1] + 1), (int) glm::floor(pos[2]));
    if (currTopBlock == LAVA) {
        mp_PPQuad->color = glm::vec4(1, 0, 0, 0.3);
        mp_PPQuad->destroy();
        mp_PPQuad->create();
        mp_player->swimming = true;
    } else if (currTopBlock == WATER) {
        mp_PPQuad->color = glm::vec4(0, 0.2, 1, 0.3);
        mp_PPQuad->destroy();
        mp_PPQuad->create();
        mp_player->swimming = true;
    } else {
        mp_player->swimming = false;
    }

    // scale acceleration based on time elapsed
    mp_player->acc = elapsedTime * mp_player->baseAcc / 16.f;

    // update velocity
    mp_player->readUserInput();

    // check collisions based on new pos
    if (!glm::all(glm::equal(mp_player->vel, glm::vec3(0.f)))) {
        // the corners of the player's bounding box
        std::vector<glm::vec3> player_offset;
        player_offset.push_back(glm::vec3(-0.5f, 0.f, -0.5f));
        player_offset.push_back(glm::vec3(0.5f, 0.f, -0.5f));
        player_offset.push_back(glm::vec3(0.5f, 0.f, 0.5f));
        player_offset.push_back(glm::vec3(-0.5f, 0.f, 0.5f));
        player_offset.push_back(glm::vec3(-0.5f, 1.f, -0.5f));
        player_offset.push_back(glm::vec3(0.5f, 1.f, -0.5f));
        player_offset.push_back(glm::vec3(0.5f, 1.f, 0.5f));
        player_offset.push_back(glm::vec3(-0.5f, 1.f, 0.5f));
        player_offset.push_back(glm::vec3(-0.5f, 2.f, -0.5f));
        player_offset.push_back(glm::vec3(0.5f, 2.f, -0.5f));
        player_offset.push_back(glm::vec3(0.5f, 2.f, 0.5f));
        player_offset.push_back(glm::vec3(-0.5f, 2.f, 0.5f));
        glm::vec3 dir = glm::normalize(mp_player->vel);
        float min_dist = glm::length(mp_player->vel);
        //glm::vec3 collision_offset = glm::vec3(-1.f);
        if (!mp_player->fly_mode) {
            for (int i = 0; i < player_offset.size(); i++) {
                glm::vec2 curr_dist = distToBlock(mp_player->pos + player_offset[i], dir, min_dist);
                if (curr_dist[0] != -1.f && curr_dist[0] < min_dist) {
                    min_dist = curr_dist[0];
                    mp_player->vel = glm::vec3(0.f);
                    //collision_offset = player_offset[i];
                }
            }
        }

        // use velocity to update pos
        mp_player->pos = mp_player->pos + (min_dist * dir);
        // update other entities if necessary

        // std::cout << mp_player->aerial << std::endl;
        // std::cout << "vel: " << glm::to_string(mp_player->vel) << std::endl;
        // std::cout << "pos: " << glm::to_string(mp_player->pos) << std::endl;

        // Cathrine's print statements:
        // std::cout << "global pos: " << glm::to_string(mp_player->pos) << std::endl;
        // std::cout << "terrain pos: " << glm::to_string(pos) << std::endl;
    }

    // set camera pos to player pos
    mp_camera->eye = mp_player->pos + glm::vec3(0, 1.8f, 0);
    mp_camera->ref = mp_player->pos + glm::vec3(0, 1.8f, 0) + mp_camera->look;
    mp_camera->RecomputeAttributes();

    // Blinn-Phong addition
    // set Camera pos -- global
    mp_progLambert->setCameraPos(glm::vec4(mp_camera->eye, 1.f));

    // Time addition 6
    // SET TIME IN SHADER
    mp_progLambert->setTime(QDateTime::currentMSecsSinceEpoch() - startTime);

    update();
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mp_progFlat->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setViewProjMatrix(mp_camera->getViewProj());

    mp_progSky->setViewProjMatrix(glm::inverse(mp_camera->getViewProj()));
    mp_progSky->useMe();
    this->glUniform3f(mp_progSky->unifEye, mp_camera->eye.x, mp_camera->eye.y, mp_camera->eye.z);
    this->glUniform1f(mp_progSky->unifTime, float(double(QDateTime::currentMSecsSinceEpoch() - startTime)));
    mp_progSky->draw(*mp_geomSkyQuad);


    // std::cout << "there is no problem before this point" << std::endl;

    GLDrawScene();
    if (mp_player->swimming)
    {
        mp_progFlat->setViewProjMatrix(glm::mat4(1.f));
        mp_progFlat->setModelMatrix(glm::mat4(1.f));
        mp_progFlat->draw(*mp_PPQuad);
        // performPostprocessRenderPass();
    }

    glDisable(GL_DEPTH_TEST);
    mp_progFlat->setViewProjMatrix(mp_camera->getViewProj());
    mp_progFlat->setModelMatrix(glm::mat4());
    // mp_progFlat->draw(*mp_worldAxes);
    glEnable(GL_DEPTH_TEST);
}

void MyGL::GLDrawScene()
{
    // how draw scene is implemented for createTestScene() method
    /*
    for(int x = 0; x < mp_currTerrain->dimensions.x; ++x)
    {
        for(int y = 0; y < mp_currTerrain->dimensions.y; ++y)
        {
            for(int z = 0; z < mp_currTerrain->dimensions.z; ++z)
            {
                BlockType t;
                if((t = mp_currTerrain->m_blocks[x][y][z]) != EMPTY)
                {
                    switch(t)
                    {
                    case DIRT:
                        mp_progLambert->setGeometryColor(glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f);
                        break;
                    case GRASS:
                        mp_progLambert->setGeometryColor(glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f);
                        break;
                    case STONE:
                        mp_progLambert->setGeometryColor(glm::vec4(0.5f));
                        break;
                    default:
                        // Other types are as of yet not defined
                        break;
                    }
                    mp_progLambert->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(x, y, z)));
                    mp_progLambert->draw(*mp_geomCube);
                }
            }
        }
    }
    */

    // mp_currTerrain->drawChunks(mp_progLambert.get());
    // m_terrains.at(std::pair<int, int>(0, 0))->drawChunks(mp_progLambert.get());

    // for post-processing render pass
    /*
    if (mp_player->swimming)
    {
        // Render to our framebuffer rather than the viewport
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
        // Render on the whole framebuffer, complete from the lower left corner to the upper right
        glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
        // Clear the screen so that we only see newly drawn images
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw the background texture first
        // mp_modelCurrent->bindBGTexture();
        // mp_progPostprocessNoOp->draw(m_geomQuad, 2);

        // Set the surface shader's transformation matrices
        // mp_progSurfaceCurrent->setViewProjMatrix(m_camera.getView(), m_camera.getProj());
        // mp_progSurfaceCurrent->setModelMatrix(glm::mat4());

        // bindAppropriateTexture();

        // Draw the model
        // mp_progSurfaceCurrent->draw(*mp_modelCurrent, 0);
    }
    */

    createCompletedTerrain();

    for (auto const& x : m_terrains)
    {
        (x.second)->drawChunks(mp_progLambert.get());
    }
}

void MyGL::performPostprocessRenderPass()
{
    // Render the frame buffer as a texture on a screen-size quad

    // Tell OpenGL to render to the viewport's frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Bind our texture in Texture Unit 2
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);

    mp_progPPSwimming->draw(*mp_geomQuad, 0);
}



void MyGL::createRenderBuffers()
{
    // Initialize the frame buffers and render textures
    glGenFramebuffers(1, &m_frameBuffer);
    glGenTextures(1, &m_renderedTexture);
    glGenRenderbuffers(1, &m_depthRenderBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Bind our texture so that all functions that deal with textures will interact with this one
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio(), 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);

    // Set the render settings for the texture we've just created.
    // Essentially zero filtering on the "texture" so it appears exactly as rendered
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Clamp the colors at the edge of our texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Initialize our depth buffer
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);

    // Set m_renderedTexture as the color output of our frame buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_renderedTexture, 0);

    // Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0, which we previously set to m_renderedTextures[i]
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers); // "1" is the size of drawBuffers

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Frame buffer did not initialize correctly..." << std::endl;
        printGLErrorLog();
    }
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    }
    mp_player->keyPressEvent(e);

    /*
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        mp_camera->RotateAboutUp(-amount);
    } else if (e->key() == Qt::Key_Left) {
        mp_camera->RotateAboutUp(amount);
    } else if (e->key() == Qt::Key_Up) {
        mp_camera->RotateAboutRight(-amount);
    } else if (e->key() == Qt::Key_Down) {
        mp_camera->RotateAboutRight(amount);
    } else if (e->key() == Qt::Key_1) {
        mp_camera->fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        mp_camera->fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        mp_camera->TranslateAlongLook(amount);
    } else if (e->key() == Qt::Key_S) {
        mp_camera->TranslateAlongLook(-amount);
    } else if (e->key() == Qt::Key_D) {
        mp_camera->TranslateAlongRight(amount);
    } else if (e->key() == Qt::Key_A) {
        mp_camera->TranslateAlongRight(-amount);
    } else if (e->key() == Qt::Key_Q) {
        mp_camera->TranslateAlongUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        mp_camera->TranslateAlongUp(amount);
    } else if (e->key() == Qt::Key_R) {
        *mp_camera = Camera(this->width(), this->height());
    }
    mp_camera->RecomputeAttributes();*/
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    mp_player->keyReleaseEvent(e);
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    mp_player->mouseMoveEvent(e);
    MoveMouseToCenter();
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        delBlock();
    } else if (e->button() == Qt::RightButton) {
        addBlock();
    }
}

// Add & Delete blocks--Anne
void MyGL::addBlock(){

    //std::cout << "Add block" << std::endl;
    glm::vec2 result = distToBlock(mp_camera->eye, mp_camera->look, 4.f);
    //std::cout << "Add block result: " << glm::to_string(result) << std::endl;

    //no block found
    if (result[0] == -1){
        return;
    }

    //position at the point of collision
    glm::vec3 position = mp_camera->eye  + mp_camera->look * (result[0] - 0.01f);

    //find interger location of block
    glm::ivec3 cell = glm::ivec3(glm::floor(position[0]),
                                glm::floor(position[1]),
                                glm::floor(position[2]));


    int i = result[1];

    if (i == -1){
        return;
    }

    glm::vec3 newPos = globalToTerrain(glm::vec3(cell));

    /*
    if(mp_camera->look[i] > 0){
        newPos[i] = newPos[i] - 1;
    }*/

    std::cout << glm::to_string(newPos) << std::endl;
    mp_currTerrain->setBlockAt((int)newPos[0], (int)newPos[1], (int)newPos[2], LAVA);
    mp_currTerrain->destroyChunks();
    mp_currTerrain->createChunks();
}

void MyGL::delBlock() {

    //std::cout << "Delete block" << std::endl;
    glm::vec2 result = distToBlock(mp_camera->eye, mp_camera->look, 4.f);
    //std::cout << "Delete block result: " << glm::to_string(result) << std::endl;

    //no block found
    if (result[0] == -1){
        return;
    }

    //position at the point of collision
    glm::vec3 position = mp_camera->eye + mp_camera->look * (result[0] + 0.01f);

    //find interger location of block
    glm::ivec3 cell = glm::ivec3(glm::floor(position[0]),
                                glm::floor(position[1]),
                                glm::floor(position[2]));
    cell = globalToTerrain(glm::vec3(cell));
    mp_currTerrain->setBlockAt(cell[0], cell[1], cell[2], EMPTY);
    mp_currTerrain->destroyChunks();
    mp_currTerrain->createChunks();
    //mp_currTerrain -> m_tesetBlockAt(cell[0], cell[1], cell[2], EMPTY);
}

void MyGL::newTerrainCheck(){

    //check if x or z position is near boundary
    for (int i = 0; i < 3; i++) {

        //don't check for y
        if(i == 1){
            continue;
        }

        float mod = fabs(fmod(mp_player->pos[i], 64.f));
        // std::cout << i << ": " << mp_player->pos[i] << std::endl;
        // std::cout << "mod: " << mod << std::endl;
        if (mod <= 15.f
            || mod >= 50.f) {

            //newPos is the terrain near player's current terrain
            glm::vec3 newPos = glm::vec3(floor(mp_player->pos[0]/64.0),
                                      floor(mp_player->pos[1]/64.0),
                                      floor(mp_player->pos[2]/64.0));

            //change position of i
            int roundPos = glm::round(mp_player->pos[i]/64.0);

            //minus one if terrain is on left side
            if (newPos[i] == roundPos) {
                newPos[i] = newPos[i] -1;
            } else {
                newPos[i] = roundPos;
            }

            newTerrain(newPos);
        }
    }

    //check both x and z direction, see if diagonal terrain must be created
    float modx = fabs(fmod(mp_player->pos[0], 64.f));
    float modz = fabs(fmod(mp_player->pos[2], 64.f));
    if ((modx <= 15.f || modx >= 50.f) &&
        (modz <= 15.f || modz >= 50.f)) {

        //newPos is the terrain near player's current terrain
        glm::vec3 newPos = glm::vec3(floor(mp_player->pos[0]/64.0),
                                  floor(mp_player->pos[1]/64.0),
                                  floor(mp_player->pos[2]/64.0));

        for(int i = 0; i <= 2; i++){
            //change position of i
            int roundPos = glm::round(mp_player->pos[i]/64.0);

            //minus one if terrain is on left side
            if(newPos[i] == roundPos){
                newPos[i] = newPos[i] -1;
            } else{
                newPos[i] = roundPos;
            }
        }

        newTerrain(newPos);
  }
}

void MyGL::newTerrain(glm::vec3 newPos){
    //if map has this terrain, dont generate new one
    if(m_terrains.find(std::pair<int,int>(newPos[0],newPos[2])) != m_terrains.end()){
        return;
    }

     // std::cout<<"generate terrain"<<std::endl;
     // std::cout<<"new pos:  "<< glm::to_string(newPos)<<std::endl;

    //create new terrain
     uPtr<Terrain> newTerrain =  mkU<Terrain>(this, (int)newPos[0], (int)newPos[2]);
    //create with x and z values
    newTerrain->CreateTerrain();
    // add terrain that is being computed to a queue
    m_terrainToBeCreated.push(newTerrain.get());
    // add to map
    // we are no longer creating the chunks immediately becasue we
    // have to wait until all of the fbm data has been generated
    // createChunks is now called in createCompletedChunks()
    // newTerrain->createChunks();
    m_terrains.insert(std::pair<std::pair<int,int>,uPtr<Terrain>>(std::pair<int,int>(newPos[0],newPos[2]),std::move(newTerrain)));
}

void MyGL::createCompletedTerrain()
{
    // std::cout << "Queue size: " << m_terrainToBeCreated.size() << std::endl;
    if (!m_terrainToBeCreated.empty())
    {
        // std::cout << "Checknig the oldest terrain object to see if its done computing" << std::endl;
        if (m_terrainToBeCreated.front()->isDone())
        {
            m_terrainToBeCreated.front()->destroyChunks();
            m_terrainToBeCreated.front()->createChunks();
            m_terrainToBeCreated.pop();
            std::cout << "here" << std::endl;
        }
    }
    return;
}

// ORIGIN O NEEDS TO BE IN WORLD COORDINATES
glm::vec2 MyGL::distToBlock(glm::vec3 o, glm::vec3 dir, float max){
    glm::vec3 origin = o + dir * 0.001f;

    //cell corner coordinate
    glm::vec3 cell = glm::vec3(glm::floor(origin[0]),
                                glm::floor(origin[1]),
                                glm::floor(origin[2]));

    //sign of direction
    // if the component is 1 then the sign of that componnent was
    // positive otherwise (negative or zero) the component will be 0
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

    // min_i indicates whether x, y, or z are just incremented
    int min_i = -1;

    int w = 0;
    // keep checking t as long as it's less than the max
    // and you should try to raycast no more than 10 times
    while(t < max && w < 10) {
        w++;
        //find terrain position
        int terrainX = glm::floor(cell[0]/ 64.f);
        int terrainZ = glm::floor(cell[2]/ 64.f);
        Terrain* terrain = nullptr;
        std::pair<int, int> key = std::pair<int, int>(terrainX, terrainZ);
        try {
            terrain = m_terrains.at(key).get();
        } catch (const std::exception& e) {
            for ( auto it = m_terrains.begin(); it != m_terrains.end(); ++it ) {
                std::cout << " " << it->first.first << ", " << it->first.second << ":" << it->second.get();
                std::cout << std::endl;
            }

            std::cout << "m_terrains was accessed in the distToBlock() method "
                      << "with a key that does not exist" << std::endl
                      << "the pair that was used to access the m_terrains map "
                      << "was: " << terrainX << ", " << terrainZ << std::endl;
            std::cout << mp_player->pos[0] << ", " << mp_player->pos[1] << ", " << mp_player->pos[2] << std::endl;
            std::cout <<std::endl << "SIZE OF TERRAINS MAP: " << m_terrains.size() << std::endl;
            std::cout << e.what() << std::endl;
        }

        int cellX = globalToTerrain(cell)[0];
        int cellZ = globalToTerrain(cell)[2];

        // break when non-permeable block is hit
        BlockType hitBlock = terrain->getBlockAt(int(cellX), int(cell[1]), int(cellZ));
        if (hitBlock != EMPTY && hitBlock != LAVA && hitBlock != WATER) {
            return glm::vec2(t, float(min_i));
        }

        // do the following in case a permeable block is hit...
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

        // compute cell position
        cell = glm::floor(origin + (float)fabs(minT) * dir);

        // subtract one if ray from the positive direction
        if(dir[min_i] < 0){
            cell[min_i] = cell[min_i] - 1;
        }

        // calculate new want
        want = cell + sign;
        t = minT;
    }

    // no block found
    return glm::vec2(-1,-1);
}

glm::vec3 MyGL::globalToTerrain(glm::vec3 pos) {
    float x = fmod(pos[0], 64.f);
    float z = fmod(pos[2], 64.f);
    if(x < 0){
        x += 64;
    }
    if(z < 0){
        z += 64;
    }
    return glm::vec3(x, pos[1], z);
}

// Texture & normal addition 6
void MyGL::createTexNor() {
    // make texture
    uPtr<Texture> texture = mkU<Texture>(this);
    // SWITCH BELOW COMMENTS TO SWITCH TEXTURE MAP
    // texture->create(":/minecraft_textures_all/minecraft_textures_all.png");
    texture->create(":/minecraft_textures_all/minecraft_textures_all_grey_grass.png");
    mp_texture = std::move(texture);

    // make normal
    uPtr<Texture> normal = mkU<Texture>(this);
    normal->create(":/minecraft_textures_all/minecraft_normals_all.png");
    mp_normal = std::move(normal);
}

// texture & normal addition 11
void MyGL::loadTexNor() {
    mp_texture->load(0);
    mp_normal->load(1);
}

// Texture & normal addition 9
void MyGL::bindTexNor() {
    mp_texture->bind(0);
    mp_normal->bind(1);
}
