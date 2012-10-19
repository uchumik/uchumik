# ifndef __BERNOULLI__
# define __BERNOULLI__

# include <ctime>
# include <cstdio>
# include <cstdlib>
# include <xorshift.hpp>
namespace randomsampling
{
   class bernoulli
   {
      public:
         bernoulli();
         virtual ~bernoulli();
         static unsigned int sampling(double u)
         {
            if (u < 0 || u > 1)
            {
               throw "input invalid mean value";
            }
            return (uniform::xor128n() > u) ? 0 : 1;
            //srand( (unsigned int)time(NULL) );
            //(rand()/RAND_MAX > u) ? return 0: return 1;
         }
   };
}
# endif /* __BERNOULLI__ */
