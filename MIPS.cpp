#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>

using namespace std;

#define ADDU 1
#define SUBU 3
#define AND 4
#define OR  5
#define NOR 7

// Memory size.
// In reality, the memory size should be 2^32, but for this lab and space reasons,
// we keep it as this large number, but the memory is still 32-bit addressable.
#define MemSize 65536


//Registor File
class RF
{
  public:
    bitset<32> ReadData1, ReadData2;
    RF()
    {
        Registers.resize(32);
        Registers[0] = bitset<32> (0);
    }

    void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
    {
        // TODO: implement!
        //Read
        if (WrtEnable == 0){
            long int add_1 = RdReg1.to_ulong();
            long int add_2 = RdReg2.to_ulong();
            ReadData1 = Registers[add_1];
            ReadData2 = Registers[add_2];
        }
        //Write
        if (WrtEnable == 1){
            long int add_wrt = WrtReg.to_ulong();
            if (add_wrt < 32)
                Registers[add_wrt] = WrtData;
        }
    }

    void OutputRF()
    {
        ofstream rfout;
        rfout.open("RFresult.txt",std::ios_base::app);
        if (rfout.is_open())
        {
            rfout<<"A state of RF:"<<endl;
            for (int j = 0; j<32; j++)
            {
                rfout << Registers[j] << endl;
            }
        }
        else cout<<"Unable to open file";
        rfout.close();
    }

private:
    vector<bitset<32> >Registers;
};

//ALU
class ALU
{
public:
    bitset<32> ALUresult;
    bitset<32> ALUOperation (bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
    {
        // TODO: implement!
        //R type
        string op_str = ALUOP.to_string<char, std::string::traits_type, std::string::allocator_type>();
        string op_set[5] = {"001","011","100","101","111"};
        int op_no = 0;
        for(int i=0;i<5;i++){
            if(op_str.compare(op_set[i])==0)
            {
                op_no = i;
                break;
            }
        }
        switch (op_no) {
            case 0:{
                long int op1 = oprand1.to_ulong();
                long int op2 = oprand2.to_ulong();
                op1 = op1 + op2;
                bitset<32> result(op1);
                ALUresult = result;
                break;
            }
            case 1:{
                //subu
                long int op1 = oprand1.to_ulong();
                long int op2 = oprand2.to_ulong();
                op1 = op1 - op2;
                bitset<32> result(op1);
                ALUresult = result;
                break;
            }
            case 2:{
                //and
                ALUresult = oprand1 & oprand2;
                break;
            }
            case 3:{
                //or
                ALUresult = oprand1 | oprand2;
                break;
            }
            case 4:{
                //nor
                ALUresult = ~(oprand1 | oprand2);
                break;
            }
            default:
                break;
        }

        return ALUresult;
    }
};

//Havard Memory
//Memory for instructions
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
        else cout<<"Unable to open instruction file";
        imem.close();
    }

    bitset<32> ReadMemory (bitset<32> ReadAddress)
    {
        // TODO: implement!
        // (Read the byte at the ReadAddress and the following three byte).
        int i_index = 31;
        long int add_i = ReadAddress.to_ulong();
        bitset<8> current_i;
        for (int i = 0; i < 4; i++){
            current_i = IMem[add_i+i];
            for (int j = 7; j >= 0; j--){
                Instruction[i_index] = current_i[j];
                i_index--;
            }
        }
        return Instruction;
    }

private:
    vector<bitset<8> > IMem;
};

//Memory for data
class DataMem
{
public:
    bitset<32> readdata;

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
        else cout<<"Unable to open data file";
        dmem.close();
    }

    bitset<32> MemoryAccess (bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem)
    {
        // TODO: implement!
        int d_index = 31;
        long int add_d = Address.to_ulong();
        if (readmem == 1){
            for (int i = 0; i < 4; i++){
                bitset<8> current_d;
                current_d = DMem[add_d+i];
                for (int j = 7; j >= 0; j--){
                    readdata[d_index] = current_d[j];
                    d_index--;
                }
            }
        }
        if (writemem == 1){
            for (int i = 0; i < 4; i++){
                bitset<8> current;
                for (int j = 7; j >= 0; j--){
                    current[j] = WriteData[d_index];
                    d_index--;
                }
                DMem[add_d+i] = current;
            }
        }
        return readdata;
    }

    void OutputDataMem()
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

int main()
{
    RF myRF;
    ALU myALU;
    INSMem myInsMem;
    DataMem myDataMem;
    bitset<32> myPC(0);
    while (1){
        bool I_type = 0, J_type = 0;
        bitset<5> Rd_reg1, Rd_reg2, Wrt_reg;
        bitset<32> Wrt_data,Alu_result,operand1,operand2;
        bitset<3> Aluop;
        bitset<1> enable,is_load,is_store,is_branch;

        // Fetch
        bitset<32> current_ins;
        current_ins = myInsMem.ReadMemory(myPC);
        string ins_str = current_ins.to_string<char, std::string::traits_type, std::string::allocator_type>();
        // If current insturciton is "11111111111111111111111111111111", then break;
        if (ins_str.compare("11111111111111111111111111111111")==0){
            //halt
            break;
        }

        bitset<6> op;
        for(int i=0;i<6;i++)
            op[i] = current_ins[i+26];
        string str_op = op.to_string<char, std::string::traits_type, std::string::allocator_type>();
        // decode(Read RF)
        if(str_op == "100011")
            is_load.set();
        if(str_op == "101011")
            is_store.set();
        if(str_op != "000000" && str_op != "000010")
        {
            I_type = 1;
            string str_op = op.to_string<char, std::string::traits_type, std::string::allocator_type>();
            if(str_op.compare("000100")==0)
                is_branch.set();
        }
        if(str_op == "000010")
            J_type = 1;

        if (is_branch.all() || is_store.all() || J_type)
            enable.reset();

        // Next PC and Read RF
        if(I_type){
            int index = 0;
            for(int i=16;i<21;i++){
                Rd_reg2[index] = current_ins[i];//16-20
                Rd_reg1[index] = current_ins[i+5];//21-25
                Wrt_reg[index] = current_ins[i];//16-20
                index++;
            }
            enable.reset();
            myRF.ReadWrite(Rd_reg1, Rd_reg2, Wrt_reg, Wrt_data, enable);
            if (myRF.ReadData1 == myRF.ReadData2 && is_branch.all()){
                bitset<32> immediate;
                for(int i=0;i<16;i++){
                    immediate[i] = current_ins[i];
                    immediate[i+16] = current_ins[15];
                }
                bitset<32> imm;
                for (int i=2;i<32;i++){
                    imm[i] = immediate[i-2];
                }
                imm[1] = 0;
                imm[0] = 0;
                long int int_imm = imm.to_ulong();
                long int int_pc = myPC.to_ulong();
                int_pc = int_pc + 4 + int_imm;
                bitset<32> next_pc(int_pc);
                myPC = next_pc;
                myRF.OutputRF();
                continue;
            }
            else{
                long int int_pc = myPC.to_ulong();
                int_pc = int_pc + 4;
                bitset<32> next_pc(int_pc);
                myPC = next_pc;
            }
        }

        else if(J_type){
            long int int_pc = myPC.to_ulong();
            int_pc = int_pc + 4;
            bitset<32> tmp(int_pc);
            bitset<32> next_pc;
            for(int i=0;i<2;i++){
                next_pc[i] = 0;
            }
            for(int i=2;i<28;i++){
                next_pc[i] = current_ins[i-2];
            }
            for(int i=28;i<32;i++){
                next_pc[i] = tmp[i];
            }
            myPC = next_pc;
            myRF.OutputRF();
            continue;
        }

        else{
            long int int_pc = myPC.to_ulong();
            int_pc = int_pc + 4;
            bitset<32> next_pc(int_pc);
            myPC = next_pc;

            int index = 0;
            for(int i=16;i<21;i++){
                Rd_reg1[index] = current_ins[i+5];//21-25
                Rd_reg2[index] = current_ins[i];//16-20
                Wrt_reg[index] = current_ins[i-5];//11-15
                index++;
            }
            enable.reset();
            myRF.ReadWrite(Rd_reg1, Rd_reg2, Wrt_reg, Wrt_data, enable);
        }

        // Execute
        //Aluop
        if(str_op == "100011" || str_op == "101011")
            Aluop = 001;
            
        else if (str_op == "000000"){
            for(int i=0;i<3;i++)
                Aluop[i] = current_ins[i];
        }
        else{
            for(int i=0;i<3;i++)
                Aluop[i] = current_ins[i+26];
        }

        //oprand
        operand1 = myRF.ReadData1;
        if(I_type){
            for(int i = 0; i < 16; i++){
                operand2[i] = current_ins[i];
                operand2[i+16] = current_ins[15];
            }
        }
        else{
            operand2 = myRF.ReadData2;
        }

        Alu_result = myALU.ALUOperation(Aluop, operand1, operand2);
        // Read/Write Mem
        // Write back to RF
        enable.set();
        if(is_load.all()){
            bitset<32> mem_read_data;
            mem_read_data = myDataMem.MemoryAccess(Alu_result, myRF.ReadData2, is_load, is_store);
            myRF.ReadWrite(Rd_reg1, Rd_reg2, Wrt_reg, mem_read_data, enable);
        }
        else if(is_store.all()){
            bitset<32> mem_read_data;
            mem_read_data = myDataMem.MemoryAccess(Alu_result, myRF.ReadData2, is_load, is_store);
        }
        else{
            myRF.ReadWrite(Rd_reg1, Rd_reg2, Wrt_reg, Alu_result, enable);
        }

        myRF.OutputRF(); // dump RF;

    }
    myDataMem.OutputDataMem(); // dump data mem
    return 0;
}

