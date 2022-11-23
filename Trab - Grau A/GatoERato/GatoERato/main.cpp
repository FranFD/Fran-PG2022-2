#include "Window.hpp"
#include "Render.hpp"
#include "Textures.hpp"
#include "Scene.hpp"
#include "Random.hpp"
#include "World.hpp"

#include "UINodes.hpp"
#include "LoopFrame.hpp"

#include <glm/ext.hpp>

int main(void)
{
    Window::Create(); // inicializa GLFW, cria a janela e inicializa a OpenGL
    Scene gameScene;

    GameGUI* GUI = new GameGUI(); // Menu do jogo
    LoopFrame frame; // Frame rate

    Random::Init(); // Gerado de numeros
    Render::Initialize(); // Inicializa o renderizador
    Textures::Load(); // Carrega as texturas

    gameScene.SetGUI(GUI); // Configura a scena do jogo
    gameScene.projection = glm::ortho(WORLD_LEFT, WORLD_RIGHT, WORLD_BOTTOM, WORLD_TOP); // Matrix de projeção

    frame.StartFrame();
    glClearColor(0.2f, 0.34f, 0.46f, 1.0f);
    while (!Window::ShouldClose())
    {
        float ds = frame.NextFrame();
        gameScene.Process(ds); // Atualiza entidades caso tenha alguma

        // Renderiza o game, entidades, menu, etc.
        Window::PreRender();
        gameScene.Render(ds);
        Window::PostRender();
    }

    // Desinicialização em ordem reversa a inicialização
    Textures::Free();
    Render::ShutDown();
    Random::Shutdown();
    
    Window::Destroy();

    exit(EXIT_SUCCESS);
}
