
#include "SearchEngine.h"

int main(int argc, char const *argv[]) {

	SearchEngine();

	return 0;
}



/*

Run time to parse 11995 json files (9:30, including 30 seconds writing to file)
Run time to parse  1000 json files (0:50)


Run time to parse the corpus and populate index: 1:20


Total number of articles indexed:            11995
Total numer of words indexed:                7662939
Total number of unique words indexed:        349614
Total number of unique authors indexed:      30313
Average number of words indexed per article: 638
Average number of stop words in per article: 1884

Top 50 most frequent words => 
report: 10593
high: 10557
data: 10450
addit: 10382
show: 10334
infect: 10244
time: 10205
import: 9935
similar: 9919
...




Run times for different queries:

mitochondria (0:07)
AND mitochondria barrier (0:05)
OR membrane barrier NOT virus (0:07)
OR membrane barrier NOT virus AUTHOR David (0:01)


cell (1:22)
bio (0:06)
cell NOT virus (0:17)
AND cell bio NOT virus (0:02)
OR cell bio NOT virus AUTHOR Liu (0:07)

boichemical (search term not found)

*/



