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

        float prevX;
        float prevY;
        float xrel;
        float yrel;
        float cameraSpeed;
        bool mouseCapture;
        bool wasCaptured;

        float yaw;
        float pitch;
        
        float angle;

        // Bufor klawiatury 2^10
        bool keys[1024] = {false};

        // Vertex + Normal
        std::vector<float> vertices;
        std::vector<float> normals;

        GLuint shaderProgProp;
        GLint modelLocProp;
        GLint viewLocProp;
        GLint projLocProp;
        GLint objColorLocProp;
        GLint lightPosLoc;
        GLint lightColorLoc;
        GLint lightPos2Loc;
        GLint lightColor2Loc;

        GLuint VAO, VBO, NBO;
        glm::mat4 propModel;
        glm::mat4 projection;

        GLuint shaderProgLine;
        GLint objColorLocLine;
        GLint modelLocLine;
        GLint viewLocLine;
        GLint projLocLine;

        GLuint lineVAO, lineVBO;
        glm::mat4 lineModel;

        

    public:

        void Run();
        bool Init();

    private:

        void processInput();
        void mouseMovement(int xoffset, int yoffset);

        std::string readShaderFile(const std::string& path);
        GLuint compileShader(GLenum type, const char* src);
        GLuint createShaderProgram(const std::string& vertName, const std::string& fragName);
        bool loadObj(const std::string& path);

        void EarlyUpdate();
        void Update();
        void LateUpdate();

        void Render();

        Simulation();
        ~Simulation();
};

#endif