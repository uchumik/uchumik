# ifndef __MLR_ENSEMBLE__
# define __MLR_ENSEMBLE__

# include "learner.hpp"
# include "ranker.hpp"


namespace mlr
{
   typedef std::vector<Learner*> Learners;
   typedef std::vector<Ranker*> Rankers;

   enum LearnerType
   {
      TypeListNet = 0,
      TypeListMLE = 1,
      TypeLambdaListNet = 2,
      TypeLambdaListMLE = 3,
      TypeNeuroListNet = 4,
      TypeNeuroListMLE = 5,
      TypeLambdaNeuroListNet = 6,
      TypeLambdaNeuroListMLE = 7
   };

   enum EnsembleType
   {
      TypeBagging = 0,
      TypeBoosting = 1
   };

   class EnsembleLearner
   {
      public:
         /** 
          * @param type :Learning Algorithm(e.g. "ListNet" or "ListMLE")
          */
         EnsembleLearner(LearnerType type);
         virtual ~EnsembleLearner();
         /**
          * @param corpus :corpus file in LETOR format
          */
         void setcorpus(const char *corpus);
         /**
          * @param s :sampling size [0-1] for bagging
          */
         void setsamplesize(double s);
         /**
          * @param p :penalty weight for regularization
          */
         void setpenalty(double p);
         /**
          * @param model :modelfile name to save
          */
         void savemodel(const char *model);

         void bagging(unsigned int size, unsigned int iter);
         void boosting(unsigned int size, unsigned int iter);
      protected:
         double samplesize;
         double penalty;
         LearnerType type;
         EnsembleType etype;

         ranklist corpus;
         Learners learners;
         int denominator;

         void sampling(ranklist& subset);
         Learner* factory(ranklist& corpus);
         void updater(unsigned int iter, Learner& learner, ranklist& corpus, double cnf = 0, const clist& clist = clist());
         double decay(int t, unsigned int size);
         void savelinearmodel(FILE *fp);
         void savenonlinearmodel(FILE *fp);
      private:
         EnsembleLearner(){};
         EnsembleLearner(const EnsembleLearner&);
         EnsembleLearner& operator=(const EnsembleLearner&);
   };

   class EnsembleRanker
   {
      public:
         EnsembleRanker(const char *model);
      protected:
         Rankers ranker;
      private:
         EnsembleRanker();
         EnsembleRanker(const EnsembleRanker&);
         EnsembleRanker& operator=(const EnsembleRanker&);
   };
}
# endif /* __MLR_ENSEMBLE__ */
