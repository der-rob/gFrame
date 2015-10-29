#version 120

uniform sampler2DRect tex0;
uniform sampler2DRect imageMask;
uniform bool invert;

void main() {
    vec4 texel0 = texture2DRect(tex0, gl_TexCoord[0].st);
    vec4 texel1 = texture2DRect(imageMask, gl_TexCoord[0].st);
    
    // first three values are the rgb, fourth is the alpha
    if(invert) {
        gl_FragColor = vec4(texel0.rgb, texel0.a * (1-texel1.a));
    }
    else
    {
        gl_FragColor = vec4(texel0.rgb, texel0.a * (texel1.a));
    }
}