# ifndef __MLR_LISTMLE__
# define __MLR_LISTMLE__

# include "learner.hpp"
# include "ranker.hpp"
# include "listnet.hpp"

namespace mlr
{
   class ListMLELearner : public ListNetLearner
   {
      public:
         ListMLELearner(ranklist& corpus);
         ~ListMLELearner();
         void update(double eta, ilist& i, double cnf = 0, const _cache& c = _cache());
         //void test(ranklist& corpus, clist *c = NULL);
         void test(ranklist& corpus, const clist& c = clist());
      protected:
         int topk;
         //double logloss(ranklist& corpus, clist *c);
         double logloss(ranklist& corpus, const clist& c);
      private:
         ListMLELearner(const ListMLELearner&);
         ListMLELearner& operator=(const ListMLELearner&);
   };
   // ListNetRanker can use as ListMLERanker
}
# endif /* __MLR_LISTMLE__ */
