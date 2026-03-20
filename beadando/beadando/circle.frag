#version 330 core
out vec4 FragColor;

uniform vec2 uCircleCenter;
uniform float uRadius;
uniform vec3 uInnerColor;
uniform vec3 uOuterColor;
uniform vec3 uBgColor;

void main(){
    vec2 fragCoord = gl_FragCoord.xy;
    float dist = length(fragCoord - uCircleCenter);

    if(dist <= uRadius){
        float t = dist / uRadius;
        vec3 color = mix(uInnerColor,uOuterColor,t);
        FragColor = vec4(color,1.0);
    } else {
        FragColor = vec4(uBgColor,1.0);
    }
}