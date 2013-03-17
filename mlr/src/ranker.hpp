# ifndef __MLR_RANKER__
# define __MLR_RANKER__

# include "instance.hpp"
# include "learner.hpp"
# include <cstdio>

namespace mlr
{
   class Ranker
   {
      public:
         Ranker():model(NULL){}
         Ranker(const double *w, long fsize)
         {
            std::memcpy(this->model, w, sizeof(double)*fsize);
         };
         virtual ~Ranker(){}
         virtual void predict(ngilist& instance, const double cnf = 0, _cache* c = NULL) = 0;
      protected:
         double *model;
         long fsize;
      private:
         Ranker(const Ranker&);
         Ranker& operator=(const Ranker&);
   };
}
# endif /* __MLR_RANKER__ */
