#include "raylib.h" // Usado para compilar mídia em c;
#include <stdlib.h> // Geração de números aleatórios e manipulação de memória/processos;
#include <stdio.h> // Manipulação de entrada e saída padrão (impressão no console e leitura do teclado);
#include <time.h> // Inicialização do gerador de números aleatórios com base no tempo;

typedef enum  // Struct permite agrupar varios dados em um unico tipo de dado, uma variavel estilo matriz talvez?
{
    SCREEN_MAIN_MENU,
    SCREEN_POKEMON_SELECTION_PLAYER1,
    SCREEN_POKEMON_SELECTION_PLAYER2,
    SCREEN_SCENARIO_SELECTION,
    SCREEN_BATTLE,
    SCREEN_CREDITS
} GameScreen;

typedef struct 
{
    char name[20];
    Color color;
    Texture2D texture;
} Pokeball;

typedef struct 
{
    char name[20];
    Texture2D backgroundTexture;
} Scenario;

typedef struct 
{
    char name[20];
    Texture2D poke1Texture;   // Sprite parado P1 antes idleTexture;
    Texture2D poke2Texture; // Sprite de ataque P2 attackTexture;
    Texture2D hitTexture;    // Sprite recebendo ataque;
    Color color;
} Pokemon;

typedef enum 
{
    ACTION_IDLE,
    ACTION_ATTACK,
    ACTION_HIT
} PokemonAction;

typedef struct 
{
    Pokemon selectedPokemon;
    PokemonAction currentAction;
    int actionFrameCounter;  // Contador para sincronizar ações;
} PlayerState;

typedef struct 
{
    int hp;
    int maxHp;
    int attack;
    int defense;
} PokemonStats;

// Variáveis globais;
PokemonStats player1Stats = {100, 100, 20, 12}; // 12 - x = 75%
PokemonStats player2Stats = {100, 100, 20, 16}; // 16 - 100%
int currentPlayer = 1;
bool battleEnded = false;
char battleMessage [64] = "Jogador 1, escolha sua ação!";
char battleMessage1 [64] = "Pressione ENTER para finalizar a batalha!";
char battleMessage2 [64] = "Pressione ENTER para finalizar a batalha!";

bool isFlashingPlayer1 = false;
bool isFlashingPlayer2 = false;
int flashCounterPlayer1 = 0;
int flashCounterPlayer2 = 0;
const int flashDuration = 30; // Número de frames que o Pokémon piscará;

PlayerState player1State;
PlayerState player2State;

// Sons;
Sound attackSound;
Sound defendSound;
Sound victorySound;

// Funções utilitárias;
void DrawCenteredText(const char *text, int y, int fontSize, Color color, int screenWidth) 
{
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, (screenWidth - textWidth) / 2, y, fontSize, color);
}

void DrawCenteredTexture(Texture2D texture, int y, float scale, int screenWidth, int screenHeight) 
{
    float textureWidth = texture.width * scale;
    float textureHeight = texture.height * scale;
    DrawTextureEx(texture, (Vector2){(screenWidth - textureWidth) / 2, (screenHeight - textureHeight) / 2}, 0.0f, scale, WHITE);
}

void DrawScaledTexture(Texture2D texture, int screenWidth, int screenHeight) 
{
    float scaleX = (float)screenWidth / texture.width;
    float scaleY = (float)screenHeight / texture.height;
    float scale = scaleX < scaleY ? scaleX : scaleY;
    DrawTextureEx(texture, (Vector2){0, 0}, 0.0f, scale, WHITE);
}

// Controla os sprites e suas transições;
void UpdatePlayerAction(PlayerState *player, PokemonAction action) 
{
    player->currentAction = action;
    player->actionFrameCounter = 10; // Duração do sprite antes de voltar para o estado parado;
}

void HandlePlayerActions() 
{
    // Jogador 1;
    if (currentPlayer == 1) 
    {
        if (IsKeyPressed(KEY_A)) 
        {
            PlaySound(attackSound);
            UpdatePlayerAction(&player1State, ACTION_ATTACK);
            UpdatePlayerAction(&player2State, ACTION_HIT); // Player 2 recebe o ataque;
            int damage = player1Stats.attack - player2Stats.defense / 2;
            player2Stats.hp -= damage > 0 ? damage : 1;
        }
    }

    // Jogador 2;
    if (currentPlayer == 2) 
    {
        if (IsKeyPressed(KEY_J)) 
        {
            PlaySound(attackSound);
            UpdatePlayerAction(&player2State, ACTION_ATTACK);
            UpdatePlayerAction(&player1State, ACTION_HIT); // Player 1 recebe o ataque;
            int damage = player2Stats.attack - player1Stats.defense / 2;
            player1Stats.hp -= damage > 0 ? damage : 1;
        }
    }

    // Reduz o contador de frames para voltar ao estado parado;
    if (player1State.actionFrameCounter > 0) player1State.actionFrameCounter--;
    if (player1State.actionFrameCounter == 0) player1State.currentAction = ACTION_IDLE;

    if (player2State.actionFrameCounter > 0) player2State.actionFrameCounter--;
    if (player2State.actionFrameCounter == 0) player2State.currentAction = ACTION_IDLE;
}

// Função de partículas para ataques;
void DrawAttackEffect(int x, int y, Color color) 
{
    for (int i = 0; i < 50; i++) 
    {
        Vector2 position = {
            x + GetRandomValue(-20, 20),
            y + GetRandomValue(-20, 20)
        };
        DrawCircleV(position, GetRandomValue(2, 5), color);
    }
}

void FlashPokemonPlayer1() 
{
    if (isFlashingPlayer1) 
    {
        flashCounterPlayer1++;
        if (flashCounterPlayer1 % 10 < 5) 
        {
            // Aqui o Pokémon "some" (pisca);
            DrawTextureEx(player1State.selectedPokemon.poke1Texture, 
                            (Vector2){100, 400},  // Posição na diagonal inferior esquerda;
                            0.0f,                 // Sem rotação;
                            3.0f,                 // Escala (ajuste conforme necessário);
                            BLANK);               // Pisca;
        } else 
        {
            // Aqui o Pokémon reaparece
            DrawTextureEx(player1State.selectedPokemon.poke1Texture, 
                            (Vector2){100, 400},  // Posição na diagonal inferior esquerda;
                            0.0f,                 // Sem rotação;
                            3.0f,                 // Escala (ajuste conforme necessário);
                            WHITE);               // Cor;
        }

        if (flashCounterPlayer1 >= flashDuration) 
        {
            isFlashingPlayer1 = false;
            flashCounterPlayer1 = 0;
        }
    }
}

void FlashPokemonPlayer2() 
{
    if (isFlashingPlayer2) 
    {
        flashCounterPlayer2++;
        if (flashCounterPlayer2 % 10 < 5) 
        {
            // Aqui o Pokémon "some" (pisca);
            DrawTextureEx(player2State.selectedPokemon.poke2Texture, 
                            (Vector2){750, 100}, // Posição na diagonal superior direita;
                            0.0f,                 // Sem rotação;
                            3.0f,                 // Escala (ajuste conforme necessário);
                            BLANK);               // Pisca;
        } else 
        {
            // Aqui o Pokémon reaparece;
             DrawTextureEx(player2State.selectedPokemon.poke2Texture, 
                            (Vector2){750, 100}, // Posição na diagonal superior direita;
                            0.0f,                 // Sem rotação;
                            3.0f,                 // Escala (ajuste conforme necessário);
                            WHITE);               // Cor;
        }

        if (flashCounterPlayer2 >= flashDuration) 
        {
            isFlashingPlayer2 = false;
            flashCounterPlayer2 = 0;
        }
    }
}

void AtaqueJogador1() 
{
    isFlashingPlayer2 = true;
}

void AtaqueJogador2() 
{
    isFlashingPlayer1 = true;
}

// Inicia o main;
int main(void) 
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Pokemon Battle Game");
    InitAudioDevice();

    // Carregar sons;
    attackSound = LoadSound("attackSound.mp3"); // Caminho do áudio;
    defendSound = LoadSound("defendSound.mp3"); // Caminho do áudio;
    victorySound = LoadSound("victorySound.mp3"); // Caminho do áudio;

    // Música do menu principal;
    Music menuMusic = LoadMusicStream("PokemonMenuAudio.mp3"); // Caminho do áudio;
    PlayMusicStream(menuMusic);
    SetMusicVolume(menuMusic, 0.5f);

    // Música da batalha;
    Music battleMusic = LoadMusicStream("PokemonBattleAudio.mp3"); // Caminho do áudio;
    PlayMusicStream(battleMusic);
    SetMusicVolume(battleMusic, 0.5f);

    // Música do credito;
    Music creditMusic = LoadMusicStream("Katy Perry - Last Friday Night (T.G.I.F) [Lyrics].mp3"); // Caminho do áudio;
    PlayMusicStream(creditMusic);
    SetMusicVolume(creditMusic, 0.5f);

    // Tela inicial com GIF;
    int animFrames = 0;
    Image gifImage = LoadImageAnim("Gifs/GifInicial3.gif", &animFrames); // Caminho do GIF;
    Texture2D gifTexture = LoadTextureFromImage(gifImage);

    // Créditos com GIF;
    Image gifEnd = LoadImageAnim("Gifs/GifCredito.gif", &animFrames); // Caminho do GIF;
    Texture2D gifTex = LoadTextureFromImage(gifEnd);

    int currentFrame = 0;
    int frameCounter = 0;
    int frameDelay = 8;

    // Carregar cenários;
    Scenario scenarios[2] = 
    {
        {"< Cenário 1 >", LoadTexture("Imagens Png/Background1.png")}, // Caminho do cenário;
        {"< Cenário 2 >", LoadTexture("Imagens Png/Background2.png")} // Caminho do cenário;
    };

    // Carregar Pokébola;
    Pokeball pokeball[5] = 
    {
        {"Pikachu", YELLOW, LoadTexture("Imagens Png/PokeBall.png")},
        {"Blastoise", BLUE, LoadTexture("Imagens Png/GreatBall.png")},
        {"Charizard", ORANGE, LoadTexture("Imagens Png/UltraBall.png")},
        {"Venossauro", GREEN, LoadTexture("Imagens Png/FriendBall.png")},
        {"Aleatória", PURPLE, LoadTexture("Imagens Png/MasterBall.png")}
    };

    // Carregar Pokémon;
    Pokemon pokemons[5] = 
    {                              // Idle   P1                                  Attack  P2                                  Hit;
    {"Pikachu", LoadTexture("Sprites/Pikachu-back.png"), LoadTexture("Sprites/Pikachu-front.png"), LoadTexture("Sprites/Pikachu_Hit.png"), YELLOW},
    {"Blastoise", LoadTexture("Sprites/Blastoise-back.png"), LoadTexture("Sprites/Blastoise-front.png"), LoadTexture("Sprites/Blastoise_Hit.png"), BLUE},
    {"Charizard", LoadTexture("Sprites/Charizard-back.png"), LoadTexture("Sprites/Charizard-front.png"), LoadTexture("Sprites/Charizard_Hit.png"), ORANGE},
    {"Venossauro", LoadTexture("Sprites/Venusaur-back.png"), LoadTexture("Sprites/Venusaur-front.png"), LoadTexture("Sprites/Venossauro_Hit.png"), GREEN},
    {"Aleatória", LoadTexture("Sprites/Gengar-back.png"), LoadTexture("Sprites/Gengar-front.png"), LoadTexture("Sprites/Random_Hit.png"), PURPLE} // Gengar;
    };

    int currentScreen = SCREEN_MAIN_MENU;
    int selectedPokemon = 0;
    int selectedScenario = 0;
    int player = 1;
    bool isBlinking = false;
    int blinkCounter = 0;

    srand(time(NULL));
    SetTargetFPS(60); // Minimo pra jogo; <(+>+)/

    // Durante a seleção de Pokémon, atribua os Pokémon escolhidos aos jogadores.
    player1State.selectedPokemon = pokemons[selectedPokemon]; 
    player1State.currentAction = ACTION_IDLE;

    player2State.selectedPokemon = pokemons[selectedPokemon]; 
    player2State.currentAction = ACTION_IDLE;

    while (!WindowShouldClose()) // Inicio da tela do jogo;
    {
        UpdateMusicStream(menuMusic);

        blinkCounter++;
        if (blinkCounter >= 30) 
        {
            isBlinking = !isBlinking;
            blinkCounter = 0;
        }

        switch (currentScreen) 
        {
            case SCREEN_MAIN_MENU: 
            {
                frameCounter++;
                if (frameCounter >= frameDelay) 
                {
                    currentFrame++;
                    if (currentFrame >= animFrames) currentFrame = 0;

                    unsigned int nextFrameOffset = gifImage.width * gifImage.height * 4 * currentFrame;
                    UpdateTexture(gifTexture, ((unsigned char *)gifImage.data) + nextFrameOffset);
                    frameCounter = 0;
                }

                if (IsKeyPressed(KEY_ENTER)) 
                {
                    currentScreen = SCREEN_POKEMON_SELECTION_PLAYER1;
                }
                break;
            }

            case SCREEN_POKEMON_SELECTION_PLAYER1:
            case SCREEN_POKEMON_SELECTION_PLAYER2: 
            {
                if (IsKeyPressed(KEY_RIGHT)) selectedPokemon = (selectedPokemon + 1) % 5;
                if (IsKeyPressed(KEY_LEFT)) selectedPokemon = (selectedPokemon - 1 + 5) % 5;

                if (IsKeyPressed(KEY_ENTER)) 
                {
                 if (currentScreen == SCREEN_POKEMON_SELECTION_PLAYER1) 
                 {
                    player1State.selectedPokemon = pokemons[selectedPokemon];
                    currentScreen = SCREEN_POKEMON_SELECTION_PLAYER2;
                    player = 2;
                 } else 
                 {
                    player2State.selectedPokemon = pokemons[selectedPokemon];
                    currentScreen = SCREEN_SCENARIO_SELECTION;
                 }
                }
                break;
            }

            case SCREEN_SCENARIO_SELECTION: 
            {
                if (IsKeyPressed(KEY_RIGHT)) selectedScenario = (selectedScenario + 1) % 2;
                if (IsKeyPressed(KEY_LEFT)) selectedScenario = (selectedScenario - 1 + 2) % 2;

                if (IsKeyPressed(KEY_ENTER)) 
                {
                    currentScreen = SCREEN_BATTLE;
                }break;
            }
            case SCREEN_BATTLE:
            {
                if(IsKeyPressed(KEY_ENTER) && (player1Stats.hp <= 0 || player2Stats.hp <= 0))
                {
                    currentScreen = SCREEN_CREDITS;
                }break;
            }
            
            case SCREEN_CREDITS:
            { 
                frameCounter++;
                if (frameCounter >= frameDelay) 
                {
                    currentFrame++;
                    if (currentFrame >= animFrames) currentFrame = 0;

                    unsigned int nextFrameOffset = gifEnd.width * gifEnd.height * 4 * currentFrame;
                    UpdateTexture(gifTex, ((unsigned char *)gifEnd.data) + nextFrameOffset);
                    frameCounter = 0;
                }
             }break;

        }

        // Começa o desenho;
        BeginDrawing();
        ClearBackground(BLACK);

        switch (currentScreen) 
        {
            case SCREEN_MAIN_MENU: 
            {
                DrawScaledTexture(gifTexture, screenWidth, screenHeight);
                break;
            }

            case SCREEN_POKEMON_SELECTION_PLAYER1:
            case SCREEN_POKEMON_SELECTION_PLAYER2: 
            {
                DrawCenteredText(player == 1 ? "Jogador 1: Escolha seu Pokémon" : "Jogador 2: Escolha seu Pokémon", 50, 30, WHITE, screenWidth);

                const float iconSize = 128.0f;
                const float spacing = 50.0f;
                const float totalWidth = 5 * iconSize + 4 * spacing;
                const float startX = (screenWidth - totalWidth) / 2;
                const float startY = 250;

                for (int i = 0; i < 5; i++) 
                {
                    float posX = startX + i * (iconSize + spacing);
                    DrawTextureEx(
                        pokeball[i].texture,
                        (Vector2){posX, startY},
                        0.0f,
                        iconSize / pokeball[i].texture.width,
                        WHITE
                    );
                    DrawText(
                        pokeball[i].name,
                        posX + (iconSize / 2) - MeasureText(pokeball[i].name, 20) / 2,
                        startY + iconSize + 10,
                        20,
                        WHITE
                    );
                    if (i == selectedPokemon && isBlinking) {
                        DrawRectangleLines(posX - 5, startY - 5, iconSize + 10, iconSize + 10, WHITE);
                    }
                }
                break;
            }

            case SCREEN_SCENARIO_SELECTION: 
            {
                DrawCenteredText("Selecione um Cenário", 50, 30, WHITE, screenWidth);
                DrawScaledTexture(scenarios[selectedScenario].backgroundTexture, screenWidth, screenHeight);
                DrawCenteredText(scenarios[selectedScenario].name, screenHeight - 100, 30, WHITE, screenWidth);
                break;
            }

            case SCREEN_BATTLE: 
            {
                // Controla as musicas;
                StopMusicStream(menuMusic);
                UpdateMusicStream(battleMusic);

                // Desenhar o cenário em backgroundTexture;
                DrawTexture(scenarios[selectedScenario].backgroundTexture, 0, 0, WHITE);

                // Dentro do loop principal
                if (!isFlashingPlayer1) 
                {
                    DrawTextureEx(player1State.selectedPokemon.poke1Texture, 
                            (Vector2){100, 400},  // Posição na diagonal inferior esquerda;
                            0.0f,                 // Sem rotação;
                            3.0f,                 // Escala (ajuste conforme necessário);
                            WHITE);               // Cor;
                }
                FlashPokemonPlayer1();

                if (!isFlashingPlayer2) 
                {
                    DrawTextureEx(player2State.selectedPokemon.poke2Texture, 
                            (Vector2){750, 100}, // Posição na diagonal superior direita;
                            0.0f,                 // Sem rotação;
                            3.0f,                 // Escala (ajuste conforme necessário);
                            WHITE);               // Cor;
                }
                FlashPokemonPlayer2();

                // Atualizar ações dos jogadores
                HandlePlayerActions();

                 // Exibir o título da batalha;
                DrawCenteredText("Batalha Pokémon!", 50, 40, WHITE, screenWidth);

                // Mostrar barras de vida;
                DrawRectangle(50, 100, 300, 20, RED); // Barra total Jogador 1;
                DrawRectangle(50, 100, (player1Stats.hp * 300) / player1Stats.maxHp, 20, GREEN); // Barra de vida Jogador 1;

                DrawRectangle(screenWidth - 350, 100, 300, 20, RED); // Barra total Jogador 2;
                DrawRectangle(screenWidth - 350, 100, (player2Stats.hp * 300) / player2Stats.maxHp, 20, GREEN); // Barra de vida Jogador 2;

                // Exibir nomes e mensagens;
                DrawText("Jogador 1", 50, 70, 20, WHITE);
                DrawText("Jogador 2", screenWidth - 150, 70, 20, WHITE);
                DrawCenteredText(battleMessage, screenHeight - 150, 20, WHITE, screenWidth);
                //DrawText("Pressione A para atacar ou D para defender", 50, screenHeight - 200, 20, WHITE);
                //DrawText("Pressione J para atacar ou L para defender", screenWidth - 400, screenHeight - 200, 20, WHITE);

                // Lógica de ataque com efeitos p1;
                if (!battleEnded) 
                {
                    if (currentPlayer == 1 && IsKeyPressed(KEY_A)) 
                    {
                        PlaySound(attackSound);
                        DrawAttackEffect(screenWidth - 350, 100, YELLOW);
                        //player2Stats.hp -= player1Stats.attack;
                        AtaqueJogador1();
                    }
                }

                // Lógica de defesa com efeitos p1;
                if (!battleEnded) 
                {
                    if (currentPlayer == 1 && IsKeyPressed(KEY_D)) 
                    {
                        PlaySound(defendSound);
                        //DrawAttackEffect(screenWidth - 50, 100, YELLOW);
                        //player2Stats.hp -= player1Stats.defense;
                    }
                }

                // Lógica de ataque com efeitos p2;
                if (!battleEnded) 
                {
                    if (currentPlayer == 2 && IsKeyPressed(KEY_J)) 
                    {
                        PlaySound(attackSound);
                        DrawAttackEffect(screenWidth - (920), (115), YELLOW); // Tive que descobrir na mão pq não deu a matemática; (T-T)
                        //player1Stats.hp -= player2Stats.attack;
                        AtaqueJogador2();
                    }
                }

                // Lógica de defesa com efeitos p2;
                if (!battleEnded) 
                {
                    if (currentPlayer == 2 && IsKeyPressed(KEY_L)) 
                    {
                        PlaySound(defendSound);
                        //DrawAttackEffect(screenWidth - 350, 100, YELLOW);
                        //player1Stats.hp -= player2Stats.defense;
                    }
                }

                if (!battleEnded) 
                {
                    // Ações do jogador atual;
                    if (currentPlayer == 1) 
                    {
                        if (IsKeyPressed(KEY_A)) // Ataque;
                        {
                            int damage = player1Stats.attack - player2Stats.defense / 2; // Dano = P1A - P2D / 2;
                            player2Stats.hp -= damage > 0 ? damage : 1; // P2HP = P2HP - Dano;
                            snprintf(battleMessage, sizeof(battleMessage), "Jogador 1 atacou! Causou %d de dano.", damage);
                            currentPlayer = 2;
                        } else if (IsKeyPressed(KEY_D)) // Defesa;
                        {
                            player1Stats.defense += 12; // P1D = P1D + x;
                            snprintf(battleMessage, sizeof(battleMessage), "Jogador 1 se defendeu! Defesa aumentada.");
                            currentPlayer = 2;
                        }
                    } else if (currentPlayer == 2)
                    {
                        if (IsKeyPressed(KEY_J)) // Ataque;
                        {
                            int damage = player2Stats.attack - player1Stats.defense / 2;
                            player1Stats.hp -= damage > 0 ? damage : 1;
                            snprintf(battleMessage, sizeof(battleMessage), "Jogador 2 atacou! Causou %d de dano.", damage);
                            currentPlayer = 1;
                        } else if (IsKeyPressed(KEY_L)) // Defesa;
                        {
                            player2Stats.defense += 9;
                            snprintf(battleMessage, sizeof(battleMessage), "Jogador 2 se defendeu! Defesa aumentada.");
                            currentPlayer = 1;
                        }
                    }

                    // Verificar fim da batalha;
                    if (player1Stats.hp <= 0 || player2Stats.hp <= 0)
                    {
                        battleEnded = true;          
                        snprintf(battleMessage, sizeof(battleMessage), player1Stats.hp > 0 ? "Jogador 1 venceu!" : "Jogador 2 venceu!");
                        StopMusicStream(battleMusic);
                        PlaySound(victorySound); // Toca som da vitoria;
                    }
                }
            }break;

            case SCREEN_CREDITS: 
            {
                StopSound(victorySound);
                UpdateMusicStream(creditMusic);
                DrawScaledTexture(gifTex, screenWidth, screenHeight);
                break;
            } 
         
        }    
            
        EndDrawing(); // Termina o desenho;
    }

    // Descarrega todas as midias usadas;
    UnloadMusicStream(menuMusic);
    UnloadMusicStream(battleMusic);
    UnloadMusicStream(creditMusic);
    UnloadSound(attackSound);
    UnloadSound(defendSound);
    UnloadSound(victorySound);
    UnloadTexture(gifTexture);
    UnloadImage(gifImage);
    for (int i = 0; i < 5; i++) UnloadTexture(pokeball[i].texture);
    for (int i = 0; i < 2; i++) UnloadTexture(scenarios[i].backgroundTexture);
    for (int i = 0; i < 5; i++) UnloadTexture(pokemons[i].poke1Texture);
    for (int i = 0; i < 5; i++) UnloadTexture(pokemons[i].poke2Texture);
    for (int i = 0; i < 5; i++) UnloadTexture(pokemons[i].hitTexture);
    CloseWindow();

    return 0;
}