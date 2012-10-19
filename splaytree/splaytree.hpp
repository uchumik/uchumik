/**
 * Splay Tree class for using mmap
 * written by Kei Uchiumi
 */
# ifndef __SPLAYTREE__
# define __SPLAYTREE__

extern "C"
{
# include <stdint.h>
}

# include <iostream>
# include <iterator>
# include <vector>

namespace mysplay
{
   typedef uint64_t offset_t;

   template <class T>
      class mmallocsplaynode
      {
         public:
            mmallocsplaynode();
            ~mmallocsplaynode();
            T d;
            offset_t parent; // parent offset
            offset_t lchild; // left child offset
            offset_t rchild; // right child offset
      };

   template <class T>
      mmallocsplaynode<T>::mmallocsplaynode()
      : parent((offset_t)-1), lchild((offset_t)-1), rchild((offset_t)-1)
      {
      }

   template <class T>
      mmallocsplaynode<T>::~mmallocsplaynode()
      {
      }

   template <class T>
      class splayiterator : public std::iterator<std::input_iterator_tag, T>
      {
         public:
            // constractor
            splayiterator() :b(NULL),n(NULL),myoff((offset_t)-1){}
            splayiterator(const void *b, mmallocsplaynode<T> *n)
               :b(b),n(n),myoff((n)?(offset_t)((char*)n-(char*)b):(offset_t)-1){}
            splayiterator(const splayiterator& i): b(i.b),n(i.n),myoff(i.myoff){};
            // copy constractor
            splayiterator& operator=(const splayiterator& i)
            {
               this->b = i.b;
               this->n = i.n;
               this->myoff = i.myoff;
               return *this;
            }
            virtual ~splayiterator() {}
            // access
            T& operator*() const
            {
               return this->n->d;
            };
            // move
            splayiterator& operator++()
            {
               // enditerator check
               if (this->myoff == (offset_t)-1)
               {
                  return *this;
               }
               if (this->n->lchild != (offset_t)-1)
               {
                  this->myoff = this->n->lchild;
                  if (this->n->rchild != (offset_t)-1)
                  {
                     this->_next.push_back(this->n->rchild);
                  }
                  this->n = (mmallocsplaynode<T>*)((char*)this->b+this->n->lchild);
               }
               else if (this->n->rchild != (offset_t)-1)
               {
                  this->myoff = this->n->rchild;
                  this->n = (mmallocsplaynode<T>*)((char*)this->b+this->n->rchild);
               }
               else
               {
                  // root check
                  if (this->n->parent == (offset_t)-1)
                  {
                     // this tree is only root node
                     this->myoff = (offset_t)-1;
                     return *this;
                  }
                  if (!this->_next.empty())
                  {
                     this->myoff = this->_next.back();
                     this->n = (mmallocsplaynode<T>*)((char*)this->b+this->myoff);
                     this->_next.pop_back();
                  }
                  else
                  {
                     this->myoff = (offset_t)-1;
                  }
               }
               return *this;
            };
            splayiterator operator++(int)
            {
               splayiterator tmp = *this;
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
            friend bool operator==(const splayiterator<T>& x, const splayiterator<T>& y)
            {
               if (x.myoff == (offset_t)-1 && y.myoff == (offset_t)-1)
               {
                  return true;
               }
               else if (x.b != y.b)
               {
                  return false;
               }
               return x.myoff == y.myoff;
            }
            friend bool operator!=(const splayiterator<T>& x, const splayiterator<T>& y)
            {
               return !(x == y);
            }
         protected:
            const void *b;
            mmallocsplaynode<T> *n;
            std::vector<offset_t> _next;
            offset_t myoff;
      };


   template <class T, class Comparator = std::less<T> >
      class splaytree
      {
         public:
            splaytree()
               : base(NULL),root(NULL),r((offset_t)-1),biter(NULL),eiter(NULL),size_(0)
               //: base(NULL),root(NULL),r((offset_t)-1),biter(NULL),eiter(new splayiterator<T>),size_(0)
            {
            }

            ~splaytree()
            {
               if (this->biter)
               {
                  delete this->biter;
               }
               if (this->eiter)
               {
                  delete this->eiter;
               }
            }

            void insertion(mmallocsplaynode<T> *n)
            {
               this->check();
               if (!this->root)
               {
                  this->root = n;
                  this->r = this->offset(this->root);
                  this->root->parent = (offset_t)-1;
                  this->root->lchild = (offset_t)-1;
                  this->root->rchild = (offset_t)-1;
                  ++this->size_;
                  return;
               }

               //mmallocsplaynode<T> *p = this->root;
               n->parent = this->r;
               if (Comparator()(n->d, this->root->d))
               {
                  this->insert_t(this->root->lchild, n);
               }
               else
               {
                  this->insert_t(this->root->rchild, n);
               }

               this->splay(this->offset(n), this->root->parent);
               ++this->size_;
            }

            /// deletion doesn't free memory which is used by n, 
            /// and thus deallocation must be done out of splaytree class
            void deletion(mmallocsplaynode<T> *n)
            {
               this->check();
               if (!n || !this->root)
               {
                  return;
               }
               else if (this->root->rchild == (offset_t)-1 && this->root->lchild == (offset_t)-1)
               {
                  this->root = NULL;
                  this->r = (offset_t)-1;
                  --this->size_;
                  return;
               }
               else if (n != this->root)
               {
                  //this->splay(n, this->root);
                  this->splay(this->offset(n), this->root->parent);
                  this->r = this->offset(n);
               }

               mmallocsplaynode<T> *newroot = NULL;
               if (this->root->rchild != (offset_t)-1)
               {
                  newroot = this->rightmin(this->root->rchild);
                  mmallocsplaynode<T> *rootright = this->ptrnode(this->root->rchild);
                  this->splay(this->offset(newroot), rootright->parent);
                  if (newroot->lchild != (offset_t)-1)
                  {
                     throw "Unexpected data was input";
                  }
                  mmallocsplaynode<T> *leftsubtree = this->ptrnode(this->root->lchild);
                  this->setleftchild(newroot, leftsubtree);
                  //leftsubtree->parent = newroot - (mmallocsplaynode<T>*)this->base;
                  //newroot->lchild = leftsubtree - (mmallocsplaynode<T>*)this->base;
               }
               else
               {
                  newroot = this->leftmax(this->root->lchild);
                  mmallocsplaynode<T> *rootleft = this->ptrnode(this->root->lchild);
                  this->splay(this->offset(newroot), rootleft->parent);
                  if (newroot->rchild != (offset_t)-1)
                  {
                     throw "Unexpected data was input";
                  }
                  mmallocsplaynode<T> *rightsubtree = this->ptrnode(this->root->rchild);
                  this->setrightchild(newroot, rightsubtree);
                  //rightsubtree->parent = newroot - (mmallocsplaynode<T>*)this->base;
                  //newroot->rchild = rightsubtree - (mmallocsplaynode<T>*)this->base;
               }
               this->root = newroot;
               if (this->root->parent != (offset_t)-1)
               {
                  this->root->parent = (offset_t)-1;
               }
               this->r = this->offset(this->root);
               --this->size_;
            }

            void setleftchild(mmallocsplaynode<T> *p, mmallocsplaynode<T> *c)
            {
               offset_t coff = (offset_t)-1;
               offset_t poff = this->offset(p);
               if (c)
               {
                  coff = this->offset(c);
                  c->parent = poff;
               }
               p->lchild = coff;
            }

            void setrightchild(mmallocsplaynode<T> *p, mmallocsplaynode<T> *c)
            {
               offset_t coff = (offset_t)-1;
               offset_t poff = this->offset(p);
               if (c)
               {
                  coff = this->offset(c);
                  c->parent = poff;
               }
               p->rchild = coff;
            }

            mmallocsplaynode<T>* search(T& v)
            {
               this->check();
               mmallocsplaynode<T> *c = this->search_t(this->r, v);
               if (c)
               {
                  this->splay(this->offset(c), this->root->parent);
                  this->r = this->offset(this->root);
               }

               return c;
            }

            mmallocsplaynode<T>* rightbestfit(T& v)
            {
               this->check();
               mmallocsplaynode<T> *c = this->rightbestfit_t(this->r, v);
               if (c)
               {
                  this->splay(this->offset(c), this->root->parent);
                  this->r = this->offset(this->root);
               }
               return c;
            }

            void init(const void **base)
            {
               this->base = base;
               this->check();
            }

            void init(const void **base, offset_t r)
            {
               if (r == (offset_t)-1)
               {
                  this->init(base);
                  return;
               }
               this->base = base;
               this->r = r;
               this->root = this->ptrnode(this->r);
               this->check();
            }

            offset_t rootoffset()
            {
               return this->r;
            }

            offset_t offset(mmallocsplaynode<T> *n)
            {
               if (!n)
               {
                  throw "NULL pointer";
               }
               return (offset_t)((char*)n-(char*)*this->base);
            }

            mmallocsplaynode<T>* ptrnode(offset_t b)
            {
               if (b == (offset_t)-1)
               {
                  return NULL;
               }
               return (mmallocsplaynode<T>*)((char*)*this->base+b);
            }

            unsigned int size()
            {
               return this->size_;
            }

            void sizeset(unsigned int s)
            {
               this->size_ = s;
            }

            void dumptree()
            {
               this->dumptree_t(this->root);
            }

            splayiterator<T>& begin()
            {
               if (this->biter)
               {
                  delete this->biter;
               }
               this->biter = new splayiterator<T>(*this->base,this->root);
               return *this->biter;
            }
            splayiterator<T>& end()
            {
               if (!this->eiter)
               {
                  this->eiter = new splayiterator<T>;
               }
               return *this->eiter;
            }
         protected:
            const void **base; // pointer of pointing head address of splay tree on mmap
            mmallocsplaynode<T> *root;
            offset_t r;
            splayiterator<T> *biter;
            splayiterator<T> *eiter;
            unsigned int size_;

            void dumptree_t(mmallocsplaynode<T> *root)
            {
               if (!root)
               {
                  return;
               }
               std::cout << "[offset]: " << this->offset(root) << std::endl;
               std::cout << "[node val]: " << root->d.val << std::endl;
               //std::cout << "[node value]: " << root->d.size_h << std::endl;
               //std::cout << "[used]:" << root->d.used_h << std::endl;
               //std::cout << "[next]:" << root->d.next << std::endl;
               //std::cout << "[size]:" << root->d.size_h << std::endl;
               //std::cout << "[word]: " << root->d.word << std::endl;
               std::cout << "[left child offset]: " << root->lchild << std::endl;
               std::cout << "[right child offset]: " << root->rchild << std::endl;
               std::cout << "[parent offset]: " << root->parent << std::endl;

               if (root->lchild != (offset_t)-1)
               {
                  mmallocsplaynode<T> *lc = this->ptrnode(root->lchild);
                  this->dumptree_t(lc);
               }
               if (root->rchild != (offset_t)-1)
               {
                  mmallocsplaynode<T> *rc = this->ptrnode(root->rchild);
                  this->dumptree_t(rc);
               }
            }
            //void splay(offset_t n, offset_t superroot);
            void splay(offset_t xx, offset_t sr)
            {
               mmallocsplaynode<T> *n = this->ptrnode(xx);
               mmallocsplaynode<T> *superroot = NULL;
               offset_t sulc = this->r;
               offset_t surc = this->r;
               if (sr != (offset_t)-1)
               {
                  superroot = this->ptrnode(sr);
                  sulc = superroot->lchild;
                  surc = superroot->rchild;
               }
               if (sr == n->parent)
               {
                  return;
               }
               // Performs the Zig step
               else if (n->parent == sulc || n->parent == surc)
               {
                  mmallocsplaynode<T> *p = this->ptrnode(n->parent);
                  if (xx == p->lchild)
                  {
                     this->rightrotation(n->parent);
                  }
                  else
                  {
                     this->leftrotation(n->parent);
                  }
               }
               else
               {
                  mmallocsplaynode<T> *p = this->ptrnode(n->parent);
                  mmallocsplaynode<T> *q = this->ptrnode(p->parent);

                  offset_t pb = this->offset(p);
                  // Performs the Zig-zig step
                  // when n is left and n's parent is left
                  if (xx == p->lchild && pb == q->lchild)
                  {
                     this->rightrotation(p->parent);
                     this->rightrotation(n->parent);
                  }
                  // Performs the Zig-zig step
                  // when n is right and n's parent is right
                  else if (xx == p->rchild && pb == q->rchild)
                  {
                     this->leftrotation(p->parent);
                     this->leftrotation(n->parent);
                  }
                  // Performs the Zig-zag step
                  // when n is right and n's parent is left
                  else if (xx == p->rchild && pb == q->lchild)
                  {
                     this->leftrotation(n->parent);
                     this->rightrotation(n->parent);
                  }
                  // Performs the Zig-zag step
                  // when n is left and n's parent is right
                  else if (xx == p->lchild && pb == q->rchild)
                  {
                     this->rightrotation(n->parent);
                     this->leftrotation(n->parent);
                  }
                  this->splay(xx, sr);
               }
            }

            void leftrotation(offset_t poffset)
            {
               mmallocsplaynode<T> *parent = this->ptrnode(poffset);
               mmallocsplaynode<T> *child = this->ptrnode(parent->rchild);
               mmallocsplaynode<T> *grandbaby = NULL;
               offset_t gfoffset = parent->parent;
               offset_t myoffset = parent->rchild;

               if (child->lchild != (offset_t)-1)
               {
                  grandbaby = this->ptrnode(child->lchild);
               }

               this->setleftchild(child, parent);
               this->setrightchild(parent, grandbaby);

               if (poffset == this->r)
               {
                  child->parent = (offset_t)-1;
                  this->root = child;
                  this->r = myoffset;
               }
               else
               {
                  mmallocsplaynode<T> *grandfather = this->ptrnode(gfoffset);
                  if (poffset == grandfather->lchild)
                  {
                     this->setleftchild(grandfather, child);
                  }
                  else
                  {
                     this->setrightchild(grandfather, child);
                  }
               }
            }

            void rightrotation(offset_t poffset)
            {
               mmallocsplaynode<T> *parent = this->ptrnode(poffset);
               mmallocsplaynode<T> *child = this->ptrnode(parent->lchild);
               mmallocsplaynode<T> *grandbaby = NULL;
               offset_t myoffset = parent->lchild;
               offset_t gfoffset = parent->parent;

               if (child->rchild != (offset_t)-1)
               {
                  grandbaby = this->ptrnode(child->rchild);
               }

               this->setrightchild(child, parent);
               this->setleftchild(parent, grandbaby);

               if (poffset == this->r)
               {
                  child->parent = (offset_t)-1;
                  this->root = child;
                  this->r = myoffset;
               }
               else
               {
                  mmallocsplaynode<T> *grandfather = this->ptrnode(gfoffset);
                  if (poffset == grandfather->lchild)
                  {
                     this->setleftchild(grandfather, child);
                  }
                  else
                  {
                     this->setrightchild(grandfather, child);
                  }
               }
            }

            void insert_t(offset_t& b, mmallocsplaynode<T> *n)
            {
               if (b == (offset_t)-1)
               {
                  b = this->offset(n);
                  return;
               }
               n->parent = b;
               mmallocsplaynode<T> *c = this->ptrnode(b);
               if (Comparator()(n->d, c->d))
               {
                  this->insert_t(c->lchild,n);
               }
               else
               {
                  this->insert_t(c->rchild,n);
               }
            }

            mmallocsplaynode<T>* search_t(offset_t b, T& v)
            {
               if (b == (offset_t)-1)
               {
                  return NULL; // Not Found
               }
               mmallocsplaynode<T> *c = this->ptrnode(b);
               if (!Comparator()(v, c->d) && !Comparator()(c->d,v))
               {
                  return c;
               }
               else if (Comparator()(v, c->d))
               {
                  return this->search_t(c->lchild,v);
               }
               else
               {
                  return this->search_t(c->rchild,v);
               }
            }

            mmallocsplaynode<T>* leftmax(offset_t b)
            {
               mmallocsplaynode<T> *n = this->ptrnode(b);
               if (n->rchild == (offset_t)-1)
               {
                  return n;
               }
               return this->leftmax(n->rchild);
            }

            mmallocsplaynode<T>* rightmin(offset_t b)
            {
               mmallocsplaynode<T> *n = this->ptrnode(b);
               if (n->lchild == (offset_t)-1)
               {
                  return n;
               }
               return this->rightmin(n->lchild);
            }

            mmallocsplaynode<T>* rightbestfit_t(offset_t b, T& v)
            {
               if (b == (offset_t)-1)
               {
                  return NULL; // Not Found
               }
               mmallocsplaynode<T> *c = this->ptrnode(b);
               if (!Comparator()(v, c->d) && !Comparator()(c->d,v))
               {
                  return c;
               }
               if (Comparator()(v, c->d))
               {
                  if (c->lchild == (offset_t)-1)
                  {
                     return c;
                  }
                  mmallocsplaynode<T> *n = this->rightbestfit_t(c->lchild,v);
                  if (n)
                  {
                     return n;
                  }
                  else
                  {
                     return c;
                  }
               }
               else
               {
                  if (c->rchild == (offset_t)-1)
                  {
                     return NULL;
                  }
                  return this->rightbestfit_t(c->rchild, v);
               }
               throw "ERR: rightbestfit";
            }

            void check()
            {
               if (!this->base)
               {
                  throw "Unset base address";
               }
               if (this->size_ < 0)
               {
                  throw "Unexpected number";
               }
            }
         private:
            splaytree(const splaytree&);
            splaytree operator=(const splaytree&);
      };
}
# endif /* __SPLAYTREE__ */
