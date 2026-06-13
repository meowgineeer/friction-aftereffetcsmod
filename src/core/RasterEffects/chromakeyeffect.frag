#version 330 core
layout(location = 0) out vec4 fragColor;

in vec2 texCoord;

uniform sampler2D tex;

uniform vec3 keyColor;
uniform float tolerance;
uniform float softness;

vec3 rgb2yuv(vec3 rgb) {
    float y = 0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b;
    float u = 0.492 * (rgb.b - y);
    float v = 0.877 * (rgb.r - y);
    return vec3(y, u, v);
}

void main(void) {
    vec4 color = texture(tex, texCoord);
    if(color.a < 0.00001) {
        fragColor = color;
        return;
    }
    vec3 yuvColor = rgb2yuv(color.rgb / color.a);
    vec3 yuvKey = rgb2yuv(keyColor);
    
    float dist = distance(yuvColor.yz, yuvKey.yz);
    float blend = smoothstep(tolerance, tolerance + softness, dist);
    
    fragColor = vec4(color.rgb * blend, color.a * blend);
}
