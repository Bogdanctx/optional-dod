#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class SpatialGrid {
public:
    SpatialGrid(int width, int height, int cellSize);

    void clear(); // sterge grid-ul
    void insert(float x, float y, int id); // insereaza obiectul cu id-ul `id` in celula corespunzatoare pozitiei (x,y)
    std::vector<int> get_cell(int col, int row); // getter pt celula corespunzatoare pozitiei (x,y)
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