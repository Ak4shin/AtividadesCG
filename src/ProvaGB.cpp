#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const GLuint WIDTH = 1000, HEIGHT = 1000;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool firstMouse = true;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;

struct Vertex
{
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
};

struct Material
{
    glm::vec3 Ka = glm::vec3(0.25f);
    glm::vec3 Kd = glm::vec3(0.8f);
    glm::vec3 Ks = glm::vec3(0.3f);
    float Ns = 32.0f;
    string nomeTextura;
};

class Camera
{
public:
    glm::vec3 position = glm::vec3(0.0f, 1.5f, 6.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float yaw = -90.0f;
    float pitch = 0.0f;
    float speed = 4.0f;
    float sensitivity = 0.08f;

    glm::mat4 getViewMatrix()
    {
        return glm::lookAt(position, position + front, up);
    }

    void mover(GLFWwindow* window, float dt)
    {
        float cameraSpeed = speed * dt;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            position += cameraSpeed * front;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            position -= cameraSpeed * front;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            position -= glm::normalize(glm::cross(front, up)) * cameraSpeed;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            position += glm::normalize(glm::cross(front, up)) * cameraSpeed;
    }

    void rotacionar(float xoffset, float yoffset)
    {
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f)
            pitch = 89.0f;

        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        front = glm::normalize(direction);

        glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};

Camera camera;

struct Objeto3D
{
    string nome;

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint textureID = 0;

    vector<Vertex> vertices;

    glm::vec3 posicao = glm::vec3(0.0f);
    glm::vec3 rotacao = glm::vec3(0.0f);
    glm::vec3 escala = glm::vec3(1.0f);

    string arquivoMTL;
    Material material;
};

vector<Objeto3D> objetos;
int objetoSelecionado = 0;

bool keyLightOn = true;
bool fillLightOn = true;
bool backLightOn = true;

bool animacaoBezierAtiva = false;
float tBezier = 0.0f;
float velocidadeBezier = 0.25f;
int indiceSuzanne = 1;

vector<glm::vec3> pontosBezier = {
    glm::vec3(-4.0f, 0.0f,  2.5f),
    glm::vec3(-4.0f, 0.0f, -4.0f),
    glm::vec3( 4.0f, 0.0f, -4.0f),
    glm::vec3( 4.0f, 0.0f,  2.5f)
};

enum ModoTransformacao
{
    ROTACAO,
    TRANSLACAO,
    ESCALA
};

ModoTransformacao modoAtual = TRANSLACAO;
int eixoAtual = 0;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

GLuint setupShader();
bool carregarOBJ(const string& caminhoOBJ, Objeto3D& objeto);
bool carregarMTL(const string& caminhoMTL, Material& material);
GLuint carregarTextura(const string& caminhoTextura);
GLuint criarTexturaBranca();
void configurarBuffers(Objeto3D& objeto);
void normalizarModelo(Objeto3D& objeto);
void criarChao(Objeto3D& chao, const string& textura);
void criarPlanoFundo(Objeto3D& fundo, const string& nome, const string& textura,
    glm::vec3 posicao, glm::vec3 rotacao, float largura, float altura);
void aplicarTransformacao(float direcao);
glm::vec3 calcularBezier(float t);
void atualizarBezier(float dt);
void imprimirControles();

const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec2 texCoord;\n"
"layout (location = 2) in vec3 normal;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"\n"
"out vec2 TexCoord;\n"
"out vec3 FragPos;\n"
"out vec3 Normal;\n"
"\n"
"void main()\n"
"{\n"
"    FragPos = vec3(model * vec4(position, 1.0));\n"
"    Normal = mat3(transpose(inverse(model))) * normal;\n"
"    TexCoord = texCoord;\n"
"    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
"}\0";

const GLchar* fragmentShaderSource = "#version 450\n"
"in vec2 TexCoord;\n"
"in vec3 FragPos;\n"
"in vec3 Normal;\n"
"\n"
"out vec4 color;\n"
"\n"
"uniform sampler2D texture1;\n"
"uniform vec3 viewPos;\n"
"\n"
"uniform vec3 Ka;\n"
"uniform vec3 Kd;\n"
"uniform vec3 Ks;\n"
"uniform float Ns;\n"
"\n"
"uniform vec3 lightPositions[3];\n"
"uniform vec3 lightIntensities[3];\n"
"uniform int lightEnabled[3];\n"
"\n"
"vec3 calcularLuz(vec3 lightPos, vec3 intensidade, int ativa, vec3 texColor, vec3 norm, vec3 viewDir)\n"
"{\n"
"    if (ativa == 0)\n"
"        return vec3(0.0);\n"
"\n"
"    vec3 lightDir = normalize(lightPos - FragPos);\n"
"    vec3 reflectDir = reflect(-lightDir, norm);\n"
"\n"
"    float distancia = length(lightPos - FragPos);\n"
"    float atenuacao = 1.0 / (1.0 + 0.09 * distancia + 0.032 * distancia * distancia);\n"
"\n"
"    float diff = max(dot(norm, lightDir), 0.0);\n"
"    vec3 diffuse = Kd * diff * texColor * intensidade * atenuacao;\n"
"\n"
"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), Ns);\n"
"    vec3 specular = Ks * spec * intensidade;\n"
"\n"
"    return diffuse + specular;\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    vec3 texColor = texture(texture1, TexCoord).rgb;\n"
"    vec3 norm = normalize(Normal);\n"
"    vec3 viewDir = normalize(viewPos - FragPos);\n"
"\n"
"    vec3 ambient = Ka * texColor;\n"
"\n"
"    vec3 resultado = ambient;\n"
"    resultado += calcularLuz(lightPositions[0], lightIntensities[0], lightEnabled[0], texColor, norm, viewDir);\n"
"    resultado += calcularLuz(lightPositions[1], lightIntensities[1], lightEnabled[1], texColor, norm, viewDir);\n"
"    resultado += calcularLuz(lightPositions[2], lightIntensities[2], lightEnabled[2], texColor, norm, viewDir);\n"
"\n"
"    color = vec4(resultado, 1.0);\n"
"}\n\0";

int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Prova GB - Diorama 3D", nullptr, nullptr);

    if (!window)
    {
        cout << "Erro ao criar janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Erro ao inicializar GLAD" << endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);

    GLuint shaderID = setupShader();

    Objeto3D chao;
    criarChao(chao, "../assets/tex/grass1.png");
    configurarBuffers(chao);
    objetos.push_back(chao);

    Objeto3D suzanne;
    suzanne.nome = "Suzanne";
    suzanne.posicao = glm::vec3(0.0f, 0.0f, 0.0f);
    suzanne.escala = glm::vec3(1.3f);

    if (!carregarOBJ("../assets/Modelos3D/Suzanne.obj", suzanne))
    {
        cout << "Erro ao carregar Suzanne" << endl;
        return -1;
    }

    configurarBuffers(suzanne);
    objetos.push_back(suzanne);

    Objeto3D fundoFlorestaTras;
    criarPlanoFundo(
    fundoFlorestaTras,
    "Fundo Floresta Tras",
    "../assets/tex/fundo_floresta3.png",
    glm::vec3(0.0f, 8.0f, -30.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    60.0f,
    50.0f
    );
    configurarBuffers(fundoFlorestaTras);
    objetos.push_back(fundoFlorestaTras);

    

    Objeto3D arvore1;
    arvore1.nome = "Arvore 1";
    arvore1.posicao = glm::vec3(-6.0f, 0.7f, -5.5f);
    arvore1.escala = glm::vec3(2.5f);
    arvore1.rotacao.x = glm::radians(-90.0f);
    if (!carregarOBJ("../assets/Modelos3D/Tree.obj", arvore1)) return -1;
    configurarBuffers(arvore1);
    objetos.push_back(arvore1);

    Objeto3D arvore2;
    arvore2.nome = "Arvore 2";
    arvore2.posicao = glm::vec3(-2.5f, 0.7f, -6.0f);
    arvore2.escala = glm::vec3(2.5f);
    arvore2.rotacao.x = glm::radians(-90.0f);
    if (!carregarOBJ("../assets/Modelos3D/Tree.obj", arvore2)) return -1;
    configurarBuffers(arvore2);
    objetos.push_back(arvore2);

    Objeto3D arvore3;
    arvore3.nome = "Arvore 3";
    arvore3.posicao = glm::vec3(3.5f, 0.7f, -5.2f);
    arvore3.escala = glm::vec3(2.5f);
    arvore3.rotacao.x = glm::radians(-90.0f);
    if (!carregarOBJ("../assets/Modelos3D/Tree.obj", arvore3)) return -1;
    configurarBuffers(arvore3);
    objetos.push_back(arvore3);

    Objeto3D arvore4;
    arvore4.nome = "Arvore 4";
    arvore4.posicao = glm::vec3(7.0f, 0.7f, -4.8f);
    arvore4.escala = glm::vec3(2.5f);
    arvore4.rotacao.x = glm::radians(-90.0f);
    if (!carregarOBJ("../assets/Modelos3D/Tree.obj", arvore4)) return -1;
    configurarBuffers(arvore4);
    objetos.push_back(arvore4);

    Objeto3D arvore5;
    arvore5.nome = "Arvore 5";
    arvore5.posicao = glm::vec3(-7.0f, 0.7f, 2.5f);
    arvore5.escala = glm::vec3(2.5f);
    arvore5.rotacao.x = glm::radians(-90.0f);
    if (!carregarOBJ("../assets/Modelos3D/Tree.obj", arvore5)) return -1;
    configurarBuffers(arvore5);
    objetos.push_back(arvore5);

    Objeto3D arvore6;
    arvore6.nome = "Arvore 6";
    arvore6.posicao = glm::vec3(6.5f, 0.7f, 2.8f);
    arvore6.escala = glm::vec3(2.5f);
    arvore6.rotacao.x = glm::radians(-90.0f);
    if (!carregarOBJ("../assets/Modelos3D/Tree.obj", arvore6)) return -1;
    configurarBuffers(arvore6);
    objetos.push_back(arvore6);

    Objeto3D arvore7;
    arvore7.nome = "Arvore 7";
    arvore7.posicao = glm::vec3(1.0f, 0.7f, 6.5f);
    arvore7.escala = glm::vec3(2.5f);
    arvore7.rotacao.x = glm::radians(-90.0f);
    if (!carregarOBJ("../assets/Modelos3D/Tree.obj", arvore7)) return -1;
    configurarBuffers(arvore7);
    objetos.push_back(arvore7);

    Objeto3D pedra1;
    pedra1.nome = "Pedra 1";
    pedra1.posicao = glm::vec3(-4.5f, -0.3f, 0.8f);
    pedra1.escala = glm::vec3(0.7f);
    if (!carregarOBJ("../assets/Modelos3D/rock1.obj", pedra1)) return -1;
    configurarBuffers(pedra1);
    objetos.push_back(pedra1);

    Objeto3D pedra2;
    pedra2.nome = "Pedra 2";
    pedra2.posicao = glm::vec3(2.5f, -0.3f, 1.5f);
    pedra2.escala = glm::vec3(0.7f);
    if (!carregarOBJ("../assets/Modelos3D/rock1.obj", pedra2)) return -1;
    configurarBuffers(pedra2);
    objetos.push_back(pedra2);

    Objeto3D pedra3;
    pedra3.nome = "Pedra 3";
    pedra3.posicao = glm::vec3(5.5f, -0.3f, 3.5f);
    pedra3.escala = glm::vec3(0.7f);
    if (!carregarOBJ("../assets/Modelos3D/rock1.obj", pedra3)) return -1;
    configurarBuffers(pedra3);
    objetos.push_back(pedra3);

    Objeto3D pedra4;
    pedra4.nome = "Pedra 4";
    pedra4.posicao = glm::vec3(-6.0f, -0.3f, -1.5f);
    pedra4.escala = glm::vec3(0.7f);
    if (!carregarOBJ("../assets/Modelos3D/rock1.obj", pedra4)) return -1;
    configurarBuffers(pedra4);
    objetos.push_back(pedra4);

    Objeto3D pedra5;
    pedra5.nome = "Pedra 5";
    pedra5.posicao = glm::vec3(0.8f, -0.3f, -3.0f);
    pedra5.escala = glm::vec3(0.7f);
    if (!carregarOBJ("../assets/Modelos3D/rock1.obj", pedra5)) return -1;
    configurarBuffers(pedra5);
    objetos.push_back(pedra5);

    Objeto3D pedra6;
    pedra6.nome = "Pedra 6";
    pedra6.posicao = glm::vec3(7.0f, -0.3f, -1.0f);
    pedra6.escala = glm::vec3(0.7f);
    if (!carregarOBJ("../assets/Modelos3D/rock1.obj", pedra6)) return -1;
    configurarBuffers(pedra6);
    objetos.push_back(pedra6);

    Objeto3D pedra7;
    pedra7.nome = "Pedra 7";
    pedra7.posicao = glm::vec3(-2.0f, -0.3f, 4.5f);
    pedra7.escala = glm::vec3(0.7f);
    if (!carregarOBJ("../assets/Modelos3D/rock1.obj", pedra7)) return -1;
    configurarBuffers(pedra7);
    objetos.push_back(pedra7);


    objetoSelecionado = 1;

    glUseProgram(shaderID);

    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    GLint projectionLoc = glGetUniformLocation(shaderID, "projection");

    GLint viewPosLoc = glGetUniformLocation(shaderID, "viewPos");

    GLint kaLoc = glGetUniformLocation(shaderID, "Ka");
    GLint kdLoc = glGetUniformLocation(shaderID, "Kd");
    GLint ksLoc = glGetUniformLocation(shaderID, "Ks");
    GLint nsLoc = glGetUniformLocation(shaderID, "Ns");

    GLint lightPositionsLoc = glGetUniformLocation(shaderID, "lightPositions");
    GLint lightIntensitiesLoc = glGetUniformLocation(shaderID, "lightIntensities");
    GLint lightEnabledLoc = glGetUniformLocation(shaderID, "lightEnabled");

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)WIDTH / (float)HEIGHT,
        0.1f,
        100.0f
    );

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shaderID, "texture1"), 0);

    imprimirControles();

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        camera.mover(window, deltaTime);
        atualizarBezier(deltaTime);

        glClearColor(0.55f, 0.75f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderID);

        glm::mat4 view = camera.getViewMatrix();

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera.position));

        glm::vec3 centroCena = glm::vec3(0.0f, 0.0f, 0.0f);

        glm::vec3 lightPositions[3];
        lightPositions[0] = centroCena + glm::vec3(-4.0f, 5.0f, 4.0f);
        lightPositions[1] = centroCena + glm::vec3(4.0f, 3.0f, 4.0f);
        lightPositions[2] = centroCena + glm::vec3(0.0f, 5.0f, -4.0f);

        glm::vec3 lightIntensities[3];
        lightIntensities[0] = glm::vec3(1.00f, 0.95f, 0.85f);
        lightIntensities[1] = glm::vec3(0.35f, 0.40f, 0.50f);
        lightIntensities[2] = glm::vec3(0.65f, 0.75f, 1.00f);

        int lightEnabled[3];
        lightEnabled[0] = keyLightOn ? 1 : 0;
        lightEnabled[1] = fillLightOn ? 1 : 0;
        lightEnabled[2] = backLightOn ? 1 : 0;

        glUniform3fv(lightPositionsLoc, 3, glm::value_ptr(lightPositions[0]));
        glUniform3fv(lightIntensitiesLoc, 3, glm::value_ptr(lightIntensities[0]));
        glUniform1iv(lightEnabledLoc, 3, lightEnabled);

        for (int i = 0; i < objetos.size(); i++)
        {
            Objeto3D& obj = objetos[i];

            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, obj.posicao);

            model = glm::rotate(model, obj.rotacao.x, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, obj.rotacao.y, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, obj.rotacao.z, glm::vec3(0.0f, 0.0f, 1.0f));

            model = glm::scale(model, obj.escala);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glUniform3fv(kaLoc, 1, glm::value_ptr(obj.material.Ka));
            glUniform3fv(kdLoc, 1, glm::value_ptr(obj.material.Kd));
            glUniform3fv(ksLoc, 1, glm::value_ptr(obj.material.Ks));
            glUniform1f(nsLoc, obj.material.Ns);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, obj.textureID);

            glBindVertexArray(obj.VAO);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());

            if (i == objetoSelecionado)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(2.0f);
                glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }

            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
    }

    for (Objeto3D& obj : objetos)
    {
        glDeleteVertexArrays(1, &obj.VAO);
        glDeleteBuffers(1, &obj.VBO);
        glDeleteTextures(1, &obj.textureID);
    }

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (objetos.empty())
        return;

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        keyLightOn = !keyLightOn;
        cout << "Luz principal: " << (keyLightOn ? "ligada" : "desligada") << endl;
    }

    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        fillLightOn = !fillLightOn;
        cout << "Luz de preenchimento: " << (fillLightOn ? "ligada" : "desligada") << endl;
    }

    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        backLightOn = !backLightOn;
        cout << "Luz de fundo: " << (backLightOn ? "ligada" : "desligada") << endl;
    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        animacaoBezierAtiva = !animacaoBezierAtiva;

        cout << "Animacao Bezier: "
             << (animacaoBezierAtiva ? "ativa" : "pausada")
            << endl;
    }

    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        tBezier = 0.0f;

        if (indiceSuzanne >= 0 && indiceSuzanne < objetos.size())
            objetos[indiceSuzanne].posicao = calcularBezier(tBezier);

        cout << "Animacao Bezier reiniciada" << endl;
    }

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        objetoSelecionado++;

        if (objetoSelecionado >= objetos.size())
            objetoSelecionado = 0;

        cout << "Objeto selecionado: " << objetos[objetoSelecionado].nome << endl;
    }

    if (key == GLFW_KEY_R)
    {
        modoAtual = ROTACAO;
        cout << "Modo: ROTACAO" << endl;
    }

    if (key == GLFW_KEY_T)
    {
        modoAtual = TRANSLACAO;
        cout << "Modo: TRANSLACAO" << endl;
    }

    if (key == GLFW_KEY_E)
    {
        modoAtual = ESCALA;
        cout << "Modo: ESCALA" << endl;
    }

    if (key == GLFW_KEY_X)
    {
        eixoAtual = 0;
        cout << "Eixo: X" << endl;
    }

    if (key == GLFW_KEY_Y)
    {
        eixoAtual = 1;
        cout << "Eixo: Y" << endl;
    }

    if (key == GLFW_KEY_Z)
    {
        eixoAtual = 2;
        cout << "Eixo: Z" << endl;
    }

    if (key == GLFW_KEY_U)
        aplicarTransformacao(1.0f);

    if (key == GLFW_KEY_J)
        aplicarTransformacao(-1.0f);

    Objeto3D& obj = objetos[objetoSelecionado];

    if (key == GLFW_KEY_UP)
        obj.posicao.z -= 0.1f;

    if (key == GLFW_KEY_DOWN)
        obj.posicao.z += 0.1f;

    if (key == GLFW_KEY_LEFT)
        obj.posicao.x -= 0.1f;

    if (key == GLFW_KEY_RIGHT)
        obj.posicao.x += 0.1f;

    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD)
        obj.escala += glm::vec3(0.05f);

    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT)
    {
        obj.escala -= glm::vec3(0.05f);

        obj.escala.x = max(obj.escala.x, 0.05f);
        obj.escala.y = max(obj.escala.y, 0.05f);
        obj.escala.z = max(obj.escala.z, 0.05f);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.rotacionar(xoffset, yoffset);
}

void aplicarTransformacao(float direcao)
{
    Objeto3D& obj = objetos[objetoSelecionado];

    if (modoAtual == ROTACAO)
    {
        obj.rotacao[eixoAtual] += glm::radians(5.0f) * direcao;
    }
    else if (modoAtual == TRANSLACAO)
    {
        obj.posicao[eixoAtual] += 0.1f * direcao;
    }
    else if (modoAtual == ESCALA)
    {
        obj.escala[eixoAtual] += 0.05f * direcao;

        if (obj.escala[eixoAtual] < 0.05f)
            obj.escala[eixoAtual] = 0.05f;
    }
}

glm::vec3 calcularBezier(float t)
{
    glm::vec3 P0 = pontosBezier[0];
    glm::vec3 P1 = pontosBezier[1];
    glm::vec3 P2 = pontosBezier[2];
    glm::vec3 P3 = pontosBezier[3];

    float u = 1.0f - t;

    glm::vec3 ponto =
        (u * u * u) * P0 +
        (3.0f * u * u * t) * P1 +
        (3.0f * u * t * t) * P2 +
        (t * t * t) * P3;

    return ponto;
}

void atualizarBezier(float dt)
{
    if (!animacaoBezierAtiva)
        return;

    if (indiceSuzanne < 0 || indiceSuzanne >= objetos.size())
        return;

    tBezier += velocidadeBezier * dt;

    if (tBezier > 1.0f)
        tBezier = 0.0f;

    objetos[indiceSuzanne].posicao = calcularBezier(tBezier);
}

void criarChao(Objeto3D& chao, const string& textura)
{
    chao.nome = "Chao";
    chao.posicao = glm::vec3(0.0f, -0.55f, 0.0f);
    chao.escala = glm::vec3(1.0f);

    chao.material.Ka = glm::vec3(0.25f);
    chao.material.Kd = glm::vec3(0.9f);
    chao.material.Ks = glm::vec3(0.1f);
    chao.material.Ns = 8.0f;

    float tamanho = 20.0f;
    float repeticao = 8.0f;
    float y = 0.0f;

    chao.vertices = {
        {{-tamanho, y, -tamanho}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ tamanho, y, -tamanho}, {repeticao, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ tamanho, y,  tamanho}, {repeticao, repeticao}, {0.0f, 1.0f, 0.0f}},

        {{-tamanho, y, -tamanho}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ tamanho, y,  tamanho}, {repeticao, repeticao}, {0.0f, 1.0f, 0.0f}},
        {{-tamanho, y,  tamanho}, {0.0f, repeticao}, {0.0f, 1.0f, 0.0f}}
    };

    chao.textureID = carregarTextura(textura);

    if (chao.textureID == 0)
        chao.textureID = criarTexturaBranca();
}

void criarPlanoFundo(Objeto3D& fundo, const string& nome, const string& textura,
    glm::vec3 posicao, glm::vec3 rotacao, float largura, float altura)
{
    fundo.nome = nome;
    fundo.posicao = posicao;
    fundo.rotacao = rotacao;
    fundo.escala = glm::vec3(1.0f);

    fundo.material.Ka = glm::vec3(0.8f);
    fundo.material.Kd = glm::vec3(0.6f);
    fundo.material.Ks = glm::vec3(0.0f);
    fundo.material.Ns = 1.0f;

    float w = largura / 2.0f;
    float h = altura / 2.0f;

    fundo.vertices = {
        {{-w, -h, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ w, -h, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ w,  h, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

        {{-w, -h, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ w,  h, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-w,  h, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
    };

    fundo.textureID = carregarTextura(textura);

    if (fundo.textureID == 0)
        fundo.textureID = criarTexturaBranca();
}

bool carregarOBJ(const string& caminhoOBJ, Objeto3D& objeto)
{
    ifstream arquivo(caminhoOBJ);

    if (!arquivo.is_open())
    {
        cout << "Erro ao abrir OBJ: " << caminhoOBJ << endl;
        return false;
    }

    vector<glm::vec3> posicoes;
    vector<glm::vec2> texCoords;
    vector<glm::vec3> normais;

    string linha;
    string pastaBase = caminhoOBJ.substr(0, caminhoOBJ.find_last_of("/\\") + 1);

    while (getline(arquivo, linha))
    {
        if (linha.empty() || linha[0] == '#')
            continue;

        stringstream ss(linha);
        string tipo;
        ss >> tipo;

        if (tipo == "mtllib")
        {
            ss >> objeto.arquivoMTL;

            string caminhoMTL = pastaBase + objeto.arquivoMTL;

            if (carregarMTL(caminhoMTL, objeto.material))
            {
                if (!objeto.material.nomeTextura.empty())
                {
                    string caminhoTextura = pastaBase + objeto.material.nomeTextura;
                    objeto.textureID = carregarTextura(caminhoTextura);
                }
            }
        }
        else if (tipo == "v")
        {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            posicoes.push_back(pos);
        }
        else if (tipo == "vt")
        {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            texCoords.push_back(uv);
        }
        else if (tipo == "vn")
        {
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            normais.push_back(normal);
        }
        else if (tipo == "f")
        {
            vector<string> tokens;
            string token;

            while (ss >> token)
                tokens.push_back(token);

            for (int i = 1; i < tokens.size() - 1; i++)
            {
                string faceTokens[3] = { tokens[0], tokens[i], tokens[i + 1] };

                for (string faceToken : faceTokens)
                {
                    stringstream fs(faceToken);

                    string posStr;
                    string texStr;
                    string normalStr;

                    getline(fs, posStr, '/');
                    getline(fs, texStr, '/');
                    getline(fs, normalStr, '/');

                    int posIndex = stoi(posStr) - 1;
                    int texIndex = -1;
                    int normalIndex = -1;

                    if (!texStr.empty())
                        texIndex = stoi(texStr) - 1;

                    if (!normalStr.empty())
                        normalIndex = stoi(normalStr) - 1;

                    glm::vec3 pos = posicoes[posIndex];

                    glm::vec2 uv = glm::vec2(0.0f);
                    if (texIndex >= 0 && texIndex < texCoords.size())
                        uv = texCoords[texIndex];

                    glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);
                    if (normalIndex >= 0 && normalIndex < normais.size())
                        normal = normais[normalIndex];

                    objeto.vertices.push_back({ pos, uv, normal });
                }
            }
        }
    }

    if (objeto.vertices.empty())
    {
        cout << "OBJ carregado, mas sem vertices validos." << endl;
        return false;
    }

    if (objeto.textureID == 0)
        objeto.textureID = criarTexturaBranca();

    normalizarModelo(objeto);

    cout << "OBJ carregado: " << caminhoOBJ << endl;
    cout << "Vertices: " << objeto.vertices.size() << endl;
    cout << "MTL: " << objeto.arquivoMTL << endl;
    cout << "Textura: " << objeto.material.nomeTextura << endl;

    return true;
}

bool carregarMTL(const string& caminhoMTL, Material& material)
{
    ifstream arquivo(caminhoMTL);

    if (!arquivo.is_open())
    {
        cout << "Erro ao abrir MTL: " << caminhoMTL << endl;
        return false;
    }

    string linha;

    while (getline(arquivo, linha))
    {
        if (linha.empty() || linha[0] == '#')
            continue;

        replace(linha.begin(), linha.end(), '\\', '/');

        stringstream ss(linha);
        string tipo;
        ss >> tipo;

        if (tipo == "Ka")
        {
            ss >> material.Ka.x >> material.Ka.y >> material.Ka.z;
        }
        else if (tipo == "Kd")
        {
            ss >> material.Kd.x >> material.Kd.y >> material.Kd.z;
        }
        else if (tipo == "Ks")
        {
            ss >> material.Ks.x >> material.Ks.y >> material.Ks.z;
        }
        else if (tipo == "Ns")
        {
            ss >> material.Ns;
        }
        else if (tipo == "map_Kd")
        {
            ss >> material.nomeTextura;

            size_t pos = material.nomeTextura.find_last_of("/\\");
            if (pos != string::npos)
                material.nomeTextura = material.nomeTextura.substr(pos + 1);
        }
    }

    cout << "MTL carregado: " << caminhoMTL << endl;
    return true;
}

GLuint carregarTextura(const string& caminhoTextura)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(
        caminhoTextura.c_str(),
        &width,
        &height,
        &nrChannels,
        0
    );

    if (!data)
    {
        cout << "Erro ao carregar textura: " << caminhoTextura << endl;
        return 0;
    }

    GLenum format = GL_RGB;

    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    cout << "Textura carregada: " << caminhoTextura << endl;
    cout << "Tamanho: " << width << "x" << height << endl;

    return textureID;
}

GLuint criarTexturaBranca()
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    unsigned char branco[] = { 255, 255, 255, 255 };

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        1,
        1,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        branco
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

void configurarBuffers(Objeto3D& objeto)
{
    glGenVertexArrays(1, &objeto.VAO);
    glGenBuffers(1, &objeto.VBO);

    glBindVertexArray(objeto.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, objeto.VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        objeto.vertices.size() * sizeof(Vertex),
        objeto.vertices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, position)
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, texCoord)
    );
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        2,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, normal)
    );
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void normalizarModelo(Objeto3D& objeto)
{
    if (objeto.vertices.empty())
        return;

    glm::vec3 minP = objeto.vertices[0].position;
    glm::vec3 maxP = objeto.vertices[0].position;

    for (const Vertex& v : objeto.vertices)
    {
        minP = glm::min(minP, v.position);
        maxP = glm::max(maxP, v.position);
    }

    glm::vec3 centro = (minP + maxP) * 0.5f;
    glm::vec3 tamanho = maxP - minP;

    float maior = max(tamanho.x, max(tamanho.y, tamanho.z));

    if (maior == 0.0f)
        maior = 1.0f;

    for (Vertex& v : objeto.vertices)
    {
        v.position = (v.position - centro) / maior;
        v.normal = glm::normalize(v.normal);
    }
}

GLuint setupShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "Erro no Vertex Shader:\n" << infoLog << endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "Erro no Fragment Shader:\n" << infoLog << endl;
    }

    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "Erro ao linkar Shader Program:\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void imprimirControles()
{
    cout << "===== CONTROLES =====" << endl;
    cout << "W/A/S/D: mover camera" << endl;
    cout << "Mouse: olhar ao redor" << endl;
    cout << "TAB: selecionar objeto" << endl;
    cout << "R/T/E: modo rotacao/translacao/escala" << endl;
    cout << "X/Y/Z: escolher eixo" << endl;
    cout << "U: aplicar positivo" << endl;
    cout << "J: aplicar negativo" << endl;
    cout << "Setas: mover objeto no chao" << endl;
    cout << "+/-: escala uniforme" << endl;
    cout << "1: liga/desliga luz principal" << endl;
    cout << "2: liga/desliga luz de preenchimento" << endl;
    cout << "3: liga/desliga luz de fundo" << endl;
    cout << "ESC: sair" << endl;
    cout << "B: iniciar/pausar animacao Bezier" << endl;
    cout << "N: reiniciar animacao Bezier" << endl;
}