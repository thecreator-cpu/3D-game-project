#version 150
// swimming.frag.glsl:
// A fragment shader used for post-processing that simply reads the
// image produced in the first render pass by the surface shader
// and outputs it to the frame buffer


in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;
uniform int u_Time;
uniform ivec2 u_Dimensions;

void main()
{
    vec4 sampledCol = texture(u_RenderedTexture, fs_UV);
    color = vec3(0, 0, sampledCol.b);
}
