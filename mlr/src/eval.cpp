# include "ensemble.hpp"
# include <iostream>
# include <string>

# ifdef __cplusplus
extern "C"
{
# endif

# include <getopt.h>

# ifdef __cplusplus
}
# endif
using namespace std;
using namespace mlr;

/* Parameters */
static LearnerType type = TypeListNet;
static string corpus;
static string model;

void usage(int argc, char **argv)
{
   cerr << "Usage:" << argv[0] << " [options]" << endl;
   cerr << "-c, --corpus=FILE\ttraining corpus" << endl;
   cerr << "-m, --model=FILE\tmodel file" << endl;
   exit(1);
}

void getparams(int argc, char **argv)
{
   int c;
   while (true)
   {
      static struct option long_options[] =
      {
         /* Long Options */
         {"corpus", required_argument, 0, 'c'},
         {"model", required_argument, 0, 'm'},
         {0, 0, 0, 0}
      };
      /* getopt_long stores the option index here. */
      int option_index = 0;
      c = getopt_long(argc, argv, "c:m:", long_options, &option_index);
      /* Detect the end of the options. */
      if (c == -1)
      {
         break;
      }
      switch (c)
      {
         case 0:
            break;
         case 'c':
            corpus = optarg;
            break;
         case 'm':
            model = optarg;
            break;
         case '?':
         default:
            usage(argc, argv);
      }
   }
   /* Print any remaining command line argument (not options). */
   if (optind < argc)
   {
      cerr << "non-option ARGV-elements: ";
      while (optind < argc)
      {
         cerr << argv[optind++] << " ";
      }
      cerr << endl;
      usage(argc, argv);
   }
   return;
}

int main(int argc, char **argv)
{
   if (argc < 3)
   {
      usage(argc, argv);
   }
   getparams(argc, argv);
   if (corpus == "" || model == "")
   {
      usage(argc, argv);
   }
   EnsembleRanker ranker(model.c_str());
   ifstream in(corpus.c_str());
   while(!in.eof())
   {
      ngilist il;
      ilread(in, il);
      ranker.predict(il);
      ildelete(il);
   }
   return 0;
}
