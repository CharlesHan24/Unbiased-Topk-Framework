#include <iostream>
#include <algorithm>
#include <fstream> 
#include <stdio.h>
#include <string.h>
#include <cmath> 
#include <string>
#include <cstdlib>
#include <unordered_map>
#include "BOBHash32.h"
#include "data.h"
#include "archive/USS.h"
#include "util.h"
#include <cassert>
using namespace std;


FILE* fout;

BOBHash32* hash_id = new BOBHash32(rand() % MAX_PRIME32);

void test_topk(int topk, int rep_time, int tot_flow, int memory){
    USS* s;
	int i, j;
	double ARE_under, ARE_unbiased, ARE_over, ARE, AAE_unbiased, throughput;
	int under_es, over_es, tot_deviate, equal;

	double avg_throughput = 0, avg_AAE_unbiased = 0, avg_ARE_under = 0, avg_ARE_unbiased = 0, avg_ARE_over = 0, avg_f1 = 0, avg_under = 0, avg_over = 0, avg_equal = 0, avg_deviate = 0, avg_badcase = 0;

	for (int t = 0; t < rep_time; t++){
		printf("%d ", t);

		s = new USS((memory) / 76);

		TP start, finish;
		start = now();
		for (i = 0; i < tot_flow; i++){
			s->Insert(hashed_value[i]);
        }
		finish = now();
		throughput = (double) tot_flow / std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000> > >(finish - start).count();
		avg_throughput += throughput / rep_time;
		

		ARE_under = ARE_over = ARE_unbiased = AAE_unbiased = 0;
		under_es = over_es = tot_deviate = 0;

		s->query_init(flow, topk, flow_cnt);

		int hit = 0, tmp_hit;
		int badcase = 0;
		equal = 0;
		for (i = 0; i < topk; i++){
			int res = s->query_topk(flow[i].hash_value);
            tmp_hit = res > 0;

			hit += tmp_hit;
			if (tmp_hit){
				tot_deviate += res - flow[i].cnt;
				AAE_unbiased += fabs(res - flow[i].cnt);
                ARE_unbiased += fabs(res - flow[i].cnt) / (double)(flow[i].cnt);
				if (fabs(res - flow[i].cnt) / (double)(flow[i].cnt) > 0.2){
					badcase++;
				}
                if (res > flow[i].cnt){
					over_es++;
                }
                else if (res < flow[i].cnt){
                    under_es++;
                }
                else{
                    equal++;
                }
			}
		}

		avg_AAE_unbiased += AAE_unbiased / (double)hit / rep_time;
		avg_ARE_unbiased += ARE_unbiased / (double)hit / rep_time;
		avg_under += (double)under_es / rep_time;
		avg_over += (double)over_es / rep_time;
		avg_equal += (double)equal / rep_time;
		avg_deviate += (double)tot_deviate / rep_time;
		avg_f1 += (double)hit / (double)topk / rep_time;
		avg_badcase += (double)badcase / rep_time;

		delete(s);
	}
	fprintf(fout, "%f %f %f %f %f %f %f %f %f %f\n", avg_throughput, avg_AAE_unbiased, avg_ARE_unbiased, avg_under, avg_over, avg_equal, avg_deviate, avg_f1, avg_badcase);
	printf("\n");
}

int main(int argc, const char* argv[]){
	int memory, percent, topk_val, blen;
	int n_case = 0, dataset_id;
	int tot_flow_cnt;
	char exp_name[100];
	FILE* fin = fopen(argv[1], "r");

	fscanf(fin, "%d %d %s", &n_case, &dataset_id, exp_name);
	fout = fopen(exp_name, "a+");
	fprintf(fout, "Stream Summary:\n");

	srand(time(NULL));
	printf("start loading\n");
	tot_flow_cnt = load_data(TOTAL_FLOW, dataset_id);
	printf("end loading\n");

	for (int t = 0; t < n_case; t++){
		printf("Test case #%d\n", t);
		fscanf(fin, "%d %d %d %d", &memory, &percent, &topk_val, &blen);
		
		test_topk(topk_val, REP_TIME, min(TOTAL_FLOW, tot_flow_cnt), memory * 1024);
		printf("\n");
	}
	fclose(fin);
    fclose(fout);
	return 0;
} 
