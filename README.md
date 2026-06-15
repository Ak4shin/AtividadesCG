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
* Aplicação de transformações nos eixos X, Y e Z
* Destaque visual do objeto selecionado
* Uso de câmera 3D
* Uso de matrizes Model, View e Projection

* Transformações:
  * Rotação
  * Translação
  * Escala



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

# Atividade Vivencial 2

Utilizando o mesmo código do Desafio M4, foi implementado um sistema de iluminação de 3 pontos com luz principal, luz de preenchimento e luz de fundo.

## Iluminação de 3 Pontos

Nesta atividade foram implementados:
- três fontes de luz pontuais
- luz principal (Key Light)
- luz de preenchimento (Fill Light)
- luz de fundo (Back Light)
- atenuação da luz difusa pela distância
- posicionamento automático das luzes com base no objeto principal
- habilitar e desabilitar cada luz individualmente

## Teclas adicionadas

| Tecla | Função |
|--------|---------|
| 1 | Liga/desliga a luz principal |
| 2 | Liga/desliga a luz de preenchimento |
| 3 | Liga/desliga a luz de fundo |

# Desafio M5

Implementação de uma câmera em primeira pessoa utilizando OpenGL e GLM.

## Câmera

Nesta atividade foram implementados:
- classe Camera
- movimentação em primeira pessoa
- controle por teclado (WASD)
- rotação da câmera com mouse
- uso de yaw e pitch
- matriz View utilizando glm::lookAt
- movimentação independente da taxa de quadros (deltaTime)

## Controles

| Tecla | Função |
|--------|---------|
| W | Frente |
| S | Trás |
| A | Esquerda |
| D | Direita |
| Mouse | Rotaciona a câmera |
| ESC | Fecha o programa |

# Desafio M6

Utilizando o mesmo código do Desafio M5, foi implementado um sistema de trajetórias para os objetos da cena através de pontos de controle.

Trajetórias

Nesta atividade foram implementados:

criação de pontos de controle para os objetos
armazenamento dos pontos em uma lista
seleção do objeto que receberá a trajetória
movimentação automática entre os pontos
trajetória cíclica (retorno ao primeiro ponto após o último)
controle de início e pausa da trajetória
remoção dos pontos da trajetória

## Teclas adicionadas
| Tecla | Função |
|--------|---------|
| TAB	| Seleciona o próximo objeto |
| Setas	| Move o objeto selecionado |
| Page Up |	Move o objeto no eixo Z positivo |
| Page Down	| Move o objeto no eixo Z negativo |
| P	| Adiciona um ponto à trajetória |
| SPACE |	Inicia/pausa a trajetória |
| C	| Limpa os pontos da trajetória |
