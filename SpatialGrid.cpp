#include "SpatialGrid.h"

SpatialGrid::SpatialGrid(int width, int height, int cellSize)
    : m_width(width), m_height(height), m_cellSize(cellSize)
{
    m_cols = (int)ceil((float)m_width / m_cellSize);
    m_rows = (int)ceil((float)m_height / m_cellSize);

    m_cells.resize(m_cols * m_rows);
}

void SpatialGrid::resize(int width, int height, int cellSize) {
    m_width = width;
    m_height = height;
    m_cellSize = cellSize;

    m_cols = (int)ceil((float)m_width / m_cellSize);
    m_rows = (int)ceil((float)m_height / m_cellSize);

    m_cells.clear();
    m_cells.resize(m_cols * m_rows);
}

void SpatialGrid::clear() {
    for (auto& cell : m_cells) {
        cell.clear();
    }
}

void SpatialGrid::insert(float x, float y, int id) {
    int col = (int)(x / m_cellSize);
    int row = (int)(y / m_cellSize);

    if (col < 0) col = 0;
    if (col >= m_cols) col = m_cols - 1;
    if (row < 0) row = 0;
    if (row >= m_rows) row = m_rows - 1;

    m_cells[row * m_cols + col].push_back(id);
}

std::vector<int> SpatialGrid::get_cell(int col, int row) {
    if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) {
        return {};
    }

    return m_cells[row * m_cols + col];
}

size_t SpatialGrid::get_memory_usage() const {
    size_t total = 0;

    total += sizeof(m_cells);

    for (const auto& cell : m_cells) {
        total += sizeof(std::vector<int>);
        total += cell.capacity() * sizeof(int);
    }
    
    return total;
}