#include "AI.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <random>
#include <stdexcept>

namespace Reversi
{
    void AI::Learn(const Logic& game_over_state) {}

    DecisionTreeAI::DecisionTreeAI(int depth) : Depth(depth)
    {
    }

    constexpr static float MIN_SCORE = -100;
    constexpr static float MAX_SCORE = 100;

    std::optional<std::tuple<int, int>> DecisionTreeAI::Decide(const Logic& state)
    {
        std::optional<std::tuple<int, int>> result;
        if (state.GetCurrentTurn() == Side::None || state.IsGameOver())
            return result;
        int best_x = -1;
        int best_y = -1;
        float best_score = MIN_SCORE;
        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                if (state.CanMakeMove(x, y))
                {
                    auto new_state = state;
                    new_state.MakeMove(x, y);
                    float score = CalculateScore(new_state, state.GetCurrentTurn(), Depth, best_score, MAX_SCORE);
                    if (score > best_score)
                    {
                        best_x = x;
                        best_y = y;
                        best_score = score;
                    }
                }
            }
        }
        if (best_x != -1)
        {
            result = std::make_tuple(best_x, best_y);
        }
        return result;
    }

    int DecisionTreeAI::GetDepth()
    {
        return Depth;
    }

    void DecisionTreeAI::SetDepth(int value)
    {
        Depth = value;
    }

    float DecisionTreeAI::CalculateScore(const Logic& state, Side side, int depth, float alpha, float beta)
    {
        bool should_maximize_score = state.GetCurrentTurn() == side;
        int count = 0;
        float score = should_maximize_score ? MIN_SCORE : MAX_SCORE;
        if (depth <= 0)
            return CalculateScoreTerminal(state, side);
        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                if (state.CanMakeMove(x, y))
                {
                    auto new_state = state;
                    new_state.MakeMove(x, y);
                    int local_score = CalculateScore(new_state, side, depth - 1, alpha, beta);
                    if (should_maximize_score && local_score > score)
                    {
                        score = local_score;
                        if (score >= beta)
                            return score;
                        if (score > alpha)
                            alpha = score;
                    }
                    else if (!should_maximize_score && local_score < score)
                    {
                        score = local_score;
                        if (score <= alpha)
                            return score;
                        if (score < beta)
                            beta = score;
                    }
                    count += 1;
                }
            }
        }
        if (count == 0)
            return CalculateScoreTerminal(state, side);
        return score;
    }

    float DecisionTreeAI::CalculateScoreTerminal(const Logic& state, Side side)
    {
        int win_points = 0;
        int lose_points = 0;
        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                Side disk_side = state.Get(x, y);
                if (disk_side == side)
                {
                    win_points++;
                }
                else if (disk_side != Side::None)
                {
                    lose_points++;
                }
            }
        }
        return (float)win_points / (float)(win_points + lose_points);
    }

    constexpr float EVOLVING_AI_MIN_SCORE = -100;

    EvolvingAI::EvolvingAI(std::string DataFilePath, float LearningRate, float Generalization)
        : DataFilePath(DataFilePath), LearningRate(LearningRate), Generalization(Generalization)
    {
        if (LearningRate < 0)
            LearningRate = 0;
        if (LearningRate > 1)
            LearningRate = 1;
        if (Generalization < 0)
            Generalization = 0;
        if (Generalization > 1)
            Generalization = 1;
        Load();
    }

    std::optional<std::tuple<int, int>> EvolvingAI::Decide(const Logic& state)
    {
        if (state.GetCurrentTurn() == Side::None || state.IsGameOver())
            return std::optional<std::tuple<int, int>>();
        std::vector<std::tuple<int, int>> best_moves;
        float best_score = EVOLVING_AI_MIN_SCORE;
        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                if (state.CanMakeMove(x, y))
                {
                    float score = 0;
                    for (auto features : GetFeatures(state, x, y))
                        score += GetScore(features);
                    if (score > best_score)
                    {
                        best_score = score;
                        best_moves.clear();
                        best_moves.push_back(std::make_tuple(x, y));
                    }
                    else if (score == best_score)
                    {
                        best_moves.push_back(std::make_tuple(x, y));
                    }
                }
            }
        }
        if (best_moves.size() == 0) // Robust code
            return std::optional<std::tuple<int, int>>();
        if (best_moves.size() == 1)
            return best_moves[0];
        std::random_device device;
        std::mt19937 mt(device());
        std::uniform_int_distribution<std::mt19937::result_type> dist(0, best_moves.size() - 1);
        int choice = dist(mt);
#if REVERSI_DEBUG
        Log("AI: Multiple best choices:", " ");
        for (auto item : best_moves)
            Log("(" + std::to_string(std::get<0>(item)) + ", " + std::to_string(std::get<1>(item)) + ")", " ");
        Log();
        Log(
            "AI: Chose: ("
            + std::to_string(std::get<0>(best_moves[choice]))
            + ", "
            + std::to_string(std::get<1>(best_moves[choice]))
            + ")"
        );
#endif
        return best_moves[choice];
    }

    constexpr float EVOLVING_AI_LEARNING_WIN_BASE_FEEDBACK = 0.25;
    constexpr float EVOLVING_AI_LEARNING_IMPACT_REDUCTION_COEFFICIENT = 0.125;

    constexpr int sign(int n) { return n == 0 ? 0 : (n < 0 ? -1 : 1); }

    void EvolvingAI::Learn(const Logic& game_over_state)
    {
        if (!game_over_state.IsGameOver() || LearningRate == 0)
            return;
        int black_count = 0;
        int white_count = 0;
        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                auto side = game_over_state.Get(x, y);
                if (side == Side::Black)
                    black_count++;
                else if (side == Side::White)
                    white_count++;
            }
        }
        float black_score = (float)black_count / (black_count + white_count); // [0, 1]
        float black_learning_feedback = black_score * 2 - 1; // [-1, 1]
        black_learning_feedback =
            black_learning_feedback * (1 - EVOLVING_AI_LEARNING_WIN_BASE_FEEDBACK)
            + (black_count == white_count ? 0
                : (black_count > white_count ?
                    EVOLVING_AI_LEARNING_WIN_BASE_FEEDBACK : -EVOLVING_AI_LEARNING_WIN_BASE_FEEDBACK));
        float white_learning_feedback = -black_learning_feedback;

        // Calculate move impacts by simulating the game again

        /// Simulation state
        Logic state;
        /// Move causing the impacts -> Impact location -> Impact (latest change and impact factor)
        std::map<Logic::Change, std::map<std::tuple<int, int>, std::shared_ptr<std::tuple<Logic::Change, float>>>> move_to_impacts;
        /// Impacts location -> Move causing the impact -> Impact (latest change and impact factor)
        std::map<std::tuple<int, int>, std::map<Logic::Change, std::shared_ptr<std::tuple<Logic::Change, float>>>> location_to_impacts;
        /// Features calculated from simulation states and stored for learning at the end.
        std::map<Logic::Change, std::vector<Features>> move_to_features;
        /// Adds impact, replacing if there's an old impact in place with lower factor.
        auto add_impact = [&](Logic::Change original_move, std::shared_ptr<std::tuple<Logic::Change, float>> new_impact)
        {
            auto location = std::make_tuple(get<0>(*new_impact).X, get<0>(*new_impact).Y);
            if (move_to_impacts[original_move].contains(location)
                    && get<1>(*move_to_impacts[original_move][location])
                        > get<1>(*new_impact))
                return;
            move_to_impacts[original_move][location] = new_impact;
            location_to_impacts[location][original_move] = new_impact;
        };
#if REVERSI_DEBUG
        // Learn debug info
        std::map<Logic::Change, std::tuple<Logic, Logic>> move_to_states;
        std::map<Logic::Change, float> move_to_feedback;
#endif
        for (auto move : game_over_state.GetHistory())
        {
            auto move_action = move.Changes[0];
            auto turn = state.GetCurrentTurn();
            if (turn == Side::None || turn != move_action.NewState)
                throw std::logic_error("Wrong game history.");
            for (auto change : move.Changes)
            {
                auto location = std::make_tuple(change.X, change.Y);
                // Add/update indirect impacts (root: the flipped disks)
                if (change.OldState != Side::None)
                {
                    for (auto existing_impact : location_to_impacts[location])
                    {
                        float new_impact_factor =
                            std::get<1>(*existing_impact.second)
                            * EVOLVING_AI_LEARNING_IMPACT_REDUCTION_COEFFICIENT;
                        // Update impact (where a disk is flipped)
                        std::get<0>(*existing_impact.second) = change;
                        std::get<1>(*existing_impact.second) = new_impact_factor;
                        // Add impact (where the disk is placed: move_action)
                        auto new_impact = std::make_shared<std::tuple<Logic::Change, float>>(move_action, new_impact_factor);
                        auto original_move = existing_impact.first;
                        add_impact(original_move, new_impact);
                    }
                }
            }
            for (auto end : move.Ends)
            {
                // Add indirect impacts (root: the unchanged end disks that caused flips)
                auto end_location = std::make_tuple(end.X, end.Y);
                for (auto change : move.Changes)
                {
                    auto change_location = std::make_tuple(change.X, change.Y);
                    if (change.OldState == Side::None || (
                            sign(change.X - move_action.X) == sign(end.X - move_action.X)
                            &&
                            sign(change.Y - move_action.Y) == sign(end.Y - move_action.Y)
                        ))
                    {
                        for (auto existing_impact : location_to_impacts[end_location])
                        {
                            float new_impact_factor =
                                std::get<1>(*existing_impact.second)
                                * EVOLVING_AI_LEARNING_IMPACT_REDUCTION_COEFFICIENT;
                            auto new_impact = std::make_shared<std::tuple<Logic::Change, float>>(change, new_impact_factor);
                            auto original_move = existing_impact.first;
                            add_impact(original_move, new_impact);
                        }
                    }
                }
            }
            for (auto change : move.Changes)
            {
                auto location = std::make_tuple(change.X, change.Y);
                // Add direct impacts (caused by the new move)
                auto impact = std::make_shared<std::tuple<Logic::Change, float>>(change, 1);
                move_to_impacts[move_action][location] = impact;
                location_to_impacts[location][move_action] = impact;
            }
            move_to_features[move_action] = GetFeatures(state, move_action.X, move_action.Y);
#if REVERSI_DEBUG
            // Learn debug info
            auto prev_state = state;
#endif
            state.MakeMove(move_action.X, move_action.Y);
#if REVERSI_DEBUG
            // Learn debug info
            move_to_states[move_action] = std::make_tuple(prev_state, state);
#endif
        }

        // Calculate raw impacts for learning
        std::map<Logic::Change, float> move_to_raw_impact;
        float max_raw_black_impact = 0.000001;
        float max_raw_white_impact = 0.000001;
        for (auto item : move_to_impacts)
        {
            std::map<std::tuple<int, int>, float> location_to_impact;
            for (auto impact : item.second)
            {
                auto impact_change = std::get<0>(*impact.second);
                float impact_number = std::get<1>(*impact.second);
                if (impact_change.NewState == game_over_state.GetWinner())
                {
                    auto location = std::make_tuple(impact_change.X, impact_change.Y);
                    if (location_to_impact.contains(location))
                    {
                        if (impact_number > location_to_impact[location])
                            location_to_impact[location] = impact_number;
                    }
                    else
                    {
                        location_to_impact[location] = impact_number;
                    }
                }
#if REVERSI_DEBUG
                // CHECK
                if (game_over_state.Get(impact_change.X, impact_change.Y) != impact_change.NewState)
                {
                    throw std::logic_error("This is a bug. Mismatch between final impact & game over state.");
                }
#endif
            }
            float raw_impact = 0;
            for (auto item : location_to_impact)
            {
                raw_impact += item.second;
            }
            move_to_raw_impact[item.first] = raw_impact;
            if (item.first.NewState == Side::Black)
            {
                if (raw_impact > max_raw_black_impact)
                    max_raw_black_impact = raw_impact;
            }
            else if (item.first.NewState == Side::White)
            {
                if (raw_impact > max_raw_white_impact)
                    max_raw_white_impact = raw_impact;
            }
            else
            {
                throw std::logic_error("Wrong game history, or a bug.");
            }
        }

        // Learn
        for (auto item : move_to_features)
        {
            bool is_black = item.first.NewState == Side::Black;
            /// Normalized impact
            float impact = move_to_raw_impact[item.first] / (is_black ? max_raw_black_impact : max_raw_white_impact);
            float feedback = impact * (is_black ? black_learning_feedback : white_learning_feedback);
#if REVERSI_DEBUG
            // Learn debug info
            move_to_feedback[item.first] = feedback;
#endif
            // Learn based on impact-based feedback
            for (auto features : item.second)
                Learn(features, feedback);
        }
#if REVERSI_DEBUG
        // Learn debug info
        Log("----------------------------------");
        Log("GAME OVER - LEARNING...");
        Log("----------------------------------");
        Log("Overall black learning feedback:");
        Log(std::to_string(black_learning_feedback));
        Log("----------------------------------");
        for (auto move : game_over_state.GetHistory())
        {
            auto feedback = move_to_feedback[move.Changes[0]];
            Log(
                std::string("Learn with feedback=")
                + std::to_string(feedback)
                + ":"
            );
            auto& [ before, after ] = move_to_states[move.Changes[0]];
            for (int y = 7; y >= 0; y--)
            {
                for (int x = 0; x < 8; x++)
                {
                    if (move.Changes[0].X == x && move.Changes[0].Y == y)
                    {
                        if (before.Get(x, y) == Side::None)
                            Log("*", " ");
                        else
                            Log("BUG!\nBUG! MOVE MADE ON NON-EMPTY SLOT!\nBUG!");
                    }
                    else
                        Log(std::to_string((int)before.Get(x, y)), " ");
                }
                if (y == 4)
                    Log("=> ", "");
                else
                    Log("   ", "");
                for (int x = 0; x < 8; x++)
                {
                    Log(std::to_string((int)after.Get(x, y)), " ");
                }
                Log();
            }
        }
        Log("----------------------------------");
        Log("Overall black learning feedback:");
        Log(std::to_string(black_learning_feedback));
        Log("Overall white learning feedback:");
        Log(std::to_string(white_learning_feedback));
        Log("----------------------------------");
#endif
        Save();
    }

    constexpr unsigned char EVOLVING_AI_FILE_DEFAULT_DATA_VALUE = 128;
    constexpr unsigned char EVOLVING_AI_FILE_HEADER[] = {
        0xFF,
        'R','e','m','i','n','i','m','a','l','i','s','m','.','R','e','v','e','r','s','i','.','E','v','o','l','v','i','n','g','A','I',
        0xFF
    };
    constexpr unsigned char EVOLVING_AI_FILE_VERSION[] = { 0, 0, 0, 0 };

    void EvolvingAI::ResetData()
    {
        for (int i = 0; i < sizeof(Data); i++)
        {
            Data[i] = EVOLVING_AI_FILE_DEFAULT_DATA_VALUE;
        }
    }

    void EvolvingAI::RenameToBackup(bool wrong_file)
    {
        int i = 0;
        while (std::filesystem::exists(DataFilePath + "." + std::to_string(i) + (wrong_file ? ".wrong-file-backup" : ".backup")))
            i++;
        std::filesystem::rename(DataFilePath, DataFilePath + "." + std::to_string(i) + (wrong_file ? ".wrong-file-backup" : ".backup"));
    }

    void EvolvingAI::Load()
    {
        if (!std::filesystem::exists(DataFilePath))
        {
            ResetData();
            Save();
            return;
        }
        else if (std::filesystem::is_directory(DataFilePath))
        {
            RenameToBackup(true);
            ResetData();
            Save();
            return;
        }
        std::fstream file(DataFilePath, std::fstream::binary | std::fstream::in);
        unsigned char file_check[sizeof(EVOLVING_AI_FILE_HEADER)];
        file.read((char*)file_check, sizeof(EVOLVING_AI_FILE_HEADER));
        for (int i = 0; i < sizeof(EVOLVING_AI_FILE_HEADER); i++)
        {
            if (EVOLVING_AI_FILE_HEADER[i] != file_check[i])
            {
                RenameToBackup(true);
                ResetData();
                Save();
                return;
            }
        }
        file.read((char*)file_check, sizeof(EVOLVING_AI_FILE_VERSION));
        for (int i = 0; i < sizeof(EVOLVING_AI_FILE_VERSION); i++)
        {
            if (EVOLVING_AI_FILE_VERSION[i] != file_check[i])
            {
                RenameToBackup(true);
                ResetData();
                Save();
                return;
            }
        }
        file.read((char*)Data, sizeof(Data));
    }

    void EvolvingAI::Save()
    {
        std::fstream file(DataFilePath, std::fstream::binary | std::fstream::out | std::fstream::trunc);
        file.write((char*)EVOLVING_AI_FILE_HEADER, sizeof(EVOLVING_AI_FILE_HEADER));
        file.write((char*)EVOLVING_AI_FILE_VERSION, sizeof(EVOLVING_AI_FILE_VERSION));
        file.write((char*)Data, sizeof(Data));
    }

    unsigned char& EvolvingAI::DataAt(const Features& features)
    {
        return Data[
            features.GeneralizedPlace
                * 8 // Direction
                * 8 // NeighborCount
                * 7 // AffectedDisksCount
                * 7 // NeighborColorChangeCount
                * 5 // IslandsCount
            + features.Direction
                * 8 // NeighborCount
                * 7 // AffectedDisksCount
                * 7 // NeighborColorChangeCount
                * 5 // IslandsCount
            + features.NeighborCount
                * 7 // AffectedDisksCount
                * 7 // NeighborColorChangeCount
                * 5 // IslandsCount
            + features.AffectedDisksCount
                * 7 // NeighborColorChangeCount
                * 5 // IslandsCount
            + features.NeighborColorChangeCount
                * 5 // IslandsCount
            + features.IslandsCount
        ];
    }

    float EvolvingAI::GetScore(const Features& features)
    {
        if (Generalization == 0)
            return ((float)DataAt(features))/255;
        float score = ((float)DataAt(features))/255;
        float contribution_sum = 1;
        Features general_features = features;
        for (int nccc = 0; nccc < 7; nccc++)
        {
            for (int ic = 0; ic < 5; ic++)
            {
                if (features.NeighborColorChangeCount == nccc && features.IslandsCount == ic)
                    continue;
                general_features.NeighborColorChangeCount = nccc;
                general_features.IslandsCount = ic;
                score += (((float)DataAt(features)) / 255) * Generalization;
                contribution_sum += Generalization; // Readable code
            }
        }
        return score / contribution_sum;
    }

    void EvolvingAI::Learn(const Features& features, float feedback)
    {
        auto& data = DataAt(features);
        float value_f = std::clamp((float)data + feedback * 255 * LearningRate, (float)0, (float)255);
        value_f = feedback < 0 ? std::floor(value_f) : std::ceil(value_f);
        data = (unsigned char) value_f;
    }

    std::vector<EvolvingAI::Features> EvolvingAI::GetFeatures(const Logic& state, int x, int y)
    {
        std::vector<EvolvingAI::Features> result;
        for (int dx = -1; dx <= 1; dx++)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                if (dx == 0 && dy == 0)
                    continue;
                Features features;
                features.GeneralizedPlace = GetGeneralizedPlace(x, y);
                features.Direction = GetGeneralizedDirection(x, y, dx, dy);
                features.NeighborCount = 0;
                features.AffectedDisksCount = 0;
                features.NeighborColorChangeCount = 0;
                features.IslandsCount = 0;
                int new_x = x + dx;
                int new_y = y + dy;
                bool have_passed_none = false;
                bool have_passed_current_turn = false;
                Side last_side = Side::None;
                while (0 <= new_x && new_x < 8 && 0 <= new_y && new_y < 8)
                {
                    auto new_side = state.Get(new_x, new_y);
                    if (!have_passed_none)
                    {
                        if (new_side != Side::None)
                            features.NeighborCount++;
                        if (!have_passed_current_turn)
                        {
                            if (new_side == Side::None)
                                features.AffectedDisksCount = 0;
                            else if (new_side != state.GetCurrentTurn())
                                features.AffectedDisksCount++;
                            // else: the count is correct
                        }
                        if (new_side != Side::None && last_side != Side::None && new_side != last_side)
                        {
                            features.NeighborColorChangeCount++;
                        }
                    }
                    if (new_side != Side::None && last_side == Side::None)
                        features.IslandsCount++;

                    if (new_side == Side::None)
                        have_passed_none = true;
                    if (new_side == state.GetCurrentTurn())
                        have_passed_current_turn = true;
                    new_x += dx;
                    new_y += dy;
                    last_side = new_side;
                }
                if (!have_passed_current_turn)
                    features.AffectedDisksCount = 0;
                result.push_back(features);
            }
        }
        return result;
    }

    int EvolvingAI::GetGeneralizedPlace(int x, int y)
    {
        if (x >= 4)
        {
            x = 7 - x;
        }
        if (y >= 4)
        {
            y = 7 - y;
        }
        if (x < y)
        {
            int temp = x;
            x = y;
            y = temp;
        }
        // y=3:       9
        // y=2:     7 8
        // y=1:   4 5 6
        // y=0: 0 1 2 3
        //    .........
        //   x: 0 1 2 3
        switch (y)
        {
        case 0:
            return x;     // 0 1 2 3
        case 1:
            return x + 3; // - 4 5 6
        case 2:
            return x + 5; // - - 7 8
        case 3:
            return x + 6; // - - - 9
        default:
            throw std::logic_error("Something is wrong with getting generalized place.");
        }
    }

    int EvolvingAI::GetGeneralizedDirection(int x, int y, int dx, int dy)
    {
        if (x >= 4)
        {
            dx = -dx;
        }
        if (y >= 4)
        {
            dy = -dy;
        }
        if (x < y)
        {
            int temp = dx;
            dx = dy;
            dy = temp;
        }
        // dy=+: 3 2 1
        // dy=0: 4   0
        // dy=-: 5 6 7
        //      ......
        //   dx: - 0 +
        if (dy < 0)
        {
            if (dx < 0)
            {
                return 5;
            }
            else if (dx == 0)
            {
                return 6;
            }
            else // x > 0
            {
                return 7;
            }
        }
        // dy=0: 4   0
        //      ......
        //   dx: - 0 +
        else if (dy == 0)
        {
            if (dx < 0)
            {
                return 4;
            }
            else if (dx == 0)
            {
                throw std::logic_error("Something is wrong with getting generalized direction.");
            }
            else // x > 0
            {
                return 0;
            }
        }
        // dy=+: 3 2 1
        //      ......
        //   dx: - 0 +
        else // y > 0
        {
            if (dx < 0)
            {
                return 3;
            }
            else if (dx == 0)
            {
                return 2;
            }
            else // x > 0
            {
                return 1;
            }
        }
    }
}
