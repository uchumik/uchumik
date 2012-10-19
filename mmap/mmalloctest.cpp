# include <mmalloc.hpp>
# include <stdio.h>
# include <vector>

using namespace std;
using namespace mysplay;
using namespace mmallocutilizer;

void* ptr(const void **base, offset_t b)
{
   return (void*)((char*)*base+b);
}

int main (int argc, char **argv)
{
   try
   {
      if (argc < 2)
      {
         fprintf (stderr, "%s number\n",*argv);
         exit(1);
      }
      int num = 0;
      sscanf(*(argv+1),"%d",&num);
      mmalloc *m = new mmalloc("test.shm", Create);
      const void **b = m->baseaddress();
      vector<offset_t> strings;

      for (int i = 0; i < num; ++i)
      {
         int size = rand()%1023 + 1;
         offset_t off = m->allocate(size);
         strings.push_back(off);
         char *str = (char*)ptr(b, off);
         for (int j = 0; j < size-1; j++)
         {
            *(str+j) = rand()%94 + 32;
         }
         *(str+size-1) = '\0';
      }
      delete m;

      mmalloc *r = new mmalloc("test.shm", ReadWrite);
      b = r->baseaddress();
      for (int i = 0; i < num; ++i)
      {
         char *str = (char*)ptr(b, strings[i]);
         cout << str << endl;
         if (rand()%2)
         {
            r->deallocate(strings[i]);
         }
      }
      delete r;
   }
   catch (const char *ex)
   {
      cerr << ex << endl;
   }
   return 0;
}
