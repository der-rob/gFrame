#version 120

uniform sampler2DRect tex0;

void main() {
    vec4 texel0 = texture2DRect(tex0, gl_TexCoord[0].st);
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}