# include <allocmmap.hpp>
# include <cstdlib>
# include <ctime>
# include <vector>
# include <iostream>
# include <iterator>

using namespace std;
using namespace mmaputilizer;

template <int low, int high>
struct RandomGen
{
   RandomGen ()
   {
      srand((unsigned)time(NULL));
   }
   int operator () () const
   {
      return rand() % high + low;
   }
};

// vector dumper

/*
template <class T, template <class T> class alloc>
std::ostream& operator << (std::ostream& os, const std::vector<T, alloc<T> >& v)
{
   std::copy(v.begin(), v.end(), std::ostream_iterator<T> (os, " "));
}
*/

int main ()
{
   allocmmap<int> a;
   vector<int> v1(10);
   vector<int, allocmmap<int> > v2(10);

   generate(v1.begin(), v1.end(), RandomGen<0,10>());
   sleep(1);
   generate(v2.begin(), v2.end(), RandomGen<0,10>());


   vector<int>::iterator it1 = v1.begin();
   for (; it1 != v1.end(); ++it1)
   {
      cout << *it1 << '\t';
   }
   cout << endl;
   vector<int, allocmmap<int> >::iterator it2 = v2.begin();
   for (; it2 != v2.end(); ++it2)
   {
      cout << *it2 << '\t';
   }
   cout << endl;

   sleep(1);
   vector<int, allocmmap<int> > v3(10);
   cout << "size " << v3.size() << '\t';
   generate(v3.begin(), v3.end(), RandomGen<0,10>());
   cout << "size " << v3.size() << endl;
   vector<int, allocmmap<int> >::iterator it3 = v3.begin();
   for (; it3 != v3.end(); ++it3)
   {
      cout << *it3 << '\t';
   }
   cout << endl;
   vector<int, mmaputilizer::allocmmap<int> >::iterator it4 = v2.begin();
   for (; it4 != v2.end(); ++it4)
   {
      cout << *it4 << '\t';
   }
   cout << endl;
   //std::cout << v1 << std::endl;
   //std::cout << v2 << std::endl;
   return 0;
}
