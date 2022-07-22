#include "BufferGeneration.h"

#include <cmath>
#include <numbers>

namespace Reversi::BufferGeneration
{
    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateDisk(int circle_resolution, float thickness)
    {
        if (circle_resolution < 8)
            circle_resolution = 8;
        if (thickness < 0)
            thickness = -thickness;
        float half_thickness = thickness / 2;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // 0
        // center, front
        vertices.push_back(0); // position
        vertices.push_back(0);
        vertices.push_back(half_thickness);
        vertices.push_back(0); // normal
        vertices.push_back(0);
        vertices.push_back(1);
        vertices.push_back(0); // disk side color

        // 1
        // center, back
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(-half_thickness);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(-1);
        vertices.push_back(1);

        // 2
        // front (circle start), front-facing
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(half_thickness);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);
        vertices.push_back(0);

        // 3
        // back (circle start), back-facing
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(-half_thickness);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(-1);
        vertices.push_back(1);

        // 4
        // front (circle start), side-facing
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(half_thickness);
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);

        // 5
        // back (circle start), side-facing
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(-half_thickness);
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        constexpr int CENTER_FRONT_INDEX = 0;
        constexpr int CENTER_BACK_INDEX = 1;
        constexpr int FRONT_OFFSET = 0;
        constexpr int BACK_OFFSET = 1;
        constexpr int SIDE_FRONT_OFFSET = 2;
        constexpr int SIDE_BACK_OFFSET = 3;

        for (int i = 1; i < circle_resolution; i++)
        {
            double t = (2 * std::numbers::pi) * ((double)i/(double)circle_resolution);
            float x = (float)std::cos(t);
            float y = (float)std::sin(t);

            // 2 + i * 4
            // front, front-facing
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(half_thickness);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(1);
            vertices.push_back(0);

            // 2 + i * 4 + 1
            // back, back-facing
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(-half_thickness);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(-1);
            vertices.push_back(1);

            // 2 + i * 4 + 2
            // front, side-facing
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(half_thickness);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0);
            vertices.push_back(0);

            // 2 + i * 4 + 3
            // back, side-facing
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(-half_thickness);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0);
            vertices.push_back(1);

            int current_index = i * 4 + 2;
            int previous_index = current_index - 4;

            indices.push_back(CENTER_FRONT_INDEX);
            indices.push_back(previous_index + FRONT_OFFSET);
            indices.push_back(current_index + FRONT_OFFSET);

            indices.push_back(CENTER_BACK_INDEX);
            indices.push_back(previous_index + BACK_OFFSET);
            indices.push_back(current_index + BACK_OFFSET);

            indices.push_back(previous_index + SIDE_FRONT_OFFSET);
            indices.push_back(previous_index + SIDE_BACK_OFFSET);
            indices.push_back(current_index + SIDE_FRONT_OFFSET);

            indices.push_back(current_index + SIDE_FRONT_OFFSET);
            indices.push_back(previous_index + SIDE_BACK_OFFSET);
            indices.push_back(current_index + SIDE_BACK_OFFSET);
        }

        constexpr int first_index = 2;
        int last_index = (circle_resolution - 1) * 4 + 2;

        indices.push_back(CENTER_FRONT_INDEX);
        indices.push_back(last_index + FRONT_OFFSET);
        indices.push_back(first_index + FRONT_OFFSET);

        indices.push_back(CENTER_BACK_INDEX);
        indices.push_back(last_index + BACK_OFFSET);
        indices.push_back(first_index + BACK_OFFSET);

        indices.push_back(last_index + SIDE_FRONT_OFFSET);
        indices.push_back(last_index + SIDE_BACK_OFFSET);
        indices.push_back(first_index + SIDE_FRONT_OFFSET);

        indices.push_back(first_index + SIDE_FRONT_OFFSET);
        indices.push_back(last_index + SIDE_BACK_OFFSET);
        indices.push_back(first_index + SIDE_BACK_OFFSET);

        return std::make_tuple(vertices, indices);
    }

    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateDiskHalf(int circle_resolution, float thickness)
    {
        if (circle_resolution < 8)
            circle_resolution = 8;
        if (thickness < 0)
            thickness = -thickness;
        float half_thickness = thickness / 2;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // 0
        // center, front
        vertices.push_back(0); // position
        vertices.push_back(0);
        vertices.push_back(half_thickness);
        vertices.push_back(0); // normal
        vertices.push_back(0);
        vertices.push_back(1);

        // 1
        // front (circle start), front-facing
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(half_thickness);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 2
        // front (circle start), side-facing
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(half_thickness);
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(0);

        // 3
        // back (circle start), side-facing
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(0);

        constexpr int CENTER_FRONT_INDEX = 0;
        constexpr int FRONT_OFFSET = 0;
        constexpr int SIDE_FRONT_OFFSET = 1;
        constexpr int SIDE_BACK_OFFSET = 2;

        for (int i = 1; i < circle_resolution; i++)
        {
            double t = (2 * std::numbers::pi) * ((double)i/(double)circle_resolution);
            float x = (float)std::cos(t);
            float y = (float)std::sin(t);

            // 1 + i * 3
            // front, front-facing
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(half_thickness);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(1);

            // 1 + i * 3 + 1
            // front, side-facing
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(half_thickness);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0);

            // 1 + i * 3 + 2
            // back, side-facing
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0);

            int current_index = i * 3 + 1;
            int previous_index = current_index - 3;

            indices.push_back(CENTER_FRONT_INDEX);
            indices.push_back(previous_index + FRONT_OFFSET);
            indices.push_back(current_index + FRONT_OFFSET);

            indices.push_back(previous_index + SIDE_FRONT_OFFSET);
            indices.push_back(previous_index + SIDE_BACK_OFFSET);
            indices.push_back(current_index + SIDE_FRONT_OFFSET);

            indices.push_back(current_index + SIDE_FRONT_OFFSET);
            indices.push_back(previous_index + SIDE_BACK_OFFSET);
            indices.push_back(current_index + SIDE_BACK_OFFSET);
        }

        constexpr int first_index = 1;
        int last_index = (circle_resolution - 1) * 3 + 1;

        indices.push_back(CENTER_FRONT_INDEX);
        indices.push_back(last_index + FRONT_OFFSET);
        indices.push_back(first_index + FRONT_OFFSET);

        indices.push_back(last_index + SIDE_FRONT_OFFSET);
        indices.push_back(last_index + SIDE_BACK_OFFSET);
        indices.push_back(first_index + SIDE_FRONT_OFFSET);

        indices.push_back(first_index + SIDE_FRONT_OFFSET);
        indices.push_back(last_index + SIDE_BACK_OFFSET);
        indices.push_back(first_index + SIDE_BACK_OFFSET);

        return std::make_tuple(vertices, indices);
    }

    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateDiskPlace(int circle_resolution, float padding)
    {
        if (circle_resolution < 8)
            circle_resolution = 8;
        if (padding < 0)
            padding = 0;
        if (padding > 1)
            padding = 1;
        float radius = 1 - padding;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // 0
        // top-right
        vertices.push_back(1); // positions
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(0); // normal
        vertices.push_back(0);
        vertices.push_back(1);

        // 1
        // top-left
        vertices.push_back(-1);
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 2
        // bottom-left
        vertices.push_back(-1);
        vertices.push_back(-1);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 3
        // bottom-right
        vertices.push_back(1);
        vertices.push_back(-1);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 4
        // circle start
        vertices.push_back(radius);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // right triangle
        indices.push_back(0); // top-right
        indices.push_back(4); // circle start
        indices.push_back(3); // bottom-right

        int current_corner_index = 0;

        float last_x = 1;
        float last_y = 0;

        for (int i = 1; i < circle_resolution; i++)
        {
            double t = (2 * std::numbers::pi) * ((double)i/(double)circle_resolution);
            float x = (float)std::cos(t);
            float y = (float)std::sin(t);

            int current_index = i + 4;

            bool corner_updated = false;
            switch (current_corner_index)
            {
                case 0:
                if (y < last_y)
                {
                    current_corner_index = 1;
                    corner_updated = true;
                }
                break;
                case 1:
                if (x > last_x)
                {
                    current_corner_index = 2;
                    corner_updated = true;
                }
                break;
                case 2:
                if (y > last_y)
                {
                    current_corner_index = 3;
                    corner_updated = true;
                }
                break;
            }
            if (corner_updated)
            {
                // top, left and bottom triangle
                indices.push_back(current_corner_index);
                indices.push_back(current_index - 1);
                indices.push_back(current_corner_index - 1);
            }

            // i + 4
            vertices.push_back(x * radius);
            vertices.push_back(y * radius);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(1);

            indices.push_back(current_corner_index);
            indices.push_back(current_index);
            indices.push_back(current_index - 1);

            last_x = x;
            last_y = y;
        }
        indices.push_back(3); // bottom-right corner of the square
        indices.push_back(4); // first vertex on the circle
        indices.push_back(circle_resolution - 1 + 4); // last vertex on the circle

        return std::make_tuple(vertices, indices);
    }

    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateSquare()
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // 0
        // bottom-left
        vertices.push_back(-1); // positions
        vertices.push_back(-1);
        vertices.push_back(0);
        vertices.push_back(0); // normal
        vertices.push_back(0);
        vertices.push_back(1);

        // 1
        // bottom-right
        vertices.push_back(1);
        vertices.push_back(-1);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 2
        // top-right
        vertices.push_back(1);
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 3
        // top-left
        vertices.push_back(-1);
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);

        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(3);

        return std::make_tuple(vertices, indices);
    }

    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateRoundedSquare(int corner_resolution, float corner_radius_x, float corner_radius_y)
    {
        if (corner_resolution < 2)
            corner_resolution = 2;
        if (corner_radius_x < 0)
            corner_radius_x = -corner_radius_x;
        if (corner_radius_x > 1)
            corner_radius_x = 1;
        if (corner_radius_y < 0)
            corner_radius_y = -corner_radius_y;
        if (corner_radius_y > 1)
            corner_radius_y = 1;

        float corner_start_x = 1 - corner_radius_x;
        float corner_start_y = 1 - corner_radius_y;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // 0
        // center
        vertices.push_back(0); // position
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0); // normal
        vertices.push_back(0);
        vertices.push_back(1);

        // 1
        // top-right corner start (on the right edge)
        vertices.push_back(1);
        vertices.push_back(corner_start_y);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 2
        // top-left corner start (on the top edge)
        vertices.push_back(-corner_start_x);
        vertices.push_back(1);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 3
        // bottom-left corner start (on the left edge)
        vertices.push_back(-1);
        vertices.push_back(-corner_start_y);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 4
        // bottom-right corner start (on the bottom edge)
        vertices.push_back(corner_start_x);
        vertices.push_back(-1);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        constexpr int TOP_RIGHT_CORNER_OFFSET = 0;
        constexpr int TOP_LEFT_CORNER_OFFSET = 1;
        constexpr int BOTTOM_LEFT_CORNER_OFFSET = 2;
        constexpr int BOTTOM_RIGHT_CORNER_OFFSET = 3;

        for (int i = 1; i < corner_resolution; i++)
        {
            double t = (double)i/(double)(corner_resolution - 1);
            double t1 = (2 * std::numbers::pi) * (t * 0.25);
            double t2 = (2 * std::numbers::pi) * (0.25 + t * 0.25);
            double t3 = (2 * std::numbers::pi) * (0.50 + t * 0.25);
            double t4 = (2 * std::numbers::pi) * (0.75 + t * 0.25);

            int current_index = 1 + i * 4;
            int previous_index = current_index - 4;

            // top-right corner
            float x = (float)std::cos(t1);
            float y = (float)std::sin(t1);
            vertices.push_back(corner_start_x + x * corner_radius_x);
            vertices.push_back(corner_start_y + y * corner_radius_y);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(1);

            // top-left corner
            x = (float)std::cos(t2);
            y = (float)std::sin(t2);
            vertices.push_back(-corner_start_x + x * corner_radius_x);
            vertices.push_back(corner_start_y + y * corner_radius_y);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(1);

            // bottom-left corner
            x = (float)std::cos(t3);
            y = (float)std::sin(t3);
            vertices.push_back(-corner_start_x + x * corner_radius_x);
            vertices.push_back(-corner_start_y + y * corner_radius_y);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(1);

            // bottom-right corner
            x = (float)std::cos(t4);
            y = (float)std::sin(t4);
            vertices.push_back(corner_start_x + x * corner_radius_x);
            vertices.push_back(-corner_start_y + y * corner_radius_y);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(1);

            indices.push_back(current_index + TOP_RIGHT_CORNER_OFFSET);
            indices.push_back(0);
            indices.push_back(previous_index + TOP_RIGHT_CORNER_OFFSET);

            indices.push_back(current_index + TOP_LEFT_CORNER_OFFSET);
            indices.push_back(0);
            indices.push_back(previous_index + TOP_LEFT_CORNER_OFFSET);

            indices.push_back(current_index + BOTTOM_LEFT_CORNER_OFFSET);
            indices.push_back(0);
            indices.push_back(previous_index + BOTTOM_LEFT_CORNER_OFFSET);

            indices.push_back(current_index + BOTTOM_RIGHT_CORNER_OFFSET);
            indices.push_back(0);
            indices.push_back(previous_index + BOTTOM_RIGHT_CORNER_OFFSET);
        }

        constexpr int first_index = 1;
        int last_index = 1 + (corner_resolution - 1) * 4;

        indices.push_back(first_index + TOP_RIGHT_CORNER_OFFSET);
        indices.push_back(0); // center
        indices.push_back(last_index + BOTTOM_RIGHT_CORNER_OFFSET);

        indices.push_back(first_index + TOP_LEFT_CORNER_OFFSET);
        indices.push_back(0);
        indices.push_back(last_index + TOP_RIGHT_CORNER_OFFSET);

        indices.push_back(first_index + BOTTOM_LEFT_CORNER_OFFSET);
        indices.push_back(0);
        indices.push_back(last_index + TOP_LEFT_CORNER_OFFSET);

        indices.push_back(first_index + BOTTOM_RIGHT_CORNER_OFFSET);
        indices.push_back(0);
        indices.push_back(last_index + BOTTOM_LEFT_CORNER_OFFSET);

        return std::make_tuple(vertices, indices);
    }

    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateReplayIcon(float circle_resolution,
                                                                                 float z,
                                                                                 float line_radius,
                                                                                 float line_thickness,
                                                                                 float arrow_head_width,
                                                                                 float arrow_head_height)
    {
        if (circle_resolution < 7)
            circle_resolution = 7;

        float line_outer_radius = line_radius + line_thickness / 2;
        float line_inner_radius = line_radius - line_thickness / 2;
        float arrow_outer_side = line_radius + arrow_head_width / 2;
        float arrow_inner_side = line_radius - arrow_head_width / 2;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // 0
        // arrow head top
        vertices.push_back(-line_radius);
        vertices.push_back(arrow_head_height);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 1
        // arrow head left
        vertices.push_back(-arrow_outer_side);
        vertices.push_back(0);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 2
        // arrow head right
        vertices.push_back(-arrow_inner_side);
        vertices.push_back(0);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);

        // 3
        // outer top
        vertices.push_back(0);
        vertices.push_back(line_outer_radius);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 4
        // inner top
        vertices.push_back(0);
        vertices.push_back(line_inner_radius);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        constexpr int LINE_OUTER_OFFSET = 0;
        constexpr int LINE_INNER_OFFSET = 1;

        for (int i = 1; i < circle_resolution; i++)
        {
            double t = ((double)i / (double)(circle_resolution - 1)) * (std::numbers::pi * 1.5);
            float x = std::sin(t);
            float y = std::cos(t);

            // 3 + i * 2
            vertices.push_back(x * line_outer_radius);
            vertices.push_back(y * line_outer_radius);
            vertices.push_back(z);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(1);

            // 3 + i * 2 + 1
            vertices.push_back(x * line_inner_radius);
            vertices.push_back(y * line_inner_radius);
            vertices.push_back(z);
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(1);

            int current_index = 3 + i * 2;
            int previous_index = current_index - 2;

            indices.push_back(previous_index + LINE_OUTER_OFFSET);
            indices.push_back(previous_index + LINE_INNER_OFFSET);
            indices.push_back(current_index  + LINE_INNER_OFFSET);

            indices.push_back(previous_index + LINE_OUTER_OFFSET);
            indices.push_back(current_index  + LINE_INNER_OFFSET);
            indices.push_back(current_index  + LINE_OUTER_OFFSET);
        }

        return std::make_tuple(vertices, indices);
    }

    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateExitIcon(float z,
                                                                               float line_offset,
                                                                               float line_thickness,
                                                                               float arrow_head_width,
                                                                               float arrow_head_height)
    {
        float half_thickness = line_thickness / 2;
        float line_outer_offset = line_offset + half_thickness;
        float line_inner_offset = line_offset - half_thickness;
        float arrow_head_side = arrow_head_width / 2;
        float arrow_head_end = line_inner_offset + arrow_head_height;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // 0
        // top-left of top quad AND top-left of left quad
        vertices.push_back(-line_outer_offset);
        vertices.push_back(line_outer_offset);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 1
        // bottom-left of top quad AND top-right of left quad
        vertices.push_back(-line_inner_offset);
        vertices.push_back(line_inner_offset);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 2
        // bottom-right of top quad
        vertices.push_back(line_inner_offset);
        vertices.push_back(line_inner_offset);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 3
        // top-right of top quad
        vertices.push_back(line_inner_offset);
        vertices.push_back(line_outer_offset);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);

        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(3);

        // 4
        // bottom-left of left quad AND bottom-left of bottom quad
        vertices.push_back(-line_outer_offset);
        vertices.push_back(-line_outer_offset);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 5
        // bottom-right of left quad AND top-left of bottom quad
        vertices.push_back(-line_inner_offset);
        vertices.push_back(-line_inner_offset);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        indices.push_back(0);
        indices.push_back(4);
        indices.push_back(5);

        indices.push_back(0);
        indices.push_back(5);
        indices.push_back(1);

        // 6
        // bottom-right of left quad
        vertices.push_back(line_inner_offset);
        vertices.push_back(-line_outer_offset);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 7
        // top-right of left quad
        vertices.push_back(line_inner_offset);
        vertices.push_back(-line_inner_offset);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        indices.push_back(5);
        indices.push_back(4);
        indices.push_back(6);

        indices.push_back(5);
        indices.push_back(6);
        indices.push_back(7);

        // 8
        // top-left of arrow line
        vertices.push_back(-half_thickness);
        vertices.push_back(half_thickness);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 9
        // bottom-left of arrow line
        vertices.push_back(-half_thickness);
        vertices.push_back(-half_thickness);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 10
        // bottom-right of arrow line
        vertices.push_back(line_inner_offset);
        vertices.push_back(-half_thickness);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 11
        // top-right of arrow line
        vertices.push_back(line_inner_offset);
        vertices.push_back(half_thickness);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        indices.push_back(8);
        indices.push_back(9);
        indices.push_back(10);

        indices.push_back(8);
        indices.push_back(10);
        indices.push_back(11);

        // 12
        // arrow head end (right)
        vertices.push_back(arrow_head_end);
        vertices.push_back(0);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 13
        // arrow top
        vertices.push_back(line_inner_offset);
        vertices.push_back(arrow_head_side);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 14
        // arrow bottom
        vertices.push_back(line_inner_offset);
        vertices.push_back(-arrow_head_side);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        indices.push_back(12);
        indices.push_back(13);
        indices.push_back(14);

        return std::make_tuple(vertices, indices);
    }

    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateAIIcon(float z)
    {
        float face_x = 0.5;
        float face_y = 0.25;
        float eye_near_x = 0.2;
        float eye_far_x = 0.3;
        float eye_y = 0.06;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // 0
        // top-left of top quad
        vertices.push_back(-face_x);
        vertices.push_back(face_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 1
        // bottom-left of top quad
        vertices.push_back(-face_x);
        vertices.push_back(eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 2
        // bottom-right of top quad
        vertices.push_back(face_x);
        vertices.push_back(eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 3
        // top-right of top quad
        vertices.push_back(face_x);
        vertices.push_back(face_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);

        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(3);

        // 4
        // top-left of bottom quad
        vertices.push_back(-face_x);
        vertices.push_back(-eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 5
        // bottom-left of bottom quad
        vertices.push_back(-face_x);
        vertices.push_back(-face_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 6
        // bottom-right of bottom quad
        vertices.push_back(face_x);
        vertices.push_back(-face_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 7
        // top-right of bottom quad
        vertices.push_back(face_x);
        vertices.push_back(-eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        indices.push_back(4);
        indices.push_back(5);
        indices.push_back(6);

        indices.push_back(4);
        indices.push_back(6);
        indices.push_back(7);

        // 8
        // top-left of left quad
        vertices.push_back(-face_x);
        vertices.push_back(eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 9
        // bottom-left of left quad
        vertices.push_back(-face_x);
        vertices.push_back(-eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 10
        // bottom-right of left quad
        vertices.push_back(-eye_far_x);
        vertices.push_back(-eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 11
        // top-right of left quad
        vertices.push_back(-eye_far_x);
        vertices.push_back(eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        indices.push_back(8);
        indices.push_back(9);
        indices.push_back(10);

        indices.push_back(8);
        indices.push_back(10);
        indices.push_back(11);

        // 12
        // top-left of right quad
        vertices.push_back(eye_far_x);
        vertices.push_back(eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 13
        // bottom-left of right quad
        vertices.push_back(eye_far_x);
        vertices.push_back(-eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 14
        // bottom-right of right quad
        vertices.push_back(face_x);
        vertices.push_back(-eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 15
        // top-right of right quad
        vertices.push_back(face_x);
        vertices.push_back(eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        indices.push_back(12);
        indices.push_back(13);
        indices.push_back(14);

        indices.push_back(12);
        indices.push_back(14);
        indices.push_back(15);

        // 16
        // top-left of middle quad
        vertices.push_back(-eye_near_x);
        vertices.push_back(eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 17
        // bottom-left of middle quad
        vertices.push_back(-eye_near_x);
        vertices.push_back(-eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 18
        // bottom-right of middle quad
        vertices.push_back(eye_near_x);
        vertices.push_back(-eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        // 19
        // top-right of middle quad
        vertices.push_back(eye_near_x);
        vertices.push_back(eye_y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        indices.push_back(16);
        indices.push_back(17);
        indices.push_back(18);

        indices.push_back(16);
        indices.push_back(18);
        indices.push_back(19);

        return std::make_tuple(vertices, indices);
    }
}
