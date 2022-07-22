#pragma once

#include "Reversi.dec.h"

#include "Math.h"
#include "Model.h"
#include "ShaderProgram.h"

#include "../glad/include/glad/glad.h"

#include <map>
#include <memory>

namespace Reversi
{
    class Renderer final
    {
    public:
        explicit Renderer(std::shared_ptr<Window>);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        struct Rectangle final
        {
        public:
            /// @param X The starting X point, -1 being leftmost and 1 being rightmost.
            /// @param Y The starting Y point, -1 being the bottom and 1 being the top.
            /// @param Width The width, the size of screen being 2.
            /// @param Height The height, the size of screen being 2.
            Rectangle(double X, double Y, double Width, double Height);
            double X;
            double Y;
            double Width;
            double Height;
        };

        struct Layout final
        {
        public:
            Layout(
                Rectangle ReplayButton,
                Rectangle ExitButton,
                Rectangle TurnIndicator,
                Rectangle Player1AIToggle,
                Rectangle Player2AIToggle,
                Rectangle BottomLeftSlot,
                Rectangle TopRightSlot
            );
            Rectangle ReplayButton;
            Rectangle ExitButton;
            Rectangle TurnIndicator;
            Rectangle Player1AIToggle;
            Rectangle Player2AIToggle;
            Rectangle BottomLeftSlot;
            Rectangle TopRightSlot;
        };

        enum ButtonID
        {
            ReplayButton,
            ExitButton,
            TurnIndicator,
            Player1AIToggle,
            Player2AIToggle,
            Player1SideVirtualButton
        };

        struct ButtonState final
        {
        public:
            ButtonState(double Hover = 0, double Press = 0, double Highlight = 0);
            double Hover;
            double Press;
            double Highlight;
        };

        struct SlotState final
        {
        public:
            SlotState(Side Current = Side::None, Side Next = Side::None, double Transition = 0);
            Side Current;
            Side Next;
            double Transition;
        };

        void SetLayout(Layout);
        Layout GetLayout();
        void SetButtonState(ButtonID, ButtonState);
        ButtonState GetButtonState(ButtonID);
        void SetSlotState(int x, int y, SlotState);
        SlotState GetSlotState(int x, int y);
        void Render();
    private:
        std::shared_ptr<Window> _Window;

        Layout _Layout;
        std::map<ButtonID, ButtonState> ButtonStates;
        SlotState SlotStates[64];

        Model DiskHalf;
        Model DiskPlace;
        Model Square;
        Model RoundedSquare;
        Model ReplayIcon;
        Model ExitIcon;
        Model AIIcon;

        ShaderProgram Program;
        GLint ProgramModelUniform;
        GLint ProgramViewUniform;
        GLint ProgramProjectionUniform;
        GLint ProgramColorUniform;

        Math::Matrix4x4 SlotModelMatrices[64];
        Math::Matrix4x4 DiskBlackModelMatrices[64];
        Math::Matrix4x4 DiskWhiteModelMatrices[64];
        Math::Matrix4x4 LeftPanelMatrix;
        Math::Matrix4x4 RightPanelMatrix;
        Math::Matrix4x4 BottomPanelMatrix;
        Math::Matrix4x4 TopPanelMatrix;
        std::map<ButtonID, Math::Matrix4x4> ButtonModelMatrices;
        Math::Matrix4x4 ViewMatrix;
        Math::Matrix4x4 ProjectionMatrix;

        /// Gets updated by Renderer::SetLayout.
        float LayoutDiskScale[2];
        /// Gets updated by Renderer::SetLayout.
        float BoardInViewPosition[2];
        /// Is also the half of the size of the board in view.
        /// Gets updated by Renderer::SetLayout.
        float BoardInViewScale[2];
        /// Gets updated by Renderer::SetLayout.
        float ViewInWorldPosition[2];
        /// Gets updated by Renderer::SetLayout.
        float ViewInWorldScale[2];

        void UpdateSlot(int x, int y);
        Rectangle GetLayoutButtonRectangle(ButtonID);
        void UpdateButton(ButtonID);
        void UpdateView();
    };
}
