# ifndef __MMAPUTIL__
# define __MMAPUTIL__

extern "C"
{
# include <sys/types.h>
# include <sys/mman.h>
# include <stddef.h>
}

# include <limits>
# include <iostream>

namespace mmaputilizer
{
   template <class T>
   class allocmmap
   {
      public:
         typedef size_t size_type;
         typedef ptrdiff_t difference_type;
         typedef T* pointer;
         typedef const T* const_pointer;
         typedef T& reference;
         typedef const T& const_reference;
         typedef T value_type;

         // rebind
         template <class U>
         struct rebind
         {
            typedef allocmmap<U> other;
         };

         pointer address (reference value) const
         {
            return &value;
         }

         const_pointer address (const_reference value) const
         {
            return &value;
         }

         allocmmap () throw () {}

         allocmmap ( const allocmmap& ) throw () {}

         template <class U>
         allocmmap (const allocmmap<U>& ) throw () {}

         ~allocmmap () throw () {}

         size_type max_size () const throw ()
         {
            return ::std::numeric_limits <size_type>::max()/sizeof(T);
         }

         pointer allocate (size_type num, void *hint = 0)
         {
# ifdef MAP_ANON
            pointer p = (pointer) ::mmap(hint, num, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
# else
            pointer p = (pointer) ::mmap(hint, num, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
# endif
            std::cerr << p << std::endl;
            int *val = (int *)p;
            if (val && *val == -1)
            {
               p = NULL;
            }
            return p;
         }

         void construct (pointer p, const_reference value)
         {
            new ((void *)p) T(value); // placement new
         }

         void destroy (pointer p)
         {
            p->~T();
         }

         void deallocate (pointer p, size_type num)
         {
            ::munmap((caddr_t) p, num);
         }
   };

   template <class T1, class T2>
      bool operator== (allocmmap<T1> const &, allocmmap<T2> const &) throw()
      {
         return true;
      }

   template <class T1, class T2>
      bool operator!= (allocmmap<T1> const &, allocmmap<T2> const &) throw()
      {
         return false;
      }
}


# endif /* __MMAPUTIL __ */`
