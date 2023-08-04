#include "Reversi.h"

#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

int main()
{
    std::cout << "Powered by:\n";
    std::cout << Reversi::Info::DEPENDENCIES << '\n';
    std::cout << Reversi::Info::CREATOR << " - " << Reversi::Info::NAME << " v" << Reversi::Info::VERSION << "\n";
    std::cout << Reversi::Info::LICENSE << '\n';
    std::cout << "A file named 'ReversiEvolvingAI.dat' will be created in the working directory if not present, to store AI data.\n";
    std::cout << "The AI starts from scratch and will learn little by little.\n";
    std::cout << "You can make a backup of ReversiEvolvingAI.dat to save the state of the AI.\n";
#if REVERSI_DEBUG
    std::cout << "\nDEBUG MODE\n\n";
#endif

    std::shared_ptr<Reversi::Window> window(new Reversi::Window(std::string(Reversi::Info::NAME) + " v" + Reversi::Info::VERSION));
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
