#version 120

uniform sampler2DRect tex0;
uniform float alpha;


void main() {
    vec4 texel0 = texture2DRect(tex0, gl_TexCoord[0].st);
    
    // first three values are the rgb, fourth is the alpha
    gl_FragColor = vec4(texel0.rgb, texel0.a * alpha);
}