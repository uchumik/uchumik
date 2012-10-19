# include <splaytree.hpp>
# include <iostream>

using namespace mysplay;
using namespace std;

void usage(int argc, char **argv)
{
   fprintf (stderr, "%s [numbers]", *argv);
   exit (1);
}

void dump(splaytree<int> *st)
{
   splayiterator<int> it=st->begin();

   for (; it != st->end(); ++it)
   {
      cout << "data:" << *it << endl;
   }
}

int main(int argc, char **argv)
{
   splaytree<int> st;

   if (argc < 2)
   {
      usage(argc, argv);
   }

   unsigned int num = argc - 1;
   mmallocsplaynode<int> nums[num];
   for (unsigned int i = 1; i < (unsigned int)argc; ++i)
   {
      sscanf(*(argv+i),"%d",&(nums[i-1].d));
   }

   try
   {
      const void *b = nums;
      st.init(&b);
      cerr << "insertion test" << endl;
      for (unsigned int i = 0; i < num; ++i)
      {
         st.insertion(&nums[i]);
      }
      cerr << "splay tree of after insertion" << endl;
      //st.dumptree();
      dump(&st);
      cerr << endl;

      cerr << "search test" << endl;
      for (int i = 0; i < num; ++i)
      {
         if (i%2 == 0)
         {
            mmallocsplaynode<int> *rst = st.search(i);
            if (rst)
            {
               cerr << "key: " << i << " found!" << endl;
            }
            else
            {
               cerr << "key: " << i << " not found!" << endl;
            }
         }
         else
         {
            mmallocsplaynode<int> *rst = st.search(nums[i].d);
            if (rst)
            {
               cerr << "key: " << nums[i].d << " found!" << endl;
            }
            else
            {
               cerr << "key: " << nums[i].d << " not found!" << endl;
            }
         }
      }
      cerr << endl;
      cerr << "deletion test" << endl;
      for (unsigned int i = 0; i < num; ++i)
      {
         cerr << "delete node [" << i << "]:" << nums[i].d << endl;
         st.deletion(&nums[i]);
         //st.dumptree();
         dump(&st);
      }
   }
   catch (const char *ex)
   {
      cerr << ex << endl;
   }

   return 0;
}
