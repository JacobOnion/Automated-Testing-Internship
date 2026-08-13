#version 330 core

in vec3 v2fColor;
uniform vec3 uBaseColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(uBaseColor * v2fColor, 1.0);
}