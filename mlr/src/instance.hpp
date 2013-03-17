# ifndef __MLR_INSTANCE__
# define __MLR_INSTANCE__

# include <vector>
# include <string>
# include <sstream>
# include <fstream>
# include <cstring>

namespace mlr
{

typedef struct
{
   unsigned int key;
   double val;
} _feature;

typedef std::vector<_feature> _fvector;
typedef std::vector<_feature>::const_iterator _fiterator;
typedef std::string docline;

class fcomp
{
   public:
      bool operator()(const _feature& a, const _feature& b)
      {
         return (a.key < b.key);
      }
};

class instance
{
   public:
      instance();
      virtual ~instance();
      /**
       * @return const _fvector& : Usage: const _fvector& fv = instance_object.getvect()
       */
      virtual const _fvector& getvect() const;
      virtual double getlabel() const;
      virtual double getpredict() const;
      virtual long getqid() const;
      virtual long getdocid() const;
      virtual long getposition() const;
      /**
       * @param docline& line :LETOR format string
       */
      virtual void set(docline& line);
      /**
       * @param double predict :predicted label
       */
      virtual void setpredict(double predict);
      /**
       * @param long position :ranking position
       */
      virtual void setposition(long position);
   protected:
      long qid;
      long docid;
      long position;
      double label;
      double predict;
      _fvector fv;

      virtual void setid(docline& doc, int s, int e, long& id);
      virtual void setlabel(docline& doc, int s, int e, double& label);
      virtual void setfeature(docline& doc, int s, int e);
   private:
      instance(const instance&);
      instance& operator=(const instance&);
};

/**
 * @return index : index of key in a feature vector
 */
static int findkey(unsigned int key, const _fvector& fv)
{
   if (fv.size() == 0)
   {
      return -1;
   }
   int h = 0;
   int t = fv.size()-1;
   while (h < t)
   {
      int c = (h+t)/2;
      if (fv[c].key == key)
      {
         return c;
      }
      else if (fv[c].key > key)
      {
         h = c+1;
      }
      else
      {
         t = c-1;
      }
   }

   return -1;
}

static void chomp(char *str)
{
   int len = std::strlen(str);
   if (*(str+len-1) == '\n')
   {
      *(str+len-1) = '\0';
      --len;
   }
   if (*(str+len-1) == '\r')
   {
      *(str+len-1) = '\0';
   }
}

static void chomp(docline& doc)
{
   int len = doc.size();
   if (doc[len-1] == '\n')
   {
      doc[len-1] = '\0';
      --len;
   }
   if (doc[len-1] == '\r')
   {
      doc[len-1] = '\0';
   }
}

}
# endif /* __MLR_INSTANCE__ */
