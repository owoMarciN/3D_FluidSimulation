// Kompilacja:  g++ src/*.cpp src/*c -Iinclude -I/ucrt64/include/ -o turbine -lSDL3 -lopengl32
//              turbine.exe

// Implementacja testowa symulacji z wolnym ruchem kamery

#include "Simulation.hpp"

// "Proste" shaders
const char* vertexShaderSrc = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 Normal;
out vec3 FragPos;
void main() {
    FragPos = vec3(model * vec4(aPos,1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos,1.0);
}
)";

const char* fragmentShaderSrc = R"(#version 330 core
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;  // camera position

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular (Phong)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;

    // Ambient
    vec3 ambient = 0.1 * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

// R"(
// #version 330 core
// in vec3 Normal;
// in vec3 FragPos;
// out vec4 FragColor;
// uniform vec3 objectColor;
// uniform vec3 lightPos;
// uniform vec3 lightColor;
// void main() {
//     vec3 norm = normalize(Normal);
//     vec3 lightDir = normalize(lightPos - FragPos);
//     float diff = max(dot(norm, lightDir),0.0);
//     vec3 diffuse = diff * lightColor;
//     vec3 result = (0.2 + diffuse) * objectColor;
//     FragColor = vec4(result,1.0);
// }
// )";

Simulation::Simulation(): mTimer(Timer::Instance()) {
    // Initializujemy symulacje
    window_name = "Kaplan Propeller";
    running = true;
    if (!Init()) running = false;

    // Kamera
    camPos   = glm::vec3(0.0f, 0.0f, 5.0f);
    camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    camUp    = glm::vec3(0.0f, 1.0f, 0.0f);

    yaw   = -90.0f;
    pitch = 0.0f;
    lastX = SCREEN_WIDTH / 2;
    lastY = SCREEN_HEIGHT / 2;

    // Mysz
    firstMouse  = true;
    cameraSpeed = 0.05f;

    // Tablica przechowująca naciśnięte przyciski
    keys[1024] = { false };

    // Kąt obrotu naszej śruby
    angle = 0.0f;
}

Simulation::~Simulation() {
    if (glContext) {
        SDL_GL_DestroyContext(glContext);
        glContext = nullptr;
    }

    if (mWindow) {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }
    SDL_Quit();
}

void Simulation::processInput() {
    // Przód - Tył
    if (keys[SDL_SCANCODE_W]) camPos += cameraSpeed * camFront;
    if (keys[SDL_SCANCODE_S]) camPos -= cameraSpeed * camFront;

    // Lewo - Prawo
    if (keys[SDL_SCANCODE_A]) camPos -= glm::normalize(glm::cross(camFront, camUp)) * cameraSpeed;
    if (keys[SDL_SCANCODE_D]) camPos += glm::normalize(glm::cross(camFront, camUp)) * cameraSpeed;
}

void Simulation::mouseMovement(int xoffset, int yoffset) {
    float sensitivity = 0.08f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // yaw - rotacja pozioma (y-axis), pitch - rotacja pionowa (x-axis)
    yaw   += xoffset;
    pitch += -yoffset; // odwrócone Y myszy

    // Blokada kamery
    pitch = std::clamp(pitch, -89.0f, 89.0f);

    // Koordynaty sferyczne na kartezjańskie
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camFront = glm::normalize(front);
}

// Kompilacja shadera
GLuint Simulation::compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cerr << "Shader compile error: " << info << std::endl;
    }
    return shader;
}

// Tworzenie programu z shadera
GLuint Simulation::createShaderProgram() {
    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint program = glCreateProgram();

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) std::cerr << "Program link failed\n";

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

// Load OBJ
bool Simulation::loadObj(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    if (!ret) {
        std::cerr << "OBJ load error: " << warn << err << std::endl;
        return false;
    }

    for (const auto& shape : shapes) {
        for (const auto& idx : shape.mesh.indices) {
            // Każdy - idx - posiada:
            //      vertex_index -> index w attrib.vertices
            //      normal_index -> index w attrib.normals
            //      texcoord_index -> index w attrib.texcoords

            // Prosty 1D array [x0, y0, z0, x1, y1, z1, ...]
            vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
            vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
            vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

            normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
            normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
            normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
        }
    }
    return true;
}

bool Simulation::Init() {
    // Initializacja SDL3, opengl, shaders i załadowanie pliku Propeller.obj
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    mWindow = SDL_CreateWindow(window_name.c_str(), SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    glContext = SDL_GL_CreateContext(mWindow);

    if (!gladLoadGL()) { std::cerr << "Failed to init GLAD\n"; return -1; }

    glEnable(GL_DEPTH_TEST);

    if (!loadObj("assets/Propeller.obj")) return -1;

    // Funkcja niepotrzebna poniewarz został zaimplementowna clasa Timer
    //SDL_GL_SetSwapInterval(1); // V-sync
    SDL_SetWindowRelativeMouseMode(mWindow, true); // Mysz w trybie 'capture'

    // Initializacja buferów/tablic na wierzchołki, wektory jednostkowe
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &NBO);
    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    // Shadery
    shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);

    modelLoc      = glGetUniformLocation(shaderProgram, "model"); 
    viewLoc       = glGetUniformLocation(shaderProgram, "view"); 
    projLoc       = glGetUniformLocation(shaderProgram, "projection"); 
    objColorLoc   = glGetUniformLocation(shaderProgram, "objectColor"); 
    lightPosLoc   = glGetUniformLocation(shaderProgram, "lightPos"); 
    lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor"); 

    // Zmiana odpowiednio koloru śmigła, pozycji światła i kolor światła
    glUniform3f(objColorLoc, 0.5f, 0.5f, 0.5f); 
    glUniform3f(lightPosLoc, 0.0f, 0.0f, 3.0f); 
    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); 

    // Jeśli chcemy zmieniać rozmiar okna, przenieść tą funckję do Update
    projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
    return 1;
}


void Simulation::EarlyUpdate() {
	// Updating the input state before any other updates are run to make sure the Input check is accurate
    SDL_GetRelativeMouseState(&xrel,&yrel);
    mouseMovement(xrel,yrel);
    processInput();
}

void Simulation::Update() {
	std::string FPS = std::to_string(1.0f / mTimer.DeltaTime());
    std::string ms = std::to_string(mTimer.DeltaTime() * 1000.0f);
    SDL_SetWindowTitle(mWindow, (window_name + " - " + FPS + "FPS / " + ms + "ms.").c_str());
}

void Simulation::LateUpdate() {
	angle += mTimer.DeltaTime() * glm::radians(80.0f); // Obrót 100°/s
    model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1));
	mTimer.Reset();
}

void Simulation::Render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);


    glm::mat4 view = glm::lookAt(camPos, camPos+camFront, camUp);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

    SDL_GL_SwapWindow(mWindow);
}

void Simulation::Run() {
    while (running) {
        mTimer.Update();
        while (SDL_PollEvent(&mEvents)) {
            if (mEvents.type == SDL_EVENT_QUIT) 
                running = false;

            if (mEvents.type == SDL_EVENT_KEY_DOWN) {
                keys[mEvents.key.scancode] = true;

                // Sprawdzenie klawisza ESC
                if (mEvents.key.scancode == SDL_SCANCODE_ESCAPE)
                    running = false;
            }

            if (mEvents.type == SDL_EVENT_KEY_UP) 
                keys[mEvents.key.scancode] = false;
        }

        if (mTimer.DeltaTime() >= (1.0f / FRAME_RATE)) {
            EarlyUpdate();
            Update();
            LateUpdate();
            Render();
        }
    }
}