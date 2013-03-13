# include "listmle.hpp"
# include <iostream>

using namespace std;
using namespace mlr;

ListMLELearner::ListMLELearner(ranklist& corpus)
: topk(5)
{
   this->fsize = this->initmodel(corpus);
}

ListMLELearner::~ListMLELearner()
{
   delete[] this->model;
   delete[] this->pcache;
}

//void ListMLELearner::test(ranklist& corpus, clist *c)
void ListMLELearner::test(ranklist& corpus, const clist& c)
{
   double loss = this->logloss(corpus, c);
   cout << "Log likelihood: " << loss << endl;
}

// clist must be sorted in order to correct relevancy
//double ListMLELearner::logloss(ranklist& corpus, clist *c)
double ListMLELearner::logloss(ranklist& corpus, const clist& c)
{
   long id = 0;
   double loss = 0;
   // ranking list
   rliterator it = corpus.begin();
   for (; it != corpus.end(); ++it, ++id)
   {
      // instance list
      sort( (*it)->begin(), (*it)->end(), lcomp() );
      ciliterator iit = (*it)->begin();
      /*
      const _cache *cache;
      if (c)
      {
         cache = (*c)[id];
      }
      */
      vector<double> x;
      for (int cid = 0; iit != (*it)->end(); ++iit)
      {
         const _fvector& fv = (*iit)->getvect();
         double v = this->product(this->model, fv);
         if (corpus.size() == c.size())
         {
            v += (*c[id])[cid++];
         }
         x.push_back(v);
      }
      // partition
      reverse(x.begin(), x.end());
      int n = (this->topk > x.size())? x.size() : this->topk;
      double z = 0;
      vector<double> zz;
      vector<double>::iterator xit = x.begin();
      for (; xit != x.end(); ++xit)
      {
         z = logsumexp(z, *xit, ( xit == x.begin() ) );
         zz.push_back(z);
      }
      reverse(zz.begin(), zz.end());
      reverse(x.begin(), x.end());
      for (int k = 0; k < n; ++k)
      {
         double logloss = x[k] - zz[k];
         loss += logloss;
      }
   }
   return -loss;
}

void ListMLELearner::update(double eta, ilist& i, double cnf, const _cache& c)
{
   // sort instances by relevancy label
   sort(i.begin(), i.end(), lcomp());

   // scoring
   vector<double> x;
   iliterator it = i.begin();
   for (; it != i.end(); ++it)
   {
      const _fvector& fv = (*it)->getvect();
      double v = this->product(this->model, fv);
      x.push_back(v);
   }
   // cache check
   if (x.size() == c.size())
   {
      _cache::const_iterator cit = c.begin();
      for (int i = 0; cit != c.end(); ++cit, ++i)
      {
         x[i] += *cit;
      }
   }
   // partition
   reverse(x.begin(), x.end());
   int n = (this->topk > x.size())? x.size() : this->topk;
   double z = 0;
   vector<double> zz;
   vector<double>::iterator xit = x.begin();
   for (; xit != x.end(); ++xit)
   {
      z = logsumexp(z, *xit, ( xit == x.begin() ) );
      zz.push_back(z);
   }
   reverse(zz.begin(), zz.end());
   reverse(x.begin(), x.end());

   // calc gradient
   _fvector grad;
   it = i.begin();
   for (int k = 0; k < n; ++k)
   {
      double partition = zz[k];
      // correct
      double c = 1. - myexp(x[k] - partition);
      const _fvector& fv = (*it)->getvect();
      _fiterator fit = fv.begin();
      for (; fit != fv.end(); ++fit)
      {
         _feature f;
         f.key = (*fit).key;
         f.val = c * (*fit).val;
         grad.push_back(f);
      }
      // other
      iliterator oit = ++it;
      for (int j = k+1; oit != i.end(); ++oit, ++j)
      {
         double p = myexp(x[j] - partition);
         const _fvector& ffv = (*oit)->getvect();
         _fiterator ffit = ffv.begin();
         for (; ffit != ffv.end(); ++ffit)
         {
            unsigned int key = (*ffit).key;
            _feature f;
            f.key = key;
            f.val = -p * (*ffit).val;
            int id = findkey(key, grad);
            if (id < 0)
            {
               grad.push_back(f);
               sort(grad.begin(), grad.end(), fcomp() );
            }
            else
            {
               grad[id].val += f.val;
            }
         }
      }
   }
   // update
   _fiterator git = grad.begin();
   for (; git != grad.end(); ++git)
   {
      unsigned int key = (*git).key;
      *(this->model+key) += eta * (*git).val;
      // l2 regularization
      double p = this->cc - this->pcache[key];
      this->pcache[key] = this->cc;
      *(this->model+key) /= (1. + p);
   }
   this->cc += eta * this->c;
}
