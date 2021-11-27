/* This balloon program allows one to change the memory of a program. */
#include<iostream>
#include<chrono>
#include<cstdlib>
#include<thread>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<string>
#include<fcntl.h>
#include<fstream>
#include<vector>

unsigned long long TARGET_MEMORY = 0;
unsigned long long CGROUP_MEMORY = 0;
int NUM_BALLOONS = 0;
int BALLOON_ID = 0;
int memory_profile_type = 0;
unsigned long long MEMORY = 0ULL;
std::chrono::system_clock::time_point t_start;
std::vector<long> times = std::vector<long>(); //this is an array containing all the timestamps in a memory profile
std::vector<long> memory_values = std::vector<long>(); //this is an array containing all the memory values in a memory profile

/*this program accepts as input the total cgroup memory, the number of balloons being run, and the target memory we wish
to use in our main program. Everything is in bytes.
Note that at a minimum, the main algorithm will use at least C/(B+1) memory.
*/

unsigned long long set_memory_in_bytes(unsigned long long cgroup_memory, unsigned long long target_memory, int num_balloons){
  if (TARGET_MEMORY == 0 || CGROUP_MEMORY == 0 || NUM_BALLOONS == 0){
    std::cout << "Invalid value! Error!\n";
    exit(1);
  }
  std::cout << cgroup_memory << " " << target_memory << std::endl;
  if (cgroup_memory > target_memory){
    unsigned long long result = (cgroup_memory - target_memory)/num_balloons - 256*1024; //this is a magic number, don't question it
    return result >= 0ULL ? result : 0ULL;
  }
  else {
    return 50ULL; //another magic number, do not question
  }
}


void read_memory_profile(std::string mem_profile_filename) {
  bool timestamp = true;
  double value;
  std::ifstream input_mem_profile = std::ifstream(mem_profile_filename.c_str());
  while (input_mem_profile >> value){
    std::cout << "Value: " << value << std::endl;
    if (timestamp)
      times.push_back((long)value);
    else
      memory_values.push_back((long)value);
    timestamp = !timestamp;
  }
  input_mem_profile.close();
  for (unsigned int i = 0; i < times.size(); i++)
    std::cout << "Time: " << times[i] << " Memory: " << memory_values[i] << std::endl; 
}


int main(int argc, char *argv[]){
  std::cout << "Starting balloon program" << std::endl;
  if (argc < 6){
    std::cout << "Insufficient arguments! Usage: balloon.cpp <memory_profile_type>"
    "<cgroup_memory> <desired_memory> <num_balloons> <balloon_id> <memory_profile_file>\n";
    exit(1);
  }
  memory_profile_type = atoi(argv[1]);
  CGROUP_MEMORY = 1048576ULL * atol(argv[2]);
  TARGET_MEMORY = 1048576ULL * atol(argv[3]);
  NUM_BALLOONS = atoi(argv[4]);
  BALLOON_ID = atoi(argv[5]);
  std::string mem_profile_filename = argv[6];
  
  int fdout, ipcfd;
  std::string filename = "./balloon_data/balloon_data" + std::to_string(BALLOON_ID);
  std::cout << "Balloon data filename: " << filename << std::endl;
  if ((fdout = open(filename.c_str(), O_RDWR, 0x0777 )) < 0){
    printf ("can't create balloon data files for writing\n");
    return 0;
  }
  if ((ipcfd = open("./balloon_data/IPCTEST", O_RDWR, 0x0777 )) < 0) {
    printf ("can't create file for writing\n");
    return 0;
  }
  unsigned long long* dst2;
  if (((dst2 = (unsigned long long*) mmap(0, sizeof(unsigned long long)*1, PROT_READ | PROT_WRITE, MAP_SHARED, ipcfd, 0)) == (unsigned long long*)MAP_FAILED)) {
    printf ("mmap error for output with code 1");
    return 0;
  }
  dst2[0] = TARGET_MEMORY;
  // Worst-case memory profile is given
  if (memory_profile_type == 1)
    read_memory_profile(mem_profile_filename);

  MEMORY = set_memory_in_bytes(CGROUP_MEMORY, TARGET_MEMORY, NUM_BALLOONS);
  std::cout << "Each balloon is mmapping " << MEMORY << std::endl;
  std::cout << "cgroup memory " << CGROUP_MEMORY << ", target memory " << TARGET_MEMORY << std::endl;
  std::cout << "num balloons " <<   NUM_BALLOONS  << std::endl;

  int* dst;
  if (((dst = (int*) mmap(0, MEMORY, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0)) == (int*)MAP_FAILED)){
    printf ("mmap error for output with code 2");
    return 0;
  }
  
  t_start = std::chrono::system_clock::now();
  std::chrono::system_clock::time_point current = std::chrono::system_clock::now();
  unsigned long index = 0;
  unsigned int counter = 0;
  while(true){
    index %= (MEMORY/4);
    //std::cout << index << "here" << MEMORY << std::endl;
    //dst[rand()%MEMORY] = 1;
    *(dst + rand()%(MEMORY/4)) = 1;
    *(dst + index) = 1;
    //dst[index] = 1;
    index += 1000;
    std::chrono::system_clock::time_point t_end = std::chrono::system_clock::now();
    auto wall_time = std::chrono::duration<double, std::milli>(t_end-t_start).count();
    wall_time = wall_time / 1000;
    if (memory_profile_type == 1){
      if (counter < times.size() && 4 * wall_time > times[counter]){
        std::cout << "value stored in times array: " << times[counter] << std::endl;
        munmap(dst,MEMORY);
        MEMORY = set_memory_in_bytes(CGROUP_MEMORY, memory_values[counter], NUM_BALLOONS);
        std::cout << "Changing memory to " << MEMORY << std::endl;
        if (((dst = (int*) mmap(0, MEMORY, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (int*)MAP_FAILED)){
          printf ("mmap error for output with code");
          return 0;
        }
        std::cout << "Done changing memory " << MEMORY << std::endl;
        counter++;
      }
    }
    else if (memory_profile_type == 2){
      if (std::chrono::duration<double, std::milli>(std::chrono::system_clock::now()-current).count() > 15000){
        current = std::chrono::system_clock::now();
        munmap(dst,MEMORY);
        if (MEMORY == 50){
          MEMORY = set_memory_in_bytes(CGROUP_MEMORY, TARGET_MEMORY, NUM_BALLOONS);
        }else{
          MEMORY = 50;
        }
        std::cout << "Changing memory to " << MEMORY << std::endl;
        if (((dst = (int*) mmap(0, MEMORY, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (int*)MAP_FAILED)){
          printf ("mmap error for output with code");
          return 0;
        }
        std::cout << "Done changing memory " << MEMORY << std::endl;
      }
    }
    else if (memory_profile_type == 3){
      std::cout << (int)*dst2 << std::endl;
      if ((int)dst2[0] != 0) { //} && MEMORY != set_memory_in_bytes(CGROUP_MEMORY, (int)dst2[0], NUM_BALLOONS)){
        munmap(dst,MEMORY);
        MEMORY = set_memory_in_bytes(CGROUP_MEMORY, (int)dst2[0], NUM_BALLOONS);
        std::cout << "Changing memory to " << MEMORY << std::endl;
        if (((dst = (int*) mmap(0, MEMORY, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (int*)MAP_FAILED)){
          printf ("mmap error for output with code");
          return 0;
        }
        std::cout << "Done changing memory " << MEMORY << std::endl;
      }
    }
  }
  std::cout << "Balloon done executing." << std::endl;
}