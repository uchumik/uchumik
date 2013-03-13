# include "ensemble.hpp"
# include "listnet.hpp"
# include "listmle.hpp"
# include "xorshift.hpp"
# include <iostream>

using namespace std;
using namespace mlr;
using namespace randomsampling;

EnsembleLearner::EnsembleLearner(LearnerType type)
: samplesize(1.0),penalty(0.0001),type(type)
{
}

EnsembleLearner::~EnsembleLearner()
{
   Learners::iterator it = this->learners.begin();
   for (; it != this->learners.end(); ++it)
   {
      delete *it;
   }

   rliterator rit = this->corpus.begin();
   for (; rit != this->corpus.end(); ++rit)
   {
      ildelete(**rit);
      delete *rit;
   }
}

void EnsembleLearner::setcorpus(const char *corpus)
{
   int n = 0;
   ifstream in(corpus);
   while (!in.eof())
   {
      ilist *il = new ilist;
      ilread(in, *il);
      this->corpus.push_back(il);
      ++n;
   }
   this->denominator = n;
   cout << "corpus size: " << n << endl;
}

void EnsembleLearner::setsamplesize(double s)
{
   if (s <= 0 || s > 1)
   {
      throw "Unexpected value: penalty must be (0,1]";
   }
   this->samplesize = s;
   cout << "sample size: " << s << endl;
}

void EnsembleLearner::setpenalty(double p)
{
   if (p < 0)
   {
      throw "Unexpected value: penalty must be [0,.]";
   }
   this->penalty = p;
   cout << "penalty: " << p << endl;
}

Learner* EnsembleLearner::factory(ranklist& corpus)
{
   cout << "called factory" << endl;
   switch (this->type)
   {
      case TypeListNet:
         cout << "construct ListNetLearner" << endl;
         return new ListNetLearner(corpus);
      case TypeListMLE:
         cout << "construct ListMLELearner" << endl;
         return new ListMLELearner(corpus);
      dafault:
         return NULL;
   }
}

double EnsembleLearner::decay(int t, unsigned int size)
{
   double d = 1. + (double)t/size;
   return 1./d;
}

void EnsembleLearner::updater(unsigned int iter, Learner& learner, ranklist& corpus, double cnf, const clist& clist)
{
   int t = 0;
   unsigned int size = corpus.size();
   unsigned int csize = clist.size();
   for (unsigned int i = 0; i < iter; ++i)
   {
      cout << "iter: " << i << endl;
      rliterator it = corpus.begin();
      for (int cid = 0; it != corpus.end(); ++it, ++cid)
      {
         double eta = this->decay(t++, size);
         if (csize == size)
         {
            learner.update(eta, **it, cnf, *(clist[cid]));
         }
         else
         {
            learner.update(eta, **it);
         }
      }
      cout << "test: ";
      if (csize == size)
      {
         learner.test(this->corpus, clist);
      }
      else
      {
         learner.test(this->corpus);
      }
   }
}

void EnsembleLearner::sampling(ranklist& subset)
{
   int n = (int)(this->samplesize*this->denominator);
   int size = 0;
   while (size < n)
   {
      uint32_t id = uniform::xor128()%this->denominator;
      subset.push_back(this->corpus[id]);
      ++size;
   }
}

void EnsembleLearner::bagging(unsigned int size, unsigned int iter)
{
   cout << "bagging start" << endl;
   for (unsigned int i = 0; i < size; ++i)
   {
      cout << "Learner epoch: " << i << endl;
      ranklist subset;
      this->sampling(subset);
      this->learners.push_back( this->factory(subset) );
      this->updater(iter, *(this->learners[i]), subset);
   }
}

void EnsembleLearner::boosting(unsigned int size, unsigned int iter)
{
   clist cl(this->denominator);
   rliterator it = this->corpus.begin();
   for (int i = 0; it != this->corpus.end(); ++it, ++i)
   {
      unsigned int ilsize = (*it)->size();
      cl[i] = new _cache(ilsize);
   }
   for (unsigned int i = 0; i < size; ++i)
   {
      double cnf = (double)size/(size+i+1);
      this->learners.push_back( this->factory(this->corpus) );
      this->updater(iter, *(this->learners[i]), this->corpus, cnf, cl);
      (this->learners[i])->cache(this->corpus, cnf, &cl);
   }
}
