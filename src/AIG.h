//
// Created by grorge on 5/15/23.
//

#ifndef MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_AIG_H
#define MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_AIG_H
#include "aiger.h"
#include <string>

using namespace std;
class AIG {
public:
    AIG(string name){
        aiger *input = aiger_init();
        const char *err_msg = aiger_open_and_read_from_file(input, name.c_str());
        printf("ERROR:%s\n", err_msg);
        fflush(stdout);
        FILE *fp;
        fp = fopen("./output.aig", "w");
        // 寫入 ASCII 的 AIGER 檔案
        aiger_mode mode = aiger_ascii_mode;
        int err = aiger_write_to_file(input, mode, fp);

        aiger_reset(input);
    }

};


#endif //MULTI_BIT_LARGE_SCALE_BOOLEAN_MATCHING_AIG_H
