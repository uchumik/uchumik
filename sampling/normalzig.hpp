# ifndef __NORMAL_DISTRIBUTION_ZIGGURAT__
# define __NORMAL_DISTRIBUTION_ZIGGURAT__

extern "C"
{
# include <stdint.h>
}
# include <ziggurat.hpp>
# include <cmath>
# include <ctime>
# include <cstdio>
# include <cstdlib>
# include <xorshift.hpp>

// m = 32
// l = 7, n = 2^l = 128
// e^{-x}: 
// r = 6.898315116616
// v = 0.0797322953955
namespace randomsampling
{
   class normalzig
   {
      public:
         normalzig();
         virtual ~normalzig();
         double sampling(double u, double sigma);

      protected:
         void preprocessing();
         double *w;
         double *f;
         ubit *k;
         double *x;
         static double func(double x)
         {
            return exp(-pow(x,2.0)/2);
         }
   };
   normalzig::normalzig()
   {
      this->w = new double[n];
      this->f = new double[n];
      this->k = new ubit[n];
      this->x = new double[n];
      this->preprocessing();
   }

   normalzig::~normalzig()
   {
      delete[] this->w;
      delete[] this->f;
      delete[] this->k;
      delete[] this->x;
   }

   double normalzig::sampling(double u, double sigma)
   {
      //srand( (unsigned int)time(NULL) );
      // step 1
step1:
      //ubit uni = (ubit)rand()*UINT32_MAX/(1.0+RAND_MAX);
      ubit uni = uniform::xor128();
      ubit mask = 0x0000007F;
      ubit ui = uni & mask;
      ubit um = uni >> l;
      int j = (um & 1);
      int sign = j == 0 ? 1 : -1;
      ubit umk = um >> 1;
      if (umk < k[ui])
      {
         double ux = w[ui] * umk;
         /// go to step 6
         double y = ux * sign;
         return u + sigma * y;
      }
      else if (ui == n-1)
      {
step5a:
         //double u1 = (double)rand()/(1.0+RAND_MAX);
         //double u2 = (double)rand()/(1.0+RAND_MAX);
         double u1 = uniform::xor128l();
         double u2 = uniform::xor128l();
         double y = -(log(1.0 - u1)/nr);
         double z = -log(u2);
         if (y*y < z*z)
         {
            double ux = nr + y;
            return u + sigma * ux * sign;
         }
         goto step5a;
      }
      else
      {
         double ux = w[ui] * umk;
         double g = exp(-pow(ux,2.0)/2);
         //double u = (double)rand()/(1.0+RAND_MAX);
         double u = uniform::xor128l();
         if (u * (f[ui] - f[ui+1]) <= g - f[ui+1])
         {
            return u + sigma * ux * sign;
         }
         goto step1;
      }
      throw "Unexpected process";
   }

   void normalzig::preprocessing()
   {
      this->w[n-1] = nv*exp(pow(nr,2)/2)/pow(2.0, m - l - 1);
      this->w[n-2] = nr/pow(2.0, m - l - 1);
      this->k[n-1] = (ubit)floor(nr/this->w[n-1]);
      this->f[n-1] = exp(-pow(nr,2)/2);
      this->x[n-1] = nr;
      for (unsigned int i = n-2; i > 0; --i)
      {
         this->x[i] = sqrt(-2.0*log(normalzig::func(this->x[i+1])+nv/this->x[i+1]));
         this->w[i-1] = x[i]/pow(2.0,m - l - 1);
         this->k[i] = (ubit)floor(this->x[i]/this->w[i]);
         this->f[i] = exp(-pow(this->x[i],2)/2);
      }
      this->k[0] = 0;
      this->f[0] = 1;

   }
}
# endif /* __NORMAL_DISTRIBUTION_ZIGGURAT__ */
