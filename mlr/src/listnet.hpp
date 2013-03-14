# ifndef __MLR_LISTNET__
# define __MLR_LISTNET__

# include "learner.hpp"
# include "ranker.hpp"

namespace mlr
{
   class lcomp
   {
      public:
         bool operator()(const instance *a, const instance* b) const
         {
            return (a->getlabel() > b->getlabel());
         }
   };
   class ListNetLearner : public Learner
   {
      public:
         ListNetLearner(ranklist& corpus);
         ~ListNetLearner();
         void update(double eta, ilist& i, double cnf = 0, const _cache& c = _cache());
         void save(const char *modelfile);
         //virtual void test(ranklist& corpus, clist *c = NULL);
         void test(ranklist& corpus, const clist& c = clist());
         //virtual void cache(ranklist& corpus, clist *c = NULL);
         void cache(ranklist& corpus, double cnf, clist *c = NULL);
      protected:
         ListNetLearner(){}
         long initmodel(ranklist& corpus);
         //double crossent(ranklist& corpus, clist *c);
         double crossent(ranklist& corpus, const clist& c);
         double product(double *model, const _fvector& fv);
      private:
         ListNetLearner(const ListNetLearner&);
         ListNetLearner& operator=(const ListNetLearner&);
   };

   class ListNetRanker : public Ranker
   {
      public:
         ListNetRanker(const char *modelfile);
         ~ListNetRanker();
         void predict(ngilist& instance, const double cnf = 0, _cache *c = NULL);
      protected:
         void readmodel(const char *modelfile);
         double product(double *model, const _fvector& fv);
      private:
         ListNetRanker(){};
         ListNetRanker(const ListNetRanker&);
         ListNetRanker& operator=(const ListNetRanker&);
   };
}
# endif /* __MLR_LISTNET__ */
