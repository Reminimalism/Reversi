#include "MouseEventManager.h"

namespace Reversi
{
    MouseEventManager::MouseEventManager(bool one_object_at_a_time, double indexing_margin)
        : OneObjectAtATime(one_object_at_a_time), IndexingMargin(indexing_margin), NextId(0) {}
    MouseEventManager::~MouseEventManager() {}

    int MouseEventManager::AddRectangle(
            double x, double y, double width, double height,
            std::function<void(MouseEvent)> callback
        )
    {
        auto rect = std::make_shared<ClickableRectangle>(x, y, width, height, callback);
        ClickableObjectsById[NextId] = rect;
        AddToClickableObjects(rect);
        AreClickableObjectsBeingHovered[rect] = false;
        AreClickableObjectsBeingClicked[rect] = false;
        return NextId++;
    }

    int MouseEventManager::AddOval(
            double x, double y, double width, double height,
            std::function<void(MouseEvent)> callback
        )
    {
        auto oval = std::make_shared<ClickableOval>(x, y, width, height, callback);
        ClickableObjectsById[NextId] = oval;
        AddToClickableObjects(oval);
        AreClickableObjectsBeingHovered[oval] = false;
        AreClickableObjectsBeingClicked[oval] = false;
        return NextId++;
    }

    bool MouseEventManager::UpdateClickableObject(
            int id, double x, double y, double width, double height
        )
    {
        if (ClickableObjectsById.contains(id))
        {
            auto obj = ClickableObjectsById[id];
            RemoveFromClickableObjects(obj);
            obj->UpdateLayout(x, y, width, height);
            AddToClickableObjects(obj);
            return true;
        }
        return false;
    }

    bool MouseEventManager::RemoveClickableObject(int id)
    {
        if (ClickableObjectsById.contains(id))
        {
            auto obj = ClickableObjectsById[id];
            RemoveFromClickableObjects(obj);
            AreClickableObjectsBeingHovered.erase(obj);
            for (auto i = HoveredClickableObjects.begin(); i != HoveredClickableObjects.end(); ++i)
            {
                if ((*i) == obj)
                {
                    (*i)->Call(MouseEvent::Leave);
                    HoveredClickableObjects.erase(i);
                    break;
                }
            }
            AreClickableObjectsBeingClicked.erase(obj);
            ClickableObjectsById.erase(id);
            return true;
        }
        return false;
    }

    void MouseEventManager::Clear(double indexing_margin)
    {
        ClickableObjects.clear();
        ClickableObjectsById.clear();
        AreClickableObjectsBeingHovered.clear();
        HoveredClickableObjects.clear();
        AreClickableObjectsBeingClicked.clear();
        this->IndexingMargin = indexing_margin;
        NextId = 0;
    }

    void MouseEventManager::UpdateMousePosition(double x, double y)
    {
        for (auto i = HoveredClickableObjects.begin(); i != HoveredClickableObjects.end();)
        {
            if (!(*i)->Test(x, y))
            {
                (*i)->Call(MouseEvent::Leave);
                AreClickableObjectsBeingHovered[*i] = false;
                AreClickableObjectsBeingClicked[*i] = false;
                auto temp = i;
                ++i;
                HoveredClickableObjects.erase(temp);
            }
            else
            {
                ++i;
            }
        }
        for (auto item : ClickableObjects[GetIndexing(x, y)])
        {
            if (!AreClickableObjectsBeingHovered[item] && item->Test(x, y))
            {
                if (OneObjectAtATime && HoveredClickableObjects.size() != 0)
                {
                    for (auto& other_item : HoveredClickableObjects)
                    {
                        other_item->Call(MouseEvent::Leave);
                        AreClickableObjectsBeingHovered[other_item] = false;
                        AreClickableObjectsBeingClicked[other_item] = false;
                    }
                    HoveredClickableObjects.clear();
                }
                item->Call(MouseEvent::Enter);
                AreClickableObjectsBeingHovered[item] = true;
                HoveredClickableObjects.push_back(item);
                if (OneObjectAtATime)
                    return;
            }
            else if (OneObjectAtATime && AreClickableObjectsBeingHovered[item] && item->Test(x, y))
                return;
        }
    }

    void MouseEventManager::MouseDown(double x, double y)
    {
        UpdateMousePosition(x, y);
        for (auto item : ClickableObjects[GetIndexing(x, y)])
        {
            if (item->Test(x, y))
            {
                item->Call(MouseEvent::Down);
                AreClickableObjectsBeingClicked[item] = true;
                if (OneObjectAtATime)
                    return;
            }
        }
    }

    void MouseEventManager::MouseUp(double x, double y)
    {
        UpdateMousePosition(x, y);
        for (auto item : ClickableObjects[GetIndexing(x, y)])
        {
            if (item->Test(x, y))
            {
                item->Call(MouseEvent::Up);
                if (AreClickableObjectsBeingClicked[item])
                {
                    item->Call(MouseEvent::Click);
                }
                AreClickableObjectsBeingClicked[item] = false;
                if (OneObjectAtATime)
                    return;
            }
        }
    }

    MouseEventManager::ClickableObject::ClickableObject(std::function<void(MouseEvent)> callback)
        : Callback(callback)
    {
    }

    void MouseEventManager::ClickableObject::Call(MouseEvent e)
    {
        Callback(e);
    }

    MouseEventManager::ClickableRectangle::ClickableRectangle(
            double x, double y, double width, double height,
            std::function<void(MouseEvent)> callback
        ) : ClickableObject(callback), MinX(x), MinY(y), MaxX(x + width), MaxY(y + height)
    {
        UpdateLayout(x, y, width, height);
    }

    MouseEventManager::ClickableOval::ClickableOval(
            double x, double y, double width, double height,
            std::function<void(MouseEvent)> callback
        ) : ClickableObject(callback)
    {
        UpdateLayout(x, y, width, height);
    }

    bool MouseEventManager::ClickableRectangle::Test(double x, double y)
    {
        return MinX <= x && x <= MaxX && MinY <= y && y <= MaxY;
    }

    bool MouseEventManager::ClickableOval::Test(double x, double y)
    {
        double dx = (x - CenterX) / RadiusX;
        double dy = (y - CenterY) / RadiusY;
        return (dx * dx + dy * dy) <= 1;
    }

    MouseEventManager::Area MouseEventManager::ClickableRectangle::GetArea()
    {
        Area result;
        result.MinX = MinX;
        result.MaxX = MaxX;
        result.MinY = MinY;
        result.MaxY = MaxY;
        return result;
    }

    MouseEventManager::Area MouseEventManager::ClickableOval::GetArea()
    {
        Area result;
        result.MinX = CenterX - RadiusX;
        result.MaxX = CenterX + RadiusX;
        result.MinY = CenterY - RadiusY;
        result.MaxY = CenterY + RadiusY;
        return result;
    }

    void MouseEventManager::ClickableRectangle::UpdateLayout(double x, double y, double width, double height)
    {
        MinX = x;
        MinY = y;
        MaxX = x + width;
        MaxY = y + height;
    }

    void MouseEventManager::ClickableOval::UpdateLayout(double x, double y, double width, double height)
    {
        RadiusX = width * 0.5;
        RadiusY = height * 0.5;
        CenterX = x + RadiusX;
        CenterY = y + RadiusY;
    }

    inline void MouseEventManager::AddToClickableObjects(std::shared_ptr<ClickableObject> obj)
    {
        auto area = obj->GetArea();
        int sx = (int)(area.MinX / IndexingMargin);
        int sy = (int)(area.MinY / IndexingMargin);
        int ex = (int)(area.MaxX / IndexingMargin);
        int ey = (int)(area.MaxY / IndexingMargin);
        for (int x = sx; x <= ex; x++)
            for (int y = sy; y <= ey; y++)
                ClickableObjects[{x, y}].push_front(obj);
    }

    inline void MouseEventManager::RemoveFromClickableObjects(std::shared_ptr<ClickableObject> obj)
    {
        auto area = obj->GetArea();
        int sx = (int)(area.MinX / IndexingMargin);
        int sy = (int)(area.MinY / IndexingMargin);
        int ex = (int)(area.MaxX / IndexingMargin);
        int ey = (int)(area.MaxY / IndexingMargin);
        for (int x = sx; x <= ex; x++)
        {
            for (int y = sy; y <= ey; y++)
            {
                auto l = ClickableObjects[{x, y}];
                for (auto i = l.begin(); i != l.end(); ++i)
                {
                    if ((*i) == obj)
                    {
                        l.erase(i);
                        break;
                    }
                }
            }
        }
    }

    inline MouseEventManager::IntVec2 MouseEventManager::GetIndexing(double x, double y)
    {
        return IntVec2 {(int)(x / IndexingMargin), (int)(y / IndexingMargin)};
    }
}
