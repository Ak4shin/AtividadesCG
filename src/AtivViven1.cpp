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

const GLuint WIDTH = 1000, HEIGHT = 1000;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
GLuint setupShader();
bool carregarOBJ(const string& caminho, vector<float>& vertices);
GLuint configurarVAO(vector<float>& vertices);

struct Objeto3D
{
    string nome;
    GLuint VAO;
    vector<float> vertices;

    glm::vec3 posicao;
    glm::vec3 rotacao;
    glm::vec3 escala;
    glm::vec3 cor;
};

vector<Objeto3D> objetos;
int objetoSelecionado = 0;

enum ModoTransformacao
{
    ROTACAO,
    TRANSLACAO,
    ESCALA
};

ModoTransformacao modoAtual = ROTACAO;
int eixoAtual = 1; // 0 = X, 1 = Y, 2 = Z

const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(position, 1.0);\n"
"   finalColor = vec4(color, 1.0);\n"
"}\0";

const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"   color = finalColor;\n"
"}\n\0";

int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Atividade Vivencial 1 - OBJ 3D", nullptr, nullptr);
    if (!window)
    {
        cout << "Erro ao criar janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Erro ao inicializar GLAD" << endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);

    GLuint shaderID = setupShader();

    // ===== OBJETO 1 =====
    Objeto3D obj1;
    obj1.nome = "Objeto 1";
    obj1.posicao = glm::vec3(-1.2f, 0.0f, 0.0f);
    obj1.rotacao = glm::vec3(0.0f);
    obj1.escala = glm::vec3(1.0f);
    obj1.cor = glm::vec3(0.2f, 0.5f, 1.0f);

    if (!carregarOBJ("../assets/Modelos3D/Suzanne.obj", obj1.vertices))    {
        cout << "Nao foi possivel carregar Suzanne.obj" << endl;
        return -1;
    }

    for (int i = 0; i < obj1.vertices.size(); i += 6)
    {
        obj1.vertices[i + 3] = obj1.cor.r;
        obj1.vertices[i + 4] = obj1.cor.g;
        obj1.vertices[i + 5] = obj1.cor.b;
    }

    obj1.VAO = configurarVAO(obj1.vertices);
    objetos.push_back(obj1);

    // ===== OBJETO 2 =====
    Objeto3D obj2;
    obj2.nome = "Objeto 2";
    obj2.posicao = glm::vec3(1.2f, 0.0f, 0.0f);
    obj2.rotacao = glm::vec3(0.0f);
    obj2.escala = glm::vec3(1.0f);
    obj2.cor = glm::vec3(1.0f, 0.4f, 0.2f);

    if (!carregarOBJ("../assets/Modelos3D/Suzanne.obj", obj2.vertices))    {
        cout << "Nao foi possivel carregar Suzanne.obj" << endl;
        return -1;
    }

    for (int i = 0; i < obj2.vertices.size(); i += 6)
    {
        obj2.vertices[i + 3] = obj2.cor.r;
        obj2.vertices[i + 4] = obj2.cor.g;
        obj2.vertices[i + 5] = obj2.cor.b;
    }

    obj2.VAO = configurarVAO(obj2.vertices);
    objetos.push_back(obj2);

    glUseProgram(shaderID);

    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    GLint projectionLoc = glGetUniformLocation(shaderID, "projection");

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 1.5f, 5.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)WIDTH / (float)HEIGHT,
        0.1f,
        100.0f
    );

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    cout << "===== CONTROLES =====" << endl;
    cout << "TAB: selecionar proximo objeto" << endl;
    cout << "R: modo rotacao" << endl;
    cout << "T: modo translacao" << endl;
    cout << "S: modo escala" << endl;
    cout << "X/Y/Z: escolher eixo" << endl;
    cout << "W: aplicar positivo" << endl;
    cout << "Q: aplicar negativo" << endl;
    cout << "A/D: mover no eixo X" << endl;
    cout << "Setas: mover no eixo X/Y" << endl;
    cout << "+/-: escala uniforme" << endl;
    cout << "ESC: sair" << endl;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

            glBindVertexArray(obj.VAO);

            if (i == objetoSelecionado)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(3.0f);
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }

            glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size() / 6);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
    }

    for (Objeto3D& obj : objetos)
    {
        glDeleteVertexArrays(1, &obj.VAO);
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

    if (key == GLFW_KEY_TAB)
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

    if (key == GLFW_KEY_S)
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

    Objeto3D& obj = objetos[objetoSelecionado];

    float direcao = 0.0f;

    if (key == GLFW_KEY_W)
        direcao = 1.0f;

    if (key == GLFW_KEY_Q)
        direcao = -1.0f;

    if (direcao != 0.0f)
    {
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

    if (key == GLFW_KEY_LEFT)
        obj.posicao.x -= 0.1f;

    if (key == GLFW_KEY_RIGHT)
        obj.posicao.x += 0.1f;

    if (key == GLFW_KEY_UP)
        obj.posicao.y += 0.1f;

    if (key == GLFW_KEY_DOWN)
        obj.posicao.y -= 0.1f;

    if (key == GLFW_KEY_A)
        obj.posicao.x -= 0.1f;

    if (key == GLFW_KEY_D)
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

bool carregarOBJ(const string& caminho, vector<float>& vertices)
{
    ifstream arquivo(caminho);

    if (!arquivo.is_open())
    {
        cout << "Erro ao abrir arquivo OBJ: " << caminho << endl;
        return false;
    }

    vector<glm::vec3> posicoes;
    string linha;

    while (getline(arquivo, linha))
    {
        stringstream ss(linha);
        string tipo;
        ss >> tipo;

        if (tipo == "v")
        {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            posicoes.push_back(pos);
        }
        else if (tipo == "f")
        {
            vector<int> indices;
            string token;

            while (ss >> token)
            {
                size_t barra = token.find('/');
                string indiceStr;

                if (barra == string::npos)
                    indiceStr = token;
                else
                    indiceStr = token.substr(0, barra);

                int indice = stoi(indiceStr) - 1;
                indices.push_back(indice);
            }

            for (int i = 1; i < indices.size() - 1; i++)
            {
                int i1 = indices[0];
                int i2 = indices[i];
                int i3 = indices[i + 1];

                glm::vec3 p1 = posicoes[i1];
                glm::vec3 p2 = posicoes[i2];
                glm::vec3 p3 = posicoes[i3];

                glm::vec3 corPadrao = glm::vec3(1.0f, 1.0f, 1.0f);

                vertices.insert(vertices.end(), {
                    p1.x, p1.y, p1.z, corPadrao.r, corPadrao.g, corPadrao.b,
                    p2.x, p2.y, p2.z, corPadrao.r, corPadrao.g, corPadrao.b,
                    p3.x, p3.y, p3.z, corPadrao.r, corPadrao.g, corPadrao.b
                });
            }
        }
    }

    if (vertices.empty())
    {
        cout << "OBJ carregado, mas sem vertices validos." << endl;
        return false;
    }

    // Normaliza o modelo para caber melhor na cena
    glm::vec3 minP(vertices[0], vertices[1], vertices[2]);
    glm::vec3 maxP(vertices[0], vertices[1], vertices[2]);

    for (int i = 0; i < vertices.size(); i += 6)
    {
        glm::vec3 p(vertices[i], vertices[i + 1], vertices[i + 2]);
        minP = glm::min(minP, p);
        maxP = glm::max(maxP, p);
    }

    glm::vec3 centro = (minP + maxP) * 0.5f;
    glm::vec3 tamanho = maxP - minP;
    float maior = max(tamanho.x, max(tamanho.y, tamanho.z));

    if (maior == 0.0f)
        maior = 1.0f;

    for (int i = 0; i < vertices.size(); i += 6)
    {
        vertices[i] = (vertices[i] - centro.x) / maior;
        vertices[i + 1] = (vertices[i + 1] - centro.y) / maior;
        vertices[i + 2] = (vertices[i + 2] - centro.z) / maior;
    }

    cout << "OBJ carregado com sucesso: " << caminho << endl;
    cout << "Quantidade de vertices: " << vertices.size() / 6 << endl;

    return true;
}

GLuint configurarVAO(vector<float>& vertices)
{
    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
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