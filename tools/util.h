#include <iostream>
#include <utility>
#include <time.h>
#include <sys/time.h>
#include <unistd.h> 
#include <vector>

using namespace std;

double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

double get_cpu_time(){
    return (double)clock() / CLOCKS_PER_SEC;
}

std::ofstream memoryProfileSetup (string type_of_run, 
    unsigned long long num_elements, 
    unsigned long long memory_bytes) {
    std::ofstream out;
    if (type_of_run == "adversarial") {
        std::string tmp = "mem_profiles/sorting_profile_adversarial_" 
            + std::to_string(num_elements) + "_" 
            + std::to_string(memory_bytes) + ".txt";
        out = std::ofstream(tmp, std::ofstream::out);
    }
    else if (type_of_run == "benevolent") {
        std::string tmp = "mem_profiles/sorting_profile_benevolent_" 
            + std::to_string(num_elements) + "_" 
            + std::to_string(memory_bytes) + ".txt";
        out = std::ofstream(tmp, std::ofstream::out);
    }
    return out;
}

void disable_oom_killer() {
    int pid = getpid();
    char cmd[200];
    sprintf(cmd,"sudo echo -1000 > /proc/%d/oom_score_adj",pid);
    system(cmd);
}

std::vector<long long> io_counter_sda(11);
std::vector<long long> io_counter_sda_new(11);
std::vector<long long> io_counter_sdc(11);
std::vector<long long> io_counter_sdc_new(11);

void print_io_data() {
    ifstream in;
    in.open("/proc/diskstats");
    string word;
    int count = -1;
    int type = 0;
    while (in >> word){
        if (count >= 0 && count < 11) {
            if (type == 0) {
                io_counter_sda_new[count] = stol(word);
            } else {
                io_counter_sdc_new[count] = stol(word);
            }
            count++;
        }
        if (word == "sda5") {
            type = 0;
            count = 0;          
        } else if (word == "sdc") {
            type = 1;
            count = 0;
        }
    }
    cout << "Activity report for sda5: ";
    for (int i = 0; i < 8; i++){
        cout << io_counter_sda_new[i] - io_counter_sda[i] << ", ";
    }
    cout << endl;
    cout << "Activity report for sdc: ";
    for (int i = 0; i < 8; i++){
        cout << io_counter_sdc_new[i] - io_counter_sdc[i] << ", ";
    }
    cout << endl;
    io_counter_sda = io_counter_sda_new;
    io_counter_sdc = io_counter_sdc_new;
    in.close();
}

void print_mem_data(){
    char res[200];
    int pid = getpid();
    sprintf(res,"/proc/%d/statm",pid);
    printf("%s\n",res);
    FILE* fp = fopen(res,"r");
    if (fp == NULL){
        printf("Null\n");
        return;
    }
    char c;
    c = fgetc(fp);
    while (c != EOF){
        printf("%c", c);
        c = fgetc(fp);
    }
    fclose(fp);
}

