#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D ourTexture;

uniform vec3 textColor; 

void main() {
    vec4 texColor = texture(ourTexture, TexCoord); 
    FragColor = texColor;

}

