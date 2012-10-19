# include "shdcrf.hpp"

using namespace SHDCRF;
using namespace std;

int main(int argc, char **argv)
{
   ShdCrfTagger crf("test");

   crf.tagging(*(argv+1));

   return 0;
}
