#include <stdio.h>
#include <unistd.h>
#include <libunwind.h>
#include <libunwind-x86_64.h>
#include <libunwind-ptrace.h>
#include <sys/ptrace.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <vector>
#include <unordered_set>
#include <dirent.h>
#include <cstring>
#include <string>
/* Add more header files if you need */

#include <endian.h>
#include <stdlib.h>
#include <assert.h>

using namespace std;

void print_backtrace(int pid);
long convert_str_2_int(string);
bool is_num(string);
vector<pid_t> pause_and_get_tids(pid_t);


/* Changed till here */

extern char **environ;

int main(){
   int target_pid = fork();
   if (target_pid == 0){
      char *exe_name = "sort\0";
      char *argv[] = {exe_name, NULL};
      char **envp = environ;
      execve(argv[0], &argv[0], envp);
   }
   else {
      sleep(0.1);
      int status;
	
      while (true){
         printf("[tracer] please enter 'P' to print the functions on the call stack of 'sort'\n");
         printf("[tracer] or enter other letters to exit the program.\n");
         char input;	
         scanf("%1s", &input);

         if (input=='P'){
            if (waitpid(target_pid, &status, WNOHANG)==target_pid){
               printf("[tracer] timeout\n");
               return 0;
            }
            printf("[tracer] functions in the call stack are:\n");
            
            vector<pid_t> tids = pause_and_get_tids(target_pid);

	    for (pid_t tid: tids){
               print_backtrace(tid);
	    }

	    for (pid_t tid:tids){
               assert(ptrace(PTRACE_DETACH, tid, NULL, NULL) != -1);
	    }
         }
         else{
            printf("[tracer] exit the program\n"); 
            kill(target_pid, SIGKILL);
            return 0;
         }
      }
   }	
   return 0;
}




/* Do the job */
// print the stack frames
// ignored most of the error-handlings, asserted valid
void print_backtrace(int pid) {

    printf("================================== tid = %3d ==================================\n", pid);
	// get an address space and a context
    unw_addr_space_t addr_space = unw_create_addr_space(&_UPT_accessors, __BYTE_ORDER__);
	assert(addr_space);
    unw_context_t context;
    assert(!unw_getcontext(&context));

	// in libunwind-ptrace.h, get the machine state info-structure
    void *rctx = _UPT_create(pid);

	// get a stack cursor pointed to the top
    unw_cursor_t cursor;
    assert(!unw_init_remote(&cursor, addr_space, rctx));
    unw_cursor_t resume_cursor = cursor;

	// take a tour to count the size of the stack, then go back
	int frame_number = 0;
	while (unw_step(&cursor) > 0) {
		++frame_number;
	}
	cursor = resume_cursor;

	// the buffer to which fetches the proc name
    const size_t bufflen = 128;
    char *buff = (char*)malloc(sizeof(char) * bufflen);
	// some examples of getting register values
    unw_word_t ip, sp, rcx;
	unw_word_t offp;

	// print the result
    printf("%3s\t%20s\t%12s\t%12s\t%16s\n", "num", "name", "IP", "SP", "RCX");
    do {
        assert(!unw_get_reg(&cursor, UNW_REG_IP, &ip));
        assert(!unw_get_reg(&cursor, UNW_REG_SP, &sp));
        assert(!unw_get_reg(&cursor, UNW_X86_64_RCX, &rcx));
        assert(!unw_get_proc_name(&cursor, buff, bufflen, &offp));
        printf("%3d\t%20s\t%12lx\t%12lx\t%16lx\n", frame_number--, buff, ip, sp, rcx);
    } while (unw_step(&cursor) > 0);
    
    free(buff);

	// return the address space and the context
    _UPT_destroy(rctx);

	// resume the process that are unwond
    //assert(ptrace(PTRACE_DETACH, pid, NULL, NULL) != -1);
}

long convert_str_2_int(string str){
   long result = 0;
   for (unsigned i=0; i<str.size(); i++){
      result += str[i] - '0';
      if (i!=str.size()-1){
         result = result * 10;
      }
   }
   return result;
}


bool is_num(string word){
   for (unsigned i=0; i<word.size(); i++){
      if (!((word[i]>='0')&&(word[i]<='9'))){
         return false;
      }
   }
   return true;
}


vector<pid_t> pause_and_get_tids(pid_t target_pid){
   vector<pid_t> tids;
   unordered_set<pid_t> tids_set;
   while (true) {
      string path_cpp = "/proc/"+to_string(target_pid)+"/task";
      char * path = new char [path_cpp.length()+1];
      strcpy (path, path_cpp.c_str());

      struct dirent *entry;
      DIR *dir = opendir(path);
      if (dir == NULL) {
         exit(-1);
      }

      bool has_new_tid = false;
      while ((entry = readdir(dir)) != NULL) {
         string tid(entry->d_name);
         if (is_num(tid)) {
            pid_t tid_number = convert_str_2_int(tid);
            if (tids_set.find(tid_number)==tids_set.end()){
               tids.push_back((pid_t) tid_number);
               tids_set.insert(tid_number);
               has_new_tid = true;
            }
         }
      }
      closedir(dir);
      if (!has_new_tid) break;
   }
   for (unsigned i=0; i<tids.size(); i++){
      ptrace(PTRACE_ATTACH, tids[i], 0, 0);
   }

   for (unsigned i=0; i<tids.size(); i++){
      int status;
      waitpid(tids[i], &status, 0);
   }
   return tids;
}


