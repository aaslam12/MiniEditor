#include "piecetable.h"

piece_table::piece_table()
{}

piece_table::~piece_table()
{}

piece_table::piece_table(const std::string& initial_content)
{}

void piece_table::insert(size_t position, const std::string& text)
{
    if (position > length())
    {
        throw std::out_of_range("Insert position is out of range");
    }
}

void piece_table::remove(size_t position, size_t length)
{}

std::string piece_table::to_string() const
{
    return "";
}

size_t piece_table::length() const
{
    return -1;
}
