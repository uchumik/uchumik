# ifndef __MHASH__
# define __MHASH__

extern "C"
{
# include <limits.h>
# include <sys/mman.h>
# include <sys/stat.h>
}
# include <mmalloc.hpp>
# include <splaytree.hpp>
# include <typeinfo>
# include <string>
# include <iterator>

# define HASHSIZE 2597
# include <iostream>

namespace hashmmap
{
   template <class K>
   class mhashfuncs
   {
      public:
         unsigned int operator()(K key)
         {
            return (unsigned int)key % HASHSIZE;
         }
   };

   template <>
   class mhashfuncs<char*>
   {
      public:
         unsigned int operator()(const char *s)
         {
            unsigned int v;
            for (v = 0; *s != '\0'; ++s)
            {
               v = ((v << CHAR_BIT) + *s) % HASHSIZE;
            }
            return (unsigned int)v;
         }
   };

   template <class K, class V>
      class hashnode
      {
         public:
            K key;
            V val;
      };

   template <class K, class V>
      class hashnode<K*,V>
      {
         public:
            const void **base;
            mysplay::offset_t koff;
            K **key;
            V val;
      };

   template <class K, class V>
      class hashnode<K,V*>
      {
         public:
            const void **base;
            mysplay::offset_t voff;
            K key;
            V **val;
      };

   template <class K, class V>
      class hashnode<K*,V*>
      {
         public:
            const void **base;
            mysplay::offset_t koff;
            mysplay::offset_t voff;
            K **key;
            V **val;
      };

   template <class K, class V>
      class mhashcompare
      {
         public:
            bool operator()(const hashnode<K,V>& a, const hashnode<K,V>& b)
            {
               return (a.key < b.key);
            }
      };

   template <class K, class V>
      class mhashcompare<K*,V>
      {
         public:
            bool operator()(const hashnode<K*,V>& a, const hashnode<K*,V>& b)
            {
               if (a.koff == (mysplay::offset_t)-1 ||
                     b.koff == (mysplay::offset_t)-1)
               {
                  return false;
               }
               if (a.koff != (mysplay::offset_t)-1)
               {
                  *(a.key) = (K*)((char*)(*(a.base))+a.koff);
               }
               if (b.koff != (mysplay::offset_t)-1)
               {
                  *(b.key) = (K*)((char*)(*(b.base))+b.koff);
               }
               return (std::strcmp(*(a.key),*(b.key)) < 0);
            }
      };

   template <class K, class V>
      class mhashcompare<K,V*>
      {
         public:
            bool operator()(const hashnode<K,V*>& a, const hashnode<K,V*>& b)
            {
               return (a.key < b.key);
            }
      };

   template <class K, class V>
      class mhashcompare<K*,V*>
      {
         public:
            bool operator()(const hashnode<K*,V*>& a, const hashnode<K*,V*>& b)
            {
               if (a.koff == (mysplay::offset_t)-1 ||
                     b.koff == (mysplay::offset_t)-1)
               {
                  return false;
               }
               if (a.koff != (mysplay::offset_t)-1)
               {
                  *(a.key) = (K*)((char*)*(a.base)+a.koff);
               }
               if (b.koff != (mysplay::offset_t)-1)
               {
                  *(b.key) = (K*)((char*)*(b.base)+b.koff);
               }
               return (std::strcmp(*(a.key),*(b.key)) < 0);
            }
      };

   template <class K, class V>
      class keyset
      {
         public:
            void operator()(hashnode<K,V>& n, K& key, const void **base)
            {
               n.key = key;
            }
      };

   template <class K, class V>
      class keyset<K*,V>
      {
         public:
            void operator()(hashnode<K*,V>& n, K*& key, const void **base)
            {
               n.key = &key;
               n.koff = (mysplay::offset_t)-1;
               n.base = base;
            }
      };

   template <class K, class V>
      class nodeset
      {
         public:
            void operator()(K key, V val, unsigned int keysize, unsigned int valsize, mysplay::mmallocsplaynode< hashnode<K,V> > *n, mmallocutilizer::mmalloc *m)
            {
               n->d.key = key;
               n->d.val = val;
            }
      };

   template <class K, class V>
      class nodeset<K,V*>
      {
         public:
            void operator()(K key, V *val, unsigned int keysize, unsigned int valsize, mysplay::mmallocsplaynode< hashnode<K,V*> > *n, mmallocutilizer::mmalloc *m)
            {
               try
               {
                  mysplay::offset_t voff = m->allocate(valsize);
                  n->d.base = m->baseaddress();
                  n->d.voff = voff;
                  V *v = (V*)((char*)n->d.base+n->d.voff);
                  memcpy(v,val,valsize);
                  n->d.val = &v;
                  n->d.key = key;
               }
               catch (const char *ex)
               {
                  throw "failed to allocate valsize";
               }
            }
      };

   template <class K, class V>
      class nodeset<K*,V>
      {
         public:
            void operator()(K *key, V val, unsigned int keysize, unsigned int valsize, mysplay::mmallocsplaynode< hashnode<K*,V> > *n, mmallocutilizer::mmalloc *m)
            {
               try
               {
                  mysplay::offset_t koff = m->allocate(keysize);
                  n->d.koff = koff;
                  n->d.base = m->baseaddress();
                  K *k = (K*)((char*)*(n->d.base)+n->d.koff);
                  memcpy(k,key,keysize);
                  n->d.key = &k;
                  n->d.val = val;
               }
               catch (const char *ex)
               {
                  throw "failed to allocate key and val";
               }
            }
      };

   template <class K, class V>
      class nodeset<K*,V*>
      {
         public:
            void operator()(K *key, V *val, unsigned int keysize, unsigned int valsize, mysplay::mmallocsplaynode< hashnode<K*,V*> > *n, mmallocutilizer::mmalloc *m)
            {
               try
               {
                  mysplay::offset_t koff = m->allocate(keysize);
                  mysplay::offset_t voff = m->allocate(valsize);
                  n->d.base = m->baseaddress();
                  n->d.koff = koff;
                  n->d.voff = voff;
                  K *k = (K*)((char*)*(n->d.base)+n->d.koff);
                  memcpy(k,key,keysize);
                  n->d.key = &k;
                  V *v = (V*)((char*)*(n->d.base)+n->d.voff);
                  memcpy(v,val,valsize);
                  n->d.val = &v;
               }
               catch (const char *ex)
               {
                  throw "failed to allocate key and val";
               }
            }
      };

   template <class K, class V>
      class returner
      {
         public:
            V* operator()(hashnode<K,V>& n)
            {
               return &n.val;
            }
      };

   template <class K, class V>
      class returner<K,V*>
      {
         public:
            V** operator()(hashnode<K,V*>& n)
            {
               *(n.val) = (V*)((char*)*(n.base)+n.voff);
               return n.val;
            }
      };

   template <class K, class V>
      class returner<K*,V>
      {
         public:
            V* operator()(hashnode<K*,V>& n)
            {
               return &n.val;
            }
      };

   template <class K, class V>
      class returner<K*,V*>
      {
         public:
            V** operator()(hashnode<K*,V*>& n)
            {
               *(n.val) = (V*)((char*)*(n.base)+n.voff);
               return n.val;
            }
      };

   template <class K, class V>
      class remover
      {
         public:
            void operator()(mysplay::mmallocsplaynode<hashnode<K,V> > *n, mysplay::offset_t off, mmallocutilizer::mmalloc *m)
            {
               m->deallocate(off);
            }
      };

   template <class K, class V>
      class remover<K,V*>
      {
         public:
            void operator()(mysplay::mmallocsplaynode<hashnode<K,V*> > *n, mysplay::offset_t off, mmallocutilizer::mmalloc *m)
            {
               m->deallocate(n->d.voff);
               m->deallocate(off);
            }
      };

   template <class K, class V>
      class remover<K*,V>
      {
         public:
            void operator()(mysplay::mmallocsplaynode<hashnode<K*,V> > *n, mysplay::offset_t off, mmallocutilizer::mmalloc *m)
            {
               m->deallocate(n->d.koff);
               m->deallocate(off);
            }
      };

   template <class K, class V>
      class remover<K*,V*>
      {
         public:
            void operator()(mysplay::mmallocsplaynode<hashnode<K*,V> > *n, mysplay::offset_t off, mmallocutilizer::mmalloc *m)
            {
               m->deallocate(n->d.koff);
               m->deallocate(n->d.voff);
               m->deallocate(off);
            }
      };

   template <class K, class V>
      class offsetter
      {
         public:
            void operator()(hashnode<K,V>& n, mmallocutilizer::mmalloc *m)
            {
               return;
            }
      };

   template <class K, class V>
      class offsetter<K*,V>
      {
         public:
            void operator()(hashnode<K*,V>& n, mmallocutilizer::mmalloc *m)
            {
               n.base = m->baseaddress();
               //*(n.key) = (K*)((char*)*(n.base)+n.koff);
               K *k = (K*)((char*)*(n.base)+n.koff);
               n.key = &k;
               return;
            }
      };

   template <class K, class V>
      class offsetter<K,V*>
      {
         public:
            void operator()(hashnode<K,V*>& n, mmallocutilizer::mmalloc *m)
            {
               n.base = m->baseaddress();
               V *v = (V*)((char*)*(n.base)+n.voff);
               n.val = &v;
               return;
            }
      };

   template <class K, class V>
      class offsetter<K*,V*>
      {
         public:
            void operator()(hashnode<K,V*>& n, mmallocutilizer::mmalloc *m)
            {
               n.base = m->baseaddress();
               K *k = (K*)((char*)*(n.base)+n.koff);
               n.key = &k;
               V *v = (V*)((char*)*(n.base)+n.voff);
               n.val = &v;
               //*(n.key) = (K*)((char*)*(n.base)+n.koff);
               //*(n.val) = (V*)((char*)*(n.base)+n.voff);
               return;
            }
      };

   template <class K, class V, class HashFunc = mhashfuncs<K>, class Comparator = mhashcompare<K,V> >
      class mhashiterator : public std::iterator<std::input_iterator_tag, hashnode<K,V> >
   {
      public:
         // constractor
         mhashiterator() :tableid(HASHSIZE), table(NULL), m(NULL){}
         mhashiterator(unsigned int id, mysplay::splaytree<hashnode<K,V>, Comparator> *table, mmallocutilizer::mmalloc *m)
            //: tableid(id), table(table), m(m), it(this->table[this->tableid].begin())
            : tableid(id), table(table), m(m), it(this->table[id].begin())
         {
            while(this->it == this->table[this->tableid].end())
            {
               if (this->tableid+1 == (unsigned int)HASHSIZE)
               {
                  this->table = NULL;
                  ++this->tableid;
                  break;
               }
               this->it = this->table[++this->tableid].begin();
            }
         }
         mhashiterator(const mhashiterator& i):it(i.it),table(i.table),m(i.m),tableid(i.tableid){}
         // copy constractor
         mhashiterator& operator=(const mhashiterator& i)
         {
            this->it = i.it;
            this->table = i.table;
            this->m = i.m;
            this->tableid = i.tableid;
            return *this;
         }
         // access
         hashnode<K,V>& operator*() const
         {
            offsetter<K,V>()(*this->it, this->m);
            return *this->it;
         }
         // move
         mhashiterator& operator++()
         {
            try
            {
               if (!this->table)
               {
                  return *this;
               }
               if (this->it != this->table[this->tableid].end())
               {
                  ++this->it;
                  if (this->it == this->table[this->tableid].end())
                  {
                     ++*this;
                  }
               }
               else if (this->tableid < (unsigned int)HASHSIZE)
               {
                  this->it = this->table[++this->tableid].begin();
                  while(this->it == this->table[this->tableid].end())
                  {
                     if (this->tableid+1 == (unsigned int)HASHSIZE)
                     {
                        this->table = NULL;
                        ++this->tableid;
                        break;
                     }
                     this->it = this->table[++this->tableid].begin();
                  }
               }
               else
               {
                  this->table = NULL;
               }
            }
            catch (const char *ex)
            {
               throw ex;
            }
            return *this;
         }
         mhashiterator operator++(int)
         {
            mhashiterator tmp = *this;
            try
            {
               ++*this;
            }
            catch (const char *ex)
            {
               throw ex;
            }
            return tmp;
         }
         // comparator
         friend bool operator==(const mhashiterator<K,V,HashFunc,Comparator>& x, const mhashiterator<K,V,HashFunc,Comparator>& y)
         {
            if (x.tableid == HASHSIZE && y.tableid == HASHSIZE &&
                  (!(x.table) && !(y.table)) )
            {
               return true;
            }
            return (x.tableid == y.tableid && x.it == y.it);
         }
         friend bool operator!=(const mhashiterator<K,V,HashFunc,Comparator>& x, const mhashiterator<K,V,HashFunc,Comparator>& y)
         {
            return !(x == y);
         }
      protected:
         unsigned int tableid; // table index
         mysplay::splaytree<hashnode<K,V>, Comparator> *table;
         mmallocutilizer::mmalloc *m;
         mysplay::splayiterator<hashnode<K,V> > it;
   };

   template <class K, class V, class HashFunc = mhashfuncs<K>, class Comparator = mhashcompare<K,V> >
      class mhash
      {
         public:
            mhash(mmallocutilizer::mmalloc *m, const char *save)
               : m(m), fp(NULL), registered(0), biter(NULL), eiter(new mhashiterator<K,V,HashFunc,Comparator>)
            {
               this->b = this->m->baseaddress();
               struct stat st;
               if (stat(save,&st) != 0) // file not exist
               {
                  if ((this->fp = fopen(save, "wb")) == NULL)
                  {
                     throw "Couldn't open savefile";
                  }
                  for (unsigned int i = 0; i < (unsigned int)HASHSIZE; ++i)
                  {
                     this->table[i].init(this->b);
                  }
               }
               else // file is exist
               {
                  if ((this->fp = fopen(save,"rb")) == NULL)
                  {
                     throw "Couldn't open savefile";
                  }
                  fread(&this->registered,sizeof(unsigned int),1,this->fp);
                  for (unsigned int i = 0; i < (unsigned int)HASHSIZE; ++i)
                  {
                     unsigned int size_ = 0;
                     mysplay::offset_t roff = (mysplay::offset_t)-1;
                     fread(&roff,sizeof(mysplay::offset_t),1,this->fp);
                     fread(&size_,sizeof(unsigned int),1,this->fp);
                     this->table[i].init(this->b,roff);
                     this->table[i].sizeset(size_);
                  }
                  fclose(this->fp);
                  if ((this->fp = fopen(save,"wb")) == NULL)
                  {
                     throw "Couldn't open savefile";
                  }
               }
            }
            virtual ~mhash()
            {
               fwrite(&this->registered,1,sizeof(unsigned int),this->fp);
               for (unsigned int i = 0; i < (unsigned int)HASHSIZE; ++i)
               {
                  mysplay::offset_t roff = this->table[i].rootoffset();
                  unsigned int size_ = this->table[i].size();
                  fwrite(&roff,sizeof(mysplay::offset_t),1,this->fp);
                  fwrite(&size_,sizeof(unsigned int),1,this->fp);
               }
               fclose(this->fp);
               if (this->biter)
               {
                  delete this->biter;
               }
               if (this->eiter)
               {
                  delete this->eiter;
               }
            };
            V* operator[](K key)
            {
               unsigned int h = HashFunc()(key);
               mysplay::splaytree< hashnode<K,V>, Comparator> *spt = this->table+h;

               hashnode<K,V> a;
               keyset<K,V>()(a,key,this->b);
               mysplay::mmallocsplaynode< hashnode<K,V> > *v = spt->search( a );
               if (v)
               {
                  return returner<K,V>()(v->d);
               }
               else
               {
                  return NULL;
               }
            }
            bool insert(K key, V val, unsigned int keysize, unsigned int valsize)
            {
               try
               {
                  mysplay::offset_t off = this->m->allocate(sizeof(mysplay::mmallocsplaynode< hashnode<K,V> >));
                  mysplay::mmallocsplaynode< hashnode<K,V> > *n = (mysplay::mmallocsplaynode< hashnode<K,V> >*)this->ptr(off);

                  nodeset<K,V>()(key, val, keysize, valsize, n, this->m);
                  unsigned int h = HashFunc()(key);
                  mysplay::splaytree< hashnode<K,V>, Comparator> *spt = this->table+h;
                  spt->insertion(n);
                  ++this->registered;
               }
               catch (const char *ex)
               {
                  throw "failed to allocate hashnode<>";
               }
               return true;
            }
            bool remove(K key)
            {
               unsigned int h = HashFunc()(key);
               mysplay::splaytree< hashnode<K,V>, Comparator> *spt = this->table+h;
               hashnode<K,V> a;
               keyset<K,V>()(a,key,this->b);
               mysplay::mmallocsplaynode< hashnode<K,V> > *v = spt->search( a );
               if (v)
               {
                  spt->deletion( v );
                  remover<K,V>()(v, spt->offset(v), this->m);
                  --this->registered;
                  return true;
               }
               else
               {
                  return false;
               }
            }
            unsigned int size()
            {
               return this->registered;
            };
            mhashiterator<K,V,HashFunc,Comparator>& begin()
            {
               if (this->biter)
               {
                  delete this->biter;
               }
               this->biter = new mhashiterator<K,V,HashFunc,Comparator>(0, this->table, this->m);
               return *this->biter;
            }
            mhashiterator<K,V,HashFunc,Comparator>& end()
            {
               return *this->eiter;
            }
         protected:
            mmallocutilizer::mmalloc *m; // mmap allocator
            FILE *fp; // filepointer for saving offsets
            unsigned int registered;
            mhashiterator<K,V,HashFunc,Comparator> *biter;
            mhashiterator<K,V,HashFunc,Comparator> *eiter;
            const void **b; // base pointer
            mysplay::splaytree<hashnode<K,V>, Comparator> table[HASHSIZE];

            mhash();
            void* ptr(mysplay::offset_t b)
            {
               return (void*)((char*)*this->b+b);
            }
      };

}
# endif /* __MHASH__ */
