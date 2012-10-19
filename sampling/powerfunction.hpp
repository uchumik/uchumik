# ifndef __POWER_FUNCTION_DISTRIBUTION__
# define __POWER_FUNCTION_DISTRIBUTION__

# include <gamma.hpp>
# include <exponentialzig.hpp>
# include <triangular.hpp>

namespace randomsampling
{
   class powerfunc
   {
      public:
         powerfunc();
         virtual ~powerfunc();
         double sampling(double gamma, double a, double b);
      protected:
         gamma gmm;
         exponentialzig exdist;
   };
   powerfunc::powerfunc()
   {
   }

   powerfunc::~powerfunc()
   {
   }

   double powerfunc::sampling(double gamma, double a, double b)
   {
      double s = b - a;
      if (gamma == 1)
      {
         return a + s*uniform::xor128r();
      }
      else if (gamma == 2)
      {
         return rtriangular::sampling(a, b);
      }
      double y1 = this->gmm.sampling(gamma,1);
      double y2 = this->exdist.sampling(1);
      while (y2 == 0)
      {
         y2 = this->exdist.sampling(1);
      }
      double y = y1/(y1+y2);
      return a + s*y;
   }
}
# endif /* __POWER_FUNCTION_DISTRIBUTION__ */
