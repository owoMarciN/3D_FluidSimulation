#include "Simulation.h"

Simulation::Simulation(): mTimer(Timer::Instance()) {
    // Nazwa okna
    window_name = "Fluid Simulation";

    // Ścieżka do naszego modelu 3D
    model_path  = "assets/models/Propeller.obj";

    // Ścieżka do shaderów
    shader_path = "assets/shaders/";

    // Zmienna upewniająca się, że symulacja pracuje
    running = true;
    
    // Initializujemy symulacje
    if (!Init()) running = false;

    // Inicjalizacja tablic
    std::fill(keys, keys + numKeys, false);
    std::fill(simState, simState + numStates, true);

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
    propAngle = 0.0f;

    // Matryce modeli przydatne jeśli chcemy transformacje, rotacje, czy skalownie
    propModel = glm::mat4(1.0f);
    voxelMeshModel = glm::mat4(1.0f);
    lineModel = glm::mat4(1.0f);

    projMatrix = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);
}

Simulation::~Simulation() {
    std::cout << "Simulation -- Destroyed" << std::endl;
    
    if (propVAO) glDeleteVertexArrays(1, &propVAO);
    if (propVBO) glDeleteBuffers(1, &propVBO);
    if (propNBO) glDeleteBuffers(1, &propNBO);

    if (lineVAO) glDeleteVertexArrays(1, &lineVAO);
    if (lineVBO) glDeleteBuffers(1, &lineVBO);

    if (voxelMeshVAO) glDeleteVertexArrays(1, &voxelMeshVAO);
    if (voxelMeshVBO) glDeleteBuffers(1, &voxelMeshVBO);

    if (shaderProgramProp) glDeleteProgram(shaderProgramProp);
    if (shaderProgramLine) glDeleteProgram(shaderProgramLine);
    if (shaderProgramMesh) glDeleteProgram(shaderProgramMesh);

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
            propVertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
            propVertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
            propVertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

            propNormals.push_back(attrib.normals[3 * idx.normal_index + 0]);
            propNormals.push_back(attrib.normals[3 * idx.normal_index + 1]);
            propNormals.push_back(attrib.normals[3 * idx.normal_index + 2]);
        }
    }
    return true;
}

// Initializacja SDL3, Glad (opengl), shaderów i załadowanie pliku Propeller.obj
bool Simulation::Init() {
    // Initializacja SDL3
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "[ERROR] Inicjalizacja SDL nie powiodła się: " << SDL_GetError() << std::endl;
        return false;
    }

    // Wymagana wersja OpenGL przynajmniej 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Tworzenie okna SDL z kontekstem OpenGL
    mWindow = SDL_CreateWindow(window_name.c_str(), SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

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
        return false; 
    }

    std::cout << "Wersja OpenGL: " << glGetString(GL_VERSION) << '\n' << std::endl;

    glEnable(GL_DEPTH_TEST);

    // Wczytanie modelu z pliku OBJ
    if (!loadObj(model_path)) return false;

    // Funkcja niepotrzebna ponieważ została zaimplementowna klasa Timer
    // SDL_GL_SetSwapInterval(1); // V-sync

    if (!CreatePropeller()) {
        std::cerr << "[ERROR] Nie utworzono programu dla shadera śruby." << std::endl;
        return false;
    }
 
    if (!CreateVoxelMesh()) {
        std::cerr << "[ERROR] Nie utworzono programu dla shadera siatki." << std::endl;
        return false;
    }
    
    if (!CreateAxis()) {
        std::cerr << "[ERROR] Nie utworzono programu dla shadera linii osi." << std::endl;
        return false;
    }
    
    return true;
}

bool Simulation::CreatePropeller() {
    // Tworzenie i wiązanie Vertex Array Object (VAO)
    glGenVertexArrays(1, &propVAO);
    glBindVertexArray(propVAO);

    // Tworzenie Vertex Buffer Object (VBO) dla wierzchołków
    glGenBuffers(1, &propVBO);
    glBindBuffer(GL_ARRAY_BUFFER, propVBO);
    glBufferData(GL_ARRAY_BUFFER, propVertices.size() * sizeof(float), propVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Tworzenie Vertex Buffer Object (NBO) dla wektorów normalnych
    glGenBuffers(1, &propNBO);
    glBindBuffer(GL_ARRAY_BUFFER, propNBO);
    glBufferData(GL_ARRAY_BUFFER, propNormals.size() * sizeof(float), propNormals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    // Shadery
    shaderProgramProp = createShaderProgram(
        "vertex_shader.glsl", 
        "fragment_shader.glsl"
    );

    if (!shaderProgramProp) return false;

    glUseProgram(shaderProgramProp);

    // Pobranie informacji z shaderów
    uLocPropeller["model"] = glGetUniformLocation(shaderProgramProp, "model");
    uLocPropeller["view"] = glGetUniformLocation(shaderProgramProp, "view");
    uLocPropeller["projection"] = glGetUniformLocation(shaderProgramProp, "projection");
    uLocPropeller["objectColor"] = glGetUniformLocation(shaderProgramProp, "objectColor");
    uLocPropeller["lightPos"] = glGetUniformLocation(shaderProgramProp, "lightPos");
    uLocPropeller["lightColor"] = glGetUniformLocation(shaderProgramProp, "lightColor");
    uLocPropeller["lightPos2"] = glGetUniformLocation(shaderProgramProp, "lightPos2");
    uLocPropeller["lightColor2"] = glGetUniformLocation(shaderProgramProp, "lightColor2");

    // Ustawienie koloru obiektu (szare)
    glUniform3f(uLocPropeller["objectColor"], 0.5f, 0.5f, 0.5f); 

    // Ustawienie pozycji i koloru pierwszego światła
    glUniform3f(uLocPropeller["lightPos"], 0.0f, 0.0f, 3.0f); 
    glUniform3f(uLocPropeller["lightColor"], 1.0f, 1.0f, 1.0f); 

    // Ustawienie pozycji i koloru drugiego światła
    glUniform3f(uLocPropeller["lightPos2"], 0.0f, 0.0f, -3.0f); 
    glUniform3f(uLocPropeller["lightColor2"], 1.0f, 1.0f, 1.0f); 

    return true;
}

void Simulation::DrawPropeller() {
    // Renderowanie śruby
    glUseProgram(shaderProgramProp);
    glUniformMatrix4fv(uLocPropeller["model"], 1, GL_FALSE, glm::value_ptr(propModel));
    glUniformMatrix4fv(uLocPropeller["view"], 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(uLocPropeller["projection"], 1, GL_FALSE, glm::value_ptr(projMatrix));

    glBindVertexArray(propVAO);
    glDrawArrays(GL_TRIANGLES, 0, propVertices.size() / 3);
    glBindVertexArray(0);
}

bool Simulation::CreateVoxelMesh() {
    float voxelMeshScale = 0.2f; // zmiana skali na 1:5
    glm::vec3 voxelMeshOffset = glm::vec3(cubeNum, cubeNum, cubeNum) * voxelMeshScale * 0.5f;

    const glm::vec3 faceVertices[faceNum][vertsPerFace] = {
        // +X
        { {1,0,0},{1,1,0},{1,1,1},  {1,1,1},{1,0,1},{1,0,0} },
        // -X
        { {0,0,0},{0,0,1},{0,1,1},  {0,1,1},{0,1,0},{0,0,0} },
        // +Y
        { {0,1,0},{0,1,1},{1,1,1},  {1,1,1},{1,1,0},{0,1,0} },
        // -Y
        { {0,0,0},{1,0,0},{1,0,1},  {1,0,1},{0,0,1},{0,0,0} },
        // +Z
        { {0,0,1},{1,0,1},{1,1,1},  {1,1,1},{0,1,1},{0,0,1} },
        // -Z
        { {0,0,0},{0,1,0},{1,1,0},  {1,1,0},{1,0,0},{0,0,0} }
    };

    // Kierunki są używane do sprawdzania sąsiadów każdej komórki 'voxel'
    // Każdy indeks odpowiada jednej ścianie (face) w kolejności: +X, -X, +Y, -Y, +Z, -Z
    const int dirX[faceNum] = {1, -1, 0, 0, 0, 0}; // zmiana w osi X dla sąsiada
    const int dirY[faceNum] = {0, 0, 1, -1, 0, 0}; // zmiana w osi Y dla sąsiada
    const int dirZ[faceNum] = {0, 0, 0, 0, 1, -1}; // zmiana w osi Z dla sąsiada

    voxelMeshVertices.clear();

    // Inicjalizacja wszystkich voxelów jako "solid" (pełne)
    for(int x = 0; x < cubeNum; x++)
        for(int y = 0; y < cubeNum; y++)
            for(int z = 0; z < cubeNum; z++)
                voxels[x][y][z] = true;

    // Generowanie wierzchołków siatki tylko dla widocznych ścian
    for (int x = 0; x < cubeNum; x++)
        for (int y = 0; y < cubeNum; y++)
            for (int z = 0; z < cubeNum; z++) {

                if (!voxels[x][y][z]) continue; // pomiń puste komórki

                // Sprawdzenie każdej ze 6 ścian komórki
                for (int face = 0; face < faceNum; face++) {

                    // Pozycja sąsiada w danym kierunku
                    int nx = x + dirX[face];
                    int ny = y + dirY[face];
                    int nz = z + dirZ[face];

                    // Sprawdzenie, czy sąsiad istnieje i jest pełny
                    bool neighborSolid =
                        nx >= 0 && nx < cubeNum &&
                        ny >= 0 && ny < cubeNum &&
                        nz >= 0 && nz < cubeNum &&
                        voxels[nx][ny][nz];

                    if (!neighborSolid) {
                        // Ta ściana jest widoczna (nie ma pełnego sąsiada)
                        for (int i = 0; i < vertsPerFace; i++) {
                            // Dodanie wierzchołków ściany do wektora mesh
                            // Uwzględniamy skalowanie i przesunięcie, żeby cała siatka była wycentrowana
                            voxelMeshVertices.push_back((faceVertices[face][i].x + x) * voxelMeshScale - voxelMeshOffset.x);
                            voxelMeshVertices.push_back((faceVertices[face][i].y + y) * voxelMeshScale - voxelMeshOffset.y);
                            voxelMeshVertices.push_back((faceVertices[face][i].z + z) * voxelMeshScale - voxelMeshOffset.z);
                        }
                    }
                }
            }
            
    glGenVertexArrays(1, &voxelMeshVAO);
    glGenBuffers(1, &voxelMeshVBO);

    glBindVertexArray(voxelMeshVAO);
    glBindBuffer(GL_ARRAY_BUFFER, voxelMeshVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 voxelMeshVertices.size() * sizeof(float),
                 voxelMeshVertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    shaderProgramMesh = createShaderProgram(
        "mesh_vert_shader.glsl", 
        "mesh_frag_shader.glsl"
    );

    if (!shaderProgramMesh) return false;

    glUseProgram(shaderProgramMesh);

    uLocMesh["projection"] = glGetUniformLocation(shaderProgramMesh, "projection");
    uLocMesh["view"] = glGetUniformLocation(shaderProgramMesh, "view");
    uLocMesh["model"] = glGetUniformLocation(shaderProgramMesh, "model");
    uLocMesh["meshColor"] = glGetUniformLocation(shaderProgramMesh, "meshColor");
    
    std::cout << "model=" << uLocMesh["model"] 
          << ", view=" << uLocMesh["view"]
          << ", projection=" << uLocMesh["projection"]
          << ", meshColor=" << uLocMesh["meshColor"] << std::endl;

    return true;
}

void Simulation::DrawVoxelMesh() {
    glUseProgram(shaderProgramMesh);

    glUniformMatrix4fv(uLocMesh["model"], 1, GL_FALSE, glm::value_ptr(voxelMeshModel));
    glUniformMatrix4fv(uLocMesh["view"], 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(uLocMesh["projection"], 1, GL_FALSE, glm::value_ptr(projMatrix));
    glUniform3f(uLocMesh["meshColor"], 1.0f, 0.0f, 0.0f);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glBindVertexArray(voxelMeshVAO);
    glDrawArrays(GL_TRIANGLES, 0, voxelMeshVertices.size() / 3);
    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool Simulation::CreateAxis() {
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

    shaderProgramLine = createShaderProgram(
        "line_vert_shader.glsl", 
        "line_frag_shader.glsl"
    );

    if (!shaderProgramLine) return false;

    glUseProgram(shaderProgramLine);
    uLocLine["projection"] = glGetUniformLocation(shaderProgramLine, "projection");
    uLocLine["view"] = glGetUniformLocation(shaderProgramLine, "view");
    uLocLine["model"] = glGetUniformLocation(shaderProgramLine, "model");
    uLocLine["lineColor"] = glGetUniformLocation(shaderProgramLine, "lineColor");

    return true;
}

// Renderowanie linii osi
void Simulation::DrawAxis() {
    glUseProgram(shaderProgramLine);

    glUniformMatrix4fv(uLocLine["model"], 1, GL_FALSE, glm::value_ptr(lineModel));
    glUniformMatrix4fv(uLocLine["view"], 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(uLocLine["projection"], 1, GL_FALSE, glm::value_ptr(projMatrix));

    glBindVertexArray(lineVAO);
    glLineWidth(2.0f);
    // Draw x-axis (red)
    glUniform3f(uLocLine["lineColor"], 1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINES, 0, 2);
    
    // Draw y-axis (green)  
    glUniform3f(uLocLine["lineColor"], 0.0f, 1.0f, 0.0f);
    glDrawArrays(GL_LINES, 2, 2);
    
    // Draw z-axis (blue)
    glUniform3f(uLocLine["lineColor"], 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, 4, 2);

    glBindVertexArray(0);
}

void Simulation::EarlyUpdate() {
    // Tworzenie macierzy projekcji perspektywy
    // Jeśli chcemy zmieniać rozmiar okna, przenieść tą funckję do Update
    SDL_GetWindowSize(mWindow, &widthResize, &heightResize);
    projMatrix = glm::perspective(glm::radians(45.0f), (float)widthResize / (float)heightResize, 0.1f, 100.0f);

    // Aktualizacja kamery
    viewMatrix = glm::lookAt(camPos, camPos+camFront, camUp);

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
}

void Simulation::Update() {
    // Wyświetlenie FPS
    // Jak na razie symulacja nie jest pod wielkim obciążeniem, więc wszystko jest dobrze
    std::string FPS = std::to_string(1.0f / mTimer.DeltaTime());
    std::string ms = std::to_string(mTimer.DeltaTime() * 1000.0f);
    SDL_SetWindowTitle(mWindow, (window_name + " - " + FPS + "FPS / " + ms + "ms.").c_str());
}

void Simulation::LateUpdate() {
    // Reset Timer'a, żeby liczył następne deltaTime
     // Obrót śmigłem :)
    propAngle += mTimer.DeltaTime() * glm::radians(80.0f); // Obrót 80°/s
    propModel = glm::rotate(glm::mat4(1.0f), propAngle, glm::vec3(0, 0, 1));

    mTimer.Reset();
}

void Simulation::Render() {
    // Ustawia kolor tła na czarny (RGBA)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    // Czyści bufor kolorów i bufor głębokości
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 0-9
    if (simState[0]) DrawAxis();
    
    if (simState[1]) {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        DrawVoxelMesh();
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    if (simState[2]) DrawPropeller();
    
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

            // Wykrywanie zmiany rozmiarów okna
            if (mEvents.type == SDL_EVENT_WINDOW_RESIZED) {
                widthResize = mEvents.window.data1;
                heightResize = mEvents.window.data2;
                glViewport(0, 0, widthResize, heightResize);
            }

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

                if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_0) {
                    int index = sc - SDL_SCANCODE_1;   // 0–9
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
