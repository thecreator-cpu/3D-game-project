#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

// Texture addition 1
uniform sampler2D u_Texture; // The texture to be read from by this shader
// Normal addition 1
uniform sampler2D u_Normal; // The normal to be read from by this shader
// Time addition 1
uniform int u_Time; // The time to be read from by this shader
uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform vec4 u_CameraPos;

// light direction addition
const float timeScale = 0.0001;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;
in vec4 fs_Pos;
// Texture & Normal addition 15
in vec2 fs_UV;
// Blinn-Phong addition
in float fs_Cos;
in vec4 fs_ViewVec;
// Time addition
in float fs_Animateable;


out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.








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
    // Material base color (before shading)

    // Time addition
    vec2 computedUV = fs_UV;
    if (fs_Animateable == 1) {
        computedUV[1] += mod(float(u_Time) / 10000.f, (1.f / 16.f)); // offsets texture by 0 to 1/16
    }

    // Texture addition
    vec4 diffuseColor;
    diffuseColor = texture(u_Texture, computedUV);
    // apply coloring through multiply
    if (fs_Col != vec4(0, 0, 0, 1)) {
        diffuseColor *= fs_Col;
    }
    if (diffuseColor[3] <= 0) {
        diffuseColor = fs_Col;
    }




    //vec4 mappedNor = fs_Nor;

    // Normal addition
    vec4 mappedNor = texture(u_Normal, computedUV);
    if (mappedNor[3] == 1) {
        // convert color (0, 1) to normal (-1, 1)
        mappedNor = vec4((mappedNor[0] - 0.5) * 2.0, (mappedNor[1] - 0.5) * 2.0,
                (mappedNor[2] - 0.5) * 2.0, 0);

        // rotate UV for each side
        // because of how Terrain is set up (left hand rule, z going back, opposite of grid),
        // must use left hand rule to find rotations, then negate z
        if (fs_Nor == vec4(0, 0, -1, 0)) { // front

        } else if (fs_Nor == vec4(0, 0, 1, 0)) { // back
            mappedNor = vec4(-mappedNor[0], mappedNor[1], -mappedNor[2], 0);
        } else if (fs_Nor == vec4(1, 0, 0, 0)) { // right
            mappedNor = vec4(-mappedNor[2], mappedNor[1], -mappedNor[0], 0);
        } else if (fs_Nor == vec4(-1, 0, 0, 0)) { // left
            mappedNor = vec4(mappedNor[2], mappedNor[1], mappedNor[0], 0);
        } else if (fs_Nor == vec4(0, 1, 0, 0)) { // top
            mappedNor = vec4(mappedNor[0], -mappedNor[2], -mappedNor[1], 0);
        } else if (fs_Nor == vec4(0, -1, 0, 0)) { // bottom
            mappedNor = vec4(mappedNor[0], mappedNor[2], mappedNor[1], 0);
        }
    } else {
        mappedNor = fs_Nor;
    }

    // Blinn-Phong addition
    // the ambientIntensity changes with the time of day
    float ambientIntensity = 0;
    float cosScale = 0.f;
    vec3 colorShift;
    // same sun direction calculations as in sky.frag.glsl
    float sunY = sin(u_Time * timeScale);
    vec4 lightDir;
    if (sunY < 0) {
        ambientIntensity = 0.05;
        cosScale = 2000;
        // same moon direction calculations as in sky.frag.glsl
        float moonY = -sin(u_Time * timeScale);
        float moonZ = -cos(u_Time * timeScale);
        colorShift = vec3(0, moonY * 0.1f, moonY * 0.2f);
        lightDir = vec4(0.f, moonY, moonZ, 1.f);
    } else {
        ambientIntensity = 0.8;
        cosScale = 2000;
        float sunZ = cos(u_Time * timeScale);
        colorShift = vec3((1 - ((1 + sunY) * 0.5)) * 0.5, 0, 0);
        lightDir = vec4(0.f, sunY, sunZ, 1.f);
    }

    vec4 H = normalize((normalize(fs_ViewVec) + normalize(lightDir)) / 2.f);
    float specularIntensity = max(pow(dot(H, fs_Nor), fs_Cos * cosScale), 0);


    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(mappedNor), normalize(lightDir));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    // float ambientTerm = clamp(0.2 * sin(u_Time * timeScale), 0, 1);
    float ambientTerm = abs(ambientIntensity * sin(u_Time * timeScale));


    float lightIntensity = diffuseTerm + ambientTerm + specularIntensity;   //Add a small float value to the color multiplier
                                                        //to simulate ambient lighting. This ensures that faces that are not
                                                        //lit by our point light are not completely black.


    // decreases the light intensity when the light source is near
    // the horizon
    lightIntensity *= lightDir.y;
    lightIntensity = max(lightIntensity, 0.2);

    // Compute final shaded color
    out_Col = vec4((diffuseColor.rgb + colorShift) * lightIntensity, diffuseColor.a);


    //if(true){
    if((fs_UV.x > 8.f/16.f && fs_UV.x < 9.f/16.f && fs_UV.y > 13.f/16.f && fs_UV.y < 14.f/16.f) ||
       (fs_UV.x > 3.f/16.f && fs_UV.x < 4.f/16.f && fs_UV.y > 15.f/16.f && fs_UV.y < 16.f/16.f)){
        int gridsize = 3;
        vec2 smallGrid = floor(vec2(fs_Pos[0],fs_Pos[2]) / gridsize) * gridsize;
        //green
        vec4 grass1 = vec4(0.73,0.91,0.63, 1);
        //orange
        vec4 grass2 = vec4(0.97,0.76,0.29, 1);

        //make lower grass redish
        if(fs_Pos[1] < 150){
            if(fbm(smallGrid[0], smallGrid[1]) < 0.3){
                grass2 = vec4(0.95,0.65,0.33, 1);
            }
        }

        //bluish
        if(fbm(smallGrid[0], smallGrid[1]) < 0.25){
            grass2 = vec4(0.69,0.93,0.62, 1);
        }


        float noise = fbm(smallGrid[0], smallGrid[1]);
        vec4 n = vec4(noise, noise, noise, noise);
        vec4 color = mix(grass2, grass1, n);

        //moved this line to the bottom
        //fs_Col = color;
        out_Col += color;
        //diffuseColor *= 0.6;
        //diffuseColor += color;
        out_Col *= color * 0.8;
   }









    //Anne Fog

    float fog_dist = 60;

    float distance = abs(distance(fs_Pos, u_CameraPos));
    vec4 skyColor = vec4(0.62,0.9,0.96, 1);
    if (distance > fog_dist){
        skyColor[3] = 0;
        skyColor = vec4(1,1,1, 0);
    }

    float dist = min(1.f, distance/fog_dist);

    dist = pow(smoothstep(0,1,pow(dist,4)),1);

    vec4 color = mix(out_Col, skyColor,  dist);


    out_Col = color;


}
