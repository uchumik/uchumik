# include "listnet.hpp"
# include <iostream>

using namespace mlr;
using namespace std;

ListNetLearner::ListNetLearner(ranklist& corpus)
{
   this->fsize = this->initmodel(corpus);
}

ListNetLearner::~ListNetLearner()
{
   delete[] this->model;
   delete[] this->pcache;
}

long ListNetLearner::initmodel(ranklist& corpus)
{
   long id = 0;
   // ranking list
   rliterator it = corpus.begin();
   for (; it != corpus.end(); ++it)
   {
      // instance list
      const ilist *il = *it;
      ciliterator iit = il->begin();
      for (; iit != (*it)->end(); ++iit)
      {
         // feature list
         const _fvector& fv = (*iit)->getvect();
         _fiterator fit = fv.begin();
         for (; fit != fv.end(); ++fit)
         {
            if (id < (*fit).key)
            {
               id = (*fit).key;
            }
         }
      }
   }

   this->model = new double[id];
   this->pcache = new double[id];

   for (long i = 0; i < id; ++i)
   {
      *(this->model+i) = 0;
      *(this->pcache+i) = 0;
   }

   return id;
}

double ListNetLearner::product(double *model, const _fvector& fv)
{
   double v = 0;
   _fiterator fit = fv.begin();
   for (; fit != fv.end(); ++fit)
   {
      v += *(this->model+(*fit).key)*(*fit).val;
   }

   return v;
}

void ListNetLearner::save(const char *model)
{
   if (!model)
   {
      return;
   }
   FILE *fp = NULL;
   if ((fp = fopen(model, "wb")) == NULL)
   {
      return;
   }
   fwrite(&this->fsize, sizeof(long), 1, fp);
   fwrite(this->model, sizeof(double), this->fsize, fp);
   fclose(fp);
}

void ListNetLearner::update(double eta, ilist &i, double cnf, const _cache& c)
{
   // sort instances by relevancy label
   sort(i.begin(), i.end(), lcomp());

   // assume that the first instance has the most highest relevancy
   vector<double> x;
   iliterator it = i.begin();
   for (; it != i.end(); ++it)
   {
      const _fvector& fv = (*it)->getvect();
      double v = this->product(this->model, fv);
      x.push_back(v);
   }
   // cache check
   // this block is prosessed in only case calling by boosting
   if (x.size() == c.size())
   {
      _cache::const_iterator cit = c.begin();
      for (int i = 0; cit != c.end(); ++cit, ++i)
      {
         x[i] *= cnf;
         x[i] += *cit;
      }
   }
   // partition
   double z = 0.;
   vector<double>::iterator xit = x.begin();
   for (; xit != x.end(); ++xit)
   {
      z = logsumexp(z,*xit,( xit == x.begin() ) );
   }

   // calc gradient
   _fvector grad;
   it = i.begin();
   long id = 0;
   // top 1
   double label = (*it)->getlabel();
   double y = 1. - myexp(x[id++] - z);
   const _fvector& fv = (*it)->getvect();
   _fiterator fit = fv.begin();
   for (; fit != fv.end(); ++fit)
   {
      _feature f;
      f.key = (*fit).key;
      f.val = y * (*fit).val;
      grad.push_back(f);
   }
   ++it;
   // other
   for (; it != i.end(); ++it)
   {
      double p = myexp(x[id++] - z);
      const _fvector& ffv = (*it)->getvect();
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

void ListNetLearner::cache(ranklist& corpus, double cnf, clist *c)
{
   if (!c)
   {
      return;
   }
   long id = 0;
   // ranking list
   rliterator it = corpus.begin();
   for (; it != corpus.end(); ++it, ++id)
   {
      // instance list
      sort( (*it)->begin(), (*it)->end(), lcomp() );
      ciliterator iit = (*it)->begin();
      _cache& cache = *(*c)[id];
      for (int cid = 0; iit != (*it)->end(); ++iit)
      {
         const _fvector& fv = (*iit)->getvect();
         cache[cid++] += this->product(this->model, fv);
      }
   }
}

// clist must be sorted in order to correct relevancy
//double ListNetLearner::crossent(ranklist& corpus, clist *c)
double ListNetLearner::crossent(ranklist& corpus, const clist& c)
{
   long id = 0;
   double top1ent = 0;
   // ranking list
   rliterator it = corpus.begin();
   for (; it != corpus.end(); ++it, ++id)
   {
      // instance list
      sort( (*it)->begin(), (*it)->end(), lcomp() );
      ciliterator iit = (*it)->begin();
      /*
      const _cache *cache;
      //= _cache();
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
         //if (c)
         // cache check
         if (c.size() == corpus.size())
         {
            //v += (*cache)[cid++];
            v += (*c[id])[cid++];
         }
         x.push_back(v);
      }
      // partition
      double z = 0;
      vector<double>::iterator xit = x.begin();
      for (; xit != x.end(); ++xit)
      {
         z = logsumexp(z, *xit, ( xit == x.begin() ) );
      }
      //double lossfunc = -1. * log( myexp(x[0] - z) );
      double lossfunc = (x[0] - z);
      top1ent += lossfunc;
   }
   return -top1ent;
}

//void ListNetLearner::test(ranklist& corpus, clist *c)
void ListNetLearner::test(ranklist& corpus, const clist& c)
{
   double ent = this->crossent(corpus, c);
   cout << "Cross Entropy: " << ent << endl;
}

ListNetRanker::~ListNetRanker()
{
}

ListNetRanker::ListNetRanker(const char *modelfile)
{
   if (!modelfile)
   {
      throw "Invalid modelfile";
   }
   this->readmodel(modelfile);
}

void ListNetRanker::readmodel(const char *modelfile)
{
   FILE *fp = NULL;
   if ((fp = fopen(modelfile, "rb")) == NULL)
   {
      return;
   }
   fread(&this->fsize, sizeof(long), 1, fp);
   this->model = new double[this->fsize];
   fread(this->model, sizeof(double), this->fsize, fp);
}

double ListNetRanker::product(double *model, const _fvector& fv)
{
   double v = 0;
   _fiterator fit = fv.begin();
   for (; fit != fv.end(); ++fit)
   {
      v += *(this->model+(*fit).key)*(*fit).val;
   }

   return v;
}

// in prediction, it doesn't have to sort by relevancy
void ListNetRanker::predict(ngilist& instance, const double cnf, _cache *c)
{
   ngiliterator it = instance.begin();
   vector<double> x;
   double z = 0;
   for (int cid = 0; it != instance.end(); ++it)
   {
      const _fvector& fv = (*it)->getvect();
      double v = this->product(this->model, fv);
      //if (c.size() == instance.size())
      if (c)
      {
         (*c)[cid] += cnf * v;
         v = (*c)[cid++];
      }
      x.push_back(v);
      z = logsumexp(z, v, ( it == instance.begin() ) );
   }

   it = instance.begin();
   for (int id = 0; it != instance.end(); ++it)
   {
      double p = myexp(x[id++] - z);
      (*it)->setpredict(p);
   }
}
