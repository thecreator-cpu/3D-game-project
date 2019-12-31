#version 150
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

uniform vec4 u_Color;       // When drawing the cube instance, we'll set our uniform color to represent different block types.

// Blinn-Phong addition
uniform vec4 u_CameraPos;   // The vector that defines the camera's world-space position



in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec4 vs_Col;             // The array of vertex colors passed to the shader.

// Texture & Normal addition 13
in vec2 vs_UV;              // The array of vertex texture coordinates passed to the shader

// Blinn-Phong addition
in float vs_Cos;            // The cosine power for the current texture

// Time addition
in float vs_Animateable;    // The animateable flag, 1 for animateable, 0 for not animateable

out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.
// Texture & Normal addition 14
out vec2 fs_UV;             // The UV of each vertex. This is implicitly passed to the fragment shader.
// Blinn-Phong addition
out float fs_Cos;           // The cosine power of each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_ViewVec;        // The view vector of each vertex. This is implicitly passed to the fragment shader.
// Time addition
out float fs_Animateable;   // The animateable flag, 1 for animateable, 0 for not animateable. This is implicitly passed to the fragment shader.

const vec4 lightDir = vec4(1,1,1,0);  // The direction of our virtual light, which is used to compute the shading of
                                        // the geometry in the fragment shader.

//ANNE
out vec4 fs_Pos;

float rand(vec2 n) {
    return (fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453));
}

float interpNoise2D(float x, float y) {
    float intX = floor(x);
    float fractX = fract(x);
    float intY = floor(y);
    float fractY = fract(y);

    float v1 = rand(vec2(intX, intY));
    float v2 = rand(vec2(intX + 1, intY));
    float v3 = rand(vec2(intX, intY + 1));
    float v4 = rand(vec2(intX + 1, intY + 1));

    float i1 = mix(v1, v2, fractX);
    float i2 = mix(v3, v4, fractX);
    return mix(i1, i2, fractY);
}


float fbm(float x, float y) {
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










void main()
{
    // we're actually going to use the in vec4 vs_Col because we are calling
    // draw methods per chunk as opposed to per cube and a chunk will likely
    // have different colored cubes
    // plus i did all of that work to get the color in this shader program
    // fs_Col = u_Color;                         // Pass the vertex colors to the fragment shader for interpolation


    fs_Col = vs_Col;

    // Texture & Normal addition 15
    fs_UV = vs_UV;    // Pass the vertex UVs to the fragment shader for interpolation

    // Time addition
    fs_Animateable = vs_Animateable;

    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);          // Pass the vertex normals to the fragment shader for interpolation.
                                                            // Transform the geometry's normals by the inverse transpose of the
                                                            // model matrix. This is necessary to ensure the normals remain
                                                            // perpendicular to the surface after the surface is transformed by
                                                            // the model matrix.


    vec4 modelposition = u_Model * vs_Pos;   // Temporarily store the transformed vertex positions for use below


    // Blinn-Phong addition
    fs_Cos = vs_Cos;


    fs_ViewVec = u_CameraPos - modelposition; // world coord - world coord

    fs_LightVec = (lightDir);  // Compute the direction in which the light source lies

    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices

    //Anne sky
    fs_Pos = modelposition;





    //Anne Grass

    //if(vs_UV.x > 8.f/16.f && vs_UV.x < 9.f/16.f && vs_UV.y > 13.f/16.f && vs_UV.y < 14.f/16.f){
    /*
    if(true){
        int gridsize = 3;
        vec2 smallGrid = floor(vec2(modelposition[0],modelposition[2]) / gridsize) * gridsize;
        //green
        vec4 grass1 = vec4(0.73,0.91,0.63, 1) + vs_Col;
        //orange
        vec4 grass2 = vec4(0.97,0.76,0.29, 1) + vs_Col;

        //make lower grass redish
        if(fs_Pos[1] < 150){
            if(fbm(smallGrid[0], smallGrid[1]) < 0.3){
                grass2 = vec4(0.95,0.65,0.33, 1) + vs_Col;
            }
        }

        //bluish
        if(fbm(smallGrid[0], smallGrid[1]) < 0.25){
            grass2 = vec4(0.69,0.93,0.62, 1) + vs_Col;
        }


        float noise = fbm(smallGrid[0], smallGrid[1]);
        vec4 n = vec4(noise, noise, noise, noise);
        vec4 color = mix(grass2, grass1, n);

        //moved this line to the bottom
        fs_Col = color;
   }*/
}
