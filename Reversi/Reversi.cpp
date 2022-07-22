#include "Reversi.h"

#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

int main()
{
    std::cout << "Reminimalism - Reversi\n";
    std::cout << Reversi::LICENSE;
    std::cout << "\nA file named 'ReversiEvolvingAI.dat' will be created in the working directory if not present, to store AI data.\n";
    std::cout << "The AI starts from scratch and will learn little by little.\n";
    std::cout << "You can make a backup of ReversiEvolvingAI.dat to save the state of the AI.\n";

    std::shared_ptr<Reversi::Window> window(new Reversi::Window());
    std::shared_ptr<Reversi::AI> ai(new Reversi::EvolvingAI("ReversiEvolvingAI.dat"));
    Reversi::Board board(window, ai);
    while (!window->ShouldClose())
    {
        window->Update();
        if (board.NeedsFrequentUpdate())
            board.Update();
        else
            std::this_thread::sleep_for(50ms);
    }
    return 0;
}
