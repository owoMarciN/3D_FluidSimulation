#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "Timer.h"

// Potrzebne do załadowania modelu z Blendera 
// Dotyczy tylko pliki o rozszerzeniu .obj
#include <tiny_obj_loader.h>

static const int numKeys = 1024;
static const int numStates = 10;

static const int cubeNum = 6;
static const int faceNum = 6;
static const int vertsPerFace = 6;

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
        bool keys[numKeys];

        bool simState[numStates];

        // Vertex + Normal
        std::vector<float> propVertices;
        std::vector<float> propNormals;

        GLuint shaderProgramProp;
        GLuint shaderProgramLine;
        GLuint shaderProgramMesh;

        std::unordered_map<std::string, GLint> uLocPropeller;
        std::unordered_map<std::string, GLint> uLocLine;
        std::unordered_map<std::string, GLint> uLocMesh;

        GLuint propVAO, propVBO, propNBO;
        glm::mat4 propModel;
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;

        GLuint lineVAO, lineVBO;
        glm::mat4 lineModel;

        bool voxels[cubeNum][cubeNum][cubeNum];

        GLuint voxelMeshVAO, voxelMeshVBO;
        glm::mat4 voxelMeshModel;
        std::vector<float> voxelMeshVertices;

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

        bool CreatePropeller();
        bool CreateVoxelMesh();
        bool CreateAxis();

        void DrawPropeller();
        void DrawVoxelMesh();
        void DrawAxis();

        void EarlyUpdate();
        void Update();
        void LateUpdate();

        void Render();

        Simulation();
        ~Simulation();
};

#endif