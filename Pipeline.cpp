#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem;
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu
    bool        wrt_enable;
    bool        nop;
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem;
    bool        wrt_enable;
    bool        nop;
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public:
        bitset<32> Reg_data;
         RF()
        {
            Registers.resize(32);
            Registers[0] = bitset<32> (0);
        }

        //Read
        bitset<32> readRF(bitset<5> Reg_addr)
        {
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }

        //Write
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
         
        void outputRF()
        {
            ofstream rfout;
            rfout.open("RFresult.txt",std::ios_base::app);
            if (rfout.is_open())
            {
                rfout<<"State of RF:\t"<<endl;
                for (int j = 0; j<32; j++)
                {
                    rfout << Registers[j]<<endl;
                }
            }
            else cout<<"Unable to open file";
            rfout.close();
        }
            
    private:
        vector<bitset<32> >Registers;
};

class INSMem
{
    public:
        bitset<32> Instruction;
        INSMem()
        {
            IMem.resize(MemSize);
            ifstream imem;
            string line;
            int i=0;
            imem.open("imem.txt");
            if (imem.is_open())
            {
                while (getline(imem,line))
                {
                    line = line.substr(0,8);
                    IMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
            imem.close();
        }
        //Read
        bitset<32> readInstr(bitset<32> ReadAddress)
        {
            string insmem;
            insmem.append(IMem[ReadAddress.to_ulong()].to_string());
            insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
            insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
            insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
            Instruction = bitset<32>(insmem);        //read instruction memory
            return Instruction;
        }
      
    private:
        vector<bitset<8> > IMem;
};

class DataMem
{
    public:
        bitset<32> ReadData;
        DataMem()
        {
            DMem.resize(MemSize);
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {
                    line = line.substr(0,8);
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();
        }
        //Read data
        bitset<32> readDataMem(bitset<32> Address)
        {
            string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);        //read data memory
            return ReadData;
        }
        //Write data
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));
        }
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();
        }
      
    private:
        vector<bitset<8> > DMem;
};

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl;

        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl;

        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl;
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;

        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl;
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl;
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl;
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl;
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl;
    }
    else cout<<"Unable to open file";
    printstate.close();
}
 

int main()
{
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
    stateStruct state;
    int cycle = 0;

    state.IF.PC.reset();
    state.IF.nop = 0;
    state.ID.nop = 1;
    state.EX.nop = 1;
    state.MEM.nop = 1;
    state.WB.nop = 1;
    state.EX.wrt_enable = 0;
    state.EX.rd_mem = 0;
    state.EX.wrt_mem = 0;
    state.EX.is_I_type = 0;
    state.EX.alu_op = 1;
    string ins_str;

    while (1) {
        stateStruct newState;

        /* --------------------- WB stage --------------------- */
        if(state.WB.nop == 0){
            if(state.WB.wrt_enable == 1){
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
            }
        }
        if(state.IF.nop == 1 && state.ID.nop == 1 && state.EX.nop == 1 && state.MEM.nop == 1){
            newState.WB.nop = 1;
        }

        /* --------------------- MEM stage --------------------- */
        if(state.MEM.nop == 0){
            newState.WB.nop = 0;
            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            newState.WB.wrt_enable = state.MEM.wrt_enable;

            if(state.MEM.rd_mem == 1)
            {
                //lw
                newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
            }
            else if(state.MEM.wrt_mem == 1)
            {
                //sw
                myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
                newState.WB.Wrt_data = state.MEM.Store_data;
            }
            else{
                //add or sub result
                newState.WB.Wrt_data = state.MEM.ALUresult;
            }

        }
        else{
            newState.WB.nop = 1;
        }
        if(state.IF.nop == 1 && state.ID.nop == 1 && state.EX.nop == 1){
            newState.MEM.nop = 1;
        }

        /* --------------------- EX stage --------------------- */
        if(state.EX.nop == 0){
            newState.MEM.nop = 0;
            newState.MEM.Rs = state.EX.Rs;
            newState.MEM.Rt = state.EX.Rt;
            newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
            newState.MEM.wrt_enable = state.EX.wrt_enable;
            newState.MEM.rd_mem = state.EX.rd_mem;
            newState.MEM.wrt_mem = state.EX.wrt_mem;
            newState.MEM.Store_data = state.EX.Read_data2;
            bitset<32> operand1;
            bitset<32> operand2;

            bool is_ex_ex_frd = 0;
            if(state.EX.is_I_type){
                //lw or sw
                if(state.EX.Rs == state.MEM.Wrt_reg_addr){
                    operand1 = state.MEM.ALUresult;
                    is_ex_ex_frd = 1;
                }
                if(state.EX.Rs == state.WB.Wrt_reg_addr && is_ex_ex_frd == 0){
                    operand1 = state.WB.Wrt_data;
                }
                if(state.EX.Rs != state.MEM.Wrt_reg_addr && state.EX.Rs != state.WB.Wrt_reg_addr){
                    operand1 = state.EX.Read_data1;
                }
                for(int i = 0; i < 16; i++){
                    operand2[i] = state.EX.Imm[i];
                    operand2[i+16] = state.EX.Imm[15];
                }
                if(state.EX.wrt_mem == 1){
                    //sw
                    is_ex_ex_frd = 0;
                    if(state.EX.Rt == state.MEM.Wrt_reg_addr){
                        newState.MEM.Store_data = state.MEM.ALUresult;
                        is_ex_ex_frd = 1;
                    }
                    if(state.EX.Rt == state.WB.Wrt_reg_addr && is_ex_ex_frd == 0){
                        newState.MEM.Store_data = state.WB.Wrt_data;
                    }
                }
            }
            else{
                //R-type addu or subu
                if(state.EX.Rs ==  state.MEM.Wrt_reg_addr){
                    operand1 = state.MEM.ALUresult;
                    is_ex_ex_frd = 1;
                }
                if(state.EX.Rt == state.MEM.Wrt_reg_addr){
                    operand2 = state.MEM.ALUresult;
                    is_ex_ex_frd = 1;
                }
                if(state.EX.Rs == state.WB.Wrt_reg_addr && is_ex_ex_frd == 0){
                    operand1 = state.WB.Wrt_data;
                }
                if(state.EX.Rt == state.WB.Wrt_reg_addr && is_ex_ex_frd == 0){
                    operand2 = state.WB.Wrt_data;
                }
                if(state.EX.Rs != state.MEM.Wrt_reg_addr && state.EX.Rs != state.WB.Wrt_reg_addr){
                    operand1 = state.EX.Read_data1;
                }
                if(state.EX.Rt != state.MEM.Wrt_reg_addr && state.EX.Rt != state.WB.Wrt_reg_addr){
                    operand2 = state.EX.Read_data2;
                }
            }

            if(state.EX.alu_op == 1){
                //addu
                long int op1 = operand1.to_ulong();
                long int op2 = operand2.to_ulong();
                op1 = op1 + op2;
                bitset<32> result(op1);
                newState.MEM.ALUresult = result;
            }
            else{
                //subu
                long int op1 = operand1.to_ulong();
                long int op2 = operand2.to_ulong();
                op1 = op1 - op2;
                bitset<32> result(op1);
                newState.MEM.ALUresult = result;
            }
        }
        else{
            newState.MEM.nop = 1;
        }
        if(state.IF.nop == 1 && state.ID.nop == 1){
            newState.EX.nop = 1;
        }

        /* --------------------- ID stage --------------------- */
        if(state.ID.nop == 0){
            newState.EX.nop = 0;
            newState.EX.alu_op = 1;
            newState.EX.is_I_type = 0;
            newState.EX.wrt_enable = 0;
            newState.EX.rd_mem = 0;
            newState.EX.wrt_mem = 0;
            newState.EX.Rs.reset();
            newState.EX.Rt.reset();
            newState.EX.Wrt_reg_addr.reset();
            bitset<6> op;
            for(int i=0;i<6;i++)
                op[i] = state.ID.Instr[i+26];
            string str_op = op.to_string<char, std::string::traits_type, std::string::allocator_type>();
            if(str_op != "000000"){
                newState.EX.is_I_type = 1;
            }
            if(str_op != "101011"){
                newState.EX.wrt_enable = 1;
            }
            for(int i=0;i<16;i++){
                newState.EX.Imm[i] = state.ID.Instr[i];
            }
            if(newState.EX.is_I_type){
                //I-type
                int index = 0;
                for(int i=16;i<21;i++){
                    newState.EX.Rs[index] = state.ID.Instr[i+5];//21-25
                    newState.EX.Rt[index] = state.ID.Instr[i];//16-20
                    newState.EX.Wrt_reg_addr[index] = state.ID.Instr[i];//16-20
                    index++;
                }
                newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
                newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);
                if(str_op == "100011"){
                    //lw
                    newState.EX.rd_mem = 1;
                    newState.EX.wrt_mem = 0;
                }
                if(str_op == "101011"){
                    //sw
                    newState.EX.rd_mem = 0;
                    newState.EX.wrt_mem = 1;
                }

                if(str_op == "000100"){
                    //beq
                    newState.EX.wrt_enable = 0;
                    if(newState.EX.Read_data1 != newState.EX.Read_data2){
                        bitset<32> imm;
                        for (int i=2;i<18;i++){
                            imm[i] = newState.EX.Imm[i-2];
                        }
                        for (int i=18;i<32;i++){
                            imm[i] = newState.EX.Imm[15];
                        }
                        imm[1] = 0;
                        imm[0] = 0;
                        long int int_imm = imm.to_ulong();
                        long int int_pc = state.IF.PC.to_ulong();
                        int_pc = int_pc + int_imm;
                        bitset<32> next_pc(int_pc);
                        newState.IF.PC = next_pc;
                        state.IF.nop = 1;
                        newState.EX.nop = 1;
                    }
                }
            }

            if(newState.EX.is_I_type == 0){
                //R-type
                int index = 0;
                for(int i=16;i<21;i++){
                    newState.EX.Rs[index] = state.ID.Instr[i+5];//21-25
                    newState.EX.Rt[index] = state.ID.Instr[i];//16-20
                    newState.EX.Wrt_reg_addr[index] = state.ID.Instr[i-5];//11-15
                    index++;
                }
                newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
                newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);
                //subu
                if(state.ID.Instr[1]==1)
                    newState.EX.alu_op = 0;

                if(newState.EX.Rs == newState.MEM.Rt ||newState.EX.Rt == newState.MEM.Rt){
                    bitset<32> prev_inst;
                    long int_pc_prev = state.IF.PC.to_ulong();
                    int_pc_prev = int_pc_prev - 8;
                    bitset<32> prev_pc(int_pc_prev);
                    prev_inst = myInsMem.readInstr(prev_pc);
                    bitset<6> op_prev;
                    for(int i=0;i<6;i++)
                        op_prev[i] = prev_inst[i+26];
                    string str_op_prev = op_prev.to_string<char, std::string::traits_type, std::string::allocator_type>();
                    if(str_op_prev == "100011"){
                        //need to stall
                        long int int_pc = state.IF.PC.to_ulong();
                        int_pc = int_pc - 4;
                        bitset<32> next_pc(int_pc);
                        state.IF.PC = next_pc;
                        newState.EX.nop = 1;
                    }
                }
            }
        }
        else{
            newState.EX.nop = 1;
        }
        if(state.IF.nop == 1){
            newState.ID.nop = 1;
        }

        /* --------------------- IF stage --------------------- */
        if(state.IF.nop == 0){
            newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
            ins_str = newState.ID.Instr.to_string<char, std::string::traits_type, std::string::allocator_type>();
            if (ins_str.compare("11111111111111111111111111111111")==0){
                //halt
                newState.IF.nop = 1;
                newState.ID.nop = 1;
            }
            else{
                newState.ID.nop = 0;
                long int int_pc = state.IF.PC.to_ulong();
                int_pc = int_pc + 4;
                bitset<32> next_pc(int_pc);
                newState.IF.PC = next_pc;
            }
        }
        else{
            newState.ID.nop = 1;
        }

        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;

        printState(newState, cycle);
        cycle++;
        state.IF.nop = 0;
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */
    }

    myRF.outputRF(); // dump RF;
    myDataMem.outputDataMem(); // dump data mem
    return 0;

}
