#pragma once

#include <tuple>
#include <vector>

namespace Reversi::BufferGeneration
{
    /// @param circle_resolution The number of vertices around the disk. The minimum is 8.
    /// @param thickness The thickness of the disk.
    /// @return Vertices of vec3 position, vec3 normal and float disk side color, and triangles' indices.
    ///         Position xy is in range [-1, 1] and z in range [-thickness/2, thickness/2].
    ///         The positive side of the position.z (front) is the black side (0 disk side color).
    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateDisk(int circle_resolution, float thickness);
    /// @param circle_resolution The number of vertices around the disk. The minimum is 8.
    /// @param thickness The thickness of the disk.
    /// @return Vertices of vec3 position, vec3 normal, and triangles' indices.
    ///         Position xy is in range [-1, 1] and z in range [0, thickness/2].
    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateDiskHalf(int circle_resolution, float thickness);
    /// @brief Generates a single board place for a single disk.
    /// @param circle_resolution The number of vertices around the circle. The minimum is 8.
    /// @param padding The padding around a single disk in the square area, in other words, 1-radius. Must be in range [0, 1].
    /// @return Vertices of vec3 position and vec3 normal, and triangles' indices.
    ///         Position xy is in range [-1, 1]. Normal is (0, 0, 1) in all vertices.
    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateDiskPlace(int circle_resolution, float padding);
    /// @return Vertices of vec3 position and vec3 normal, and triangles' indices.
    ///         Position xy is in range [-1, 1]. Normal is (0, 0, 1) in all vertices.
    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateSquare();
    /// @param corner_radius_x The x radius of the round corners. In range [0, 1].
    /// @param corner_radius_y The y radius of the round corners. In range [0, 1].
    /// @param corner_resolution The number of vertices on each corner. The minimum is 2.
    /// @return Vertices of vec3 position and vec3 normal, and triangles' indices.
    ///         Position xy is in range [-1, 1]. Normal is (0, 0, 1) in all vertices.
    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateRoundedSquare(int corner_resolution, float corner_radius_x, float corner_radius_y);
    /// @param circle_resolution The number of vertices around the arrow line. The minimum is 7.
    /// @param z The z of vertices.
    /// @param line_radius The radius of the round arrow line.
    /// @param line_thickness The thickness of the round arrow line.
    /// @param arrow_head_width The width of the arrow head start.
    /// @param arrow_head_height The height of the arrow head.
    /// @return Vertices of vec3 position and vec3 normal, and triangles' indices.
    ///         Normal is (0, 0, 1) in all vertices.
    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateReplayIcon(float circle_resolution,
                                                                                 float z = 0.1,
                                                                                 float line_radius = 0.5,
                                                                                 float line_thickness = 0.1,
                                                                                 float arrow_head_width = 0.4,
                                                                                 float arrow_head_height = 0.4);
    /// @param z The z of vertices.
    /// @param line_offset The offset of the lines from center.
    /// @param line_thickness The thickness of the lines.
    /// @param arrow_head_width The width of the arrow head start.
    /// @param arrow_head_height The height of the arrow head.
    /// @return Vertices of vec3 position and vec3 normal, and triangles' indices.
    ///         Normal is (0, 0, 1) in all vertices.
    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateExitIcon(float z = 0.1,
                                                                               float line_offset = 0.5,
                                                                               float line_thickness = 0.1,
                                                                               float arrow_head_width = 0.3,
                                                                               float arrow_head_height = 0.3);
    /// @param z The z of vertices.
    /// @return Vertices of vec3 position and vec3 normal, and triangles' indices.
    ///         Normal is (0, 0, 1) in all vertices.
    std::tuple<std::vector<float>, std::vector<unsigned int>> GenerateAIIcon(float z = 0.1);
}
