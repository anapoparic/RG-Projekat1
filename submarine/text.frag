#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform vec3 textColor;

void main() {
    float alpha = texture(ourTexture, TexCoord).r;  
    if (alpha < 0.1) {
        discard;  
    }
    FragColor = vec4(textColor, alpha);  
}
