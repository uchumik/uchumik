# ifndef __MLR_RANKER__
# define __MLR_RANKER__

# include "instance.hpp"
# include "learner.hpp"

namespace mlr
{
   class Ranker
   {
      public:
         Ranker():model(NULL){}
         virtual ~Ranker(){}
         virtual void predict(const ilist& instance, const double cnf, const _cache& c) const = 0;
      protected:
         double *model;
      private:
         Ranker(const Ranker&);
         Ranker& operator=(const Ranker&);
   };
}
# endif /* __MLR_RANKER__ */
