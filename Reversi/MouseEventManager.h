#pragma once

#include "Reversi.dec.h"

#include <functional>
#include <list>
#include <map>
#include <memory>

namespace Reversi
{
    class MouseEventManager final
    {
    public:
        /// @param one_object_at_a_time Whether to only hover/click the item in front.
        /// @param indexing_margin Use a number that moderately divides the screen, like 1/10 of the screen.
        ///                        Too high and it won't optimize speed, too low and it will take a lot of memory.
        MouseEventManager(bool one_object_at_a_time, double indexing_margin);
        ~MouseEventManager();

        MouseEventManager(const MouseEventManager&) = delete;
        MouseEventManager(MouseEventManager&&) = delete;
        MouseEventManager& operator=(const MouseEventManager&) = delete;
        MouseEventManager& operator=(MouseEventManager&&) = delete;

        enum MouseEvent
        {
            Leave = 0,
            Enter = 1,
            Down = 2,
            Up = 3,
            Click = 4
        };

        /// @param x Top-left position x
        /// @param y Top-left position y
        /// @return Id to be used in UpdateClickableObject.
        int AddRectangle(
            double x, double y, double width, double height,
            std::function<void(MouseEvent)> callback
        );
        /// @param x Top-left position x
        /// @param y Top-left position y
        /// @return Id to be used in UpdateClickableObject.
        int AddOval(
            double x, double y, double width, double height,
            std::function<void(MouseEvent)> callback
        );

        /// @param id The integer returned by AddRectangle or AddOval.
        /// @param x Top-left position x
        /// @param y Top-left position y
        bool UpdateClickableObject(
            int id, double x, double y, double width, double height
        );

        /// @param id The integer returned by AddRectangle or AddOval.
        bool RemoveClickableObject(int id);

        /// @param indexing_margin Use a number that moderately divides the screen, like 1/10 of the screen.
        ///                       Too high and it won't optimize speed, too low and it will take a lot of memory.
        void Clear(double indexing_margin);

        void UpdateMousePosition(double x, double y);
        void MouseDown(double x, double y);
        void MouseUp(double x, double y);
    private:
        struct Area
        {
        public:
            double MinX, MinY, MaxX, MaxY;
        };

        class ClickableObject
        {
        public:
            ClickableObject(std::function<void(MouseEvent)> callback);
            virtual ~ClickableObject() = default;
            void Call(MouseEvent);
            virtual bool Test(double x, double y) = 0;
            virtual Area GetArea() = 0;
            virtual void UpdateLayout(double x, double y, double width, double height) = 0;
        private:
            std::function<void(MouseEvent)> Callback;
        };

        class ClickableRectangle : public ClickableObject
        {
        public:
            ClickableRectangle(
                double x, double y, double width, double height,
                std::function<void(MouseEvent)> Callback
            );
            virtual bool Test(double x, double y) override;
            virtual Area GetArea() override;
            virtual void UpdateLayout(double x, double y, double width, double height) override;
        protected:
            double MinX, MinY, MaxX, MaxY;
        };

        class ClickableOval : public ClickableObject
        {
        public:
            ClickableOval(
                double x, double y, double width, double height,
                std::function<void(MouseEvent)> callback
            );
            virtual bool Test(double x, double y) override;
            virtual Area GetArea() override;
            virtual void UpdateLayout(double x, double y, double width, double height) override;
        protected:
            double CenterX, CenterY, RadiusX, RadiusY;
        };

        struct IntVec2 final
        {
        public:
            int x;
            int y;
            auto operator<=>(const IntVec2&) const = default;
        };

        bool OneObjectAtATime;
        double IndexingMargin;
        int NextId;
        std::map<IntVec2, std::list<std::shared_ptr<ClickableObject>>> ClickableObjects;
        std::map<int, std::shared_ptr<ClickableObject>> ClickableObjectsById;
        std::map<std::shared_ptr<ClickableObject>, bool> AreClickableObjectsBeingHovered;
        std::list<std::shared_ptr<ClickableObject>> HoveredClickableObjects;
        std::map<std::shared_ptr<ClickableObject>, bool> AreClickableObjectsBeingClicked;

        void AddToClickableObjects(std::shared_ptr<ClickableObject>);
        void RemoveFromClickableObjects(std::shared_ptr<ClickableObject>);
        IntVec2 GetIndexing(double x, double y);
    };
}
