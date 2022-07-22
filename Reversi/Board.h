#pragma once

#include "Reversi.dec.h"

#include "Logic.h"
#include "Renderer.h"

#include "MouseEventManager.h"

#include <chrono>
#include <map>
#include <memory>

namespace Reversi
{
    /// @brief Manages the game state and layout by containing and controlling Logic and Renderer.
    class Board final
    {
    public:
        explicit Board(std::shared_ptr<Window>, std::shared_ptr<AI>);
        ~Board();

        Board(const Board&) = delete;
        Board(Board&&) = delete;
        Board& operator=(const Board&) = delete;
        Board& operator=(Board&&) = delete;

        /// @brief Whether frequent and continuous updates are needed.
        ///
        /// So that the loop can manage CPU usage.
        bool NeedsFrequentUpdate();
        void Update();
    private:
        int Width, Height;
        Logic _Logic;
        Renderer _Renderer;
        std::shared_ptr<Window> _Window;
        std::shared_ptr<AI> _AI;

        class ButtonAnimation
        {
        public:
            ButtonAnimation() = default; // Required for use in std::map
            ButtonAnimation(
                Renderer::ButtonState Current,
                Renderer::ButtonState Next,
                double DurationInSeconds,
                std::chrono::steady_clock::time_point TimeStart
            );
            Renderer::ButtonState GetAnimatedState(std::chrono::steady_clock::time_point, bool& is_over);
            Renderer::ButtonState GetStartState();
            Renderer::ButtonState GetStopState();
        private:
            std::chrono::steady_clock::time_point TimeStart;
            double DurationInSeconds;
            Renderer::ButtonState StartState;
            Renderer::ButtonState StopState;
        };

        class SlotAnimation
        {
        public:
            SlotAnimation() = default; // Required for use in std::map
            /// @param TDelay Directly subtracted from t when animating.
            ///               Positive values will delay animation by TDelay * DurationInSeconds seconds.
            SlotAnimation(
                Side StartState,
                Side StopState,
                double DurationInSeconds,
                std::chrono::steady_clock::time_point TimeStart,
                double TDelay = 0
            );
            void SetTimeStart(std::chrono::steady_clock::time_point);
            Renderer::SlotState GetAnimatedState(std::chrono::steady_clock::time_point, bool& is_over);
            Side GetStartState();
            Side GetStopState();
        private:
            std::chrono::steady_clock::time_point TimeStart;
            double DurationInSeconds;
            Side StartState;
            Side StopState;
            double TDelay;
        };

        struct IntVec2 final
        {
        public:
            IntVec2(int X, int Y);
            int X;
            int Y;
            auto operator<=>(const IntVec2&) const = default;
        };

        bool NeedUpdate;
        std::map<Renderer::ButtonID, ButtonAnimation> ButtonAnimations;
        std::map<Renderer::ButtonID, float> ButtonHighlights;
        std::map<IntVec2, SlotAnimation> SlotAnimations;
        std::map<IntVec2, SlotAnimation> QueuedSlotAnimations;

        Side Player1Side;
        bool IsPlayer1AI;
        bool IsPlayer2AI;

        std::chrono::steady_clock::time_point LastMoveTime;

        void UpdateSize();
        /// @brief Updates the slots to the current state.
        void UpdateBoard();
        void SlotMouseCallback(int x, int y, MouseEventManager::MouseEvent e);
        void ButtonMouseCallback(Renderer::ButtonID, MouseEventManager::MouseEvent e);
        void Replay();
        /// @brief Has to be updated on each update.
        void UpdateAI();
        bool IsAIsTurn();
        void MakeMove(int x, int y);
        void UpdateTurnIndicator();
        void UpdateAIToggles();
    };
}
