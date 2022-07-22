#pragma once

#include "Reversi.dec.h"

#include <list>
#include <vector>

namespace Reversi
{
    /// @brief Has board info and game rules.
    class Logic final
    {
    public:
        struct Change
        {
        public:
            Change(int X, int Y, Side OldState, Side NewState);
            int X;
            int Y;
            Side OldState;
            Side NewState;
        };
        struct Move
        {
        public:
            Move(Side Turn, std::vector<Change> Changes);
            Side Turn;
            std::vector<Change> Changes;
        };
        Logic();
        void Reset();
        Side GetCurrentTurn() const;
        bool CanMakeMove(int x, int y) const;
        /// @return The changes that have been made. Returns with Turn=Side::None if unsuccessful.
        Move MakeMove(int x, int y);
        bool CanUndo() const;
        /// @return The changes that have been reversed. Returns with Turn=Side::None if unsuccessful.
        Move Undo();
        bool CanRedo() const;
        /// @return The changes that have been made. Returns with Turn=Side::None if unsuccessful.
        Move Redo();
        Side Get(int x, int y) const;
        bool IsGameOver() const;
        /// @return What side wins. If IsGameOver() returns flase, still returns what side wins so far.
        ///         Side::None means draw.
        Side GetWinner() const;
        std::list<Move> GetHistory() const;
    private:
        void Set(int x, int y, Side);
        bool CanMakeMove(int x, int y, Side turn) const;
        void ApplyNextTurn();

        Side Slots[64];
        Side CurrentTurn;
        bool GameOver;
        std::list<Move> History;
        std::list<Move> Future;
    };
}
