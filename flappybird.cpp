#include <iostream>
#include <string>
using namespace std;

#include "olcConsoleGameEngine.h" // Inclui a biblioteca da engine de console

// Definição da classe que herda da olcConsoleGameEngine
class OneLoneCoder_FlappyBird : public olcConsoleGameEngine {
    public:
        // Construtor, define o nome do jogo
        OneLoneCoder_FlappyBird(){
            m_sAppName = L"Flappy Bird"; 
        }
    
    private:
        // Variáveis relacionadas ao comportamento do pássaro
        float fBirdPosition = 0.0f;     // Posição vertical do pássaro
        float fBirdVelocity = 0.0f;     // Velocidade do pássaro (para cima/baixo)
        float fBirdAcceleration = 0.0f; // Aceleração do pássaro (afetada pela gravidade)

        // Constante da gravidade que afeta o movimento do pássaro
        float fGravity = 100.0f; 
        
        // Largura de uma seção (um segmento de obstáculo) e a lista que representa os obstáculos
        float fSectionWidth; 
        list<int> listSection; // Armazena as posições dos obstáculos
        float fLevelPosition = 0.0f; // Posição horizontal do nível (scroll)

        // Variáveis de controle de colisão e reinício do jogo
        bool bHasCollided = false; 
        bool bResetGame = false; 
        
        // Contadores para tentativas e batidas de asas
        int nAttemptCount = 0;   // Contador de tentativas
        int nFlapCount = 0;      // Quantidade de vezes que o pássaro bateu asas
        int nMaxFlapCount = 0;   // Máximo número de batidas de asas em uma tentativa
	
    protected:
        // Método chamado quando o jogo é inicializado (sobrescreve método da engine)
        virtual bool OnUserCreate() {
            // Inicializa as seções dos obstáculos (4 seções iniciais)
            listSection = { 0, 0, 0, 0 };
            bResetGame = true; // Sinaliza que o jogo deve começar/resetar
            // Calcula a largura de uma seção com base no tamanho da tela
            fSectionWidth = (float)ScreenWidth() / (float)(listSection.size() - 1);
            return true;
        }
        
        // Método chamado a cada frame do jogo (sobrescreve método da engine)
        virtual bool OnUserUpdate(float fElapsedTime) {
            // Se o jogo está sendo reiniciado
            if (bResetGame) {
                // Reseta todas as variáveis para começar de novo
                bHasCollided = false;
                bResetGame = false;
                listSection = { 0, 0, 0, 0 }; // Zera as seções
                fBirdAcceleration = 0.0f;
                fBirdVelocity = 0.0f;
                fBirdPosition = ScreenHeight() / 2.0f; // Posição inicial do pássaro (meio da tela)
                nFlapCount = 0;
                nAttemptCount++; // Incrementa o número de tentativas
            }
            
            // Se houve colisão, aguarda o jogador reiniciar
            if (bHasCollided) {
                if (m_keys[VK_SPACE].bReleased)
                    bResetGame = true; // Reinicia quando a barra de espaço for solta
            }
            else {
                // Lógica de movimentação do pássaro
                if (m_keys[VK_SPACE].bPressed && fBirdVelocity >= fGravity / 10.0f) {
                    fBirdAcceleration = 0.0f;
                    fBirdVelocity = -fGravity / 4.0f; // Bate asa (movimenta para cima)
                    nFlapCount++; // Conta uma batida de asa
                    if (nFlapCount > nMaxFlapCount)
                        nMaxFlapCount = nFlapCount; // Atualiza pontuação máxima se aplicável
                } else {
                    // Acelera o pássaro para baixo (gravidade)
                    fBirdAcceleration += fGravity * fElapsedTime;
                }
            
                if (fBirdAcceleration >= fGravity)
                    fBirdAcceleration = fGravity; // Limita a aceleração pela gravidade
            
                fBirdVelocity += fBirdAcceleration * fElapsedTime; // Atualiza velocidade
                fBirdPosition += fBirdVelocity * fElapsedTime; // Atualiza a posição
                fLevelPosition += 14.0f * fElapsedTime; // Move o nível para a esquerda
            
                // Gera novas seções de obstáculos quando o nível avança
                if (fLevelPosition > fSectionWidth) {
                    fLevelPosition -= fSectionWidth; // Move a posição do nível
                    listSection.pop_front(); // Remove a seção mais à esquerda
                    int i = rand() % (ScreenHeight() - 20); // Gera um novo obstáculo
                    if (i <= 10) i = 0; // Define o tamanho mínimo dos obstáculos
                    listSection.push_back(i); // Adiciona nova seção
                }
            
                // Limpa a tela
                Fill(0, 0, ScreenWidth(), ScreenHeight(), L' ');
            
                // Desenha os obstáculos
                int nSection = 0;
                for (auto s : listSection) {
                    if (s != 0) {
                        // Desenha a parte inferior do obstáculo
                        Fill(nSection * fSectionWidth + 10 - fLevelPosition, ScreenHeight() - s, nSection * fSectionWidth + 15 - fLevelPosition, ScreenHeight(), PIXEL_SOLID, FG_GREEN);
                        // Desenha a parte superior do obstáculo
                        Fill(nSection * fSectionWidth + 10 - fLevelPosition, 0, nSection * fSectionWidth + 15 - fLevelPosition, ScreenHeight() - s - 15, PIXEL_SOLID, FG_GREEN);
                    }
                    nSection++; // Próxima seção
                }
            
                // Posição horizontal do pássaro (estático horizontalmente)
                int nBirdX = (int)(ScreenWidth() / 3.0f);
            
                // Detecção de colisão (com o chão, teto ou obstáculos)
                bHasCollided = fBirdPosition < 2 || fBirdPosition > ScreenHeight() - 2 || // Checa se o pássaro saiu da tela
                    m_bufScreen[(int)(fBirdPosition + 0) * ScreenWidth() + nBirdX].Char.UnicodeChar != L' ' || // Colisão parte superior
                    m_bufScreen[(int)(fBirdPosition + 1) * ScreenWidth() + nBirdX].Char.UnicodeChar != L' ' || // Colisão parte inferior
                    m_bufScreen[(int)(fBirdPosition + 0) * ScreenWidth() + nBirdX + 6].Char.UnicodeChar != L' ' || // Colisão parte traseira
                    m_bufScreen[(int)(fBirdPosition + 1) * ScreenWidth() + nBirdX + 6].Char.UnicodeChar != L' '; // Colisão parte frontal
            
                // Desenha o pássaro na tela
                if (fBirdVelocity > 0) { // Se o pássaro está descendo
                    DrawString(nBirdX, fBirdPosition + 0, L"\\\\\\");    // Parte superior
                    DrawString(nBirdX, fBirdPosition + 1, L"<\\\\\\=Q"); // Parte inferior
                } else { // Se o pássaro está subindo
                    DrawString(nBirdX, fBirdPosition + 0, L"<///=Q");
                    DrawString(nBirdX, fBirdPosition + 1, L"///");
                }
            
                // Exibe a pontuação na tela
                DrawString(1, 1, L"Tentativa: " + to_wstring(nAttemptCount) + L" Pontuação: " + to_wstring(nFlapCount) + L" Pontuação Máxima: " + to_wstring(nMaxFlapCount));
            }
            
            return true; // Continua o jogo
        }
};

// Função principal do jogo
int main() {
    OneLoneCoder_FlappyBird game; // Instancia o jogo
    game.ConstructConsole(80, 48, 16, 16); // Define as dimensões da janela do console
    game.Start(); // Inicia o jogo
    
    return 0;
}
