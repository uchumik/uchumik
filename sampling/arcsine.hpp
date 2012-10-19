# ifndef __ARCSINE_INV__
# define __ARCSINE_INV__

# include <cmath>
# include <xorshift.hpp>

namespace randomsampling
{
   class arcsine
   {
      public:
         arcsine();
         virtual ~arcsine();
         static double sampling()
         {
            //double u = (double)rand()/(1.0+RAND_MAX);
            double u = uniform::xor128l();
            double y = sin(M_PI*u/2.0);
            return pow(y,2.0);
         }
   };
}
# endif /* __ARCSINE_INV__ */
