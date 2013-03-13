# include "ensemble.hpp"
# include <string>
# include <iostream>

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
static string algorithm = "bagging";
static double penalty = 0.0001;
static double sample = 1.0;
static int epoch = 1;
static unsigned int iter = 100;

void usage(int argc, char **argv)
{
   cerr << "Usage:" << argv[0] << " [options]" << endl;
   cerr << "-c, --corpus=FILE\ttraining corpus" << endl;
   cerr << "-s, --sample=FLOAT\tsampling rate" << endl;
   cerr << "-p, --penalty=FLOAT\tpenalty weight" << endl;
   cerr << "-t, --type=STRING\tlearner type" << endl;
   cerr << "-m, --model=STRING\tmodel file" << endl;
   cerr << "-e, --epoch=INT\tbagging or boosting size" << endl;
   cerr << "-a, --algorithm=STRING\tbagging or boosting" << endl;
   cerr << "-i, --iteration=INT\titeration size" << endl;
   exit(1);
}

LearnerType gettype(string& t)
{
   if (t == "listnet")
   {
      return TypeListNet;
   }
   else if (t == "listmle")
   {
      return TypeListMLE;
   }
   else if (t == "lambdalistnet")
   {
      return TypeLambdaListNet;
   }
   else if (t == "lambdalistmle")
   {
      return TypeLambdaListMLE;
   }
   else if (t == "neurolistnet")
   {
      return TypeNeuroListNet;
   }
   else if (t == "neurolistmle")
   {
      return TypeNeuroListMLE;
   }
   else if (t == "lambdaneurolistnet")
   {
      return TypeLambdaNeuroListNet;
   }
   else if (t == "lambdaneurolistmle")
   {
      return TypeLambdaNeuroListMLE;
   }
   return TypeListNet;
}

int setopt(string pname, string optarg)
{
   if (pname == "sample")
   {
      sample = atof(optarg.c_str());
   }
   else if (pname == "penalty")
   {
      penalty = atof(optarg.c_str());
   }
   else if (pname == "type")
   {
      type = gettype(optarg);
   }
   else if (pname == "epoch")
   {
      epoch = atoi(optarg.c_str());
   }
   else if (pname == "algorithm")
   {
      algorithm = optarg;
   }
   else if (pname == "iteration")
   {
      iter = (unsigned int)atoi(optarg.c_str());
   }

   return 0;
}

int getparams(int argc, char **argv)
{
   int c;
   while (true)
   {
      static struct option long_options[] =
      {
         /* Long Options */
         {"corpus", required_argument, 0, 'c'},
         {"sample", required_argument, 0, 's'},
         {"penalty", required_argument, 0, 'p'},
         {"type", required_argument, 0, 't'},
         {"model", required_argument, 0, 'm'},
         {"epoch", required_argument, 0, 'e'},
         {"algorithm", required_argument, 0, 'a'},
         {"iteration", required_argument, 0, 'i'},
         {0, 0, 0, 0}
      };
      /* getopt_long stores the option index here. */
      int option_index = 0;
      c = getopt_long(argc, argv, "c:s:p:t:m:e:a:i:", long_options, &option_index);
      /* Detect the end of the options. */
      if (c == -1)
      {
         break;
      }
      switch (c)
      {
         case 0:
            if (long_options[option_index].flag != 0)
            {
               break;
            }
            setopt(long_options[option_index].name, optarg);
            break;
         case 'c':
            corpus = optarg;
            break;
         case 'm':
            model = optarg;
            break;
         case 'a':
         case 'e':
         case 'i':
         case 's':
         case 'p':
         case 't':
            setopt(long_options[option_index].name, optarg);
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
   return 0;
}

int main(int argc, char **argv)
{
   getparams(argc, argv);
   if (corpus == "" || model == "")
   {
      usage(argc, argv);
   }
   EnsembleLearner learner(type);
   learner.setcorpus(corpus.c_str());
   learner.setsamplesize(sample);
   learner.setpenalty(penalty);
   if (algorithm == "bagging")
   {
      learner.bagging(epoch, iter);
   }
   else if (algorithm == "boosting")
   {
      learner.boosting(epoch, iter);
   }
   else
   {
      cerr << "non-option algorithm-type: " << algorithm << endl;
      usage(argc, argv);
   }
   return 0;
}
