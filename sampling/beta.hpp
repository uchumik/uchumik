# ifndef __BETA_DISTRIBUTION__
# define __BETA_DISTRIBUTION__

# include <gamma.hpp>
# include <arcsine.hpp>
# include <powerfunction.hpp>
# include <xorshift.hpp>
# include <triangular.hpp>
namespace randomsampling
{
   class beta
   {
      public:
         beta();
         virtual ~beta();
         double sampling(double alpha, double beta);

      protected:
         randomsampling::gamma gmm;
         powerfunc pf;
   };

   beta::beta()
   {
   }

   beta::~beta()
   {
   }

   double beta::sampling(double alpha, double beta)
   {
      if (alpha == 1.0 && beta == 1.0)
      {
         //return (double)rand()/(1.0+RAND_MAX);
         return uniform::xor128r();
      }
      else if (alpha == 1.0 && beta == 2.0)
      {
         return ltriangular::sampling(0,1);
      }
      else if (alpha == 2.0 && beta == 1.0)
      {
         return rtriangular::sampling(0,1);
      }
      else if (alpha == 0.5 && beta == 0.5)
      {
         return arcsine::sampling();
         /// Arcsine distribution
      }
      else if (alpha == 1.0)
      {
         return 1.0-this->pf.sampling(beta,0,1);
      }
      else if (beta == 1.0)
      {
         return this->pf.sampling(alpha,0,1);
      }
      double y1 = this->gmm.sampling(alpha,1.0);
      double y2 = this->gmm.sampling(beta,1.0);
      return y1/(y1+y2);
   }
}
# endif /* __BETA_DISTRIBUTION__ */
