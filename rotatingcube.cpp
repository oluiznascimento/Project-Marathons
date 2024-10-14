#include <SDL2/SDL.h>
#include <vector>
#include <cmath>

// Classe responsável por gerenciar a tela, renderização e entrada do usuário
class Screen{
    SDL_Event e; // Evento para capturar a entrada do usuário
    SDL_Window* window; // Janela SDL
    SDL_Renderer* renderer; // Renderizador SDL
    std::vector<SDL_FPoint> points; // Vetor de pontos para desenhar na tela

public:
    // Construtor que inicializa a janela e o renderizador
    Screen(){
        SDL_Init(SDL_INIT_VIDEO); // Inicializa o subsistema de vídeo do SDL
        SDL_CreateWindowAndRenderer(640*2,480*2,0,&window,&renderer); // Cria uma janela de 1280x960
        SDL_RenderSetScale(renderer,2,2); // Ajusta a escala de renderização para 2x
    }

    // Função para adicionar um ponto a ser desenhado
    void pixel(float x,float y){
        points.emplace_back(x,y); // Adiciona o ponto ao vetor de pontos
    }

    // Função para mostrar os pontos na tela
    void show(){
        SDL_SetRenderDrawColor(renderer,0,0,0,255); // Define a cor de fundo (preto)
        SDL_RenderClear(renderer); // Limpa a tela

        SDL_SetRenderDrawColor(renderer,255,255,255,255); // Define a cor de renderização (branco)
        for(auto& point : points){
            SDL_RenderDrawPointF(renderer,point.x, point.y); // Desenha cada ponto na tela
        }
        SDL_RenderPresent(renderer); // Atualiza a tela
    }

    // Função para limpar o vetor de pontos
    void clear(){
        points.clear(); // Limpa todos os pontos desenhados
    }

    // Função para capturar eventos de entrada do usuário
    void input(){
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT){
                SDL_Quit(); // Fecha a janela se o evento de sair for detectado
                exit(0); // Termina o programa
            }
        }
    }
};

// Estrutura que representa um ponto tridimensional
struct vec3{
    float x,y,z;
};

// Estrutura que representa uma conexão entre dois pontos
struct connection{
    int a,b; // Índices dos dois pontos conectados
};

// Função para rotacionar um ponto em torno dos eixos X, Y e Z
void rotate(vec3& point, float x = 1, float y= 1, float z = 1){
    float rad = 0;

    // Rotação em torno do eixo X
    rad = x;
    point.y = std::cos(rad) * point.y - std::sin(rad) * point.z;
    point.z = std::sin(rad) * point.y + std::cos(rad) * point.z;

    // Rotação em torno do eixo Y
    rad = y;
    point.x = std::cos(rad) * point.x - std::sin(rad) * point.z;
    point.z = std::sin(rad) * point.x + std::cos(rad) * point.z;

    // Rotação em torno do eixo Z
    rad = z;
    point.y = std::cos(rad) * point.x- std::sin(rad) * point.y;
    point.x = std::sin(rad) * point.x + std::cos(rad) * point.y;
}

// Função para desenhar uma linha de um ponto a outro
void line(Screen& screen, float x1, float y1, float x2, float y2){
    float dx = x2 - x1;
    float dy = y2 - y1;

    float length = std::sqrt(dx * dx + dy * dy); // Calcula o comprimento da linha
    float angle = std::atan2(dy,dx); // Calcula o ângulo da linha

    // Desenha a linha pixel a pixel
    for(float i = 0; i < length; i++){
        screen.pixel(x1 + std::cos(angle) * i, y1 + std::sin(angle) * i);
    }
}

int main(){
    Screen screen; // Instancia a tela

    // Vetor de pontos tridimensionais (um cubo)
    std::vector<vec3> points {
        {100,100,100},
        {200,100,100},
        {200,200,100},
        {100,200,100},

        {100,100,200},
        {200,100,200},
        {200,200,200},
        {100,200,200}
    };

    // Vetor de conexões entre os pontos para formar arestas
    std::vector<connection> connections{
        {0,4}, {1,5}, {2,6}, {3,7}, // Conexões verticais
        {0,1}, {1,2}, {2,3}, {3,0}, // Conexões da face frontal
        {4,5}, {5,6}, {6,7}, {7,4}  // Conexões da face traseira
    }; 

    // Calcula o centro de massa dos pontos
    vec3 c{0,0,0};
    for(auto& p : points){
        c.x += p.x;
        c.y += p.y;
        c.z += p.z;
    }
    c.x /= points.size();
    c.y /= points.size();
    c.z /= points.size();

    // Loop principal de renderização
    while(true){
        // Rotaciona cada ponto em torno do centro
        for(auto& p : points){
            p.x -= c.x;
            p.y -= c.y;
            p.z -= c.z;
            rotate(p, 0.002, 0.001, 0.004);
            p.x += c.x;
            p.y += c.y;
            p.z += c.z;
        }

        // Desenha cada ponto
        for(auto& p : points){
            screen.pixel(p.x, p.y);
        }

        // Desenha as linhas entre os pontos conectados
        for(auto& conn : connections){
            line(screen, points[conn.a].x, points[conn.a].y, points[conn.b].x, points[conn.b].y);
        }

        // Mostra o resultado na tela, limpa e processa a entrada do usuário
        screen.show();
        screen.clear();
        screen.input();

        SDL_Delay(30); // Pequeno atraso para limitar a taxa de frames
    }

    return 0;
}
