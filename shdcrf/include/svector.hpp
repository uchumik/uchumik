# ifndef __SVECTOR_H__
# define __SVECTOR_H__

# include "splaytree.hpp"
# define SHASH 2597

namespace sparse
{
   class shash
   {
      public:
         unsigned int operator(unsigned int k)
         {
            return key % SHASH;
         }
   };

   template <class V>
   class svnode
   {
      public:
         unsigned int key;
         V val;
   };

   template <class V>
   class svcompare
   {
      public:
         bool operator()(const svnode<V>& a, const svnode<V>& b)
         {
            return (a.key < b.key);
         }
   };

   template <class T>
   class svect
   {
      public:
         svect();
         ~svect();
         T* operator[](unsigned int k)
         {
            mysplay::mmallocsplaynode< svnode<T> > n;
         }
      protected:
         mysplay::splaytree< svnode<T>, svcompare > d;
      private:
   };
}
# endif /* __SVECTOR_H__ */
