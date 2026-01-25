#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class SpatialGrid {
public:
    SpatialGrid(int width, int height, int cellSize);

    void clear();
    void insert(float x, float y, int id);
    std::vector<int> get_cell(int col, int row);
    void resize(int width, int height, int cellSize);
    int get_cell_size() const { return m_cellSize; }
    size_t get_memory_usage() const;

private:
    int m_width;
    int m_height;
    int m_cellSize;
    int m_cols;
    int m_rows;

    std::vector<std::vector<int>> m_cells;
};