#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "Timer.h"

// Potrzebne do załadowania modelu z Blendera 
// Dotyczy tylko pliki o rozszerzeniu .obj
#include <tiny_obj_loader.h>

class Simulation {
    public:

        static Simulation &Instance() {
            static Simulation sInstance;
            return sInstance;
        }

    private:
        
        SDL_Window *mWindow;
        SDL_GLContext glContext;
        std::string window_name;
        std::string model_path;
        std::string shader_path;

        bool running;
        Timer& mTimer;

        SDL_Event mEvents;
        
        // Kamera
        glm::vec3 camPos;
        glm::vec3 camFront;
        glm::vec3 camUp;

        float xrel, yrel;
        float cameraSpeed;
        bool mouseCapture;

        float yaw;
        float pitch;
        
        float angle;

        // Bufor klawiatury 2^10
        bool keys[1024] = {false};

        // Vertex + Normal
        std::vector<float> vertices;
        std::vector<float> normals;

        GLuint shaderProgram;
        GLint modelLoc;
        GLint viewLoc;
        GLint projLoc;
        GLint objColorLoc;
        GLint lightPosLoc;
        GLint lightColorLoc;
        GLint lightPos2Loc;
        GLint lightColor2Loc;

        GLuint VAO, VBO, NBO;
        glm::mat4 model;
        glm::mat4 projection;

    public:

        void Run();
        bool Init();

    private:

        void processInput();
        void mouseMovement(int xoffset, int yoffset);

        std::string readShaderFile(const std::string& path);
        GLuint compileShader(GLenum type, const char* src);
        GLuint createShaderProgram();
        bool loadObj(const std::string& path);

        void EarlyUpdate();
        void Update();
        void LateUpdate();

        void Render();

        Simulation();
        ~Simulation();
};

#endif