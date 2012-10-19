# include "sequence.hpp"

using namespace sequential;
using namespace std;
int main(int argc, char **argv)
{
   if (argc < 2)
   {
      cerr << "Usage: " << *argv << " file" << endl;
      exit(1);
   }
   try
   {
   FILE *fp = NULL;
   if ((fp = fopen(*(argv+1), "r")) == NULL)
   {
      throw "Couldn't open file";
   }
   while (feof(fp) == 0)
   {
      sequence sq;
      sqread(fp,&sq,1024);
      sq.dump();
      cout << sq.get(-1, 0) << ' ' << sq.get(3, 0) << endl;
   }
   fclose(fp);
   }
   catch (const char *ex)
   {
      cerr << ex << endl;
   }

   return 0;
}
