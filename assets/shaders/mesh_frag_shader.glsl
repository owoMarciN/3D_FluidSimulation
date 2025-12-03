#version 330 core
out vec4 FragColor;
uniform vec3 meshColor; // Kolor, który ustawiasz w C++ na (1.0f, 0.0f, 0.0f)

void main() {
    // KLUCZOWE: Upewnij się, że ustawiasz kolor wyjściowy.
    FragColor = vec4(meshColor, 1.0); 
}