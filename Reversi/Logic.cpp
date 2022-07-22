#include "Logic.h"

namespace Reversi
{
    Logic::Change::Change(int X, int Y, Side OldState, Side NewState)
        : X(X), Y(Y), OldState(OldState), NewState(NewState)
    {}

    Logic::Move::Move(Side Turn, std::vector<Change> Changes)
        : Turn(Turn), Changes(Changes)
    {}

    Logic::Logic()
    {
        Reset();
    }

    void Logic::Reset()
    {
        for (int i = 0; i < 64; i++)
            Slots[i] = Side::None;
        Set(3, 3, Side::Black);
        Set(4, 4, Side::Black);
        Set(4, 3, Side::White);
        Set(3, 4, Side::White);
        CurrentTurn = Side::Black;
        History.clear();
        Future.clear();
        GameOver = false;
    }

    Side Logic::GetCurrentTurn() const
    {
        return CurrentTurn;
    }

    bool Logic::CanMakeMove(int x, int y) const
    {
        return CanMakeMove(x, y, CurrentTurn);
    }

    Logic::Move Logic::MakeMove(int x, int y)
    {
        std::vector<Logic::Change> changes;
        if (GameOver || Get(x, y) != Side::None)
            return Move(Side::None, changes);
        Side other_turn = CurrentTurn == Side::Black ? Side::White : Side::Black;
        for (int y_dir = -1; y_dir <= 1; y_dir++)
        {
            for (int x_dir = -1; x_dir <= 1; x_dir++)
            {
                if (y_dir != 0 || x_dir != 0)
                {
                    std::vector<Change> fraction;
                    int x_walk = x + x_dir;
                    int y_walk = y + y_dir;
                    while (Get(x_walk, y_walk) == other_turn) // Out of range => None side => false
                    {
                        fraction.push_back(Change(x_walk, y_walk, Get(x, y), CurrentTurn));
                        x_walk += x_dir;
                        y_walk += y_dir;
                    }
                    if (Get(x_walk, y_walk) == CurrentTurn // Out of range => None side => false
                        && fraction.size() != 0)
                    {
                        if (changes.size() == 0)
                            changes.push_back(Change(x, y, Get(x, y), CurrentTurn));
                        for (auto& change : fraction)
                            changes.push_back(change);
                    }
                }
            }
        }

        // Apply
        for (auto& change : changes)
            Set(change.X, change.Y, change.NewState);

        Move move(changes.size() != 0 ? CurrentTurn : Side::None, changes);

        if (changes.size() != 0)
        {
            History.push_back(move);
            Future.clear();

            ApplyNextTurn();
        }

        return move;
    }

    bool Logic::CanUndo() const
    {
        return !GameOver && History.size() != 0;
    }

    Logic::Move Logic::Undo()
    {
        if (GameOver || History.size() == 0)
            return Move(Side::None, std::vector<Change>());

        auto move = History.back();

        for (auto& change : move.Changes)
            Set(change.X, change.Y, change.OldState);
        CurrentTurn = move.Turn;

        History.pop_back();
        Future.push_back(move);

        return move;
    }

    bool Logic::CanRedo() const
    {
        return !GameOver && Future.size() != 0;
    }

    Logic::Move Logic::Redo()
    {
        if (GameOver || Future.size() == 0)
            return Move(Side::None, std::vector<Change>());

        auto move = Future.back();

        for (auto& change : move.Changes)
            Set(change.X, change.Y, change.NewState);
        CurrentTurn = move.Turn;
        ApplyNextTurn();

        Future.pop_back();
        History.push_back(move);

        return move;
    }

    Side Logic::Get(int x, int y) const
    {
        if (x < 0 || x >= 8 || y < 0 || y >= 8)
            return Side::None;
        return Slots[y << 3 | x];
    }

    bool Logic::IsGameOver() const
    {
        return GameOver;
    }

    Side Logic::GetWinner() const
    {
        int black_diff_to_white = 0;
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                auto side = Get(i, j);
                if (side == Side::Black)
                    black_diff_to_white++;
                else if (side == Side::White)
                    black_diff_to_white--;
            }
        }
        if (black_diff_to_white == 0)
            return Side::None;
        if (black_diff_to_white > 0)
            return Side::Black;
        return Side::White;
    }

    std::list<Logic::Move> Logic::GetHistory() const
    {
        return History;
    }

    void Logic::Set(int x, int y, Side side)
    {
        if (x < 0 || x >= 8 || y < 0 || y >= 8)
            return;
        Slots[y << 3 | x] = side;
    }

    bool Logic::CanMakeMove(int x, int y, Side turn) const
    {
        if (GameOver || Get(x, y) != Side::None || turn == Side::None)
            return false;
        Side other_turn = turn == Side::Black ? Side::White : Side::Black;
        for (int y_dir = -1; y_dir <= 1; y_dir++)
        {
            for (int x_dir = -1; x_dir <= 1; x_dir++)
            {
                if (y_dir != 0 || x_dir != 0)
                {
                    int count = 0;
                    int x_walk = x + x_dir;
                    int y_walk = y + y_dir;
                    while (Get(x_walk, y_walk) == other_turn) // Out of range => None side => false
                    {
                        count++;
                        x_walk += x_dir;
                        y_walk += y_dir;
                    }
                    if (Get(x_walk, y_walk) == turn // Out of range => None side => false
                        && count != 0)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void Logic::ApplyNextTurn()
    {
        if (CurrentTurn == Side::None)
            return;
        Side other_turn = CurrentTurn == Side::Black ? Side::White : Side::Black;
        bool can_make_move = false;
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (CanMakeMove(i, j, other_turn))
                {
                    can_make_move = true;
                    break;
                }
            }
            if (can_make_move)
                break;
        }
        if (can_make_move)
            CurrentTurn = other_turn;
        else
        {
            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    if (CanMakeMove(i, j, CurrentTurn))
                    {
                        can_make_move = true;
                        break;
                    }
                }
                if (can_make_move)
                    break;
            }
            if (!can_make_move)
            {
                CurrentTurn = Side::None;
                GameOver = true;
            }
        }
    }
}
