# ifndef __MLR_LEANER__
# define __MLR_LEANER__

# include "instance.hpp"
# include <cmath>

namespace mlr
{

typedef std::vector<const instance*> ilist;
typedef std::vector<instance*> ngilist;
typedef std::vector<const instance*>::iterator iliterator;
typedef std::vector<instance*>::iterator ngiliterator;
typedef std::vector<const instance*>::const_iterator ciliterator;
//typedef std::vector<const ilist*> ranklist;
typedef std::vector<ilist*> ranklist;
typedef std::vector<ilist*>::iterator rliterator;
//typedef std::vector<const ilist*>::iterator rliterator;
typedef std::vector<ilist*>::const_iterator crliterator;
typedef std::vector<double> _cache;
typedef std::vector<_cache*> clist;

class Learner
{
   public:
      Learner():model(NULL),pcache(NULL),c(0.0001),cc(0){}
      virtual ~Learner(){}
      /**
       * pure virtual
       * @param modelfile :finename for saving parameters
       */
      virtual void save(const char *modelfile) = 0;
      /**
       * pure virtual
       * @param eta :learning rate
       * @param i :instances
       * @param cnf :confidence parameter of this learner using only case of boosting
       * @param c :cache values of predicted by other learners for boosting
       */
      virtual void update(double eta, ilist& i, double cnf = 0, const _cache& c = _cache()) = 0;
      /**
       * pure virtual
       * @param model :model parameters
       * @param fv :feature vector
       */
      virtual double product(double *model, const _fvector& fv) = 0;
      /**
       * pure virtual
       * @param corpus :training corpus
       * @param cnf : confidence parameter
       * @param c :cache list
       */
      virtual void cache(ranklist& corpus, double cnf, clist *c = NULL) = 0;
      /**
       * virtual
       * @param corpus :training corpus
       * @param c :cache list
       */
      virtual void test(ranklist& corpus, const clist& c = clist()) = 0;
      /**
       * setter of penalty
       @param penalty :penalty weight for L2 regularization
       */
      virtual void setpenalty(double penalty)
      {
         if (penalty < 0)
         {
            throw "penalty must be more than 0";
         }
         this->c = penalty;
      }
   protected:
      double *model;
      double *pcache;

      double c;
      double cc;
   private:
      Learner(const Learner&);
      Learner& operator=(const Learner&);

};

static inline double myexp(double x)
{
# define A0 (1.0)
# define A1 (0.125)
# define A2 (0.0078125)
# define A3 (0.00032552083)
# define A4 (1.0172526e-5)
   if (x < -13.0)
   {
      return 0;
   }
   bool reverse = false;
   if (x < 0)
   {
      x = -x;
      reverse = true;
   }
   double y;
   y = A0+x*(A1+x*(A2+x*(A3+x*A4)));
   y *= y;
   y *= y;
   y *= y;
   if (reverse)
   {
      y = 1./y;
   }
   return y;
# undef A0
# undef A1
# undef A2
# undef A3
# undef A4
}

static inline double max (double x, double y)
{
   return x > y ? x: y;
}

static inline double min (double x, double y)
{
   return x > y ? y: x;
}

static inline double logsumexp(double x, double y, bool flg)
{
   if (flg)
   {
      return y; // init mode
   }
   if (x == y)
   {
      return x + 0.69314718055; // log(2)
   }
   double vmin = min(x,y);
   double vmax = max(x,y);
   if (vmax > vmin + 50)
   {
      return vmax;
   }
   else
   {
      return vmax + std::log(myexp(vmin-vmax)+1.0);
   }
}

static bool IsChangeQuery(long prev, long cur)
{
   return (prev != cur);
}

static void ilread(std::ifstream& in, ilist& l)
{
   if (in.eof())
   {
      return;
   }
   // init
   docline line;
   std::getline(in,line);
   instance *i = new instance;
   i->set(line);
   l.push_back(i);

   // current position
   std::streampos pos = in.tellg();
   long prev = i->getqid();
   long cur = prev;
   while (!in.eof())
   {
      docline line;
      std::getline(in, line);
      instance *i = new instance;
      i->set(line);
      cur = i->getqid();
      if ( IsChangeQuery(prev, cur) )
      {
         // return to saved position
         in.seekg(pos, std::ios_base::beg);
         break;
      }
      l.push_back(i);
      // save current position
      pos = in.tellg();
      prev = cur;
   }
}

static void ildelete(ilist& l)
{
   iliterator it = l.begin();
   for (; it != l.end(); ++it)
   {
      delete *it;
   }
}

}
# endif /* __MLR_LEANER__  */
