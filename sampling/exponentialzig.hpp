# ifndef __EXPONENTIAL_DISTRIBUTION_ZIGGURAT__
# define __EXPONENTIAL_DISTRIBUTION_ZIGGURAT__

extern "C"
{
# include <stdint.h>
}
# include <cmath>
# include <ctime>
# include <cstdio>
# include <cstdlib>
# include <ziggurat.hpp>
# include <iostream>
# include <xorshift.hpp>

// m = 32
// l = 7, n = 2^l = 128
// e^{-x}:
// r = 6.898315116616
// v = 0.0797322953955
namespace randomsampling
{
   class exponentialzig
   {
      public:
         exponentialzig();
         virtual ~exponentialzig();
         double sampling(double theta);
      protected:
         void preprocessing();
         double *w;
         double *f;
         ubit *k;
         double *x;
         static double func(double x)
         {
            return exp(-x);
         }
   };
   exponentialzig::exponentialzig()
   {
      this->w = new double[n];
      this->f = new double[n];
      this->k = new ubit[n];
      this->x = new double[n];
      this->preprocessing();
   }

   exponentialzig::~exponentialzig()
   {
      delete[] w;
      delete[] f;
      delete[] k;
      delete[] x;
   }

   double exponentialzig::sampling(double theta)
   {
      if (theta <= 0)
      {
         throw "theta must be than zero";
      }
      //srand( (unsigned int)time(NULL) );
      // step 1
step1:
      //ubit uni = (ubit)rand()/(1.0+RAND_MAX)*UINT32_MAX;
      ubit uni = uniform::xor128();
      ubit mask = 0x0000007F;
      ubit ui = uni & mask;
      ubit umk = uni >> l;

      if (umk < k[ui]) // step 3
      {
         double ux = this->w[ui] * umk;
         /// go to step 5
         return theta * ux;
      }
      else if (ui == n - 1)
      {
         //double u = (double)rand()/(1.0+RAND_MAX);
         double u = uniform::xor128l();
         double y = er - log(1.0-u);
         /// go to step 5
         return theta*y;
      }
      else
      {
         double ux = this->w[ui] * umk;
         double g = exp(-ux);
         //double u = (double)rand()/(1.0+RAND_MAX);
         double u = uniform::xor128l();
         if (u * (this->f[ui] - this->f[ui+1]) <= g - this->f[ui+1])
         {
            /// go to step 5
            return theta * ux;
         }
         goto step1;
      }
      throw "Unexpected process";
   }
   void exponentialzig::preprocessing()
   {
      this->w[n-1] = ev*exp(er)/pow(2.0, m - l);
      this->w[n-2] = er/pow(2.0, m - l);
      this->k[n-1] = (ubit)floor(er/this->w[n-1]);
      this->f[n-1] = exp(-er);
      this->x[n-1] = er;
      for (unsigned int i = n-2; i > 0; --i)
      {
         this->x[i] = -log(exponentialzig::func(this->x[i+1])+ev/this->x[i+1]);
         this->w[i-1] = this->x[i]/pow(2.0,m - l);
         this->k[i] = (ubit)floor(this->x[i]/this->w[i]);
         this->f[i] = exp(-this->x[i]);
      }
      this->k[0] = 0;
      this->f[0] = 1;
   }
}
# endif /* __EXPONENTIAL_DISTRIBUTION_ZIGGURAT__ */
