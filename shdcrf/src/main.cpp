# include "shdcrf.hpp"

using namespace SHDCRF;
using namespace std;

int main(int argc, char **argv)
{
	ShdCrf crf("test", *(argv+1));
	cout << *(argv+1) << endl;
	crf.init();
	crf.learn(500);
	return 0;
}
