#include "shaderprogram.h"
#include <QFile>
#include <QStringBuilder>
#include <QTextStream>
#include <QDebug>

#include <iostream>


ShaderProgram::ShaderProgram(OpenGLContext *context)
    : vertShader(), fragShader(), prog(),
      attrPos(-1), attrNor(-1), attrCol(-1), attrUV(-1), attrCos(-1), attrAnimateable(-1),
      unifModel(-1), unifModelInvTr(-1), unifViewProj(-1), unifColor(-1),
      unifSamplerTex(-1), unifSamplerNor(-1), unifTime(-1), unifCameraPos(-1),
      unifDimensions(-1), unifEye(-1),
      context(context)
{}

void ShaderProgram::create(const char *vertfile, const char *fragfile)
{
    std::cout << "Setting up shader from " << vertfile << " and " << fragfile << std::endl;
    // Allocate space on our GPU for a vertex shader and a fragment shader and a shader program to manage the two
    vertShader = context->glCreateShader(GL_VERTEX_SHADER);
    fragShader = context->glCreateShader(GL_FRAGMENT_SHADER);
    prog = context->glCreateProgram();
    // Get the body of text stored in our two .glsl files
    QString qVertSource = qTextFileRead(vertfile);
    QString qFragSource = qTextFileRead(fragfile);

    char* vertSource = new char[qVertSource.size()+1];
    strcpy(vertSource, qVertSource.toStdString().c_str());
    char* fragSource = new char[qFragSource.size()+1];
    strcpy(fragSource, qFragSource.toStdString().c_str());


    // Send the shader text to OpenGL and store it in the shaders specified by the handles vertShader and fragShader
    context->glShaderSource(vertShader, 1, &vertSource, 0);
    context->glShaderSource(fragShader, 1, &fragSource, 0);
    // Tell OpenGL to compile the shader text stored above
    context->glCompileShader(vertShader);
    context->glCompileShader(fragShader);
    // Check if everything compiled OK
    GLint compiled;
    context->glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printShaderInfoLog(vertShader);
    }
    context->glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printShaderInfoLog(fragShader);
    }

    // Tell prog that it manages these particular vertex and fragment shaders
    context->glAttachShader(prog, vertShader);
    context->glAttachShader(prog, fragShader);
    context->glLinkProgram(prog);

    // Check for linking success
    GLint linked;
    context->glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked) {
        printLinkInfoLog(prog);
    }

    // Get the handles to the variables stored in our shaders
    // See shaderprogram.h for more information about these variables

    attrPos = context->glGetAttribLocation(prog, "vs_Pos");
    attrNor = context->glGetAttribLocation(prog, "vs_Nor");
    attrCol = context->glGetAttribLocation(prog, "vs_Col");
    // Texture & Normal addition
    attrUV = context->glGetAttribLocation(prog, "vs_UV");
    //std::cout << attrUV << std::endl;
    // Blinn-Phong addition
    attrCos = context->glGetAttribLocation(prog, "vs_Cos");
    //std::cout << attrCos << std::endl;
    // Time addition
    attrAnimateable = context->glGetAttribLocation(prog, "vs_Animateable");

    unifModel      = context->glGetUniformLocation(prog, "u_Model");
    unifModelInvTr = context->glGetUniformLocation(prog, "u_ModelInvTr");
    unifViewProj   = context->glGetUniformLocation(prog, "u_ViewProj");
    unifColor      = context->glGetUniformLocation(prog, "u_Color");
    // Texture addition 2
    unifSamplerTex  = context->glGetUniformLocation(prog, "u_Texture");
    // Normal addition 2
    unifSamplerNor = context->glGetUniformLocation(prog, "u_Normal");
    // Time addition 2
    unifTime = context->glGetUniformLocation(prog, "u_Time");
    // Blinn-Phong addition
    unifCameraPos = context->glGetUniformLocation(prog, "u_CameraPos");

    // Sky additions
    // tells the gpu to prepare a location in memory for 1 uniform variable
    // and another uniform variable
    unifDimensions = context->glGetUniformLocation(prog, "u_Dimensions");
    unifEye = context->glGetUniformLocation(prog, "u_Eye");
}

void ShaderProgram::useMe()
{
    context->glUseProgram(prog);
}

void ShaderProgram::setModelMatrix(const glm::mat4 &model)
{
    useMe();

    if (unifModel != -1) {
        // Pass a 4x4 matrix into a uniform variable in our shader
                        // Handle to the matrix variable on the GPU
        context->glUniformMatrix4fv(unifModel,
                        // How many matrices to pass
                           1,
                        // Transpose the matrix? OpenGL uses column-major, so no.
                           GL_FALSE,
                        // Pointer to the first element of the matrix
                           &model[0][0]);
    }

    if (unifModelInvTr != -1) {
        glm::mat4 modelinvtr = glm::inverse(glm::transpose(model));
        // Pass a 4x4 matrix into a uniform variable in our shader
                        // Handle to the matrix variable on the GPU
        context->glUniformMatrix4fv(unifModelInvTr,
                        // How many matrices to pass
                           1,
                        // Transpose the matrix? OpenGL uses column-major, so no.
                           GL_FALSE,
                        // Pointer to the first element of the matrix
                           &modelinvtr[0][0]);
    }
}

void ShaderProgram::setViewProjMatrix(const glm::mat4 &vp)
{
    // Tell OpenGL to use this shader program for subsequent function calls
    useMe();

    if(unifViewProj != -1) {
    // Pass a 4x4 matrix into a uniform variable in our shader
                    // Handle to the matrix variable on the GPU
    context->glUniformMatrix4fv(unifViewProj,
                    // How many matrices to pass
                       1,
                    // Transpose the matrix? OpenGL uses column-major, so no.
                       GL_FALSE,
                    // Pointer to the first element of the matrix
                       &vp[0][0]);
    }
}

void ShaderProgram::setGeometryColor(glm::vec4 color)
{
    useMe();

    if(unifColor != -1)
    {
        context->glUniform4fv(unifColor, 1, &color[0]);
    }
}

// time addition 5
void ShaderProgram::setTime(int t)
{
    useMe();

    if(unifTime != -1)
    {
        context->glUniform1i(unifTime, t);
    }
}

// Blinn-Phong addition
void ShaderProgram::setCameraPos(glm::vec4 cameraPos) {
    useMe();

    if(unifCameraPos != -1)
    {
        context->glUniform4fv(unifCameraPos, 1, &cameraPos[0]);
    }
}

//This function, as its name implies, uses the passed in GL widget
void ShaderProgram::draw(Drawable &d)
{
    useMe();

    // Texture addition 4
    if(unifSamplerTex != -1)
    {
        context->glUniform1i(unifSamplerTex, /*GL_TEXTURE*/0);
    }
    // Normal addition 4
    if(unifSamplerNor != -1)
    {
        context->glUniform1i(unifSamplerNor, /*GL_TEXTURE*/1);
    }

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

    // Remember, by calling bindPos(), we call
    // glBindBuffer on the Drawable's VBO for vertex position,
    // meaning that glVertexAttribPointer associates vs_Pos
    // (referred to by attrPos) with that VBO

    // Following should not run since generatePos etc. were not called
    if (attrPos != -1 && d.bindPos()) {
        context->glEnableVertexAttribArray(attrPos);
        context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 0, NULL);
    }

    if (attrNor != -1 && d.bindNor()) {
        context->glEnableVertexAttribArray(attrNor);
        context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 0, NULL);
    }

    if (attrCol != -1 && d.bindCol()) {
        context->glEnableVertexAttribArray(attrCol);
        context->glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 0, NULL);
    }
    // Texture & Normal addition
    if (attrUV != -1 && d.bindUV()) {
        context->glEnableVertexAttribArray(attrUV);
        context->glVertexAttribPointer(attrUV, 2, GL_FLOAT, false, 0, NULL);
    }
    // Blinn-Phong addition
    if (attrCos != -1 && d.bindCos()) {
        context->glEnableVertexAttribArray(attrCos);
        context->glVertexAttribPointer(attrCos, 1, GL_FLOAT, false, 0, NULL);
    }
    // Time addition
    if (attrAnimateable != -1 && d.bindAnimateable()) {
        context->glEnableVertexAttribArray(attrAnimateable);
        context->glVertexAttribPointer(attrAnimateable, 1, GL_FLOAT, false, 0, NULL);
    }

    // this checks to make sure that the handles set to variable names
    // in the shader program were set properly in the create() method
    // and makes sure that the interleave vbo was set in the input
    // drawable object

    //std::cout << attrCos << std::endl;
    //std::cout << attrAnimateable << std::endl;

    if (attrPos != -1 && attrNor != -1 && attrCol != -1 && attrUV != -1 && attrCos != -1 && attrAnimateable != -1
            && d.bindInterleaf()) {
        context->glEnableVertexAttribArray(attrPos);
        context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 16 * sizeof(float), (void*)(0 * sizeof(float)));
        // context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 0, (void*)(0 * sizeof(glm::vec4)));
        context->glEnableVertexAttribArray(attrNor);
        context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 16 * sizeof(float), (void*)(4 * sizeof(float)));
        // context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 0, (void*)(1 * sizeof(glm::vec4)));
        context->glEnableVertexAttribArray(attrCol);
        context->glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 16 * sizeof(float), (void*)(8 * sizeof(float)));
        // context->glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 0, (void*)(2 * sizeof(glm::vec4)));
        context->glEnableVertexAttribArray(attrUV);
        context->glVertexAttribPointer(attrUV, 2, GL_FLOAT, false, 16 * sizeof(float), (void*)(12 * sizeof(float)));
        context->glEnableVertexAttribArray(attrCos);
        context->glVertexAttribPointer(attrCos, 1, GL_FLOAT, false, 16 * sizeof(float), (void*)(14 * sizeof(float)));
        context->glEnableVertexAttribArray(attrAnimateable);
        context->glVertexAttribPointer(attrAnimateable, 1, GL_FLOAT, false, 16 * sizeof(float), (void*)(15 * sizeof(float)));
        // std::cout << "here" << std::endl;
    }


    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.
    d.bindIdx();
    context->glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0);


    // Transparency addition
    if (attrPos != -1 && attrNor != -1 && attrCol != -1 && attrUV != -1 && attrCos != -1 && attrAnimateable != -1
            && d.bindInterleafTrans()) {
        context->glEnableVertexAttribArray(attrPos);
        context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 16 * sizeof(float), (void*)(0 * sizeof(float)));
        // context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 0, (void*)(0 * sizeof(glm::vec4)));
        context->glEnableVertexAttribArray(attrNor);
        context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 16 * sizeof(float), (void*)(4 * sizeof(float)));
        // context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 0, (void*)(1 * sizeof(glm::vec4)));
        context->glEnableVertexAttribArray(attrCol);
        context->glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 16 * sizeof(float), (void*)(8 * sizeof(float)));
        // context->glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 0, (void*)(2 * sizeof(glm::vec4)));
        context->glEnableVertexAttribArray(attrUV);
        context->glVertexAttribPointer(attrUV, 2, GL_FLOAT, false, 16 * sizeof(float), (void*)(12 * sizeof(float)));
        context->glEnableVertexAttribArray(attrCos);
        context->glVertexAttribPointer(attrCos, 1, GL_FLOAT, false, 16 * sizeof(float), (void*)(14 * sizeof(float)));
        context->glEnableVertexAttribArray(attrAnimateable);
        context->glVertexAttribPointer(attrAnimateable, 1, GL_FLOAT, false, 16 * sizeof(float), (void*)(15 * sizeof(float)));
        // std::cout << "here" << std::endl;
    }

    d.bindIdxTrans();
    context->glDrawElements(d.drawMode(), d.elemCountTrans(), GL_UNSIGNED_INT, 0);

    if (attrPos != -1) context->glDisableVertexAttribArray(attrPos);
    if (attrNor != -1) context->glDisableVertexAttribArray(attrNor);
    if (attrCol != -1) context->glDisableVertexAttribArray(attrCol);
    // Texture & Normal addition
    if (attrUV != -1) context->glDisableVertexAttribArray(attrUV);
    // Blinn-Phong addition
    if (attrCos != -1) context->glDisableVertexAttribArray(attrCos);
    // Time addition
    if (attrAnimateable != -1) context->glDisableVertexAttribArray(attrAnimateable);

    context->printGLErrorLog();
}

char* ShaderProgram::textFileRead(const char* fileName) {
    char* text;

    if (fileName != NULL) {
        FILE *file = fopen(fileName, "rt");

        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            int count = ftell(file);
            rewind(file);

            if (count > 0) {
                text = (char*)malloc(sizeof(char) * (count + 1));
                count = fread(text, sizeof(char), count, file);
                text[count] = '\0';	//cap off the string with a terminal symbol, fixed by Cory
            }
            fclose(file);
        }
    }
    return text;
}

QString ShaderProgram::qTextFileRead(const char *fileName)
{
    QString text;
    QFile file(fileName);
    if(file.open(QFile::ReadOnly))
    {
        QTextStream in(&file);
        text = in.readAll();
        text.append('\0');
    }
    return text;
}

void ShaderProgram::printShaderInfoLog(int shader)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    context->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

    // should additionally check for OpenGL errors here

    if (infoLogLen > 0)
    {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        context->glGetShaderInfoLog(shader,infoLogLen, &charsWritten, infoLog);
        qDebug() << "ShaderInfoLog:" << endl << infoLog << endl;
        delete [] infoLog;
    }

    // should additionally check for OpenGL errors here
}

void ShaderProgram::printLinkInfoLog(int prog)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    context->glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLogLen);

    // should additionally check for OpenGL errors here

    if (infoLogLen > 0) {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        context->glGetProgramInfoLog(prog, infoLogLen, &charsWritten, infoLog);
        qDebug() << "LinkInfoLog:" << endl << infoLog << endl;
        delete [] infoLog;
    }
}

