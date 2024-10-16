#include <stdio.h>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

// Definição das cores usadas no jogo
Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};

// Tamanho da célula e quantidade de células no grid
const int cellSize = 30;
const int cellCount = 25;
const int offset = 75;

// Variável para controlar o tempo da última atualização
double lastUpdateTime = 0.0;

// Função para verificar se um elemento está na deque
bool ElementInDeque(Vector2 element, const deque<Vector2>& dq){
    for(const auto& item : dq){
        if(Vector2Equals(item, element))
            return true;
    }
    return false;
}

// Função para verificar se um evento foi acionado com base no intervalo de tempo
bool eventTriggered(double interval){
    double currentTime = GetTime();
    if(currentTime - lastUpdateTime >= interval){
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

// Classe representando a cobra
class Snake{
public:
    deque<Vector2> body = { Vector2{6,9}, Vector2{5,9}, Vector2{4,9} };
    Vector2 direction = {1, 0}; // Direção inicial: direita
    bool addSegment = false;

    // Função para desenhar a cobra na tela
    void Draw(){
        for(const auto& segmentPos : body){
            float x = segmentPos.x;
            float y = segmentPos.y;
            Rectangle segment = Rectangle{ offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize };
            DrawRectangleRounded(segment, 0.5f, 6, darkGreen);
        }
    }

    // Função para atualizar a posição da cobra
    void Update(){
        // Calcula a nova cabeça com base na direção
        Vector2 newHead = Vector2Add(body.front(), direction);
        body.push_front(newHead);
        
        if (addSegment){
            // Se for para adicionar um segmento, não remove a cauda
            addSegment = false;
        }
        else {
            // Remove a cauda para manter o tamanho
            body.pop_back();
        }
    }

    // Função para resetar a cobra para o estado inicial
    void Reset(){
        body = { Vector2{6,9}, Vector2{5,9}, Vector2{4,9} };
        direction = {1, 0};
    }
};

// Classe representando a comida
class Food{
public:
    Vector2 position;
    Texture2D texture;

    // Construtor que carrega a textura da comida e gera uma posição inicial
    Food(const deque<Vector2>& snakeBody){
        Image image = LoadImage("Graphics/food.png");
        if (image.data == NULL){
            // Se a imagem não for carregada, usa um retângulo simples vermelho
            UnloadImage(image);
            texture = LoadTexture(GenImageColor(cellSize, cellSize, RED));
        }
        else{
            texture = LoadTextureFromImage(image);
            UnloadImage(image);
        }
        position = GenerateRandomPos(snakeBody);
    }

    // Destrutor que descarrega a textura
    ~Food(){
        UnloadTexture(texture);
    }

    // Função para desenhar a comida na tela
    void Draw(){
        DrawTexture(texture, offset + (int)(position.x * cellSize), offset + (int)(position.y * cellSize), WHITE);
    }

    // Gera uma célula aleatória dentro do grid
    Vector2 GenerateRandomCell(){
        float x = (float)GetRandomValue(0, cellCount - 1);
        float y = (float)GetRandomValue(0, cellCount - 1);
        return Vector2{ x, y };
    }

    // Gera uma posição aleatória que não está ocupada pela cobra
    Vector2 GenerateRandomPos(const deque<Vector2>& snakeBody){
        Vector2 pos = GenerateRandomCell();
        while(ElementInDeque(pos, snakeBody)){
            pos = GenerateRandomCell();
        }
        return pos;
    }
};

// Classe representando o jogo
class Game{
public:
    Snake snake;
    Food food;
    bool running;
    int score;

    // Construtor
    Game() : snake(), food(snake.body), running(true), score(0) {}

    // Função para desenhar os elementos do jogo
    void Draw(){
        food.Draw();
        snake.Draw();
    }

    // Função para atualizar o estado do jogo
    void Update(){
        if(running){
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
    }

    // Verifica colisão com a comida
    void CheckCollisionWithFood(){
        if(Vector2Equals(snake.body.front(), food.position)){
            snake.addSegment = true;
            score++;
            // Reposiciona a comida
            food.position = food.GenerateRandomPos(snake.body);
        }
    }

    // Verifica colisão com as bordas do grid
    void CheckCollisionWithEdges(){
        Vector2 head = snake.body.front();
        if(head.x >= cellCount || head.x < 0 || head.y >= cellCount || head.y < 0){
            GameOver();
        }
    }

    // Verifica colisão com o próprio corpo da cobra
    void CheckCollisionWithTail(){
        if(snake.body.size() < 2)
            return; // Sem colisão possível

        Vector2 head = snake.body.front();
        for(auto it = snake.body.begin() + 1; it != snake.body.end(); ++it){
            if(Vector2Equals(head, *it)){
                GameOver();
                break;
            }
        }
    }

    // Função para tratar o fim do jogo
    void GameOver(){
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
    }
};

int main(){
    printf("Iniciando o Jogo!...");

    // Inicializa a janela do jogo
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60);

    // Cria uma instância do jogo
    Game game;

    // Loop principal do jogo
    while(!WindowShouldClose()){
        // Atualiza a direção da cobra com base nas teclas pressionadas
        if(IsKeyPressed(KEY_UP) && game.snake.direction.y != 1){
            game.snake.direction = {0, -1};
            game.running = true;
        }
        
        if(IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1){
            game.snake.direction = {0, 1};
            game.running = true;
        }
        
        if(IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1){
            game.snake.direction = {-1, 0};
            game.running = true;
        }
        
        if(IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1){
            game.snake.direction = {1, 0};
            game.running = true;
        }

        // Atualiza o estado do jogo a cada intervalo de tempo
        if(eventTriggered(0.2)){
            game.Update();
        }

        BeginDrawing();
        
        // Limpa o fundo
        ClearBackground(green);

        // Desenha a borda do grid
        DrawRectangleLinesEx(Rectangle{ (float)offset - 5, (float)offset -5, (float)(cellSize * cellCount) + 10, (float)(cellSize * cellCount) + 10 }, 5, darkGreen);

        // Desenha o título e a pontuação
        DrawText("Cobrazuda", offset -5, 20, 40, darkGreen);
        DrawText(TextFormat("Pontuação: %i", game.score), offset -5, offset + cellSize * cellCount + 10, 40, darkGreen);

        // Desenha os elementos do jogo
        game.Draw();

        EndDrawing();
    }

    // Fecha a janela
    CloseWindow();
    return 0;
}
