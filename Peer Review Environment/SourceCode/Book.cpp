#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include "Book.hpp"

const auto EPSILON = 0.0001;
std::ostream& operator<<(std::ostream& stream, const Book& book)
{
    stream << std::quoted(book._isbn) << ", " << std::quoted(book._title) << ", "
        << std::quoted(book._author) << ", " << book._price;
    
    return stream;
}


std::istream& operator>>(std::istream& stream, Book& book)
{
    Book check;
    char delim = ', ';
    stream >> std::quoted(check._isbn) >> delim >> std::quoted(check._title) >> delim
        >> std::quoted(check._author) >> delim >> check._price;

    if (stream) book = std::move(check);
    return stream;
}

//Constructors
Book::Book(std::string_view title, std::string_view author,
    std::string_view isbn, double           price)
    : _isbn(isbn), _title(title), _author(author), _price(price){}

// Queries
std::string Book::isbn() const
{
    return _isbn;
}
std::string Book::title() const
{
    return _title;
}
std::string Book::author() const
{
    return _author;
}
double      Book::price() const
{
    return _price;
}

      // Mutators
void Book::isbn(std::string_view newIsbn)
{
    _isbn = newIsbn;
}
void Book::title(std::string_view newTitle)
{
    _title = newTitle;
}
void Book::author(std::string_view newAuthor)
{
    _author = newAuthor;
}
void Book::price(double           newPrice)
{
    _price = newPrice;
}

// Relational Operators
bool operator==(const Book& lhs, const Book& rhs)
{

    auto resultIsbn = lhs._isbn.compare(rhs._isbn);
    auto resultTitle = lhs._title.compare(rhs._title);
    auto resultAuthor = lhs._author.compare(rhs._author);
    return  resultIsbn == 0
        && resultTitle == 0
        && resultAuthor == 0
        && abs(lhs._price - rhs._price) < EPSILON;
}


bool operator<(const Book& lhs, const Book& rhs)
{
   /* auto resultIsbn = lhs._isbn.compare(rhs._isbn);
    auto resultTitle = lhs._title.compare(rhs._title);
    auto resultAuthor = lhs._author.compare(rhs._author);
    return  resultIsbn < 0
        && resultTitle < 0
        && resultAuthor < 0
        && abs(lhs._price - rhs._price) >= EPSILON;
        //&& (lhs._price - rhs._price) < 0;
    */
    if (auto result = lhs._isbn.compare(rhs._isbn); result != 0) return result < 0;
    if (auto result = lhs._author.compare(rhs._author); result != 0) return result < 0;
    if (auto result = lhs._title.compare(rhs._title); result != 0) return result < 0;
    if (std::abs(lhs._price - rhs._price) >= EPSILON) return lhs._price < rhs._price;

    return false;

}

bool operator!=(const Book& lhs, const Book& rhs) { return !(lhs == rhs); }
bool operator<=(const Book& lhs, const Book& rhs) { return !(rhs < lhs); }
bool operator> (const Book& lhs, const Book& rhs) { return (rhs < lhs); }
bool operator>=(const Book& lhs, const Book& rhs) { return !(lhs < rhs); }