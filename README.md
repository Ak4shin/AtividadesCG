
# Computação Gráfica - Híbrido

Repositório de exemplos de códigos em C++ utilizando OpenGL moderna (3.3+) criado para a Atividade Acadêmica Computação Gráfica do curso de graduação em Ciência da Computação - modalidade híbrida - da Unisinos. Ele é estruturado para facilitar a organização dos arquivos e a compilação dos projetos utilizando CMake.

## 📂 Estrutura do Repositório

```plaintext
📂 CGCCHibrido/
├── 📂 include/               # Cabeçalhos e bibliotecas de terceiros
│   ├── 📂 glad/              # Cabeçalhos da GLAD (OpenGL Loader)
│   │   ├── glad.h
│   │   ├── 📂 KHR/           # Diretório com cabeçalhos da Khronos (GLAD)
│   │       ├── khrplatform.h
├── 📂 common/                # Código reutilizável entre os projetos
│   ├── glad.c                # Implementação da GLAD
├── 📂 src/                   # Código-fonte dos exemplos e exercícios
│   ├── Hello3D.cpp           # Exemplo básico de renderização com OpenGL
│   ├── ...                   # Outros exemplos e exercícios futuros
├── 📂 build/                 # Diretório gerado pelo CMake (não incluído no repositório)
├── 📂 assets/                # diretório com modelos 3D, texturas, fontes etc
├── 📄 CMakeLists.txt         # Configuração do CMake para compilar os projetos
├── 📄 README.md              # Este arquivo, com a documentação do repositório
├── 📄 GettingStarted.md      # Tutorial detalhado sobre como compilar usando o CMake
```

Siga as instruções detalhadas em [GettingStarted.md](GettingStarted.md) para configurar e compilar o projeto.

## ⚠️ **IMPORTANTE: Baixar a GLAD Manualmente**
Para que o projeto funcione corretamente, é necessário **baixar a GLAD manualmente** utilizando o **GLAD Generator**.

### 🔗 **Acesse o web service do GLAD**:
👉 [GLAD Generator](https://glad.dav1d.de/)

### ⚙️ **Configuração necessária:**
- **API:** OpenGL  
- **Version:** 3.3+ (ou superior compatível com sua máquina)  
- **Profile:** Core  
- **Language:** C/C++  

### 📥 **Baixe e extraia os arquivos:**
Após a geração, extraia os arquivos baixados e coloque-os nos diretórios correspondentes:
- Copie **`glad.h`** para `include/glad/`
- Copie **`khrplatform.h`** para `include/glad/KHR/`
- Copie **`glad.c`** para `common/`

🚨 **Sem esses arquivos, a compilação falhará!** É necessário colocar esses arquivos nos diretórios corretos, conforme a orientação acima.

--- 


# Atividades de Computação Gráfica

Repositório contendo atividades desenvolvidas em C++ utilizando OpenGL moderna, GLFW, GLAD e GLM para a disciplina de Computação Gráfica.

# Desafio M2 - Cubo3D

Atividade desenvolvida para praticar conceitos básicos de geometria 3D utilizando OpenGL.

## Funcionalidades

* Renderização de um cubo 3D
* Uso de VAO e VBO
* Aplicação de transformações
* Rotação do objeto
* Uso de shaders
* Projeção 3D utilizando GLM

## Controles

| Tecla | Função              |
| ----- | ------------------- |
| X     | Rotaciona no eixo X |
| Y     | Rotaciona no eixo Y |
| Z     | Rotaciona no eixo Z |
| ESC   | Fecha o programa    |

# Atividade Vivencial 1

Atividade vivencial envolvendo leitura de modelos `.OBJ`, seleção de objetos e aplicação de transformações 3D.

## Funcionalidades

* Leitura de arquivos `.OBJ`
* Exibição de múltiplos objetos 3D
* Seleção de objetos via teclado
* Transformações:
* Rotação
* Translação
* Escala
* Aplicação de transformações nos eixos X, Y e Z
* Destaque visual do objeto selecionado
* Uso de câmera 3D
* Uso de matrizes Model, View e Projection

## Controles

| Tecla | Função                     |
| ----- | -------------------------- |
| TAB   | Seleciona o próximo objeto |
| R     | Modo rotação               |
| T     | Modo translação            |
| S     | Modo escala                |
| X/Y/Z | Seleciona eixo             |
| W     | Aumenta transformação      |
| Q     | Diminui transformação      |
| A/D   | Move lateralmente          |
| Setas | Move objeto                |
| + / - | Escala uniforme            |
| ESC   | Fecha o programa           |

# Desafio M3

Utilizando o mesmo código da atividade vivencial, adicionando suporte a texturas em modelos `.OBJ`.

## Texturas

Nesta atividade foram implementados:
- leitura de coordenadas de textura (`vt`)
- leitura de arquivos `.MTL`
- carregamento automático da textura através de `map_Kd`
- aplicação de textura nos objetos 3D
- uso de shaders com `sampler2D`

# Desafio M4

Utilizando o mesmo código do Desafio M3, foi adicionada iluminação em modelos `.OBJ` com o modelo de Phong.

## Iluminação

Nesta atividade foram implementados:
- leitura de vetores normais (`vn`)
- leitura dos coeficientes `Ka`, `Kd`, `Ks` e `Ns` do arquivo `.MTL`
- iluminação ambiente
- iluminação difusa
- iluminação especular
- modelo de iluminação de Phong
