/**
 * Allocator class for using mmap
 * written by Kei Uchiumi
 */
# ifndef __MMALLOC__
# define __MMALLOC__

extern "C"
{
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/stat.h>
};
# include <string>
# include <splaytree.hpp>

# define MMALLOCSIZE 8096
# ifdef __GNUC__
# define PRGM_PKD __attribute__ ((packed))
# else
# define PRGM_PKD
# endif /* __GNUC__ */

namespace mmallocutilizer
{
   typedef enum
   {
      Create,
      ReadWrite
   } OpenFlg;

   typedef uint32_t memtype_t;
   typedef struct
   {
      memtype_t used_h : 1;
      memtype_t size_h : 31;
      mysplay::offset_t next;
   } /*PRGM_PKD*/mmalloc_head;

   typedef struct
   {
      memtype_t size_f : 31;
      memtype_t used_f : 1;
   } mmalloc_foot;

   class mmalloccompare
   {
      public:
         bool operator()(const mmalloc_head& a, const mmalloc_head& b)
         {
            return a.size_h < b.size_h;
         }
   };

   class mmalloc
   {
      public:
         mmalloc(const char *filename, OpenFlg flg);
         ~mmalloc();

         const void** baseaddress();
         mysplay::offset_t allocate(memtype_t allocsize);
         void deallocate(mysplay::offset_t b);

         off_t getfilesize(const char *path)
         {
            struct stat file_stats;

            if(stat(path,&file_stats))
            {
               throw "Couldn't get filesize";
            }

            return file_stats.st_size;
         }
         mysplay::offset_t offset(void *p)
         {
            return this->manager.offset((mysplay::mmallocsplaynode<mmalloc_head>*)p);
         }
      protected:
         std::string file;
         memtype_t size;
         int fd;
         void *pt;
         memtype_t headsize;
         memtype_t footsize;
         mysplay::splaytree<mmalloc_head,mmalloccompare> manager;
         memtype_t utilizable(memtype_t s);
         bool checkrestsize(memtype_t s);
         void mapping(int flg);
         void initnode(mysplay::mmallocsplaynode<mmalloc_head> *n, memtype_t nodesize);
         void init();
         void attach();
         void resize();
         void swapbottom(mysplay::mmallocsplaynode<mmalloc_head> *c);
         void pushlist(mysplay::mmallocsplaynode<mmalloc_head> *c);
         void picking(mysplay::mmallocsplaynode<mmalloc_head> *c);
         mmalloc_foot* pointfoot(mysplay::mmallocsplaynode<mmalloc_head> *p);
         mysplay::mmallocsplaynode<mmalloc_head>* pointhead(mysplay::offset_t b);
         mysplay::mmallocsplaynode<mmalloc_head>* pointnexthead(mmalloc_foot *f);
         mysplay::mmallocsplaynode<mmalloc_head>* pointprevhead(mmalloc_head *h);
         mysplay::mmallocsplaynode<mmalloc_head>* chunking(mysplay::mmallocsplaynode<mmalloc_head> *c);
      private:
         mmalloc();
         mmalloc(const mmalloc&);
         mmalloc operator=(const mmalloc&);
   };

   inline memtype_t mmalloc::utilizable(memtype_t s)
   {
      if (s < 32)
      {
         return 0;
      }
      return s - this->headsize - this->footsize + sizeof(mysplay::offset_t);
   }

   inline mmalloc::mmalloc(const char *filename, OpenFlg flg)
      : file(filename),size(MMALLOCSIZE),fd(-1),pt(NULL),headsize(sizeof(mmalloc_head)),footsize(sizeof(mmalloc_foot))
   {
      if (flg == Create)
      {
         this->init();
      }
      else if (flg == ReadWrite)
      {
         this->attach();
      }
   }

   inline void mmalloc::attach()
   {
      /// get filesize
      this->size = (memtype_t)this->getfilesize(this->file.c_str());

      /// map
      this->mapping(O_RDWR);

      /// get root offset and set base and root offset to splaytree
      mysplay::offset_t *roff = (mysplay::offset_t*)this->pt;
      this->manager.init((const void**)&this->pt,*roff);

   }

   inline void mmalloc::initnode(mysplay::mmallocsplaynode<mmalloc_head> *n, memtype_t nodesize)
   {
      n->d.used_h = 0;
      n->d.size_h = nodesize;
      n->d.next = (mysplay::offset_t)-1;
      n->parent = (mysplay::offset_t)-1;
      n->lchild = (mysplay::offset_t)-1;
      n->rchild = (mysplay::offset_t)-1;
      mmalloc_foot *nfoot = this->pointfoot(n);
      nfoot->used_f = 0;
      nfoot->size_f = nodesize;
   }

   inline void mmalloc::mapping(int flg)
   {
      if ((this->fd = open(this->file.c_str(),flg,0666)) == -1)
      {
         throw "Couldn't open file for mmap";
      }
      mysplay::offset_t psize = 0;
# ifdef BSD
      psize = getpagesize();
# else
      psize = sysconf(_SC_PAGE_SIZE);
# endif
      this->size = (this->size/psize+1)*psize;

      if (flg&O_CREAT)
      {
         if (lseek(this->fd, this->size, SEEK_SET) < 0)
         {
            throw "Failed to seek";
         }
         char c;
         if (read(this->fd,&c,sizeof(char)) == -1)
         {
            c = '\0';
         }
         if (write(this->fd,&c,sizeof(char)) == -1)
         {
            throw "Failed to write";
         }
      }

      /// map
      this->pt = (void*)mmap(0,this->size,PROT_READ|PROT_WRITE,MAP_SHARED,this->fd,0);
      if (this->pt == (void*)-1)
      {
         throw "Failed to map";
      }
   }

   inline void mmalloc::init()
   {
      this->mapping(O_RDWR|O_CREAT);

      /// set base
      this->manager.init((const void**)&this->pt);
      unsigned int offsize = sizeof(mysplay::offset_t);
      mysplay::mmallocsplaynode<mmalloc_head> *head = (mysplay::mmallocsplaynode<mmalloc_head>*)((char*)this->pt+offsize);
      this->initnode(head, this->utilizable(this->size-offsize));

      this->manager.insertion(head);
   }

   inline mmalloc::~mmalloc()
   {
      /// override parent offset of splay tree to mmap size
      mysplay::offset_t roffset = this->manager.rootoffset();
      mysplay::offset_t *roff = (mysplay::offset_t*)this->pt;
      *roff = roffset;

      msync(this->pt,this->size,0);
      /// unmap
      if (munmap(this->pt,this->size) == -1)
      {
         throw "Failed to munmap";
      }
      close(this->fd);
   }

   inline void mmalloc::resize()
   {
      /// sync
      msync(this->pt,this->size,0);
      if (munmap(this->pt,this->size) == -1)
      {
         throw "Failed to munmap";
      }
      close(this->fd);

      memtype_t prevsize = this->size;
      this->size = this->size<<1;
      /// reopen
      this->mapping(O_RDWR|O_CREAT);

      /// set base
      this->manager.init((const void**)&this->pt, this->manager.rootoffset());

      /// append node
      mysplay::mmallocsplaynode<mmalloc_head> *head = (mysplay::mmallocsplaynode<mmalloc_head>*)((char*)this->pt+prevsize);
      this->initnode(head, this->utilizable(this->size-prevsize));

      this->pushlist(this->chunking(head));
   }

   inline const void** mmalloc::baseaddress()
   {
      return (const void**)&this->pt;
   }

   inline mmalloc_foot* mmalloc::pointfoot(mysplay::mmallocsplaynode<mmalloc_head> *p)
   {
      return (mmalloc_foot*)((char*)p+this->headsize+p->d.size_h-sizeof(mysplay::offset_t));
   }

   inline mysplay::mmallocsplaynode<mmalloc_head>* mmalloc::pointhead(mysplay::offset_t b)
   {
      mysplay::mmallocsplaynode<mmalloc_head> *p = this->manager.ptrnode(b);
      return (mysplay::mmallocsplaynode<mmalloc_head>*)((char*)p-this->headsize+sizeof(mysplay::offset_t));
   }

   inline mysplay::mmallocsplaynode<mmalloc_head>* mmalloc::pointnexthead(mmalloc_foot *f)
   {
      if ((char*)f+this->footsize-(char*)this->pt-this->size == 0)
      {
         return NULL;
      }
      return (mysplay::mmallocsplaynode<mmalloc_head>*)((char*)f+this->footsize);
   }

   inline mysplay::mmallocsplaynode<mmalloc_head>* mmalloc::pointprevhead(mmalloc_head *h)
   {
      if ((char*)h - (char*)this->pt == sizeof(mysplay::offset_t))
      {
         return NULL;
      }

      mmalloc_foot *f = (mmalloc_foot*)((char*)h-this->footsize);
      return (mysplay::mmallocsplaynode<mmalloc_head>*)((char*)f-f->size_f-this->headsize+sizeof(mysplay::offset_t));
   }

   inline bool mmalloc::checkrestsize(memtype_t s)
   {
      if (this->utilizable(s) < 32)
      {
         return false;
      }
      return true;
   }

   inline void mmalloc::picking(mysplay::mmallocsplaynode<mmalloc_head> *c)
   {
      mysplay::mmallocsplaynode<mmalloc_head> *p = this->manager.ptrnode(c->parent);
      mysplay::offset_t coff = this->manager.offset(c);
      // root
      if (!p)
      {
         // not list
         if (c->d.next == (mysplay::offset_t)-1)
         {
            this->manager.deletion(c);
         }
         // list exist
         else
         {
            this->swapbottom(c);
         }
         return;
      }
      // not branch
      else if (p && coff == p->d.next)
      {
         p->d.next = c->d.next;
         if (c->d.next != (mysplay::offset_t)-1)
         {
            mysplay::mmallocsplaynode<mmalloc_head> *s = this->manager.ptrnode(c->d.next);
            s->parent = c->parent;
         }
      }
      // branch
      else if (coff == p->lchild || coff == p->rchild)
      {
         if (c->d.next != (mysplay::offset_t)-1)
         {
            this->swapbottom(c);
         }
         else
         {
            this->manager.deletion(c);
         }
      }
   }

   inline mysplay::mmallocsplaynode<mmalloc_head>* mmalloc::chunking(mysplay::mmallocsplaynode<mmalloc_head> *c)
   {
      mysplay::mmallocsplaynode<mmalloc_head> *left = this->pointprevhead((mmalloc_head*)c);
      if (left)
      {
         if (left->d.used_h == 0)
         {
            // ToDo remove left node from list
            this->picking(left);
            left->d.size_h += this->headsize + this->footsize + c->d.size_h - sizeof(mysplay::offset_t);
            c = left;
         }
      }
      mysplay::mmallocsplaynode<mmalloc_head> *right = this->pointnexthead(this->pointfoot(c));
      if (right)
      {
         if (right->d.used_h == 0)
         {
            // ToDo remove right node from list
            this->picking(right);
            c->d.size_h += this->headsize + this->footsize + right->d.size_h - sizeof(mysplay::offset_t);
         }
      }
      this->initnode(c, c->d.size_h);
      return c;
   }

   inline void mmalloc::swapbottom(mysplay::mmallocsplaynode<mmalloc_head> *c)
   {
      /// next node
      mysplay::mmallocsplaynode<mmalloc_head> *n = this->manager.ptrnode(c->d.next);
      /// parent node
      mysplay::mmallocsplaynode<mmalloc_head> *p = this->manager.ptrnode(c->parent);
      if (p)
      {
         mysplay::offset_t coff = this->manager.offset(c);
         if (coff == p->lchild)
         {
            this->manager.setleftchild(p,n);
         }
         else
         {
            this->manager.setrightchild(p,n);
         }
      }
      else // root node
      {
         n->parent = (mysplay::offset_t)-1;
         this->manager.init((const void**)&this->pt,c->d.next);
      }
      if (c->lchild != (mysplay::offset_t)-1)
      {
         mysplay::mmallocsplaynode<mmalloc_head> *l = this->manager.ptrnode(c->lchild);
         this->manager.setleftchild(n,l);
      }
      if (c->rchild != (mysplay::offset_t)-1)
      {
         mysplay::mmallocsplaynode<mmalloc_head> *r = this->manager.ptrnode(c->rchild);
         this->manager.setrightchild(n,r);
      }
   }

   inline void mmalloc::pushlist(mysplay::mmallocsplaynode<mmalloc_head> *c)
   {
      mysplay::mmallocsplaynode<mmalloc_head> *s = this->manager.search(c->d);
      if (s)
      {
         mysplay::offset_t soff = this->manager.offset(s);
         c->d.next = soff;
         if (s->parent != (mysplay::offset_t)-1)
         {
            mysplay::mmallocsplaynode<mmalloc_head> *p = this->manager.ptrnode(s->parent);
            if (c->d.next == p->lchild)
            {
               this->manager.setleftchild(p,c);
            }
            else
            {
               this->manager.setrightchild(p,c);
            }
         }
         if (s->lchild != (mysplay::offset_t)-1)
         {
            mysplay::mmallocsplaynode<mmalloc_head> *l = this->manager.ptrnode(s->lchild);
            this->manager.setleftchild(c,l);
         }
         if (s->rchild != (mysplay::offset_t)-1)
         {
            mysplay::mmallocsplaynode<mmalloc_head> *r = this->manager.ptrnode(s->rchild);
            this->manager.setrightchild(c,r);
         }
         s->parent = this->manager.offset(c);
         s->lchild = (mysplay::offset_t)-1;
         s->rchild = (mysplay::offset_t)-1;
         if (soff = this->manager.rootoffset())
         {
            this->manager.init((const void**)&this->pt,this->manager.offset(c));
         }
      }
      else
      {
         this->manager.insertion(c);
      }
   }

   inline mysplay::offset_t mmalloc::allocate(memtype_t allocsize)
   {
      if (allocsize < 32)
      {
         allocsize = 32;
      }
      /// seac best fit
      mmalloc_head key;
      key.size_h = allocsize;
      mysplay::mmallocsplaynode<mmalloc_head> *n = this->manager.rightbestfit(key);

      if (!n)
      {
         // resize mmap file
         this->resize();
         return this->allocate(allocsize);
      }

      // swap bottom of list
      if (n->d.next != (mysplay::offset_t)-1)
      {
         this->swapbottom(n);
      }
      else
      {
         this->manager.deletion(n);
      }

      n->d.used_h = 1;
      if (n->d.size_h == allocsize)
      {
         mmalloc_foot *foot = this->pointfoot(n);
         foot->used_f = 1;
         return this->manager.offset((mysplay::mmallocsplaynode<mmalloc_head>*)&n->d.next);
      }

      // resize node
      if (this->checkrestsize(n->d.size_h - allocsize))
      {
         memtype_t rest = this->utilizable(n->d.size_h - allocsize);
         n->d.size_h = allocsize;
         mmalloc_foot *foot = this->pointfoot(n);
         foot->used_f = 1;
         foot->size_f = allocsize;
         mysplay::mmallocsplaynode<mmalloc_head> *p = this->pointnexthead(foot);
         if (p)
         {
            this->initnode(p, rest);
            this->pushlist(p);
         }
      }
      else
      {
         mmalloc_foot *foot = this->pointfoot(n);
         foot->used_f = 1;
      }
      return this->manager.offset((mysplay::mmallocsplaynode<mmalloc_head>*)&n->d.next);
   }

   inline void mmalloc::deallocate(mysplay::offset_t b)
   {
      this->pushlist(this->chunking(this->pointhead(b)));
   }
}

# endif /* __MMALLOC__ */
