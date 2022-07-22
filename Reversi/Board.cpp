#include "Board.h"

#include "AI.h"
#include "Window.h"

#include <algorithm>
#include <list>

using namespace std::chrono_literals;

constexpr double SLOT_ANIMATION_DURATION = 0.5;
constexpr double BUTTON_ENTRANCE_ANIMATION_DURATION = 0.25;
constexpr double BUTTON_EXIT_ANIMATION_DURATION = 0.5;
constexpr double SLOTS_ANIMATION_PROGRESSIVE_RELATIVE_DELAY = 0.2;
constexpr double AI_TO_PLAYER_INTERVAL = SLOT_ANIMATION_DURATION;
constexpr double AI_TO_AI_INTERVAL = 0.05;
constexpr double AI_TO_AI_AUTO_RESTART_DELAY = 0.5;

namespace Reversi
{
    Board::Board(std::shared_ptr<Window> window, std::shared_ptr<AI> ai)
        : _Window(window), _AI(ai), _Renderer(window), NeedUpdate(true), Player1Side(Side::Black),
          IsPlayer1AI(false), IsPlayer2AI(false),
          LastMoveTime(std::chrono::steady_clock::now())
    {
        _Window->SetResizeCallback([this](int width, int height) {
            Width = width;
            Height = height;
            UpdateSize();
        });
        _Window->GetSize(Width, Height);
        UpdateSize();
        UpdateBoard();
    }

    Board::~Board()
    {
        _Window->SetResizeCallback(nullptr);
    }

    bool Board::NeedsFrequentUpdate()
    {
        return NeedUpdate;
    }

    void Board::Update()
    {
        UpdateAI();

        std::list<Reversi::Renderer::ButtonID> button_anims_to_remove;
        std::list<IntVec2> slot_anims_to_remove;
        bool is_over;
        auto now = std::chrono::steady_clock::now();

        for (auto anim : ButtonAnimations)
        {
            _Renderer.SetButtonState(anim.first, anim.second.GetAnimatedState(now, is_over));
            if (is_over)
                button_anims_to_remove.push_back(anim.first);
        }
        for (auto item : button_anims_to_remove)
        {
            ButtonAnimations.erase(item);
        }

        for (auto anim : SlotAnimations)
        {
            _Renderer.SetSlotState(anim.first.X, anim.first.Y, anim.second.GetAnimatedState(now, is_over));
            if (is_over)
                slot_anims_to_remove.push_back(anim.first);
        }
        for (auto item : slot_anims_to_remove)
        {
            if (QueuedSlotAnimations.contains(item))
            {
                SlotAnimations[item] = QueuedSlotAnimations[item];
                SlotAnimations[item].SetTimeStart(now);
                QueuedSlotAnimations.erase(item);
            }
            else
                SlotAnimations.erase(item);
        }

        if (ButtonAnimations.size() == 0 && SlotAnimations.size() == 0)
        {
            NeedUpdate = false;
        }

        _Renderer.Render();
    }

    /// NOTE: Does NOT check the range of t.
    double lerp(double a, double b, double t)
    {
        return a + (b - a) * t;
    }

    /// Does clip t into range [0, 1].
    double linearstep(double t)
    {
        if (t < 0)
            return 0;
        if (t > 1)
            return 1;
        return t;
    }

    Board::ButtonAnimation::ButtonAnimation(
            Renderer::ButtonState StartState,
            Renderer::ButtonState StopState,
            double DurationInSeconds,
            std::chrono::steady_clock::time_point TimeStart
        ) : StartState(StartState), StopState(StopState),
            DurationInSeconds(DurationInSeconds), TimeStart(TimeStart)
    {
    }

    Renderer::ButtonState Board::ButtonAnimation::GetAnimatedState(
            std::chrono::steady_clock::time_point current_time,
            bool& is_over
        )
    {
        double t = ((std::chrono::duration<double>)(current_time - TimeStart)).count() / DurationInSeconds;
        if (t > 1)
        {
            t = 1;
            is_over = true;
        }
        else
            is_over = false;
        t = linearstep(t);
        return Renderer::ButtonState(
            lerp(StartState.Hover, StopState.Hover, t),
            lerp(StartState.Press, StopState.Press, t),
            lerp(StartState.Highlight, StopState.Highlight, t)
        );
    }

    Renderer::ButtonState Board::ButtonAnimation::GetStartState()
    {
        return StartState;
    }

    Renderer::ButtonState Board::ButtonAnimation::GetStopState()
    {
        return StopState;
    }

    Board::SlotAnimation::SlotAnimation(
            Side StartState,
            Side StopState,
            double DurationInSeconds,
            std::chrono::steady_clock::time_point TimeStart,
            double TDelay
        ) : StartState(StartState), StopState(StopState),
            DurationInSeconds(DurationInSeconds), TimeStart(TimeStart), TDelay(TDelay)
    {
    }

    void Board::SlotAnimation::SetTimeStart(std::chrono::steady_clock::time_point time)
    {
        TimeStart = time;
    }

    Renderer::SlotState Board::SlotAnimation::GetAnimatedState(
            std::chrono::steady_clock::time_point current_time,
            bool& is_over
        )
    {
        double t = ((std::chrono::duration<double>)(current_time - TimeStart)).count() / DurationInSeconds;
        t -= TDelay;
        if (t > 1)
        {
            t = 1;
            is_over = true;
        }
        else
            is_over = false;
        t = linearstep(t);
        return Renderer::SlotState(StartState, StopState, t);
    }

    Side Board::SlotAnimation::GetStartState()
    {
        return StartState;
    }

    Side Board::SlotAnimation::GetStopState()
    {
        return StopState;
    }

    Board::IntVec2::IntVec2(int X, int Y) : X(X), Y(Y)
    {
    }

    inline double convert_size(int l, int half_width_or_height) { return (double)l / (double)half_width_or_height; }
    /// @param x In [0,2*half_width] from left to right.
    /// @return In [-1, 1] from left to right.
    inline double convert_x(int x, int half_width) { return ((double)x / (double)half_width) - 1.0; }
    /// @param x In [0,2*half_height] from top to bottom.
    /// @return In [-1, 1] from bottom to top.
    inline double convert_y(int y, int half_height) { return 1.0 - ((double)y / (double)half_height); }

    void Board::UpdateSize()
    {
        // Properties
        constexpr double SLOT_PADDING = 0.1;
        constexpr double SIDE_PANELS_SIZE = 0.1;
        constexpr double SIDE_BUTTONS_PADDING = 0.1;

        // Static calculations based on properties
        constexpr double CENTER_SPACE = 1 - SIDE_PANELS_SIZE * 2;
        constexpr double SLOT_SCALE = 1 - 2 * SLOT_PADDING;
        constexpr double SQUARE_BOARD_PADDING = SLOT_PADDING / 8;
        constexpr double SQUARE_BOARD_SCALE = 1 - 2 * SQUARE_BOARD_PADDING;
        constexpr double SIDE_BUTTONS_SCALE = 1 - SIDE_BUTTONS_PADDING * 2;
        constexpr double SIDE_HALF_BUTTONS_SCALE = 0.5 - SIDE_BUTTONS_PADDING * 1.5;
        constexpr double SIDE_BUTTONS_OFFSET_SCALE = SIDE_BUTTONS_SCALE + SIDE_BUTTONS_PADDING;
        constexpr double SIDE_HALF_BUTTONS_OFFSET_SCALE = SIDE_HALF_BUTTONS_SCALE + SIDE_BUTTONS_PADDING;

        int square_board_space_size = std::min(
            std::min(Width, Height),
            std::max((int)(Width * CENTER_SPACE), (int)(Height * CENTER_SPACE))
        );

        auto mouse_event_manager = _Window->GetMouseEventManager();

        // Clear mouse event manager

        mouse_event_manager->Clear(square_board_space_size / 8);

        // Add back objects to mouse event manager with the new layout

        int square_board_size = square_board_space_size * SQUARE_BOARD_SCALE;
        int square_board_x = (Width - square_board_size) / 2;
        int square_board_y = (Height - square_board_size) / 2;
        int slot_size = square_board_space_size * (SLOT_SCALE / 8);
        int slot_offset = square_board_space_size / 8;

        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                mouse_event_manager->AddOval(
                    square_board_x + x * slot_offset,
                    square_board_y + (7 - y) * slot_offset,
                    slot_size, slot_size,
                    [this, x, y](MouseEventManager::MouseEvent e)
                    {
                        SlotMouseCallback(x, y, e);
                    }
                );
            }
        }

        int side_panel_size = SIDE_PANELS_SIZE * std::max(Width, Height);
        int side_panel_next_button_offset = SIDE_BUTTONS_OFFSET_SCALE * side_panel_size;
        int side_panel_next_half_button_offset = SIDE_HALF_BUTTONS_OFFSET_SCALE * side_panel_size;

        int button_size = SIDE_BUTTONS_SCALE * side_panel_size;
        int half_button_size = SIDE_HALF_BUTTONS_SCALE * side_panel_size;

        int replay_button_x = SIDE_BUTTONS_PADDING * side_panel_size;
        int replay_button_y = replay_button_x;

        int exit_button_x;
        int exit_button_y;
        int turn_indicator_button_x;
        int turn_indicator_button_y;
        int player1_ai_toggle_button_x;
        int player1_ai_toggle_button_y;
        int player2_ai_toggle_button_x;
        int player2_ai_toggle_button_y;

        if (Width >= Height)
        {
            exit_button_x = replay_button_x;
            exit_button_y = replay_button_y + side_panel_next_button_offset;
            turn_indicator_button_x = (side_panel_size - half_button_size) / 2; // Centered
            turn_indicator_button_y = exit_button_y + side_panel_next_button_offset;
            player1_ai_toggle_button_x = replay_button_x;
            player1_ai_toggle_button_y = turn_indicator_button_y + side_panel_next_half_button_offset;
            player2_ai_toggle_button_x = replay_button_x + side_panel_next_half_button_offset;
            player2_ai_toggle_button_y = player1_ai_toggle_button_y;
        }
        else
        {
            exit_button_x = replay_button_x + side_panel_next_button_offset;
            exit_button_y = replay_button_y;
            turn_indicator_button_x = exit_button_x + side_panel_next_button_offset;
            turn_indicator_button_y = (side_panel_size - half_button_size) / 2; // Centered
            player1_ai_toggle_button_x = turn_indicator_button_x + side_panel_next_half_button_offset;
            player1_ai_toggle_button_y = replay_button_y;
            player2_ai_toggle_button_x = player1_ai_toggle_button_x;
            player2_ai_toggle_button_y = replay_button_y + side_panel_next_half_button_offset;
        }

        mouse_event_manager->AddRectangle(
            replay_button_x, replay_button_y,
            button_size, button_size,
            [this](MouseEventManager::MouseEvent e)
            {
                ButtonMouseCallback(Renderer::ButtonID::ReplayButton, e);
            }
        );
        mouse_event_manager->AddRectangle(
            exit_button_x, exit_button_y,
            button_size, button_size,
            [this](MouseEventManager::MouseEvent e)
            {
                ButtonMouseCallback(Renderer::ButtonID::ExitButton, e);
            }
        );
        mouse_event_manager->AddRectangle(
            player1_ai_toggle_button_x, player1_ai_toggle_button_y,
            half_button_size, half_button_size,
            [this](MouseEventManager::MouseEvent e)
            {
                ButtonMouseCallback(Renderer::ButtonID::Player1AIToggle, e);
            }
        );
        mouse_event_manager->AddRectangle(
            player2_ai_toggle_button_x, player2_ai_toggle_button_y,
            half_button_size, half_button_size,
            [this](MouseEventManager::MouseEvent e)
            {
                ButtonMouseCallback(Renderer::ButtonID::Player2AIToggle, e);
            }
        );

        // Update renderer layout

        int half_width = Width / 2;
        int half_height = Height / 2;

        double renderer_button_size_x = convert_size(button_size, half_width);
        double renderer_button_size_y = convert_size(button_size, half_height);
        double renderer_half_button_size_x = convert_size(half_button_size, half_width);
        double renderer_half_button_size_y = convert_size(half_button_size, half_height);
        double renderer_slot_size_x = convert_size(slot_size, half_width);
        double renderer_slot_size_y = convert_size(slot_size, half_height);

        _Renderer.SetLayout(Renderer::Layout(
            // Replay button:
            Renderer::Rectangle(
                convert_x(replay_button_x, half_width),
                convert_y(replay_button_y + button_size, half_height),
                renderer_button_size_x,
                renderer_button_size_y
            ),
            // Exit button
            Renderer::Rectangle(
                convert_x(exit_button_x, half_width),
                convert_y(exit_button_y + button_size, half_height),
                renderer_button_size_x,
                renderer_button_size_y
            ),
            // Turn indicator button
            Renderer::Rectangle(
                convert_x(turn_indicator_button_x, half_width),
                convert_y(turn_indicator_button_y + half_button_size, half_height),
                renderer_half_button_size_x,
                renderer_half_button_size_y
            ),
            // Player1 AI toggle button
            Renderer::Rectangle(
                convert_x(player1_ai_toggle_button_x, half_width),
                convert_y(player1_ai_toggle_button_y + half_button_size, half_height),
                renderer_half_button_size_x,
                renderer_half_button_size_y
            ),
            // Player2 AI toggle button
            Renderer::Rectangle(
                convert_x(player2_ai_toggle_button_x, half_width),
                convert_y(player2_ai_toggle_button_y + half_button_size, half_height),
                renderer_half_button_size_x,
                renderer_half_button_size_y
            ),
            // Bottom left slot
            Renderer::Rectangle(
                convert_x(square_board_x, half_width),
                convert_x(square_board_y, half_height),
                renderer_slot_size_x,
                renderer_slot_size_y
            ),
            // Top right slot
            Renderer::Rectangle(
                convert_x(square_board_x + 7 * slot_offset, half_width),
                convert_x(square_board_y + 7 * slot_offset, half_height),
                renderer_slot_size_x,
                renderer_slot_size_y
            )
        ));
        NeedUpdate = true;
    }

    void Board::UpdateBoard()
    {
        auto now = std::chrono::steady_clock::now();
        for (int y = 0; y < 8; y++)
        {
            for (int x = 0; x < 8; x++)
            {
                bool slot_animation_exists = SlotAnimations.contains(IntVec2(x, y));
                auto anim = SlotAnimation(
                    slot_animation_exists ?
                        SlotAnimations[IntVec2(x, y)].GetStopState()
                        : _Renderer.GetSlotState(x, y).Next,
                    _Logic.Get(x, y),
                    SLOT_ANIMATION_DURATION,
                    now,
                    SLOTS_ANIMATION_PROGRESSIVE_RELATIVE_DELAY * y
                );
                if (slot_animation_exists)
                    QueuedSlotAnimations[IntVec2(x, y)] = anim;
                else
                    SlotAnimations[IntVec2(x, y)] = anim;
                }
        }
        UpdateTurnIndicator();
        UpdateAIToggles();
        NeedUpdate = true;
    }

    void Board::SlotMouseCallback(int x, int y, MouseEventManager::MouseEvent e)
    {
        if (e == MouseEventManager::MouseEvent::Click && !IsAIsTurn())
        {
            MakeMove(x, y);
        }
    }

    void Board::ButtonMouseCallback(Renderer::ButtonID b_id, MouseEventManager::MouseEvent e)
    {
        switch (e)
        {
        case MouseEventManager::MouseEvent::Enter:
            ButtonAnimations[b_id] = ButtonAnimation(
                _Renderer.GetButtonState(b_id),
                Renderer::ButtonState(1, 0, ButtonHighlights[b_id]),
                BUTTON_ENTRANCE_ANIMATION_DURATION,
                std::chrono::steady_clock::now()
            );
            break;
        case MouseEventManager::MouseEvent::Down:
            ButtonAnimations[b_id] = ButtonAnimation(
                _Renderer.GetButtonState(b_id),
                Renderer::ButtonState(1, 1, ButtonHighlights[b_id]),
                BUTTON_ENTRANCE_ANIMATION_DURATION,
                std::chrono::steady_clock::now()
            );
            break;
        case MouseEventManager::MouseEvent::Leave:
            ButtonAnimations[b_id] = ButtonAnimation(
                _Renderer.GetButtonState(b_id),
                Renderer::ButtonState(0, 0, ButtonHighlights[b_id]),
                BUTTON_EXIT_ANIMATION_DURATION,
                std::chrono::steady_clock::now()
            );
            break;
        case MouseEventManager::MouseEvent::Click:
            switch (b_id)
            {
            case Renderer::ButtonID::ReplayButton:
                Replay();
                break;
            case Renderer::ButtonID::ExitButton:
                _Window->Close();
                break;
            case Renderer::ButtonID::Player1AIToggle:
                IsPlayer1AI = !IsPlayer1AI;
                UpdateAIToggles();
                break;
            case Renderer::ButtonID::Player2AIToggle:
                IsPlayer2AI = !IsPlayer2AI;
                UpdateAIToggles();
                break;
            }
            ButtonAnimations[b_id] = ButtonAnimation(
                _Renderer.GetButtonState(b_id),
                Renderer::ButtonState(1, 0, ButtonHighlights[b_id]),
                BUTTON_EXIT_ANIMATION_DURATION,
                std::chrono::steady_clock::now()
            );
            break;
        }
        NeedUpdate = true;
    }

    void Board::Replay()
    {
        _Logic.Reset();
        Player1Side = Player1Side == Side::Black ? Side::White : Side::Black;
        ButtonHighlights[Renderer::ButtonID::Player1SideVirtualButton] = Player1Side == Side::White ? 1 : 0;
        ButtonAnimations[Renderer::ButtonID::Player1SideVirtualButton] = ButtonAnimation(
            _Renderer.GetButtonState(Renderer::ButtonID::Player1SideVirtualButton),
            Renderer::ButtonState(0, 0, ButtonHighlights[Renderer::ButtonID::Player1SideVirtualButton]),
            BUTTON_EXIT_ANIMATION_DURATION,
            std::chrono::steady_clock::now()
        );
        UpdateBoard();
    }

    void Board::UpdateAI()
    {
        bool ai_vs_ai = IsPlayer1AI && IsPlayer2AI;
        double interval = ai_vs_ai ? AI_TO_AI_INTERVAL : AI_TO_PLAYER_INTERVAL;
        if (IsAIsTurn() && ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - LastMoveTime)).count() > interval)
        {
            auto result = _AI->Decide(_Logic);
            if (result.has_value())
            {
                MakeMove(std::get<0>(result.value()), std::get<1>(result.value()));
            }
        }
        else if (ai_vs_ai && _Logic.IsGameOver() && ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - LastMoveTime)).count() > AI_TO_AI_AUTO_RESTART_DELAY)
        {
            Replay();
        }
    }

    bool Board::IsAIsTurn()
    {
        return _Logic.GetCurrentTurn() == Player1Side ? IsPlayer1AI : (
            _Logic.GetCurrentTurn() != Side::None ? IsPlayer2AI : false
        );
    }

    void Board::MakeMove(int x, int y)
    {
        auto move = _Logic.MakeMove(x, y);
        auto now = std::chrono::steady_clock::now();
        for (int i = 0; i < move.Changes.size(); i++)
        {
            auto change = move.Changes[i];
            bool slot_animation_exists = SlotAnimations.contains(IntVec2(change.X, change.Y));
            auto anim = SlotAnimation(
                slot_animation_exists ?
                    SlotAnimations[IntVec2(change.X, change.Y)].GetStopState()
                    : _Renderer.GetSlotState(change.X, change.Y).Next,
                change.NewState,
                SLOT_ANIMATION_DURATION,
                now,
                SLOTS_ANIMATION_PROGRESSIVE_RELATIVE_DELAY * i
            );
            if (slot_animation_exists)
                QueuedSlotAnimations[IntVec2(change.X, change.Y)] = anim;
            else
                SlotAnimations[IntVec2(change.X, change.Y)] = anim;
        }
        if (_Logic.IsGameOver() && move.Changes.size() != 0)
        {
            _AI->Learn(_Logic);
        }
        LastMoveTime = now;
        UpdateTurnIndicator();
        NeedUpdate = true;
    }

    void Board::UpdateTurnIndicator()
    {
        if (_Logic.IsGameOver())
        {
            Side winner = _Logic.GetWinner();
            ButtonHighlights[Renderer::ButtonID::TurnIndicator] =
                winner == Side::White ?
                    0.65
                    : (
                        winner == Side::Black ?
                            0.35
                            : 0.5
                    );
        }
        else
        {
            ButtonHighlights[Renderer::ButtonID::TurnIndicator] =
                _Logic.GetCurrentTurn() == Side::White ?
                    1
                    : (
                        _Logic.GetCurrentTurn() == Side::Black ?
                            0
                            : 0.5
                    );
        }
        ButtonAnimations[Renderer::ButtonID::TurnIndicator] = ButtonAnimation(
            _Renderer.GetButtonState(Renderer::ButtonID::TurnIndicator),
            Renderer::ButtonState(0, 0, ButtonHighlights[Renderer::ButtonID::TurnIndicator]),
            BUTTON_EXIT_ANIMATION_DURATION,
            std::chrono::steady_clock::now()
        );
    }

    void Board::UpdateAIToggles()
    {
        ButtonHighlights[Renderer::ButtonID::Player1AIToggle] = IsPlayer1AI ? 1 : 0;
        auto last_stop_state = ButtonAnimations[Renderer::ButtonID::Player1AIToggle].GetStopState();
        ButtonAnimations[Renderer::ButtonID::Player1AIToggle] = ButtonAnimation(
            _Renderer.GetButtonState(Renderer::ButtonID::Player1AIToggle),
            Renderer::ButtonState(last_stop_state.Hover, last_stop_state.Press, ButtonHighlights[Renderer::ButtonID::Player1AIToggle]),
            BUTTON_ENTRANCE_ANIMATION_DURATION,
            std::chrono::steady_clock::now()
        );
        ButtonHighlights[Renderer::ButtonID::Player2AIToggle] = IsPlayer2AI ? 1 : 0;
        last_stop_state = ButtonAnimations[Renderer::ButtonID::Player2AIToggle].GetStopState();
        ButtonAnimations[Renderer::ButtonID::Player2AIToggle] = ButtonAnimation(
            _Renderer.GetButtonState(Renderer::ButtonID::Player2AIToggle),
            Renderer::ButtonState(last_stop_state.Hover, last_stop_state.Press, ButtonHighlights[Renderer::ButtonID::Player2AIToggle]),
            BUTTON_ENTRANCE_ANIMATION_DURATION,
            std::chrono::steady_clock::now()
        );
    }
}
