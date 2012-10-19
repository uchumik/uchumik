# ifndef __XORSHIFT__
# define __XORSHIFT__

extern "C"
{
# include <stdint.h>
}
# include <ctime>

namespace randomsampling
{
   class uniform
   {
      public:
         uniform();
         virtual ~uniform();
         // [0,UINT32_MAX]
         static uint32_t xor128(void)
         {
            static bool init = false;
            static uint32_t x;
            if (!init)
            {
               x = time (NULL);
               init = true;
            }
            static uint32_t y = 362436069;
            static uint32_t z = 521288629;
            static uint32_t w = 88675123;
            uint32_t t;

            t = x ^ (x << 11);
            x = y; y = z; z = w;
            return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
         }

         // [0,1]
         static double xor128n(void)
         {
            return (double)xor128()/UINT32_MAX;
         }

         // [0,1)
         static double xor128l(void)
         {
            return (double)xor128()/(1.0+UINT32_MAX);
         }

         // (0,1)
         static double xor128r(void)
         {
            return (double)(xor128()+1.0)/(1.0+UINT32_MAX);
         }
   };

}
# endif /* __XORSHIFT__ */
