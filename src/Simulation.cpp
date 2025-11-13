// Implementacja testowa symulacji z wolnym ruchem kamery
//-------------------------------------------------------
// Kompilacja WIN32:        
//            g++ src/*.cpp src/*c -Iinclude -I/ucrt64/include/ -o turbine -lSDL3 -lopengl32
//            turbine.exe
//-------------------------------------------------------
// Kompilacja LINUX: 
//            g++ src/*.cpp src/*.c -Iinclude -o turbine -lSDL3 -lGL
//            ./turbine
//-------------------------------------------------------
// JAK COŚ SIĘ SPIERDOLI!!: 
//            g++ -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer src/*.cpp src/*.c -Iinclude -o turbine $(pkg-config --cflags --libs sdl3)  
//            ./turbine                   
//-------------------------------------------------------

#include "Simulation.hpp"

Simulation::Simulation(): mTimer(Timer::Instance()) {
    window_name = "Kaplan Propeller";
    running     = true;
    model_path = "assets/models/Propeller.obj";
    shader_path = "assets/shaders/";
    // Initializujemy symulacje
    if (!Init()) running = false;

    // Pozycja początkowa kamery
    camPos   = glm::vec3(0.0f, 0.0f, 5.0f);
    camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    camUp    = glm::vec3(0.0f, 1.0f, 0.0f);

    // Mysz
    cameraSpeed = 0.05f;

    // Rotacje
    yaw   = -90.0f;
    pitch = 0.0f;

    // Kąt obrotu naszej śruby
    angle = 0.0f;
}

Simulation::~Simulation() {
    std::cout << "Simulation -- Destroyed" << std::endl;
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

std::string Simulation::readShaderFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
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

// Tworzenie shaderprogramu z shadera
GLuint Simulation::createShaderProgram() {
    std::string vertexSrcStr = readShaderFile(shader_path + "vertex_shader.glsl");
    std::string fragmentSrcStr = readShaderFile(shader_path + "fragment_shader.glsl");

    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSrcStr.c_str());
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSrcStr.c_str());
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

// Zładowanie prostego pliku .obj
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

// Initializacja SDL3, Glad (opengl), shaderów i załadowanie pliku Propeller.obj
bool Simulation::Init() {
    // Initializacja SDL3 by zarządzać oknem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Tworzenie obiektu okna oraz kontekstu OpenGL, by potem móc za jego pomocą renderować 
    mWindow = SDL_CreateWindow(window_name.c_str(), SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    glContext = SDL_GL_CreateContext(mWindow);

    // Sprawdzanie czy jest załadowana biblioteka Glad
    if (!gladLoadGL()) { std::cerr << "Failed to init GLAD\n"; return -1; }

    glEnable(GL_DEPTH_TEST);

    // Załadowanie modelu
    if (!loadObj(model_path)) return -1;

    // Funkcja niepotrzebna poniewarz został zaimplementowna klasa Timer
    //SDL_GL_SetSwapInterval(1); // V-sync

    // Mysz w trybie 'capture'
    SDL_SetWindowRelativeMouseMode(mWindow, true);

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
    // Aktualizacja wejścia z myszy i klawiatury
    SDL_GetRelativeMouseState(&xrel,&yrel);
    mouseMovement(xrel,yrel);
    processInput();
}

void Simulation::Update() {
    // Wyświetlenie FPS
    // Jak na razie symulacja nie jest pod wielkim obciążeniem, więc wszystko jest dobrze
    std::string FPS = std::to_string(1.0f / mTimer.DeltaTime());
    std::string ms = std::to_string(mTimer.DeltaTime() * 1000.0f);
    SDL_SetWindowTitle(mWindow, (window_name + " - " + FPS + "FPS / " + ms + "ms.").c_str());
}

void Simulation::LateUpdate() {
    // Obrót śmigłem :)
    angle += mTimer.DeltaTime() * glm::radians(80.0f); // Obrót 100°/s
    model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1));

    // Reset Timer'a, żeby liczył następne deltaTime
    mTimer.Reset();
}

void Simulation::Render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    // Jak sama nazwa sugeruje ta transformacja pozwala nam zauktualizować wektor, w którym kierunku "patrzymy"
    glm::mat4 view = glm::lookAt(camPos, camPos+camFront, camUp);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

    SDL_GL_SwapWindow(mWindow);
}

// Główna pętla
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