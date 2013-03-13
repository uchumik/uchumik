# include "instance.hpp"
# include <sstream>
# include <fstream>
# include <iostream>
# include <cstring>

using namespace mlr;
using namespace std;

instance::instance()
: qid(-1), docid(-1), label(0), predict(0)
{
}

instance::~instance()
{
}

const _fvector& instance::getvect() const
{
   return this->fv;
}

void instance::set(docline& line)
{
   chomp(line);
   docline c(line);
   for (int i = 0, p = 0; i < c.size(); ++i)
   {
      if (strncmp(c.c_str()+p,"#docid",6) == 0) // set docid
      {
         // #docid = [0-9]+
         for (; c[i] != '\0'; ++i);
         this->setid(c, p+9, i-p-9, this->docid);
         break; // always, docid appears at the end of instance
      }
      if (strncmp(c.c_str()+p,"qid",3) == 0)
      {
         for (; c[i] != ' '; ++i);
         this->setid(c, p+4, i-p-4, this->qid);
         p = i+1;
         continue;
      }
      if (!(c[i] == ' '))
      {
         continue;
      }
      if (p == 0) // label
      {
         this->setlabel(c, p, i-p, this->label);
      }
      else
      {
         this->setfeature(c, p, i-p);
      }
      p = i+1;
   }
   // key:0 is bias
   _feature b;
   b.key = 0;
   b.val = 1.;
   this->fv.push_back(b);
   // sort fv using key
   sort(this->fv.begin(), this->fv.end(), fcomp());
}

void instance::setpredict(double predict)
{
   if (predict < 0)
   {
      throw "predicted label must be more than 0";
   }
   this->predict = predict;
}

void instance::setfeature(docline& doc, int s, int l)
{
   if (l <= 0)
   {
      throw "Unexpected index: length <= 0";
   }

   string token(doc, s, l);
   int pos = 0;
   for (; token[pos] != ':'; ++pos);
   string key(token, 0, pos);
   string val(token, pos+1, token.size()-pos-1);
   _feature f;
   istringstream ks;
   istringstream vs;
   ks.str(key);
   vs.str(val);
   ks >> f.key;
   vs >> f.val;
   this->fv.push_back(f);
}

void instance::setlabel(docline& doc, int s, int l, double& label)
{
   if (l <= 0)
   {
      throw "Unexpected index: length <= 0";
   }
   string token(doc, s, l);
   istringstream is;
   is.str(token);
   is >> label;
}

void instance::setid(docline& doc, int s, int l, long& id)
{
   if (l <= 0)
   {
      throw "Unexpected index: length <= 0";
   }
   string token(doc, s, l);
   istringstream is;
   is.str(token);
   is >> id;
}

double instance::getlabel() const
{
   return this->label;
}

double instance::getpredict() const
{
   return this->predict;
}

long instance::getqid() const
{
   return this->qid;
}

long instance::getdocid() const
{
   return this->docid;
}
