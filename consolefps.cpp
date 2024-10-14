#include <cmath>       // Biblioteca para funções matemáticas como sin, cos, etc.
#include <iostream>    // Biblioteca padrão para entrada/saída de dados (como cout).
#include <chrono>      // Biblioteca para manipulação de tempo e medir duração.
using namespace std;

#include <Windows.h>   // Biblioteca específica do Windows para manipular o console.

int nScreenWidth = 120;  // Largura da tela do console em caracteres.
int nScreenHeight = 40;  // Altura da tela do console em caracteres.

float fPlayerX = 8.0f;   // Posição inicial do jogador no eixo X.
float fPlayerY = 8.0f;   // Posição inicial do jogador no eixo Y.
float fPlayerA = 0.0f;   // Ângulo de direção do jogador (onde ele está olhando).

int nMapHeight = 16;     // Altura do mapa em "blocos".
int nMapWidth = 16;      // Largura do mapa em "blocos".

float fFOV = 3.14159 / 4.0;  // Campo de visão (FOV) do jogador (em radianos).
float fDepth = 16.0f;        // Profundidade máxima que o jogador pode ver (distância).

int main () {
    
    // Cria uma tela de buffer onde o conteúdo será desenhado (com base em largura e altura da tela).
    wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
    // Cria um buffer de tela do console para poder imprimir os caracteres.
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);  // Define o buffer como a tela ativa.
    DWORD dwBytesWritten = 0;  // Variável para armazenar bytes escritos.

    // Criação do mapa com paredes (representadas por '#') e espaços livres ('.').
    wstring map;
    map += L"################";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";
    
    // Inicializa marcadores de tempo para calcular o tempo entre cada frame (para o movimento suave).
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();    
    
    // Loop principal do jogo.
    while(1)
    {
        // Calcula o tempo passado entre frames.
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();
        
        // Verifica se a tecla 'A' está pressionada e rotaciona o jogador para a esquerda.
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
            fPlayerA -= (0.8f) * fElapsedTime;
        }
        
        // Verifica se a tecla 'D' está pressionada e rotaciona o jogador para a direita.
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
            fPlayerA += (0.8f) * fElapsedTime;
        }
        
        // Verifica se a tecla 'W' está pressionada e move o jogador para frente.
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;  // Movimento no eixo X
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;  // Movimento no eixo Y
            
            // Verifica se o jogador colidiu com uma parede. Se sim, desfaz o movimento.
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }
        
        // Verifica se a tecla 'S' está pressionada e move o jogador para trás.
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            
            // Verifica se o jogador colidiu com uma parede. Se sim, desfaz o movimento.
            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }
        
        // Renderização de "raios" para determinar a profundidade da parede.
        for(int x = 0; x < nScreenWidth; x++) {
            // Calcula o ângulo do "raio" baseado no campo de visão (FOV).
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
            
            float fDistanceToWall = 0;  // Distância até a parede.
            bool bHitWall = false;      // Se o "raio" atingiu a parede.
            
            float fEyeX = sinf(fRayAngle);  // Direção no eixo X.
            float fEyeY = cosf(fRayAngle);  // Direção no eixo Y.
            
            // Calcula a distância do raio até a parede ou o final do mapa.
            while(!bHitWall && fDistanceToWall < fDepth) {
                fDistanceToWall += 0.1f;  // Incrementa a distância
                
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);
                
                // Se o raio ultrapassar os limites do mapa, assume que atingiu o "infinito".
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
                    bHitWall = true;
                    fDistanceToWall = fDepth;  
                }
                else {
                    // Verifica se o raio atingiu uma parede.
                    if (map[nTestY * nMapWidth + nTestX] == '#') {
                        bHitWall = true;
                    }
                }
            }
        }

        // Desenha o teto, parede e chão de acordo com a distância calculada (fDistanceToWall).
        int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceWall);
        int nFloor = nScreenHeight - nCeiling;
        
        short nShade = ' ';  // Variável para representar a "sombra" da parede com base na distância.
        
        // Define o tipo de sombra com base na distância até a parede.
        if (fDistanceToWall <= fDepth / 4.0f)           nShade = 0x2588; // Muito perto
        else if (fDistanceToWall < fDepth / 3.0f)       nShade = 0x2593;
        else if (fDistanceToWall < fDepth / 2.0f)       nShade = 0x2592;
        else if (fDistanceToWall < fDepth)              nShade = 0x2591;
        else                                            nShade = ' ';
        
        // Desenha o teto, parede e chão na tela com base na distância calculada.
        for (int y = 0; y < nScreenHeight; y++) {
            if (y < nCeiling) {
                screen[y * nScreenWidth + x] = ' ';  // Desenha o teto.
            }
            else if (y >= nCeiling && y <= nFloor) {
                screen[y * nScreenWidth + x] = nShade;  // Desenha a parede.
            }
            else {
                // Calcula o sombreamento do chão.
                float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                if (b < 0.25)           nShade = '#';
                else if (b < 0.5)       nShade = 'X';
                else if (b < 0.75)      nShade = '.';
                else if (b < 0.9)       nShade = '-';
                else                    nShade = ' ';
                screen[y * nScreenWidth + x] = ' ';
            }
        }
        
        // Atualiza a tela do console com o buffer de caracteres gerado.
        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, {0,0}, &dwBytesWritten);
    }
    return 0;
}
