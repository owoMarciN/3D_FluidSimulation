#include "Simulation.h"

Simulation::Simulation(): mTimer(Timer::Instance()) {
    // Nazwa okna
    window_name = "Fluid Simulation";

    // Ścieżka do naszego modelu 3D
    model_path 	= "assets/models/Propeller.obj";

    // Ścieżka do shaderów
    shader_path = "assets/shaders/";

    // Zmienna upewniająca się, że symulacja pracuje
    running     = true;
    
    // Initializujemy symulacje
    if (!Init()) running = false;

    // Pozycja początkowa kamery
    camPos   = glm::vec3(0.0f, 0.0f, 5.0f);
    camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    camUp    = glm::vec3(0.0f, 1.0f, 0.0f);

    // Mysz
    cameraSpeed  = 0.05f;
    mouseCapture = true;
    wasCaptured = false;

    // Rotacje modelu
    yaw   = -90.0f;
    pitch = 0.0f;

    // Kąt obrotu naszej śruby
    angle = 0.0f;
}

Simulation::~Simulation() {
    std::cout << "Simulation -- Destroyed" << std::endl;
    
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (NBO) glDeleteBuffers(1, &NBO);

    if (lineVAO) glDeleteVertexArrays(1, &lineVAO);
    if (lineVBO) glDeleteBuffers(1, &lineVBO);

    if (cubeVAO) glDeleteVertexArrays(1, &cubeVAO);
    if (cubeVBO) glDeleteBuffers(1, &cubeVBO);

    if (shaderProgProp) glDeleteProgram(shaderProgProp);
    if (shaderProgLine) glDeleteProgram(shaderProgLine);
    if (shaderProgCube) glDeleteProgram(shaderProgCube);

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
    // Czułość ruchów myszy
    float sensitivity = 0.08f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // yaw - rotacja pozioma (y-axis), pitch - rotacja pionowa (x-axis)
    yaw   += xoffset;
    pitch += -yoffset; // odwrócone Y myszy

    // Blokada kamery (góra/dół)
    pitch = std::clamp(pitch, -89.0f, 89.0f);

    // Koordynaty sferyczne na kartezjańskie, by ustawić pozycje kamery
    glm::vec3 front;
    front.x  = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y  = sin(glm::radians(pitch));
    front.z  = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camFront = glm::normalize(front);
}

// Odczytywanie shaderów z plików .glsl do obiektu string
std::string Simulation::readShaderFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Nie można otworzyć shadera: " << path << std::endl;
        return "";
    }
    // Utwórz stringstream do przetrzymywania zawartości pliku
    std::stringstream ss;

    // Zapisanie zawartości pliku do stringstream
    ss << file.rdbuf();

    // Konwersja stringstream na normalny string
    return ss.str();
}

// Kompilacja shadera
GLuint Simulation::compileShader(GLenum type, const char* src) {
    // Stwórz nowy obiekt shadera danego typu (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.)
    GLuint shader = glCreateShader(type);

    // Przypisuje kod źródłowy do obiektu shadera
    // Parametry: obiekt shadera, liczba ciągów, wskaźnik do ciągu, wskaźnik do długości ciągu (nullptr = zakończony znakiem null)
    glShaderSource(shader, 1, &src, nullptr);

    // Kompilacja shadera
    glCompileShader(shader);

    GLint success;
    // Pobiera status kompilacji shadera
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    // Jeśli kompilacja się nie powiodła, wyświetla log błędów
    if (!success) {
        char info[512]; // Bufor do przechowywania komunikatu o błędzie
        glGetShaderInfoLog(shader, 512, nullptr, info); // Pobiera log błędów
        std::cerr << "[ERROR] Kompilacja shadera nie powiodła się: " << info << std::endl;
    }

    return shader;
}

// Funkcja tworząca program shaderów z vertex i fragment shadera
GLuint Simulation::createShaderProgram(const std::string& vertName, const std::string& fragName) {
    // Wczytuje kod źródłowy vertex shadera z pliku
    std::string vertexSrcStr = readShaderFile(shader_path + vertName);
    // Wczytuje kod źródłowy fragment shadera z pliku
    std::string fragmentSrcStr = readShaderFile(shader_path + fragName);

    if (vertexSrcStr.empty() || fragmentSrcStr.empty()) {
        std::cerr << "[ERROR] Nie można załadować pliku shadera: " << vertName << " lub " << fragName << std::endl;
        return 0;
    }

    // Kompiluje vertex shader
    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSrcStr.c_str());
    // Kompiluje fragment shader
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSrcStr.c_str());

    // Tworzy nowy program shaderów
    GLuint program = glCreateProgram();

    // Przypina vertex i fragment shadery do programu
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);

    // Linkuje shadery w jeden program
    glLinkProgram(program);

    // Sprawdza, czy linkowanie się powiodło
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) std::cerr << "[ERROR] Linkowanie programu nie powiodło się." << std::endl;

    // Po połączeniu programu shadery można usunąć z pamięci GPU
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

// Załadowanie prostego pliku .obj
bool Simulation::loadObj(const std::string& path) {
    // Struktura do przechowywania współrzędnych wierzchołków, wektorów normalnych i texcoords
    tinyobj::attrib_t attrib;

    // Wektory przechowujące kształty (mesh) i materiały z pliku OBJ
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    // Ostrzeżenia
    std::string warn, err;

    // Wczytywanie pliku
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    if (!ret) {
        std::cerr << "[ERROR] Nie można załadować pliku OBJ: " << warn << err << std::endl;
        return false;
    }

    for (const auto& shape : shapes) {

        // Iteracja po wszystkich indeksach w mesh (trójkątach)
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
    // Initializacja SDL3
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "[ERROR] Inicjalizacja SDL nie powiodła się: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Wymagana wersja OpenGL przynajmniej 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Tworzenie okna SDL z kontekstem OpenGL
    mWindow = SDL_CreateWindow(window_name.c_str(), SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

    if (!mWindow) {
        std::cerr << "[ERROR] Tworzenie okna nie powiodło się: " << SDL_GetError() << std::endl;
        return false;
    }

    glContext = SDL_GL_CreateContext(mWindow);

    if (!glContext) {
        std::cerr << "[ERROR] Tworzenia kontekstu OpenGL nie powiodło się: " << SDL_GetError() << std::endl;
        return false;
    }

    // Inicjalizacja biblioteki GLAD do obsługi funkcji OpenGL
    if (!gladLoadGL()) { 
        std::cerr << "[ERROR] Inicjalizacja GLAD nie powiodła się: " << std::endl; 
        return -1; 
    }

    std::cout << "Wersja OpenGL: " << glGetString(GL_VERSION) << '\n' << std::endl;

    glEnable(GL_DEPTH_TEST);

    // Wczytanie modelu z pliku OBJ
    if (!loadObj(model_path)) return false;

    // Funkcja niepotrzebna ponieważ została zaimplementowna klasa Timer
    // SDL_GL_SetSwapInterval(1); // V-sync

    if (!SetPropeller()) {
        std::cerr << "[ERROR] Nie utworzono programu dla shadera śruby." << std::endl;
        return false;
    }
    if (!SetAxis()) {
        std::cerr << "[ERROR] Nie utworzono programu dla shadera linii osi." << std::endl;
        return false;
    }
    if (!SetCube()) {
        std::cerr << "[ERROR] Nie utworzono programu dla shadera kostki." << std::endl;
        return false;
    }

    return true;
}

bool Simulation::SetPropeller() {
    // Tworzenie i wiązanie Vertex Array Object (VAO)
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Tworzenie Vertex Buffer Object (VBO) dla wierzchołków
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Tworzenie Vertex Buffer Object (NBO) dla wektorów normalnych
    glGenBuffers(1, &NBO);
    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    // Shadery
    shaderProgProp = createShaderProgram("vertex_shader.glsl", "fragment_shader.glsl");

    if (!shaderProgProp) return false;

    glUseProgram(shaderProgProp);

    // Pobranie informacji z shaderów
    modelLocProp        = glGetUniformLocation(shaderProgProp, "model"); 
    viewLocProp         = glGetUniformLocation(shaderProgProp, "view"); 
    projLocProp         = glGetUniformLocation(shaderProgProp, "projection"); 
    objColorLocProp     = glGetUniformLocation(shaderProgProp, "objectColor"); 
    lightPosLoc     = glGetUniformLocation(shaderProgProp, "lightPos"); 
    lightColorLoc   = glGetUniformLocation(shaderProgProp, "lightColor"); 
    lightPos2Loc    = glGetUniformLocation(shaderProgProp, "lightPos2"); 
    lightColor2Loc  = glGetUniformLocation(shaderProgProp, "lightColor2"); 

    // Ustawienie koloru obiektu (szare)
    glUniform3f(objColorLocProp, 0.5f, 0.5f, 0.5f); 

    // Ustawienie pozycji i koloru pierwszego światła
    glUniform3f(lightPosLoc, 0.0f, 0.0f, 3.0f); 
    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); 

    // Ustawienie pozycji i koloru drugiego światła
    glUniform3f(lightPos2Loc, 0.0f, 0.0f, -3.0f); 
    glUniform3f(lightColor2Loc, 1.0f, 1.0f, 1.0f); 

    // Tworzenie macierzy projekcji perspektywy
    // Jeśli chcemy zmieniać rozmiar okna, przenieść tą funckję do Update
    projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(projLocProp, 1, GL_FALSE, &projection[0][0]);

    return true;
}

bool Simulation::SetCube() {
    GLfloat cubeVertices[] = {
        -0.5f,-0.5f,-0.5f,   0.5f,-0.5f,-0.5f,   0.5f, 0.5f,-0.5f,
         0.5f, 0.5f,-0.5f,  -0.5f, 0.5f,-0.5f,  -0.5f,-0.5f,-0.5f,

        -0.5f,-0.5f, 0.5f,   0.5f,-0.5f, 0.5f,   0.5f, 0.5f, 0.5f,
         0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,  -0.5f,-0.5f, 0.5f,

        -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f,-0.5f,  -0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,  -0.5f,-0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,

         0.5f, 0.5f, 0.5f,   0.5f, 0.5f,-0.5f,   0.5f,-0.5f,-0.5f,
         0.5f,-0.5f,-0.5f,   0.5f,-0.5f, 0.5f,   0.5f, 0.5f, 0.5f,

        -0.5f,-0.5f,-0.5f,   0.5f,-0.5f,-0.5f,   0.5f,-0.5f, 0.5f,
         0.5f,-0.5f, 0.5f,  -0.5f,-0.5f, 0.5f,  -0.5f,-0.5f,-0.5f,

        -0.5f, 0.5f,-0.5f,   0.5f, 0.5f,-0.5f,   0.5f, 0.5f, 0.5f,
         0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f,-0.5f
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    shaderProgCube = createShaderProgram("line_vert_shader.glsl", "line_frag_shader.glsl");

    if (!shaderProgCube) return false;

    glUseProgram(shaderProgCube);
    projLocCube     = glGetUniformLocation(shaderProgCube, "projection");
    viewLocCube     = glGetUniformLocation(shaderProgCube, "view");
    modelLocCube    = glGetUniformLocation(shaderProgCube, "model");
    objColorLocCube = glGetUniformLocation(shaderProgCube, "lineColor");
    glUniformMatrix4fv(projLocCube, 1, GL_FALSE, &projection[0][0]);

    return true;
}

bool Simulation::SetAxis() {
    GLfloat lineVertices[] = { 
        // x-axis (red)
        0.0f, 0.0f, 0.0f,   // x1, y1
        10.0f, 0.0f, 0.0f,   // x2, y2
        
        // y-axis (green) 
        0.0f, 0.0f, 0.0f,   
        0.0f, 10.0f, 0.0f,   
        
        // z-axis (blue)
        0.0f, 0.0f, 0.0f,    
        0.0f, 0.0f, 10.0f    
    };

    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    shaderProgLine = createShaderProgram("line_vert_shader.glsl", "line_frag_shader.glsl");

    if (!shaderProgLine) return false;

    glUseProgram(shaderProgLine);
    projLocLine     = glGetUniformLocation(shaderProgLine, "projection");
    viewLocLine     = glGetUniformLocation(shaderProgLine, "view");
    modelLocLine    = glGetUniformLocation(shaderProgLine, "model");
    objColorLocLine = glGetUniformLocation(shaderProgLine, "lineColor");
    glUniformMatrix4fv(projLocLine, 1, GL_FALSE, &projection[0][0]);

    return true;
}

void Simulation::EarlyUpdate() {
    // Ogarniamy zmiany trybu 'capture'
    if (mouseCapture != wasCaptured) {
        if (mouseCapture) {
            // Zapamiętaj pozycję
            SDL_GetMouseState(&prevX, &prevY);
            // Ukryj kursor
            SDL_SetWindowRelativeMouseMode(mWindow, true);
            SDL_GetRelativeMouseState(&xrel, &yrel);
        } 
        else {
            // Pokaż kursor
            SDL_SetWindowRelativeMouseMode(mWindow, false);
            // Przywraca poprzednie współrzędne
            SDL_WarpMouseInWindow(mWindow, prevX, prevY);
        }
        wasCaptured = mouseCapture;
    }

    // Jeśli jesteśmy w trybie 'capture' 
    if (mouseCapture) {
        // Aktualizacja myszy
        SDL_GetRelativeMouseState(&xrel, &yrel);
        mouseMovement(xrel, yrel);
    }

    processInput();

    // Aktualizacja kamery
    view = glm::lookAt(camPos, camPos+camFront, camUp);
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
    propModel = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1));

    // Reset Timer'a, żeby liczył następne deltaTime
    mTimer.Reset();
}

// Renderowanie linii osi
void Simulation::RenderAxis() {
    glUseProgram(shaderProgLine);

    lineModel = glm::mat4(1.0f);

    glUniformMatrix4fv(modelLocLine, 1, GL_FALSE, &lineModel[0][0]);
    glUniformMatrix4fv(viewLocLine, 1, GL_FALSE, &view[0][0]);
    glUniform3f(objColorLocLine, 1.0f, 0.0f, 0.0f); // red line

    glBindVertexArray(lineVAO);
    glLineWidth(3.0f);
    // Draw x-axis (red)
    glUniform3f(objColorLocLine, 1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINES, 0, 2);
    
    // Draw y-axis (green)  
    glUniform3f(objColorLocLine, 0.0f, 1.0f, 0.0f);
    glDrawArrays(GL_LINES, 2, 2);
    
    // Draw z-axis (blue)
    glUniform3f(objColorLocLine, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, 4, 2);

    glBindVertexArray(0);
}

void Simulation::RenderPropeller() {
    // Renderowanie śruby
    glUseProgram(shaderProgProp);
    glUniformMatrix4fv(modelLocProp, 1, GL_FALSE, &propModel[0][0]);
    glUniformMatrix4fv(viewLocProp, 1, GL_FALSE, &view[0][0]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
    glBindVertexArray(0);
}

void Simulation::RenderCube() {
    // Renderowanie kostki
    glUseProgram(shaderProgCube);

    cubeModel = glm::mat4(1.0f);

    glUniformMatrix4fv(modelLocCube, 1, GL_FALSE, &cubeModel[0][0]);
    glUniformMatrix4fv(viewLocCube, 1, GL_FALSE, &view[0][0]);
    glUniform3f(objColorLocCube, 1.0f, 0.0f, 0.0f); // red Cube

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Simulation::Render() {
    // Czyści bufor kolorów i bufor głębokości
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Ustawia kolor tła na czarny (RGBA)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if (!simState[0]) RenderAxis();
    if (!simState[1]) RenderCube();
    if (!simState[2]) RenderPropeller();
    
    // Zamień bufor
    SDL_GL_SwapWindow(mWindow);
}

// Główna pętla
void Simulation::Run() {
    while (running) {

        // Aktualizowanie deltaTime
        mTimer.Update();

        while (SDL_PollEvent(&mEvents)) {
            if (mEvents.type == SDL_EVENT_QUIT) 
                running = false;

            // Wykrycie naciśnięcia przycisku klawiatury
            if (mEvents.type == SDL_EVENT_KEY_DOWN) {
                int sc = mEvents.key.scancode;
                keys[sc] = true;

                switch(sc) {
                    // Sprawdzenie klawisza ESC
                    case SDL_SCANCODE_ESCAPE: running = false; break;

                    // Zminana trybu myszy za pomocą Q
                    case SDL_SCANCODE_Q: mouseCapture = !mouseCapture; break;

                    default: break;
                }

                if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9) {
                    int index = sc - SDL_SCANCODE_1;   // 0–8
                    simState[index] = !simState[index];
                }
            }

            if (mEvents.type == SDL_EVENT_KEY_UP) 
                keys[mEvents.key.scancode] = false;

        }

        // Upewnienie się, że program będzie działał w danym zarkresie FPS
        if (mTimer.DeltaTime() >= (1.0f / FRAME_RATE)) {
            EarlyUpdate();
            Update();
            LateUpdate();
            Render();
        }
    }
}
