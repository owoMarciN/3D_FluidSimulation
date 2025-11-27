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

        int widthResize, heightResize;

        bool running;
        Timer& mTimer;

        SDL_Event mEvents;
        
        // Kamera
        glm::vec3 camPos;
        glm::vec3 camFront;
        glm::vec3 camUp;

        float prevX, prevY;
        float xrel, yrel;

        float cameraSpeed;
        bool mouseCapture;
        bool wasCaptured;

        float yaw;
        float pitch;

        float propAngle;

        // Bufor klawiatury 2^10
        bool keys[1024];

        bool simState[10];

        // Vertex + Normal
        std::vector<float> vertices;
        std::vector<float> normals;

        GLuint shaderProgramProp;
        GLuint shaderProgramLine;
        GLuint shaderProgramCube;

        std::unordered_map<std::string, GLint> uLocPropeller;
        std::unordered_map<std::string, GLint> uLocLine;
        std::unordered_map<std::string, GLint> uLocCube;

        GLuint VAO, VBO, NBO;
        glm::mat4 propModel;
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;

        GLuint lineVAO, lineVBO;
        glm::mat4 lineModel;

        GLuint cubeVAO, cubeVBO;
        glm::mat4 cubeModel;

    public:

        void Run();
        bool Init();

    private:

        void processInput();
        void mouseMovement(int xoffset, int yoffset);

        std::string readShaderFile(const std::string& path);
        GLuint compileShader(GLenum type, const char* src);
        GLuint createShaderProgram(const std::string& vertName, const std::string& fragName);

        void AddUniformVec3(const std::string& name, const glm::vec3& vec);
        void AddUniformMat4(const std::string& name, const glm::mat4& mat);

        bool loadObj(const std::string& path);

        bool CreatePropeller();
        bool CreateCube();
        bool CreateAxis();

        void DrawPropeller();
        void DrawCube();
        void DrawAxis();

        void EarlyUpdate();
        void Update();
        void LateUpdate();

        void Render();

        Simulation();
        ~Simulation();
};

#endif