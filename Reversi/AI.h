#pragma once

#include "Reversi.dec.h"

#include "Logic.h"

#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace Reversi
{
    class AI
    {
    public:
        virtual ~AI() = default;
        virtual std::optional<std::tuple<int, int>> Decide(const Logic& state) = 0;
        virtual void Learn(const Logic& game_over_state);
    };

    class DecisionTreeAI : public AI
    {
    public:
        DecisionTreeAI(int depth);
        virtual std::optional<std::tuple<int, int>> Decide(const Logic& state) override;
        int GetDepth();
        void SetDepth(int);
    private:
        int Depth;

        float CalculateScore(const Logic& state, Side side, int depth, float alpha, float beta);
        float CalculateScoreTerminal(const Logic& state, Side side);
    };

    class EvolvingAI : public AI
    {
    public:
        /// @param LearningRate In range [0, 1].
        ///                     0: no learning, 1: immediately thinking the last game is everything and ignoring similar history.
        /// @param Generalization How much generalization is done on weak features, in range [0,1].
        ///                       0: no generalization,
        ///                       0.5: specific and generalization scores equally contribute to the score,
        ///                       1: complete generalization.
        EvolvingAI(std::string DataFilePath, float LearningRate = 0.1, float Generalization = 0.1);
        virtual std::optional<std::tuple<int, int>> Decide(const Logic& state) override;
        virtual void Learn(const Logic& game_over_state) override;
    private:

        class Features // TODO: Decide on adding the number of moves done in range [0,59].
        {
        public:
            /// @brief In range [0,9].
            int GeneralizedPlace;
            /// @brief Transformed direction in range [0,7].
            ///
            /// The direction should be transformed to the generalized space.
            int Direction;
            /// @brief The number of neighboring disks in the direction in range [0,7].
            int NeighborCount;
            /// @brief The number of affected disks in the direction in range [0,6].
            int AffectedDisksCount;
            /// @brief The number of color change in the neighboring disks in the direction in range [0,6], weak feature.
            int NeighborColorChangeCount;
            /// @brief The number of islands in the direction in range [0,4], weak feature.
            int IslandsCount;
        };

        float LearningRate;
        float Generalization;
        std::string DataFilePath;
        unsigned char Data[
            10  // GeneralizedPlace
            * 8 // Direction
            * 8 // NeighborCount
            * 7 // AffectedDisksCount
            * 7 // NeighborColorChangeCount
            * 5 // IslandsCount
        ];

        void ResetData();
        void RenameToBackup(bool wrong_file);
        void Load();
        void Save();
        unsigned char& DataAt(const Features&);
        /// @return In range [0, 1].
        float GetScore(const Features&);
        /// @param feedback In range [-1, 1].
        void Learn(const Features&, float feedback);
        /// @return 8 objects of Features type for each direction.
        std::vector<Features> GetFeatures(const Logic& state, int x, int y);
        /// @brief Gets the generalized place in range [0, 9].
        int GetGeneralizedPlace(int x, int y);
        /// @brief Converts the direction to the direction for generalized place.
        /// @param dx The x direction. Both dx and dy can't be 0 at the same time.
        /// @param dy The y direction. Both dx and dy can't be 0 at the same time.
        int GetGeneralizedDirection(int x, int y, int dx, int dy);
        /// @brief Converts the generalized direction to the actual direction on the board.
        /// @return (dx, dy) tuple, both dx and dy are in [-1, 1].
        std::tuple<int, int> GetActualDirection(int x, int y, int generalized_direction);
    };
}
