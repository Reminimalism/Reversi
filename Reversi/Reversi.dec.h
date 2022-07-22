/*
    Copyright (C) 2022  Majidzadeh (hashpragmaonce@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

/*

Game contains
    Window contains
        MouseEventManager
    Board contains
        Logic
        Renderer contains
            ShaderProgram
            Model

Board    uses Window
Board    uses AI
Renderer uses Window

*/

namespace Reversi
{
    enum Side : char { None=0, Black=1, White=2 };
    class Window;
    class MouseEventManager;
    class Board;
    class Logic;
    class AI;
    class DecisionTreeAI;
    class EvolvingAI;
    class ShaderProgram;
    class Renderer;
}
