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

struct Vertex
{
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
};

struct Material
{
    glm::vec3 Ka = glm::vec3(0.2f, 0.2f, 0.2f); // ambiente
    glm::vec3 Kd = glm::vec3(0.8f, 0.8f, 0.8f); // difusa
    glm::vec3 Ks = glm::vec3(0.5f, 0.5f, 0.5f); // especular
    float Ns = 32.0f;                            // brilho especular

    string nomeTextura;
};

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

enum ModoTransformacao
{
    ROTACAO,
    TRANSLACAO,
    ESCALA
};

ModoTransformacao modoAtual = ROTACAO;
int eixoAtual = 1; // 0 = X, 1 = Y, 2 = Z

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

GLuint setupShader();
bool carregarOBJ(const string& caminhoOBJ, Objeto3D& objeto);
bool carregarMTL(const string& caminhoMTL, Material& material);
GLuint carregarTextura(const string& caminhoTextura);
void configurarBuffers(Objeto3D& objeto);
void normalizarModelo(Objeto3D& objeto);
void aplicarTransformacao(float direcao);
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
"\n"
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Desafio M4 - Modelo de Phong", nullptr, nullptr);

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

    glm::vec3 cameraPos = glm::vec3(0.0f, 1.5f, 5.0f);

    glm::mat4 view = glm::lookAt(
        cameraPos,
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

    glUniform1i(glGetUniformLocation(shaderID, "texture1"), 0);

    glUniform3f(lightPosLoc, 2.0f, 3.0f, 3.0f);
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

    imprimirControles();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderID);

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

            if (i == objetoSelecionado)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(3.0f);
                glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());

                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDrawArrays(GL_TRIANGLES, 0, obj.vertices.size());
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

    if (key == GLFW_KEY_W)
        aplicarTransformacao(1.0f);

    if (key == GLFW_KEY_Q)
        aplicarTransformacao(-1.0f);

    if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A)
        obj.posicao.x -= 0.1f;

    if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)
        obj.posicao.x += 0.1f;

    if (key == GLFW_KEY_UP)
        obj.posicao.y += 0.1f;

    if (key == GLFW_KEY_DOWN)
        obj.posicao.y -= 0.1f;

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

    if (objeto.textureID == 0)
    {
        cout << "Aviso: nenhuma textura foi carregada para o objeto." << endl;
    }

    normalizarModelo(objeto);

    cout << "OBJ carregado: " << caminhoOBJ << endl;
    cout << "Vertices: " << objeto.vertices.size() << endl;
    cout << "MTL: " << objeto.arquivoMTL << endl;
    cout << "Textura: " << objeto.material.nomeTextura << endl;
    cout << "Ka: " << objeto.material.Ka.x << ", " << objeto.material.Ka.y << ", " << objeto.material.Ka.z << endl;
    cout << "Kd: " << objeto.material.Kd.x << ", " << objeto.material.Kd.y << ", " << objeto.material.Kd.z << endl;
    cout << "Ks: " << objeto.material.Ks.x << ", " << objeto.material.Ks.y << ", " << objeto.material.Ks.z << endl;
    cout << "Ns: " << objeto.material.Ns << endl;

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
    cout << "TAB: selecionar proximo objeto" << endl;
    cout << "R: modo rotacao" << endl;
    cout << "T: modo translacao" << endl;
    cout << "S: modo escala" << endl;
    cout << "X/Y/Z: escolher eixo" << endl;
    cout << "W: aplicar positivo" << endl;
    cout << "Q: aplicar negativo" << endl;
    cout << "A/D: mover no eixo X" << endl;
    cout << "Setas: mover objeto" << endl;
    cout << "+/-: escala uniforme" << endl;
    cout << "ESC: sair" << endl;
}