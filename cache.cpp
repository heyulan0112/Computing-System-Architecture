/*
   Cache Simulator
   Level one L1 and level two L2 cache parameters are read from file
   (block size, line per set and set per cache).
   The 32 bit address is divided into:
   -tag bits (t)
   -set index bits (s)
   -block offset bits (b)

   s = log2(#sets)   b = log2(block size)  t=32-s-b
*/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>


using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss

struct config{
  int L1blocksize;
  int L1setsize;
  int L1size;
  int L2blocksize;
  int L2setsize;
  int L2size;
};


class cache{
public:

    int rows1;
    int rows2;
    int l1_tag_bit;
    int l1_index_bit;
    int l1_offset_bit;
    int l2_tag_bit;
    int l2_index_bit;
    int l2_offset_bit;
    int l1_way;
    int l2_way;

    cache(int blocksize1, int setsize1, int size1, int blocksize2, int setsize2, int size2){
        l1_tag_bit = 0;
        l1_index_bit = 0;
        l1_offset_bit = 0;
        l2_tag_bit = 0;
        l2_index_bit = 0;
        l2_offset_bit = 0;
        l1_way = 0;
        l2_way = 0;
        if (setsize1 == 0){
            setsize1 = size1*1000/blocksize1;
            rows1 = 1;
        }
        else{
            rows1 = size1*1000/(blocksize1*setsize1);
        }
        if (setsize2 == 0){
            setsize2 = size2*1000/blocksize2;
            rows2 = 1;
        }
        else{
            rows2 = size2*1000/(blocksize2*setsize2);
        }

        L1_data = vector<vector<vector<unsigned int> > >(rows1,vector<vector<unsigned int> >(setsize1,vector<unsigned int>(blocksize1,0)));
        L2_data = vector<vector<vector<unsigned int> > >(rows2,vector<vector<unsigned int> >(setsize2,vector<unsigned int>(blocksize2,0)));
        L1_tag = vector<vector<bitset<32> > >(rows1,vector<bitset<32> >(setsize1,bitset<32>(0)));
        L2_tag = vector<vector<bitset<32> > >(rows2,vector<bitset<32> >(setsize2,bitset<32>(0)));
        L1_valid = vector<vector<bool> >(rows1,vector<bool>(setsize1,0));
        L2_valid = vector<vector<bool> >(rows2,vector<bool>(setsize2,0));
        L1_dirty = vector<vector<bool> >(rows1,vector<bool>(setsize1,0));
        L2_dirty = vector<vector<bool> >(rows2,vector<bool>(setsize2,0));
        L1_counter = vector<int>(rows1,0);
        L2_counter = vector<int>(rows2,0);
    }

    bool read(long int index, long int way, bitset<32> tags, int level){
        bool hit_status = 0;
        if(level == 1){
            if(L1_valid[index][way] == 1 && L1_tag[index][way] == tags)
                hit_status = 1;
            else
                hit_status = 0;
        }
        if(level == 2){
            if(L2_valid[index][way] == 1 && L2_tag[index][way] == tags)
                hit_status = 1;
            else
                hit_status = 0;
        }
        return hit_status;
    }

    void replace(long int index, bitset<32> tags, int level, unsigned int addr,long int offset){
        if(level == 1){
            for(int i=0;i<l1_way;i++){
                if(L1_valid[index][i] == 0){
                    L1_valid[index][i] = 1;
                    for(int j=0;j<pow(2,l1_offset_bit);j++){
                        L1_data[index][i][j] = addr-int(offset)+j;
                    }
                    for(int j=0;j<32;j++){
                        L1_tag[index][i][j] = tags[j];
                    }
                    return;
                }
            }

            if(L1_dirty[index][L1_counter[index]] == 1){
                int addr_evict = L1_data[index][L1_counter[index]][0];
                bitset<32> addr_evict_bit(addr_evict);
                bitset<32> index_e(0);
                for(int i=0;i<l2_index_bit;i++){
                    index_e[i] = addr_evict_bit[i+l2_offset_bit];
                }
                long int index_e_trans = index_e.to_ulong();
                int index_e_int = index_e_trans % rows2;
                bitset<32> tag_e(0);
                for(int i=0;i<l2_tag_bit;i++){
                    tag_e[i] = addr_evict_bit[i+l2_offset_bit+l2_index_bit];
                }
                for(int i=0;i<l2_way;i++){
                    if(read(index_e_int, i, tag_e, 2) == 1){
                        L2_dirty[index_e_int][i] = 1;
                        break;
                    }
                }
                L1_dirty[index][L1_counter[index]] = 0;
            }

            for(int j=0;j<pow(2,l1_offset_bit);j++){
                L1_data[index][L1_counter[index]][j] = addr+j-int(offset);
            }
            for(int i=0;i<32;i++){
                L1_tag[index][L1_counter[index]][i] = tags[i];
            }
            L1_counter[index]++;
            if(L1_counter[index] == l1_way)
                L1_counter[index] = 0;
            return;
        }
        if(level == 2){
            for(int i=0;i<l2_way;i++){
                if(L2_valid[index][i] == 0){
                    L2_valid[index][i] = 1;
                    for(int j=0;j<pow(2,l2_offset_bit);j++){
                        L2_data[index][i][j] = addr-int(offset)+j;
                    }
                    for(int j=0;j<32;j++){
                        L2_tag[index][i][j] = tags[j];
                    }
                    return;
                }
            }

            
            for(int i=0;i<pow(2,l2_offset_bit);i++){
                int addr_evict = L2_data[index][L2_counter[index]][i];
                bitset<32> addr_evict_bit(addr_evict);
                bitset<32> index_e(0);
                for(int j=0;j<l1_index_bit;j++){
                    index_e[j] = addr_evict_bit[j+l1_offset_bit];
                }
                long int index_e_trans = index_e.to_ulong();
                int index_e_int = index_e_trans % rows1;
                bitset<32> tag_e(0);
                for(int j=0;j<l1_tag_bit;j++){
                    tag_e[j] = addr_evict_bit[j+l1_offset_bit+l1_index_bit];
                }
                for(int j=0;j<l1_way;j++){
                    if(read(index_e_int, j, tag_e, 1) == 1){
                        
                        L1_valid[index_e_int][j] = 0;
                        L1_dirty[index_e_int][j] = 0;
                        break;
                    }
                }
            }

            if(L2_dirty[index][L2_counter[index]] == 1){
                L2_dirty[index][L2_counter[index]] = 0;
            }
            for(int j=0;j<pow(2,l2_offset_bit);j++){
                L2_data[index][L2_counter[index]][j] = addr-int(offset)+j;
            }
            for(int i=0;i<32;i++){
                L2_tag[index][L2_counter[index]][i] = tags[i];
            }
            L2_counter[index]++;
            if(L2_counter[index]==l2_way)
                L2_counter[index] = 0;
            return;
        }
    }

    bool write(long int index, long int way, bitset<32> tags, int level){
        bool if_hit = 0;
        if(level == 1){
            if(L1_valid[index][way] == 1 && L1_tag[index][way] == tags){
                if_hit = 1;
                L1_dirty[index][way] = 1;
            }
        }
        if(level == 2){
            if(L2_valid[index][way] == 1 && L2_tag[index][way] == tags){
                if_hit = 1;
                L2_dirty[index][way] = 1;
            }
        }
        return if_hit;
    }

    vector<vector<vector<unsigned int> > > L1_data;
    vector<vector<bitset<32> > > L1_tag;
    vector<vector<bool> > L1_valid;
    vector<vector<bool> > L1_dirty;
    vector<int> L1_counter;
    vector<vector<vector<unsigned int> > > L2_data;
    vector<vector<bitset<32> > > L2_tag;
    vector<vector<bool> > L2_valid;
    vector<vector<bool> > L2_dirty;
    vector<int> L2_counter;
};

int main(int argc, char* argv[]){
    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);

    while(!cache_params.eof())  // read config file
    {
        cache_params>>dummyLine;
        cache_params>>cacheconfig.L1blocksize;
        cache_params>>cacheconfig.L1setsize;
        cache_params>>cacheconfig.L1size;
        cache_params>>dummyLine;
        cache_params>>cacheconfig.L2blocksize;
        cache_params>>cacheconfig.L2setsize;
        cache_params>>cacheconfig.L2size;
    }

    // Implement by you:
    // initialize the hirearch cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like

    int l1_type,l2_type;

    cache my_cache(cacheconfig.L1blocksize,cacheconfig.L1setsize,cacheconfig.L1size,cacheconfig.L2blocksize,cacheconfig.L2setsize,cacheconfig.L2size);
    my_cache.l1_way = cacheconfig.L1setsize;
    my_cache.l2_way = cacheconfig.L2setsize;
    my_cache.l1_offset_bit = ceil(log(cacheconfig.L1blocksize)/log(2));
    my_cache.l2_offset_bit = ceil(log(cacheconfig.L2blocksize)/log(2));
    if (cacheconfig.L1setsize == 1){
        l1_type = 0;
        my_cache.l1_index_bit = ceil(log(my_cache.rows1)/log(2));
        my_cache.l1_tag_bit = 32 - my_cache.l1_offset_bit - my_cache.l1_index_bit;
    }
    else if (cacheconfig.L1setsize == 0){
        l1_type = 2;
        my_cache.l1_tag_bit = 32 - my_cache.l1_offset_bit;
    }
    else{
        l1_type = 1;
        my_cache.l1_index_bit = ceil(log(my_cache.rows1)/log(2));
        my_cache.l1_tag_bit = 32 - my_cache.l1_offset_bit - my_cache.l1_index_bit;
    }
    if (cacheconfig.L2setsize == 1){
        l2_type = 0;
        my_cache.l2_index_bit = ceil(log(my_cache.rows2)/log(2));
        my_cache.l2_tag_bit = 32 - my_cache.l2_offset_bit - my_cache.l2_index_bit;
    }
    else if (cacheconfig.L2setsize == 0){
        l2_type = 2;
        my_cache.l2_tag_bit = 32 - my_cache.l2_offset_bit;
    }
    else{
        l2_type = 1;
        my_cache.l2_index_bit = ceil(log(my_cache.rows2)/log(2));
        my_cache.l2_tag_bit = 32 - my_cache.l2_offset_bit - my_cache.l2_index_bit;
    }

    int L1AcceState =0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState =0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;

    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";

    traces.open(argv[2]);
    tracesout.open(outname.c_str());

    string line;
    string accesstype;  // the Read/Write access type from the memory trace;
    string xaddr;       // the address from the memory trace store in hex;
    unsigned int addr;  // the address from the memory trace store in unsigned int;
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;


    if (traces.is_open()&&tracesout.is_open()){
        while (getline (traces,line)){   // read mem access file and access Cache

            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr)) {break;}
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32> (addr);

            bitset<32> offset_l1(0);
            for(int i=0;i<my_cache.l1_offset_bit;i++){
                offset_l1[i] = accessaddr[i];
            }
            long int offset_l1_int = offset_l1.to_ulong();
            bitset<32> index_l1(0);
            for(int i=0;i<my_cache.l1_index_bit;i++){
                index_l1[i] = accessaddr[i+my_cache.l1_offset_bit];
            }

            long int index_l1_trans = index_l1.to_ulong();
            int index_l1_int = index_l1_trans % my_cache.rows1;
            bitset<32> tags_l1(0);
            for(int i=0;i<my_cache.l1_tag_bit;i++){
                tags_l1[i] = accessaddr[i+my_cache.l1_offset_bit+my_cache.l1_index_bit];
            }

            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R")==0)
            {
                //Implement by you:
                // read access to the L1 Cache,
                //  and then L2 (if required),
                //  update the L1 and L2 access state variable;
                bool if_hit_l1 = 0;
                if(cacheconfig.L1setsize == 0)
                    my_cache.l1_way = cacheconfig.L1size*1000/cacheconfig.L1blocksize;

                for(int i=0;i<my_cache.l1_way;i++){
                    if(my_cache.read(index_l1_int, i,tags_l1, 1) == 1){
                        if_hit_l1 = 1;
                        break;
                    }
                }
                if(if_hit_l1 == 1){
                    L1AcceState = 1;
                    L2AcceState = 0;
                }
                else{
                    L1AcceState = 2;
                    my_cache.replace(index_l1_int, tags_l1, 1, addr, offset_l1_int);
                    bitset<32> offset_l2(0);
                    for(int i=0;i<my_cache.l2_offset_bit;i++){
                        offset_l2[i] = accessaddr[i];
                    }
                    long int offset_l2_int = offset_l2.to_ulong();
                    bitset<32> index_l2(0);
                    for(int i=0;i<my_cache.l2_index_bit;i++){
                        index_l2[i] = accessaddr[i+my_cache.l2_offset_bit];
                    }
                    long int index_l2_trans = index_l2.to_ulong();
                    int index_l2_int = index_l2_trans % my_cache.rows2;
                    bitset<32> tags_l2(0);
                    for(int i=0;i<my_cache.l2_tag_bit;i++){
                        tags_l2[i] = accessaddr[i+my_cache.l2_offset_bit+my_cache.l2_index_bit];
                    }
                    bool if_hit_l2 = 0;
                    if(cacheconfig.L2setsize == 0)
                        my_cache.l2_way = cacheconfig.L2size*1000/cacheconfig.L2blocksize;
                    for(int i=0;i<my_cache.l2_way;i++){
                        if(my_cache.read(index_l2_int, i,tags_l2, 2) == 1){
                            if_hit_l2 = 1;
                            break;
                        }
                    }
                    if(if_hit_l2 == 1){
                        L2AcceState = 1;
                    }
                    else{
                        L2AcceState = 2;
                        my_cache.replace(index_l2_int, tags_l2, 2, addr, offset_l2_int);
                    }
                }
            }
            else
            {
                //Implement by you:
                // write access to the L1 Cache,
                //and then L2 (if required),
                //update the L1 and L2 access state variable;
                bool if_hit_l1 = 0;
                if(cacheconfig.L1setsize == 0)
                    my_cache.l1_way = cacheconfig.L1size*1000/cacheconfig.L1blocksize;
                for(int i=0;i<my_cache.l1_way;i++){
                    if(my_cache.write(index_l1_int, i,tags_l1, 1) == 1){
                        if_hit_l1 = 1;
                        break;
                    }
                }
                if(if_hit_l1 == 1){
                    L1AcceState = 3;
                    L2AcceState = 0;
                }
                else{
                    L1AcceState = 4;
                    bitset<32> offset_l2(0);
                    for(int i=0;i<my_cache.l2_offset_bit;i++){
                        offset_l2[i] = accessaddr[i];
                    }
                    bitset<32> index_l2(0);
                    for(int i=0;i<my_cache.l2_index_bit;i++){
                        index_l2[i] = accessaddr[i+my_cache.l2_offset_bit];
                    }
                    long int index_l2_trans = index_l2.to_ulong();
                    int index_l2_int = index_l2_trans % my_cache.rows2;
                    bitset<32> tags_l2(0);
                    for(int i=0;i<my_cache.l2_tag_bit;i++){
                        tags_l2[i] = accessaddr[i+my_cache.l2_offset_bit+my_cache.l2_index_bit];
                    }
                    bool if_hit_l2 = 0;
                    if(cacheconfig.L2setsize == 0)
                        my_cache.l2_way = cacheconfig.L2size*1000/cacheconfig.L2blocksize;
                    for(int i=0;i<my_cache.l2_way;i++){
                        if(my_cache.write(index_l2_int, i,tags_l2, 2) == 1){
                            if_hit_l2 = 1;
                            break;
                        }
                    }
                    if(if_hit_l2 == 1){
                        L2AcceState = 3;
                    }
                    else{
                        L2AcceState = 4;
                    }
                }
            }
            // Output hit/miss results for L1 and L2 to the output file
            tracesout<< L1AcceState << " " << L2AcceState << endl;
        }
        traces.close();
        tracesout.close();
    }
    else cout<< "Unable to open trace or traceout file ";
    return 0;
}
