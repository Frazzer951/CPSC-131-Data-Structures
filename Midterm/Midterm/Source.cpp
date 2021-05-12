#include <iostream>


class Midterm
{
public:
  Data_t minimum()
  {
    if( empty() ) throw std::length_error( "The List is Empty" );
    return minimum( *begin(), begin() );
  }

private:
  Data_t minimum( Data_t min, Iterator position )
  {
    if( position == _tail ) return min;

    if( *position < min ) min = *position;

    return minimum( min, ++position );
  }
};


int main()
{
  std::cout << "Hello\n";
}