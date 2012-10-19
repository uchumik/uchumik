# ifndef __SHDCRF_H__
# define __SHDCRF_H__

# include <vector>
# include <string>
# include <map>
# include <cmath>
# include "mhash.hpp"
# include "sequence.hpp"

namespace SHDCRF
{
   typedef struct
   {
      // 実数素性を今後使えるようにしておく
      std::vector<std::pair<int, double> > f;
      unsigned int label;
   } feature_t;

   typedef struct
   {
      double _alpha;
      double _beta;
      double _lcost;
   } ynode_t;

   typedef struct
   {
      double _lc;
      std::vector<ynode_t> _y;
      bool f;
   } fbnode_t;

   typedef struct _vynode
   {
      bool f;
      int l;
      int h;
      double _lcost;
      _vynode *p;
   } vynode_t;

   typedef struct
   {
      double _lc;
      std::vector<vynode_t> _y;
   } vnode_t;

   typedef std::vector<feature_t> instance_t;
   typedef std::vector<std::vector<fbnode_t> > lattice_t;
   typedef std::vector<std::vector<vnode_t> > vlattice_t;
   typedef hashmmap::mhash<char*,unsigned int> index_t;
   typedef hashmmap::mhash<unsigned int,char*> rindex_t;
   typedef hashmmap::mhashiterator<char*,unsigned int> iditerator_t;
   typedef mmallocutilizer::mmalloc alloc_t;

   class ShdCrf
   {
      public:
         ShdCrf();
         ShdCrf(const char *basename, double lambda = 0.5, double penalty = 0.0001, double alpha = 0.005);
         ShdCrf(const char *basename, const char *corpus, double lambda = 0.5, double penalty = 0.0001, double alpha = 0.005);
         virtual ~ShdCrf();
         void learn(unsigned int iter);
         void init();
         void init(const char *corpus);
         void init(unsigned int bound);
         void init(unsigned int bound, const char *corpus);
         void init(unsigned int hiddens, unsigned int bound);
         void init(unsigned int hiddens, unsigned int bound, const char *corpus);

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
      protected:
         std::string save;
         std::string corpus;
         double *model;
         double *pcache;
         bool learnable;
         alloc_t *m;
         unsigned int lsize;
         unsigned int fsize;
         unsigned int bound;
         unsigned int hiddens;
         double lambda;
         double c;
         double cc;
         double alpha;

         unsigned int parameters;
         std::vector<instance_t> tset;
         index_t *labels;
         index_t *features;
         //rindex_t *rlabels;
         double eta;

         const void **b;
         bool fext();
         void ext(sequential::sequence *s);
         void ext(sequential::sequence *s, instance_t& instance);
         void initmodel();
         void store();
         void update(instance_t& ins);
         void initlattice(lattice_t& lattice, instance_t& instance);
         void _forward(lattice_t& lattice, int p, int k);
         void _backward(lattice_t& lattice, int p, int k);
         void _update(lattice_t& lattice, instance_t& instance, double z);
         //void __update(lattice_t& lattice, instance_t& instance, double z);
         void _upuhweight(std::map<int, double>& g, feature_t& f, int h, double e);
         void _upbhweight(std::map<int, double>& g, feature_t& f, int currenth, int prevh, double e);
         void _upyweight(std::map<int, double>& g, int h, int y, double e);
         void ent(std::map<int, double>& g, std::vector<double>& py, int h);
         void regularize(std::map<int, double>& g);

         double fb(lattice_t& lattice);

         void* ptr(const void **base, mysplay::offset_t b)
         {
            return (void*)((char*)*base+b);
         }
         void decay(int t)
         {
            double d = 1. + (double)t/this->tset.size();
            this->eta = 1./(this->lambda * d);
            this->cc += this->c/d;
         }
      private:
         ShdCrf(const ShdCrf&);
         ShdCrf& operator=(const ShdCrf&);
   };

   class ShdCrfTagger : public ShdCrf
   {
      typedef std::vector<std::pair<int,int> > label_t;
      public:
         ShdCrfTagger(const char *basename);
         virtual ~ShdCrfTagger();

         void tagging(sequential::sequence *s, label_t& labelset);
         void tagging(const char *corpus);
         void print(sequential::sequence *s, label_t& labelset);
      protected:
         ShdCrfTagger();
         void learn(unsigned int iter) {}
         void init() {};
         void init(const char *corpus) {}
         void init(unsigned int bound) {}
         void init(unsigned int bound, const char *corpus) {};
         void init(unsigned int hiddens, unsigned int bound) {}
         void init(unsigned int hiddens, unsigned int bound, const char *corpus) {};
         void read(const char *model);
         void backtrack(vynode_t *p, label_t& labels);
         vynode_t* viterbi(vlattice_t &lattice);
         double viterbi(vlattice_t &lattice, int h, int y, int p);
         void initlattice(vlattice_t& l, instance_t& i);
         std::vector<std::string> surface;
      private:
         ShdCrfTagger(const ShdCrfTagger&);
         ShdCrfTagger& operator=(const ShdCrfTagger&);
   };
}
# endif /* __SHDCRF_H__ */
