# include <mhash.hpp>
# include <iostream>
# include <cstdio>

using namespace hashmmap;
using namespace mysplay;
using namespace mmallocutilizer;
using namespace std;

void usage(int argc, char **argv)
{
   if (argc < 3)
   {
      cerr << "Usage:"<< *argv
      << " string value" << endl;
      exit(1);
   }
}

void* ptr(const void *base, offset_t b)
{
   return (void*)((char*)base+b);
}

int main(int argc, char **argv)
{
   usage(argc, argv);

   mmalloc *m = new mmalloc("test.shm", ReadWrite);
   //mhash<int,char*> h(m);
   mhash<char*,unsigned int> h(m,"test.hash");
   //mhash<int,int> h(m,"test.hash");
   char *key = *(argv+1);
   //int key = 2;
   //char **v = h[key];
   unsigned int *v = h[key];
   //int *v = h[key];
   if (!v)
   {
      //cout << "not exist " << *(argv+1) << endl;
      cout << "not exist " << key << endl;
      unsigned int val = 1;
      //char *val = "hogehoge";
      sscanf(*(argv+2),"%u",&val);
      h.insert(key,val,(unsigned int)strlen(key)+1,sizeof(unsigned int));
      //h.insert(key,val,sizeof(int),strlen(val)+1);
      //h.insert(key,val,sizeof(int),sizeof(int));

      v = h[key];
      cout << "inserted " << key
     // *(argv+1)
         << ' ' << *v << endl;

      ++*v;

      cout << "increment: " << *(h[key]) << std::endl;

/*
      cout << "remove " << key << endl;
      h.remove(key);
      v = h[key];
      if (!v)
      {
         cout << "removed" << std::endl;
      }
      */
   }
   else
   {
      *v += 1;
      cout << "exist " << key
         << ' ' << *v << endl;
   }

   mhashiterator<char*,unsigned int> it = h.begin();
   //mhashiterator<int,int> it = h.begin();
   for (; it != h.end(); ++it)
   {
      cout << "offset:" << (*it).koff << endl;
      cout << "key:" << *((*it).key) << endl;
      //cout << "key:" << (*it).key << endl;
      cout << "val:" << (*it).val << endl;
      /*
      if ((*it).val < 3)
      {
         cout << "remove: " << *((*it).key) << std::endl;
         //h.remove(*((*it).key));
      }
      */
   }

   delete m;
   return 0;
}
