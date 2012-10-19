# ifndef __GAMMA_DISTRIBUTION__
# define __GAMMA_DISTRIBUTION__

# include <exponentialzig.hpp>
# include <normalzig.hpp>
# include <xorshift.hpp>

namespace randomsampling
{
   class gamma
   {
      public:
         gamma();
         virtual ~gamma();
         double sampling(double alpha, double beta);

      protected:
         double d;
         double c;
         double ialpha;
         double palpha;
         bool isinv;
         void preprocessing(double alpha);
         exponentialzig exdist;
         normalzig normaldist;
   };

   gamma::gamma()
      :palpha(0)
   {
   }

   gamma::~gamma()
   {
   }

   double gamma::sampling(double alpha, double beta)
   {
      if (alpha <= 0 || beta <= 0)
      {
         throw "input invalid parameter";
      }
      if (alpha == 1.0)
      {
         //return exponentialzig::sampling(beta);
         return this->exdist.sampling(beta);
      }
      this->preprocessing(alpha);
      /// step 1
step1:
      double z = this->normaldist.sampling(0,1);
      double v = 1.0 + this->c * z;

      /// step 2
      if (v <= 0)
      {
         goto step1;
      }
      double w = pow(v,3.0);
      double y = this->d * w;

      /// step 3
      //double u = (double)rand()/(1.0+RAND_MAX);
      double u = uniform::xor128l();
      if (u < 1.0-0.0331*pow(z,4.0))
      {
         goto step5;
      }

      /// step 4
      if (pow(z,2)/2.0 + this->d * log(w) - y < log(u))
      {
         goto step1;
      }

      /// step 5
step5:
      if (isinv)
      {
         //double t = (double)rand()/(1.0+RAND_MAX);
         double t = uniform::xor128l();
         return beta*y*pow(t,ialpha);
      }
      return beta*y;
   }

   void gamma::preprocessing(double alpha)
   {
      if (alpha == this->palpha)
      {
         return;
      }
      /// preprocessing
      this->isinv = false;
      this->ialpha = 1.0/alpha;
      if (alpha < 1)
      {
         this->isinv = true;
         alpha += 1.0;
      }
      this->d = alpha - 1.0/3;
      this->c = 1.0/sqrt(9.0*d);
   }
}
# endif /* __GAMMA_DISTRIBUTION__ */
