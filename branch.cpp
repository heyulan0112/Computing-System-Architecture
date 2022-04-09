#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>

using namespace std;

class table{
public:
    int m;
    int rows;
    table(int m_bits){
        m = m_bits;
        rows = pow(2,m);
        pht = vector<bitset<2> >(rows,bitset<2>(0));
        for(int i=0;i<rows;i++){
            //Initialized to 11
            pht[i].set();
        }
    }

    bitset<2> read(long int index){
        bitset<2> current_row;
        current_row = pht[index];
        return current_row;
    }

    void modify(long int index, int value){
        bitset<2> val(value);
        for(int i=0;i<2;i++){
            pht[index][i] = val[i];
        }
    }

    void display(){
        for(int i =0;i<rows;i++){
            cout<<pht[i]<<endl;
        }
    }

    vector<bitset<2> > pht;
};

int main (int argc, char** argv) {
    ifstream config;
    config.open(argv[1]);

    int m;
    config >> m;
    config.close();

    table my_pht = table(m);
    int mispredition = 0;
    int total_line = 0;

    ofstream out;
    string out_file_name = string(argv[2]) + ".out";
    out.open(out_file_name.c_str());

    ifstream trace;
    trace.open(argv[2]);

    while (!trace.eof()) {
        total_line++;
        unsigned long pc;
        bool taken;
        trace >> std::hex >> pc >> taken;
        bool prediction;
        prediction = true;

        bitset<32> access_pc(pc);
        bitset<32> lbs(0);
        for(int i=0;i<m;i++){
            lbs[i] = access_pc[i];
        }
        long int index = lbs.to_ulong();

        bitset<2> fetch = my_pht.read(index);
        string str_pre = fetch.to_string<char, std::string::traits_type, std::string::allocator_type>();
        if (str_pre == "11"){
            //ST
            prediction = true;
            if(prediction != taken){
                //ST -> WT
                my_pht.modify(index, 2);
                mispredition++;
            }
        }
        else if(str_pre == "10"){
            //WT
            prediction = true;
            if(prediction == taken){
                //WT -> ST
                my_pht.modify(index, 3);
            }
            else{
                //WT -> WN
                my_pht.modify(index, 1);
                mispredition++;
            }

        }
        else if (str_pre == "01"){
            //WN
            prediction = false;
            if(prediction == taken){
                //WN -> SN
                my_pht.modify(index, 0);
            }
            else{
                //WN -> WT
                my_pht.modify(index, 2);
                mispredition++;
            }
        }
        else{
            //SN
            prediction = false;
            if(prediction != taken){
                //SN -> WN
                my_pht.modify(index, 1);
                mispredition++;
            }
        }

        out << prediction << endl;
    }

    trace.close();
    out.close();

    float miss_rate = float(mispredition)/float(total_line);
    //cout<<"misprediction rate: "<<miss_rate<<endl;
}
