# ifndef __TRIANGULAR_DISTRIBUTION__
# define __TRIANGULAR_DISTRIBUTION__

# include <xorshift.hpp>
# include <algorithm>

namespace randomsampling
{
   class rtriangular
   {
      public:
         rtriangular();
         virtual ~rtriangular();
         static double sampling(double a, double b)
         {
            double s = b - a;
            double u1 = uniform::xor128n();
            double u2 = uniform::xor128n();
            double y = std::max(u1,u2);
            return a + s*y;
         }
   };

   class ltriangular
   {
      public:
         ltriangular();
         virtual ~ltriangular();
         static double sampling(double a, double b)
         {
            double s = b - a;
            double u1 = uniform::xor128n();
            double u2 = uniform::xor128n();
            double y = std::min(u1,u2);
            return a + s*y;
         }
   };
}
# endif /* __TRIANGULAR_DISTRIBUTION__ */
