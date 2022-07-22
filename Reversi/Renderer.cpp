#include "Renderer.h"

#include <cmath>
#include <numbers>

#include "BufferGeneration.h"
#include "Shaders.h"
#include "Window.h"

constexpr int DISK_RESOLUTION = 80;
constexpr float DISK_THICKNESS = 0.1f;
constexpr float DISK_PADDING = 0.05f;

constexpr int ROUNDED_SQUARE_CORNER_RESOLUTION = 20;
constexpr float ROUNDED_SQUARE_CORNER_RADIUS = 0.5f;

constexpr int REPLAY_ICON_CIRCLE_RESOLUTION = 60;
constexpr float BUTTON_ICON_Z = 0.05f;

namespace Reversi
{
    Renderer::Renderer(std::shared_ptr<Window> window)
        : _Window(window),
          _Layout(
                Rectangle(0,0,0,0), Rectangle(0,0,0,0),
                Rectangle(0,0,0,0), Rectangle(0,0,0,0), Rectangle(0,0,0,0),
                Rectangle(0,0,0,0), Rectangle(0,0,0,0)
            ),
          Program(SimpleVertexShaderSource, SimpleFragmentShaderSource),
          DiskHalf(BufferGeneration::GenerateDiskHalf(DISK_RESOLUTION, DISK_THICKNESS)),
          DiskPlace(BufferGeneration::GenerateDiskPlace(DISK_RESOLUTION, DISK_PADDING)),
          Square(BufferGeneration::GenerateSquare()),
          RoundedSquare(BufferGeneration::GenerateRoundedSquare(
                ROUNDED_SQUARE_CORNER_RESOLUTION, ROUNDED_SQUARE_CORNER_RADIUS, ROUNDED_SQUARE_CORNER_RADIUS
            )),
          ReplayIcon(BufferGeneration::GenerateReplayIcon(REPLAY_ICON_CIRCLE_RESOLUTION, BUTTON_ICON_Z)),
          ExitIcon(BufferGeneration::GenerateExitIcon(BUTTON_ICON_Z)),
          AIIcon(BufferGeneration::GenerateAIIcon(BUTTON_ICON_Z))
    {
        ProgramModelUniform = Program.GetUniformLocation("Model");
        ProgramViewUniform = Program.GetUniformLocation("View");
        ProgramProjectionUniform = Program.GetUniformLocation("Projection");
        ProgramColorUniform = Program.GetUniformLocation("Color");

        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                int i = y << 3 | x;
                float pos_x = (-1 + 0.25f/2) + 0.25f * x;
                float pos_y = (-1 + 0.25f/2) + 0.25f * y;
                SlotModelMatrices[i] = Math::Matrix4x4::Translation(pos_x, pos_y, 0)
                        * Math::Matrix4x4::Scale(0.25f/2, 0.25f/2, 0.25f/2);
            }
        }

        glEnable(GL_DEPTH_TEST);
    }

    Renderer::~Renderer()
    {
    }

    Renderer::Rectangle::Rectangle(double X, double Y, double Width, double Height)
        : X(X), Y(Y), Width(Width), Height(Height)
    {
    }

    Renderer::Layout::Layout(
            Rectangle ReplayButton,
            Rectangle ExitButton,
            Rectangle TurnIndicator,
            Rectangle Player1AIToggle,
            Rectangle Player2AIToggle,
            Rectangle BottomLeftSlot,
            Rectangle TopRightSlot
        ) : ReplayButton(ReplayButton), ExitButton(ExitButton),
            TurnIndicator(TurnIndicator), Player1AIToggle(Player1AIToggle), Player2AIToggle(Player2AIToggle),
            BottomLeftSlot(BottomLeftSlot), TopRightSlot(TopRightSlot)
    {
    }

    Renderer::ButtonState::ButtonState(double Hover, double Press, double Highlight)
        : Hover(Hover), Press(Press), Highlight(Highlight)
    {
    }

    Renderer::SlotState::SlotState(Side Current, Side Next, double Transition)
        : Current(Current), Next(Next), Transition(Transition)
    {
    }

    void Renderer::SetLayout(Layout layout)
    {
        _Layout = layout;

        BoardInViewScale[0] = (layout.TopRightSlot.X - layout.BottomLeftSlot.X) * (4.0 / 7.0);
        BoardInViewScale[1] = (layout.TopRightSlot.Y - layout.BottomLeftSlot.Y) * (4.0 / 7.0);

        BoardInViewPosition[0] =
            layout.BottomLeftSlot.X - (((BoardInViewScale[0] * 0.25f) - layout.BottomLeftSlot.Width) * 0.5f) // x - (2 padding.x) / 2
            + BoardInViewScale[0]; // board_bottomleft.x + halfsize.x
        BoardInViewPosition[1] =
            layout.BottomLeftSlot.Y - (((BoardInViewScale[1] * 0.25f) - layout.BottomLeftSlot.Height) * 0.5f) // y - (2 padding.y) / 2
            + BoardInViewScale[1]; // board_bottomleft.y + halfsize.y

        ViewInWorldScale[0] = 1 / BoardInViewScale[0];
        ViewInWorldScale[1] = 1 / BoardInViewScale[1];

        ViewInWorldPosition[0] = -BoardInViewPosition[0] * ViewInWorldScale[0];
        ViewInWorldPosition[1] = -BoardInViewPosition[1] * ViewInWorldScale[1];

        LayoutDiskScale[0] = layout.BottomLeftSlot.Width * 0.5f * ViewInWorldScale[0];
        LayoutDiskScale[1] = layout.BottomLeftSlot.Height * 0.5f * ViewInWorldScale[1];

        float left = ViewInWorldPosition[0] - ViewInWorldScale[0];
        float right = ViewInWorldPosition[0] + ViewInWorldScale[0];
        float bottom = ViewInWorldPosition[1] - ViewInWorldScale[1];
        float top = ViewInWorldPosition[1] + ViewInWorldScale[1];
        LeftPanelMatrix = Math::Matrix4x4::Translation((-1 + left) * 0.5, 0, 0)
                        * Math::Matrix4x4::Scale((-1 - left) * 0.5, 1, 1);
        RightPanelMatrix = Math::Matrix4x4::Translation((right + (1)) * 0.5, 0, 0)
                        * Math::Matrix4x4::Scale((right - (1)) * 0.5, 1, 1);
        BottomPanelMatrix = Math::Matrix4x4::Translation(0, (-1 + bottom) * 0.5, 0)
                        * Math::Matrix4x4::Scale(ViewInWorldScale[0], (-1 - bottom) * 0.5, 1);
        TopPanelMatrix = Math::Matrix4x4::Translation(0, (top + (1)) * 0.5, 0)
                        * Math::Matrix4x4::Scale(ViewInWorldScale[0], (top - (1)) * 0.5, 1);

        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                UpdateSlot(x, y);
            }
        }
        UpdateButton(ButtonID::ReplayButton);
        UpdateButton(ButtonID::ExitButton);
        UpdateButton(ButtonID::TurnIndicator);
        UpdateButton(ButtonID::Player1AIToggle);
        UpdateButton(ButtonID::Player2AIToggle);
        UpdateView();
    }

    Renderer::Layout Renderer::GetLayout()
    {
        return _Layout;
    }

    void Renderer::SetButtonState(ButtonID id, ButtonState state)
    {
        ButtonStates[id] = state;
        UpdateButton(id);
    }

    Renderer::ButtonState Renderer::GetButtonState(ButtonID id)
    {
        return ButtonStates[id];
    }

    void Renderer::SetSlotState(int x, int y, SlotState state)
    {
        if (x < 0 || x >= 8 || y < 0 || y >= 8)
            return;
        SlotStates[y << 3 | x] = state;
        UpdateSlot(x, y);
    }

    Renderer::SlotState Renderer::GetSlotState(int x, int y)
    {
        if (x < 0 || x >= 8 || y < 0 || y >= 8)
            return SlotState();
        return SlotStates[y << 3 | x];
    }

    float smoothstep(float t)
    {
        if (t < 0)
            return 0;
        if (t > 1)
            return 1;
        return t * t * (3 - 2 * t);
    }

    float smoothstart(float t)
    {
        if (t < 0)
            return 0;
        if (t > 1)
            return 1;
        return t * t * t;
    }

    float smoothstop(float t)
    {
        return 1 - smoothstart(1 - t);
    }

    void Renderer::Render()
    {
        _Window->MakeCurrent();

        constexpr float BG_R = 0;
        constexpr float BG_G = 0.4f;
        constexpr float BG_B = 0.2f;
        constexpr float FG_R = 0;
        constexpr float FG_G = 0.6f;
        constexpr float FG_B = 0.3f;

        glClearColor(BG_R, BG_G, BG_B, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Program.Use();

        // View
        glUniformMatrix4fv(ProgramViewUniform, 1, GL_FALSE, ViewMatrix.GetData());
        glUniformMatrix4fv(ProgramProjectionUniform, 1, GL_FALSE, ProjectionMatrix.GetData());

        // Panels
        glUniform3f(ProgramColorUniform, FG_R, FG_G, FG_B);
        glUniformMatrix4fv(ProgramModelUniform, 1, GL_FALSE, LeftPanelMatrix.GetData());
        Square.Render();
        glUniformMatrix4fv(ProgramModelUniform, 1, GL_FALSE, RightPanelMatrix.GetData());
        Square.Render();
        glUniformMatrix4fv(ProgramModelUniform, 1, GL_FALSE, BottomPanelMatrix.GetData());
        Square.Render();
        glUniformMatrix4fv(ProgramModelUniform, 1, GL_FALSE, TopPanelMatrix.GetData());
        Square.Render();

        // Slots and disks
        for (int i = 0; i < 64; i++)
        {
            glUniformMatrix4fv(ProgramModelUniform, 1, GL_FALSE, SlotModelMatrices[i].GetData());
            glUniform3f(ProgramColorUniform, FG_R, FG_G, FG_B);
            DiskPlace.Render();
            glUniformMatrix4fv(ProgramModelUniform, 1, GL_FALSE, DiskBlackModelMatrices[i].GetData());
            glUniform3f(ProgramColorUniform, 0, 0, 0);
            DiskHalf.Render();
            glUniformMatrix4fv(ProgramModelUniform, 1, GL_FALSE, DiskWhiteModelMatrices[i].GetData());
            glUniform3f(ProgramColorUniform, 1, 1, 1);
            DiskHalf.Render();
        }
        float hover;
        float press;
        float color;
        // Replay button
        glUniformMatrix4fv(ProgramModelUniform, 1, GL_FALSE, ButtonModelMatrices[ButtonID::ReplayButton].GetData());
        hover = smoothstep(ButtonStates[ButtonID::ReplayButton].Hover);
        press = smoothstep(ButtonStates[ButtonID::ReplayButton].Press);
        glUniform3f(
            ProgramColorUniform,
            (1 - hover) * 0.8f,
            1 - 0.15f * hover - 0.1f * press,
            1 - 0.2f * hover - 0.1f * press
        );
        RoundedSquare.Render();
        color = hover - 0.2 * press;
        glUniform3f(ProgramColorUniform, color, color, color);
        ReplayIcon.Render();
        // Exit button
        glUniformMatrix4fv(ProgramModelUniform, 1, GL_FALSE, ButtonModelMatrices[ButtonID::ExitButton].GetData());
        hover = smoothstep(ButtonStates[ButtonID::ExitButton].Hover);
        press = smoothstep(ButtonStates[ButtonID::ExitButton].Press);
        glUniform3f(
            ProgramColorUniform,
            1 - 0.2f * press,
            (1 - hover) * 0.8f,
            0.9f - 0.4f * hover - 0.1f * press
        );
        RoundedSquare.Render();
        color = hover - 0.2 * press;
        glUniform3f(ProgramColorUniform, color, color, color);
        ExitIcon.Render();
        // Turn indicator
        glUniformMatrix4fv(ProgramModelUniform, 1, GL_FALSE, ButtonModelMatrices[ButtonID::TurnIndicator].GetData());
        color = smoothstep(ButtonStates[ButtonID::TurnIndicator].Highlight);
        glUniform3f(ProgramColorUniform, color, color, color);
        DiskHalf.Render();
        // Player1 AI Toggle
        glUniformMatrix4fv(ProgramModelUniform, 1, GL_FALSE, ButtonModelMatrices[ButtonID::Player1AIToggle].GetData());
        color = smoothstep(ButtonStates[ButtonID::Player1SideVirtualButton].Highlight); // Player1's side
        hover = smoothstep(ButtonStates[ButtonID::Player1AIToggle].Hover);
        press = smoothstep(ButtonStates[ButtonID::Player1AIToggle].Press);
        color += (1 - 2 * color) * (hover * 0.2 + press * 0.2);
        float highlight = color + (1 - 2 * color) * smoothstep(ButtonStates[ButtonID::Player1AIToggle].Highlight);
        glUniform3f(ProgramColorUniform, color, color, color);
        RoundedSquare.Render();
        glUniform3f(ProgramColorUniform, highlight, highlight, highlight);
        AIIcon.Render();
        // Player2 AI Toggle
        glUniformMatrix4fv(ProgramModelUniform, 1, GL_FALSE, ButtonModelMatrices[ButtonID::Player2AIToggle].GetData());
        color = 1 - smoothstep(ButtonStates[ButtonID::Player1SideVirtualButton].Highlight); // Player2's side
        hover = smoothstep(ButtonStates[ButtonID::Player2AIToggle].Hover);
        press = smoothstep(ButtonStates[ButtonID::Player2AIToggle].Press);
        color += (1 - 2 * color) * (hover * 0.2 + press * 0.2);
        highlight = color + (1 - 2 * color) * smoothstep(ButtonStates[ButtonID::Player2AIToggle].Highlight);
        glUniform3f(ProgramColorUniform, color, color, color);
        RoundedSquare.Render();
        glUniform3f(ProgramColorUniform, highlight, highlight, highlight);
        AIIcon.Render();

        _Window->SwapBuffers();
    }

    void Renderer::UpdateSlot(int x, int y)
    {
        // Board x,y:[-1,1]
        int i = y << 3 | x;
        float pos_x = (-1 + 0.25f/2) + 0.25f * x;
        float pos_y = (-1 + 0.25f/2) + 0.25f * y;
        const auto& slot_state = SlotStates[i];
        if (slot_state.Current == Side::None || slot_state.Next == Side::None)
        {
            float t;
            float r0, rd;
            if (slot_state.Current == slot_state.Next)
            {
                t = 0;
            }
            else
            {
                t = slot_state.Current == Side::None ? slot_state.Transition : 1 - slot_state.Transition;
            }
            if (slot_state.Current == Side::Black || slot_state.Next == Side::Black)
            {
                r0 = 0.25f;
                rd = -0.25f;
            }
            else
            {
                r0 = 0.75f;
                rd = 0.25f;
            }
            t = smoothstop(t);
            DiskBlackModelMatrices[i] = Math::Matrix4x4::Translation(pos_x, pos_y, 0)
                    * Math::Matrix4x4::RotationAroundX(std::numbers::pi * (r0 + rd * t))
                    * Math::Matrix4x4::Scale(LayoutDiskScale[0] * t, LayoutDiskScale[1] * t, 0.25f);
            DiskWhiteModelMatrices[i] = Math::Matrix4x4::Translation(pos_x, pos_y, 0)
                    * Math::Matrix4x4::RotationAroundX(std::numbers::pi * (1 + r0 + rd * t))
                    * Math::Matrix4x4::Scale(LayoutDiskScale[0] * t, LayoutDiskScale[1] * t, 0.25f);
        }
        else
        {
            float t = slot_state.Current == Side::Black ? slot_state.Transition : 1 - slot_state.Transition;
            if (slot_state.Current == slot_state.Next)
            {
                t = slot_state.Current == Side::Black ? 0 : 1;
            }
            t = smoothstep(t);
            DiskBlackModelMatrices[i] = Math::Matrix4x4::Translation(pos_x, pos_y, 0)
                    * Math::Matrix4x4::RotationAroundX(std::numbers::pi * t)
                    * Math::Matrix4x4::Scale(LayoutDiskScale[0], LayoutDiskScale[1], 0.25f);
            DiskWhiteModelMatrices[i] = Math::Matrix4x4::Translation(pos_x, pos_y, 0)
                    * Math::Matrix4x4::RotationAroundX(std::numbers::pi * (1 + t))
                    * Math::Matrix4x4::Scale(LayoutDiskScale[0], LayoutDiskScale[1], 0.25f);
        }
    }
    Renderer::Rectangle Renderer::GetLayoutButtonRectangle(ButtonID id)
    {
        switch (id)
        {
            case ButtonID::ReplayButton:
                return _Layout.ReplayButton;
                break;
            case ButtonID::ExitButton:
                return _Layout.ExitButton;
                break;
            case ButtonID::TurnIndicator:
                return _Layout.TurnIndicator;
                break;
            case ButtonID::Player1AIToggle:
                return _Layout.Player1AIToggle;
                break;
            case ButtonID::Player2AIToggle:
                return _Layout.Player2AIToggle;
                break;
            default:
                return Rectangle(0, 0, 0, 0);
        }
    }
    void Renderer::UpdateButton(ButtonID id)
    {
        Rectangle rectangle = GetLayoutButtonRectangle(id);
        float pos_x = (rectangle.X + (rectangle.Width  * 0.5)) * ViewInWorldScale[0] + ViewInWorldPosition[0];
        float pos_y = (rectangle.Y + (rectangle.Height * 0.5)) * ViewInWorldScale[1] + ViewInWorldPosition[1];
        float scale_x = rectangle.Width * 0.5f * ViewInWorldScale[0];
        float scale_y = rectangle.Height * 0.5f * ViewInWorldScale[1];
        ButtonModelMatrices[id] = Math::Matrix4x4::Translation(pos_x, pos_y, 0.01) // z=0.01 to be above the background
                * Math::Matrix4x4::Scale(scale_x, scale_y, 1);
    }
    void Renderer::UpdateView()
    {
        ViewMatrix = Math::Matrix4x4::Translation(BoardInViewPosition[0], BoardInViewPosition[1], 0)
                * Math::Matrix4x4::Scale(BoardInViewScale[0], BoardInViewScale[1], -1);
        ProjectionMatrix = Math::Matrix4x4();
    }
}
