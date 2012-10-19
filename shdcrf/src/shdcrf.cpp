# include "shdcrf.hpp"
# include "xorshift.hpp"
# include <map>

extern "C"
{
# include <sys/stat.h>
}
using namespace SHDCRF;
using namespace std;
using namespace hashmmap;
using namespace mysplay;
using namespace mmallocutilizer;
using namespace sequential;
using namespace randomsampling;

ShdCrf::ShdCrf()
{
}

ShdCrf::ShdCrf(const char *basename, double lambda, double penalty, double alpha)
: save(basename), corpus(""), model(NULL), pcache(NULL), learnable(false), lsize(0), fsize(0), bound(1), hiddens(5), lambda(lambda), c(penalty), cc(0), alpha(alpha)
{
   string shm = save + ".shm";
   string l = save + ".l_index";
   //string r = save + ".r_index";
   string f = save + ".f_index";
   struct stat st;
   if (stat(shm.c_str(),&st) != 0)
   {
      this->m = new alloc_t(shm.c_str(), Create);
   }
   else
   {
      this->m = new alloc_t(shm.c_str(), ReadWrite);
   }
   this->labels = new index_t(this->m, l.c_str());
   //this->rlabels = new rindex_t(this->m, r.c_str());
   this->features = new index_t(this->m, f.c_str());
}

   ShdCrf::ShdCrf(const char *basename, const char *corpus, double lambda, double penalty, double alpha)
: save(basename), corpus(corpus), model(NULL), pcache(NULL), learnable(false), lsize(0), fsize(0), bound(1), hiddens(5), lambda(lambda), c(penalty), cc(0), alpha(alpha)
{
   string shm = save + ".shm";
   string l = save + ".l_index";
   //string r = save + ".r_index";
   string f = save + ".f_index";
   struct stat st;
   if (stat(shm.c_str(),&st) != 0)
   {
      this->m = new alloc_t(shm.c_str(), Create);
   }
   else
   {
      this->m = new alloc_t(shm.c_str(), ReadWrite);
   }
   this->labels = new index_t(this->m, l.c_str());
   //this->rlabels = new rindex_t(this->m, r.c_str());
   this->features = new index_t(this->m, f.c_str());
}

ShdCrf::~ShdCrf()
{
   delete this->labels;
   delete this->features;
   //delete this->rlabels;
   if (this->pcache)
   {
      delete[] this->pcache;
   }
   delete this->m;
}

void ShdCrf::initlattice(lattice_t& l, instance_t& i)
{
   for (int t = 0; t < (int)l.size(); ++t)
   {
      l[t].resize(this->hiddens);
      for (int j = 0; j < (int)this->hiddens; ++j)
      {
         // init hnode
         l[t][j].f = false;
         feature_t fv = i[t];
         vector<pair<int,double> >::iterator fit = fv.f.begin();
         for (; fit != fv.f.end(); ++fit)
         {
            int id = (*fit).first;
            l[t][j]._lc += *(this->model+id*this->hiddens+j)*(*fit).second;
         }

         // init ynodes
         l[t][j]._y.resize(this->lsize);
         for (int k = 0; k < (int)this->lsize; ++k)
         {
            l[t][j]._y[k]._alpha = 0.;
            l[t][j]._y[k]._beta = 0.;
            int b = this->fsize*this->hiddens
               + this->hiddens*this->hiddens;
            l[t][j]._y[k]._lcost += l[t][j]._lc + *(this->model+b+j*this->lsize+k);
         }
      }
   }
}

void ShdCrf::_backward(lattice_t& l, int p, int k)
{
   if (!l[p][k].f)
   {
      return;
   }
   if (p < (int)l.size()-1)
   {
      for (int h = 0; h < (int)this->hiddens; ++h)
      {
         this->_backward(l, p+1, h);
      }
   }
   //cout << "B:" << p << "," << k << endl;
   if (p == (int)l.size()-1)
   {
      for (int y = 0; y < (int)this->lsize; ++y)
      {
         l[p][k]._y[y]._beta = logsumexp(l[p][k]._y[y]._beta, 0., true);
      }
   }
   else
   {
      // j represents next hidden variable
      // k represents current hidden variable
      for (int y = 0; y < (int)this->lsize; ++y)
      {
         bool flg = true;
         for (int j = 0; j < (int)this->hiddens; ++j)
         {
            double t = *(this->model+this->hiddens*this->fsize+this->hiddens*j+k);
            for (int ny = 0; ny < (int)this->lsize; ++ny)
            {
               double c = l[p+1][j]._y[ny]._lcost;
               l[p][k]._y[y]._beta = logsumexp(l[p][k]._y[y]._beta, l[p+1][j]._y[ny]._beta+c+t, flg);
               if (flg)
               {
                  flg = false;
               }
            }
         }
      }
   }
   l[p][k].f = false;
}

void ShdCrf::_forward(lattice_t& l, int p, int k)
{
   if (l[p][k].f)
   {
      return;
   }
   if (p > 0)
   {
      for (int h = 0; h < (int)this->hiddens; ++h)
      {
         this->_forward(l, p-1, h);
      }
   }
   //cout << "F:" << p << "," << k << endl;
   if (p == 0)
   {
      for (int y = 0; y < (int)this->lsize; ++y)
      {
         double c = l[p][k]._y[y]._lcost;
         l[p][k]._y[y]._alpha = logsumexp(l[p][k]._y[y]._alpha, c, true);
      }
   }
   else
   {
      // j represents previous hidden variable
      // k represents current hidden variable
      for (int y = 0; y < (int)this->lsize; ++y)
      {
         double c = l[p][k]._y[y]._lcost;
         bool flg = true;
         for (int j = 0; j < (int)this->hiddens; ++j)
         {
            double t = *(this->model+this->hiddens*this->fsize+this->hiddens*k+j);
            for (int py = 0; py < (int)this->lsize; ++py)
            {
               l[p][k]._y[y]._alpha = logsumexp(l[p][k]._y[y]._alpha, l[p-1][j]._y[py]._alpha+c+t,flg);
               if (flg)
               {
                  flg = false;
               }
            }
         }
      }
   }
   l[p][k].f = true;
}

double ShdCrf::fb(lattice_t& l)
{
   for (int h = 0; h < (int)this->hiddens; ++h)
   {
      this->_forward(l, l.size()-1, h);
   }
   for (int h = 0; h < (int)this->hiddens; ++h)
   {
      this->_backward(l, 0, h);
   }

   double z1 = 0.;
   double z2 = 0.;
   bool flg = true;
   for (int h = 0; h < (int)this->hiddens; ++h)
   {
      double c = l[0][h]._lc;
      for (int y = 0; y < (int)this->lsize; ++y)
      {
         double d = l[0][h]._y[y]._lcost;
         z1 = logsumexp(z1, l[l.size()-1][h]._y[y]._alpha, flg);
         z2 = logsumexp(z2, l[0][h]._y[y]._beta+d, flg);
         if (flg)
         {
            flg = false;
         }
      }
   }

   return max(z1,z2);
}

void ShdCrf::_upuhweight(map<int, double>& g, feature_t& f, int h, double e)
{
   vector<pair<int, double> >::iterator fit = f.f.begin();
   for (; fit != f.f.end(); ++fit)
   {
      int id = (int)(*fit).first * this->hiddens + h;
      if (g.find(id) == g.end())
      {
         g[id] = 0.;
      }
      g[id] -= e*(*fit).second;;
   }
}

void ShdCrf::_upbhweight(map<int, double>& g, feature_t& f, int ch, int ph, double e)
{
   int id = this->hiddens*this->fsize+this->hiddens*ch+ph;
   if (g.find(id) == g.end())
   {
      g[id] = 0.;
   }
   g[id] -= e;
}

void ShdCrf::_upyweight(map<int, double>& g, int h, int y, double e)
{
   int id = this->hiddens*this->fsize+this->hiddens*this->hiddens+h*this->lsize+y;
   g[id] -= e;
}

void ShdCrf::ent(map<int, double>& g, vector<double>& py, int h)
{
   int b = this->hiddens*this->fsize+this->hiddens*this->hiddens;
   for (int y = 0; y < (int)this->lsize; ++y)
   {
      double et = 0.;
      for (int yy = 0; yy < (int)this->lsize; ++yy)
      {
         et += py[yy] * *(this->model+b+h*this->lsize+yy);

      }
      int id = b+h*this->lsize+y;
      g[id] += this->alpha * py[y] * (*(this->model+b+h*this->lsize+y) - et);
      //cout << "id:" << g[id] << endl;
   }
}

void ShdCrf::_update(lattice_t& l, instance_t& ins, double z)
{
   map<int, double> g;
   int length = l.size();
   // position i
   for (int i = 0; i < length; ++i)
   {
      // must culculate 
      // p(h_t-1,h_t|y^i,x) = p(h_t-1,h_t,y^i|x)/Sigma_{h_t-1,h_t}p(h_t-1,h_t,y^i|x)
      // p(y_t|x) = Sigma_{h_t-1,h_t,y_t-1}p(h_t-1,h_t,y_t-1,y_t|x)
      // p(h_t|y^i,x) = Sigma_{h_t-1}p(h_t-1,h_t|y^i,x)
      // p(h_t-1,h_t|x) = Sigma_{y_t-1,y_t} p(h_t-1,h_t,y_t-1,y_t|x)
      // p(h_t|x) = Sigma_{h_t-1,y_t-1,y_t} p(h_t-1,h_t,y_t-1,y_t|x)
      // p(h_t|x)p(h_t,y_t) => p(h_t|x)p(h_t,y_t|x)
      // p(y|h) = p(y|x)/p(h|x)

      vector< vector< vector<double> > > phhy(this->hiddens); // p(h_t-1,h_t,y_t|x)
      vector< vector<double> > phh(this->hiddens); // p(h_t-1,h_t|x)
      vector<double> phx(this->hiddens, 0.); // p(h_t|x)
      vector< vector<double> > pyh(this->hiddens); // p(y|h)
      vector< vector<double> > phy(this->hiddens); // p(h_t,y_t|x) this parameters are used for updating in a case of i==0
      for (int j = 0; j < this->hiddens; ++j)
      {
         phh[j].resize(this->hiddens, 0.);
         phhy[j].resize(this->hiddens);
         pyh[j].resize(this->lsize, 0.);
         for (int k = 0; k < this->hiddens; ++k)
         {
            phhy[j][k].resize(this->lsize, 0.);
         }
         // add
         phy[j].resize(this->lsize, 0.);
      }
      vector<double> pyx(this->lsize, 0.); // p(y_t|x)
      // current label y
      for (int y = 0; y < this->lsize; ++y)
      {
         // current hidden variable h
         for (int h = 0; h < this->hiddens; ++h)
         {
            if (i == 0)
            {
               // p(h_t = h, y_t = y|x)
               double phyx = myexp(l[i][h]._y[y]._beta+l[i][h]._y[y]._lcost-z);
               phx[h] += phyx;
               pyx[y] += phyx;
               phy[h][y] = phyx;
            }
            else
            {
               // previous label py
               for (int py = 0; py < this->lsize; ++py)
               {
                  // previous hidden variable ph
                  for (int ph = 0; ph < this->hiddens; ++ph)
                  {
                     // parameter of a edge feature g_k(h_t-1,h_t,x)
                     double t = *(this->model+this->hiddens*this->fsize+this->hiddens*h+ph);
                     // p(h_t-1,h_t,y_t-1,y_t|x)
                     double phhyyx = myexp(l[i-1][ph]._y[py]._alpha+l[i][h]._y[y]._beta+l[i][h]._y[y]._lcost+t-z);
                     phhy[h][ph][y] += phhyyx;
                     phh[h][ph] += phhyyx;
                     pyx[y] += phhyyx;
                     phx[h] += phhyyx;

                     // add
                     phy[h][y] += phhyyx;
                  }
               }
            }
         }
         // end for current hidden variable h
      }
      // end for current label y


      /*
         double check2 = 0.;
         for (int h = 0; h < this->hiddens; ++h)
         {
         for (int y = 0; y < this->lsize; ++y)
         {
         check += phy[h][y];
         for (int ph = 0; ph < this->hiddens; ++ph)
         {
         check2 += phhy[h][ph][y];
         }
         }
         }
         cout << "check1 Sigma P(h,y|x) = " << check << endl;
         cout << "check2 Sigma P(hp,h,y|x) = " << check2 << endl;
       */

      // update grad vector
      vector<double> phyix(this->hiddens); // p(h_t|y^i,x)
      for (int h = 0; h < this->hiddens; ++h)
      {
         // p(h_t|y^i,x)
         double p = (pyx[ins[i].label] == 0)? 0:phy[h][ins[i].label]/pyx[ins[i].label];
         this->_upuhweight(g, ins[i], h, -p);
         this->_upuhweight(g, ins[i], h, phx[h]);
         this->_upyweight(g, h, ins[i].label, -p);
         for (int ph = 0; ph < this->hiddens; ++ph)
         {
            double bp = (pyx[ins[i].label] == 0)? 0:phhy[h][ph][ins[i].label]/pyx[ins[i].label];
            this->_upbhweight(g, ins[i], h, ph, -bp);
            this->_upbhweight(g, ins[i], h, ph, phh[h][ph]);
         }
         for (int y = 0; y < this->lsize; ++y)
         {
            double yp = phy[h][y] * phx[h];
            this->_upyweight(g, h, y, yp);
         }
      }

      // update grad vector
      int yi = ins[i].label;
      // calc p(y|h)
      vector<double> hp(this->hiddens);
      for (int y = 0; y < this->lsize; ++y)
      {
         for (int h = 0; h < this->hiddens; ++h)
         {
            hp[h] += phy[h][y];
         }
      }
      for (int y = 0; y < this->lsize; ++y)
      {
         double denom = 0.;
         for (int h = 0; h < this->hiddens; ++h)
         {
            denom += phy[h][y];
         }
         for (int h = 0; h < this->hiddens; ++h)
         {
            pyh[h][y] = (hp[h] == 0)? 0: phy[h][y]/hp[h];
         }
      }
      // minimize entropy
      vector< vector<double> >::iterator it = pyh.begin();
      for (int h = 0; it != pyh.end(); ++it, ++h)
      {
         this->ent(g, *it, h);
         //cout << "p(h" << h << "): " <<hp[h] << " ";
      }
      //cout << endl;
      // end update grad vector

      // check
      cout << "t: " << i << " correct: " << yi;
      for (int y = 0; y < this->lsize; ++y)
      {
         cout << " p(y" << y << "|x): " << pyx[y];
      }
      cout << endl;
      for (int h = 0; h < this->hiddens; ++h)
      {
         cout << "p(h" << h << "|x): " << phx[h] << "\t";
      }
      cout << endl;
      for (int y = 0; y < this->lsize; ++y)
      {
         for (int h = 0; h < this->hiddens; ++h)
         {
            cout << "p(y" << y << "|h" << h << "): " << pyh[h][y] << "\t";
         }
         cout <<endl;
      }
   }
   // update parameters
   map<int, double>::iterator git = g.begin();
   for (; git != g.end(); ++git)
   {
      *(this->model+(*git).first) += this->eta * (*git).second;
   }
   this->regularize(g);
   // end of sequence
}

void ShdCrf::regularize(map<int, double>& g)
{
   map<int, double>::iterator git = g.begin();
   for (; git != g.end(); ++git)
   {
      double p = this->cc - this->pcache[(*git).first];
      this->pcache[(*git).first] = this->cc;
      *(this->model+(*git).first) /= (1. + p);
   }
}

void ShdCrf::update(instance_t& instance)
{
   int length = instance.size();
   // build lattice
   lattice_t l(length);
   // init lattice
   this->initlattice(l, instance);
   //partition_t z = this->fb(l);
   double z = this->fb(l);
   // update
   this->_update(l, instance, z);
   //this->__update(l, instance, z);
}

void ShdCrf::store()
{
   ifstream in(this->corpus.c_str());
   sequence s;
   while (!in.eof())
   {
      sqread(in, &s);
      if (s.getr() == 0)
      {
         continue;
      }
      instance_t ins;
      this->ext(&s, ins);
      this->tset.push_back(ins);
      s.clear();
   }
}

void ShdCrf::learn(unsigned int iter)
{
   if (!this->learnable)
   {
      return;
   }
   this->store();

   // iteration
   int t = 0;
   for (unsigned int i = 0; i < iter; ++i)
   {
      cout << "iter: " << i << endl;
      vector<instance_t>::iterator it = tset.begin();
      for (; it != tset.end(); ++it)
      {
         this->decay(t++);
         this->update(*it);
         cout << endl;
      }
   }
   // save model
   string modelfile = save + ".model";
   FILE *fp = NULL;
   if ((fp = fopen(modelfile.c_str(), "wb")) == NULL)
   {
      throw "Couldn't make modelfile";
   }
   offset_t off = this->m->offset(this->model);
   //fwrite(&this->parameters,sizeof(unsigned int),1,fp);
   fwrite(&this->hiddens,sizeof(unsigned int),1,fp);
   fwrite(&this->lsize,sizeof(unsigned int),1,fp);
   fwrite(&this->fsize,sizeof(unsigned int),1,fp);
   fwrite(&off,sizeof(offset_t),1,fp);
   fclose(fp);

}

void ShdCrf::initmodel()
{
   unsigned int ufeatures = this->hiddens * this->fsize;
   unsigned int bfeatures = this->hiddens * this->hiddens;
   //* this->fsize;
   unsigned int yfeatures = this->hiddens * this->lsize;
   this->parameters = ufeatures + bfeatures + yfeatures;

   this->b = this->m->baseaddress();
   offset_t off = m->allocate(this->parameters*sizeof(double));
   this->model = (double*)this->ptr(this->b, off);
   this->pcache = new double[this->parameters];

   for (unsigned int i = 0; i < this->parameters; ++i)
   {
      *(this->model+i) = uniform::xor128n();
      *(this->pcache+i) = 0.;
   }

   cout << "Labels: " << this->lsize << endl;
   cout << "Features: " << this->fsize << endl;
   cout << "Hidden Valiables:" << this->hiddens << endl;
   cout << "Parameters: " << this->parameters << endl;

}

void ShdCrf::init()
{
   if (this->corpus == "")
   {
      return;
   }
   if (!this->fext())
   {
      throw "ERR:";
   }
   this->initmodel();

   this->learnable = true;
}

void ShdCrf::init(const char *corpus)
{
   if (!corpus)
   {
      return;
   }
   this->corpus = corpus;
   this->init();
   /*
      if (!this->fext())
      {
      throw "ERR:";
      }
    */
}

void ShdCrf::init(unsigned int bound)
{
   this->bound = bound;
   this->init();
}

void ShdCrf::init(unsigned int bound, const char *corpus)
{
   this->bound = bound;
   this->init(corpus);
}

void ShdCrf::init(unsigned int hiddens, unsigned int bound)
{
   this->hiddens = hiddens;
   this->init(bound);
}

void ShdCrf::init(unsigned int hiddens, unsigned int bound, const char *corpus)
{
   this->hiddens = hiddens;
   this->init(bound, corpus);
}

void ShdCrf::ext(sequence *s, instance_t& instance)
{
   int l = s->getc();
   int r = s->getr();
   for (int i = 0; i < r; ++i)
   {
      feature_t fv;
      for (int j = 0; j < l-1; ++j)
      {
         string x = s->get(i,j);
         unsigned int *v = (*this->features)[(char*)x.c_str()];
         if (v)
         {
            pair<int, double> f(*v, 1.);
            fv.f.push_back(f);
         }
      }
      string y = s->get(i, l-1);
      unsigned int *c = (*this->labels)[(char*)y.c_str()];
      fv.label = *c;
      instance.push_back(fv);
   }
}

void ShdCrf::ext(sequence *s)
{
   int l = s->getc();
   int r = s->getr();

   for (int i = 0; i < r; ++i)
   {
      for (int j = 0; j < l-1; ++j)
      {
         string x = s->get(i,j);
         unsigned int *v = (*this->features)[(char*)x.c_str()];
         if (!v)
         {
            this->features->insert((char*)x.c_str(), 1,(unsigned int)x.size()+1, sizeof(unsigned int));
         }
         else
         {
            ++*v;
         }
      }
      string y = s->get(i,l-1);
      unsigned int *c = (*this->labels)[(char*)y.c_str()];
      if (!c)
      {
         //this->rlabels->insert(this->lsize, (char*)y.c_str(), sizeof(unsigned int), (unsigned int)y.size()+1);
         this->labels->insert((char*)y.c_str(), this->lsize++, (unsigned int)y.size()+1, sizeof(unsigned int));
      }
   }
}

bool ShdCrf::fext()
{
   ifstream in(this->corpus.c_str());
   sequence s;
   while (!in.eof())
   {
      sqread(in, &s);
      if (s.getr() == 0)
      {
         continue;
      }
      this->ext(&s);
      s.clear();
   }

   // pruning
   iditerator_t it = this->features->begin();
   for (; it != this->features->end(); ++it)
   {
      if ((*it).val < this->bound)
      {
         this->features->remove(*((*it).key));
      }
      else
      {
         (*it).val = this->fsize++;
      }
   }

   /*
      iditerator_t ft = this->features->begin();
      for (; ft != this->features->end(); ++ft)
      {
      cout << "key: " << *((*ft).key) << endl;
      cout << "id: " << (*ft).val << endl;
      }

      iditerator_t jt = this->labels->begin();
      for (; jt != this->labels->end(); ++jt)
      {
      cout << "key: " << *((*jt).key) << endl;
      cout << "id: " << (*jt).val << endl;
      }
    */
   return true;
}

ShdCrfTagger::ShdCrfTagger()
{
}

   ShdCrfTagger::ShdCrfTagger(const char *basename)
: ShdCrf(basename)
{
   this->b = this->m->baseaddress();
   // read model
   string modelfile = save + ".model";
   offset_t off = 0;
   FILE *fp = NULL;
   if ((fp = fopen(modelfile.c_str(), "rb")) == NULL)
   {
      throw "Couldn't open modelfile";
   }
   fread(&this->hiddens,sizeof(unsigned int),1,fp);
   fread(&this->lsize,sizeof(unsigned int),1,fp);
   fread(&this->fsize,sizeof(unsigned int),1,fp);
   fread(&off,sizeof(offset_t),1,fp);
   fclose(fp);

   this->model = (double*)this->ptr(this->b, off);
   // set label string
   this->surface.resize(this->lsize);
   iditerator_t it = this->labels->begin();
   for (; it != this->labels->end(); ++it)
   {
      this->surface[(*it).val] = *(*it).key;
   }
}

ShdCrfTagger::~ShdCrfTagger()
{
}

double ShdCrfTagger::viterbi(vlattice_t& l, int h, int y, int p)
{
   if (l[p][h]._y[y].f)
   {
      return l[p][h]._y[y]._lcost;
   }
   // current pos
   // i, h, y
   bool init = false;
   double max = 0.;
   vynode_t *btrack = NULL;
   for (int j = 0; j < (int)this->hiddens; ++j)
   {
      double t = *(this->model+this->hiddens*this->fsize+this->hiddens*h+j);
      for (int k = 0; k < (int)this->lsize; ++k)
      {
         double c = this->viterbi(l, j, k, p-1) + t;
         if (c > max || !init)
         {
            max = c;
            btrack = &l[p-1][j]._y[k];
            init = true;
         }
      }
   }
   l[p][h]._y[y]._lcost += max;
   l[p][h]._y[y].p = btrack;
   l[p][h]._y[y].f = true;

   return l[p][h]._y[y]._lcost;
}

vynode_t* ShdCrfTagger::viterbi(vlattice_t& l)
{
   int eos = l.size()-1;
   double max = 0;
   bool init = false;
   vynode_t *b = NULL;
   for (int j = 0; j < (int)this->hiddens; ++j)
   {
      for (int k = 0; k < (int)this->lsize; ++k)
      {
         double c = this->viterbi(l, j, k, eos);
         if (c > max || !init)
         {
            max = c;
            b = &l[eos][j]._y[k];
            init = true;
         }
      }
   }
   return b;
   //position_t id = this->viterbi(l, l.size()-1);
}

void ShdCrfTagger::initlattice(vlattice_t& l, instance_t& i)
{
   for (int t = 0; t < (int)l.size(); ++t)
   {
      l[t].resize(this->hiddens);
      for (int j = 0; j < (int)this->hiddens; ++j)
      {
         // init hnode
         l[t][j]._lc = 0;
         feature_t fv = i[t];
         vector<pair<int,double> >::iterator fit = fv.f.begin();
         for (; fit != fv.f.end(); ++fit)
         {
            int id = (*fit).first;
            l[t][j]._lc += *(this->model+id*this->hiddens+j)*(*fit).second;
         }

         // init ynode
         l[t][j]._y.resize(this->lsize);
         for (int k = 0; k < (int)this->lsize; ++k)
         {
            int b = this->fsize*this->hiddens
               + this->hiddens*this->hiddens;
            l[t][j]._y[k]._lcost = l[t][j]._lc + *(this->model+b+j*this->lsize+k);
            l[t][j]._y[k].p = NULL;
            l[t][j]._y[k].f = (t==0)?true:false;
            l[t][j]._y[k].l = k;
            l[t][j]._y[k].h = j;
         }
      }
   }
}

void ShdCrfTagger::backtrack(vynode_t *p, label_t& labels)
{
   for (; p != NULL; p = p->p)
   {
      pair<int, int> label;
      label.first = p->h;
      label.second = p->l;
      labels.push_back(label);
   }
   reverse( labels.begin(), labels.end() );
}

void ShdCrfTagger::tagging(sequence *s, label_t& ls)
{
   instance_t ins;
   this->ext(s, ins);
   // build lattice
   int length = ins.size();
   vlattice_t l(length);
   this->initlattice(l, ins);
   // backtrack pointer
   vynode_t *bt = this->viterbi(l);
   ls.resize(length);
   this->backtrack(bt, ls);
}

void ShdCrfTagger::print(sequence *s, label_t& l)
{
   int c = s->getc();
   int r = s->getr();
   for (int i = 0; i < r; ++i)
   {
      for (int j = 0; j < c; ++j)
      {
         cout << s->get(i,j) << "\t";
      }
      cout << l[i].first << "\t"
         << this->surface[l[i].second] << endl;
   }
}

void ShdCrfTagger::tagging(const char *corpus)
{
   ifstream in(corpus);
   sequence s;
   while(!in.eof())
   {
      sqread(in, &s);
      if (s.getr() == 0)
      {
         continue;
      }
      label_t lset;
      this->tagging(&s, lset);
      this->print(&s, lset);
      s.clear();
   }
}
