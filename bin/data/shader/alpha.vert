//#version 120
//
//uniform mat4 modelViewProjectionMatrix;
//in vec4 position;
//in vec2 texcoord;
//
//out vec2 texCoordVarying;
//
//void main() {
//    texCoordVarying = vec2(texcoord.x, texcoord.y);
//    gl_Position = modelViewProjectionMatrix * position;
//}
#version 120

void main() {
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = ftransform();
}