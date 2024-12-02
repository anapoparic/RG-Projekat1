#version 330 core

in float fadeFactor; 
out vec4 FragColor;

uniform vec3 uColor;     
uniform vec3 uBaseColor; 

void main() {
   
    vec3 finalColor = mix(uColor, uBaseColor, fadeFactor);

    FragColor = vec4(finalColor, 1.0);
}

