// Vertex Shader (sonar.vert)
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in float aFade;    

out float fadeFactor; 

uniform mat4 uModel;      
uniform mat4 uProjection; 

void main() {
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0); 
    fadeFactor = aFade; 
}
