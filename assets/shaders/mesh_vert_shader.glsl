#version 330 core
layout (location = 0) in vec3 aPos; // Wierzchołki z location 0
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // KLUCZOWE: Wierzchołki MUSZĄ być transformowane przez wszystkie 3 macierze
    gl_Position = projection * view * model * vec4(aPos, 1.0); 
}