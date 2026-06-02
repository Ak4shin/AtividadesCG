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
    glm::vec3 Ka = glm::vec3(0.2f);
    glm::vec3 Kd = glm::vec3(0.8f);
    glm::vec3 Ks = glm::vec3(0.5f);
    float Ns = 32.0f;

    string nomeTextura;
};

class Camera
{
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 worldUp;

    float yaw;
    float pitch;

    float speed;
    float sensitivity;

    Camera()
    {
        position = glm::vec3(0.0f, 1.0f, 5.0f);
        front = glm::vec3(0.0f, 0.0f, -1.0f);
        worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        up = worldUp;

        yaw = -90.0f;
        pitch = 0.0f;

        speed = 2.5f;
        sensitivity = 0.08f;
    }

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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

GLuint setupShader();
bool carregarOBJ(const string& caminhoOBJ, Objeto3D& objeto);
bool carregarMTL(const string& caminhoMTL, Material& material);
GLuint carregarTextura(const string& caminhoTextura);
void configurarBuffers(Objeto3D& objeto);
void normalizarModelo(Objeto3D& objeto);
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
"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;\n"
"\n"
"uniform vec3 Ka;\n"
"uniform vec3 Kd;\n"
"uniform vec3 Ks;\n"
"uniform float Ns;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 texColor = texture(texture1, TexCoord).rgb;\n"
"\n"
"    vec3 norm = normalize(Normal);\n"
"    vec3 lightDir = normalize(lightPos - FragPos);\n"
"    vec3 viewDir = normalize(viewPos - FragPos);\n"
"    vec3 reflectDir = reflect(-lightDir, norm);\n"
"\n"
"    vec3 ambient = Ka * texColor;\n"
"\n"
"    float diff = max(dot(norm, lightDir), 0.0);\n"
"    vec3 diffuse = Kd * diff * texColor;\n"
"\n"
"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), Ns);\n"
"    vec3 specular = Ks * spec;\n"
"\n"
"    vec3 resultado = ambient + diffuse + specular;\n"
"    color = vec4(resultado, 1.0);\n"
"}\n\0";

int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Desafio M5 - Camera em Primeira Pessoa", nullptr, nullptr);

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

    Objeto3D obj1;
    obj1.nome = "Suzanne 1";
    obj1.posicao = glm::vec3(-1.2f, 0.0f, 0.0f);

    if (!carregarOBJ("../assets/Modelos3D/Suzanne.obj", obj1))
    {
        cout << "Erro ao carregar OBJ 1" << endl;
        return -1;
    }

    configurarBuffers(obj1);
    objetos.push_back(obj1);

    Objeto3D obj2;
    obj2.nome = "Suzanne 2";
    obj2.posicao = glm::vec3(1.2f, 0.0f, 0.0f);
    obj2.rotacao.y = glm::radians(180.0f);

    if (!carregarOBJ("../assets/Modelos3D/Suzanne.obj", obj2))
    {
        cout << "Erro ao carregar OBJ 2" << endl;
        return -1;
    }

    configurarBuffers(obj2);
    objetos.push_back(obj2);

    glUseProgram(shaderID);

    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    GLint projectionLoc = glGetUniformLocation(shaderID, "projection");

    GLint lightPosLoc = glGetUniformLocation(shaderID, "lightPos");
    GLint viewPosLoc = glGetUniformLocation(shaderID, "viewPos");

    GLint kaLoc = glGetUniformLocation(shaderID, "Ka");
    GLint kdLoc = glGetUniformLocation(shaderID, "Kd");
    GLint ksLoc = glGetUniformLocation(shaderID, "Ks");
    GLint nsLoc = glGetUniformLocation(shaderID, "Ns");

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

        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderID);

        glm::mat4 view = camera.getViewMatrix();

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera.position));

        glUniform3f(lightPosLoc, 2.0f, 3.0f, 3.0f);

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
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
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
                string faceTokens[3] = {
                    tokens[0],
                    tokens[i],
                    tokens[i + 1]
                };

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

                    glm::vec2 uv = glm::vec2(0.0f, 0.0f);
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

        stringstream ss(linha);
        string tipo;
        ss >> tipo;

        if (tipo == "Ka")
            ss >> material.Ka.x >> material.Ka.y >> material.Ka.z;
        else if (tipo == "Kd")
            ss >> material.Kd.x >> material.Kd.y >> material.Kd.z;
        else if (tipo == "Ks")
            ss >> material.Ks.x >> material.Ks.y >> material.Ks.z;
        else if (tipo == "Ns")
            ss >> material.Ns;
        else if (tipo == "map_Kd")
            ss >> material.nomeTextura;
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
    cout << "W: mover para frente" << endl;
    cout << "S: mover para tras" << endl;
    cout << "A: mover para esquerda" << endl;
    cout << "D: mover para direita" << endl;
    cout << "Mouse: olhar ao redor" << endl;
    cout << "ESC: sair" << endl;
}