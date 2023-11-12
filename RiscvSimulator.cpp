#include <bits/stdc++.h>
using namespace std;

#ifndef ONLINE_JUDGE
#include <bits/debugger.h>
#else
#define graph(x, y)
#define debug(x...)
#endif

vector<int> MEM(1024);
vector<int> R(32);
int PC = 0;

int R_ALUOutputReg;
int R_MEMoutdata;
int R_writeBackData;
bool zeroFlagGenerated;
int R_RegDst;
int R_BranchOffset;
int R_JumpOffset;
int R_ALUInput1;
int R_ALUInput2;
int R_dataForStore;
int R_ALUControl;
int JA;
int BA;
bool R_Branch;
bool R_MemRead;
bool R_MemtoReg;
bool R_MemWrite;
bool R_Jump;
bool R_RegWrite;
bool R_ALUDo;
bool R_MemDo;
bool R_WritebackDo;
bool R_End;
bool JumpFlag;
bool BranchPossible;
bool ZeroFlag;
bool EndFlag;

string twoscomplement(string &binary){
    for (char& bit : binary) {
        bit = (bit == '0') ? '1' : '0';
    }
    bool carry = 1;
    for (int i = binary.size() - 1; i >= 0; --i) {
        if (carry) {
            if (binary[i] == '0') {
                binary[i] = '1';
                carry = 0;
            } else {
                binary[i] = '0';
            }
        }
    }
    return binary;
}
string toDecimal(string binary) {
    bitset<32> bits(binary);
    return to_string(bits.to_ulong());
}


class ControlUnit {
    string Instruction;
    string opcode;

   public:
    void setInstruction(string currInst){
        Instruction = currInst;
        opcode = Instruction.substr(25, 7);
    }
    int RegDst() {
        if (opcode == "0110011" || opcode == "0000011" || opcode == "0110111" || opcode == "0010011" || opcode == "1101111"){
            // for R-type/load/lui//addi//jal
            return stoi(Instruction.substr(20, 5), 0, 2);  // returns index of register to be written
        }
        return -1;
    }

    bool Branch() {  // b-type
        if (opcode == "1100011")
            return 1;
        else
            return 0;
    }

    int BranchOffset() {   // return offset -> new pc- pc+offset
        string immediate = Instruction.substr(0, 1) + Instruction.substr(20, 1) +
                            Instruction.substr(1, 6) + Instruction.substr(21, 4);

        string tmp;int offset;

        if (immediate.front() == '1') {
            twoscomplement(immediate);
            tmp = toDecimal(immediate);
            offset = - (stoi(tmp) / 4);
        } else {
            tmp = toDecimal(immediate);
            offset =  (stoi(tmp) / 4);
        }
        return offset;
    }

    bool MemRead() {  // lw
        if (opcode == "0000011")
            return 1;
        else
            return 0;
    }

    bool MemtoReg() {
        if (opcode == "0000011")  // lw
            return 1;
        else
            return 0;
    }

    bool MemWrite() {
        if (opcode == "0100011")  // sw
            return 1;
        else
            return 0;
    }

    bool Jump() {
        if (opcode == "1101111")  // jal
            return 1;
        else
            return 0;
    }

    int JumpOffset() { // return offset -> new pc- pc+offset
        string immediate =Instruction.substr(0, 1) + Instruction.substr(12, 8) + Instruction.substr(11, 1) +Instruction.substr(1, 10);
        string tmp;int offset;

        if (immediate.front() == '1') {
            twoscomplement(immediate);
            tmp = toDecimal(immediate);
            offset = - (stoi(tmp) / 4);
        } else {
            tmp = toDecimal(immediate);
            offset =  (stoi(tmp) / 4);
        }
        return offset;
    }

    int ALUInput1() {  // rs1
        if(opcode == "0110011" || opcode == "0010011" || opcode == "0000011" || opcode == "0100011" || opcode == "1100011"){
             //r/addi/lw/s/B/sw
            return R[stoi(Instruction.substr(12, 5), 0, 2)];
        }else if(opcode == "0110111"){ //lui
            return 0;
        }
    }

    int ALUInput2() {   
        if(opcode == "0110011" || opcode == "1100011"){//r/b
            return R[stoi(Instruction.substr(7, 5), 0, 2)]; //rs2
        }else if(opcode == "0000011"){ //lw
            return 4*stoi(Instruction.substr(0, 12),0,2); //offset
        }else if(opcode == "0100011"){ //sw
            string offset = Instruction.substr(0, 7) + Instruction.substr(20, 5);
            return 4*stoi(offset,0,2);
        }else if(opcode == "0110111"){ //lui
            int data = stoi(Instruction.substr(0, 20),0,2);
            data *=  (1 << 12);
            return data;
        }else if(opcode == "0010011"){//addi
            string immediate  = Instruction.substr(0, 12);
            int imm = stoi(toDecimal(immediate));
            if(immediate.front() == '1'){
                twoscomplement(immediate);
                imm = - stoi(toDecimal(immediate));
            }
            return imm;
        }
    }

    bool RegWrite() {
        if (opcode == "0100011" || opcode == "1100011")  // sw/branch
            return 0;
        else
            return 1;
    }

    int dataForStore() {  // which data to store for st
        int rs2 = stoi(Instruction.substr(7, 5), 0, 2);
        return R[rs2];
    }

    int ALUControl() {
        // tells the ALU which operation to perform
        /*
          -1: error
          0: add
          1: sub
          2: mul
          3. Div
          4. Rem
          5. <
          6. >=
          7. =
          8. !=
          9.jal
          */
       
        if (opcode == "0000011" || opcode == "0100011" || opcode == "0010011")
            return 0;                    // load/store //addi
        else if (opcode == "0110011") {  // R-type
            string funct7 = (Instruction.substr(0, 7));
            string funct3 = (Instruction.substr(17, 3));

            if (funct3 == "000" && funct7 == "0000000")
                return 0;
            else if (funct3 == "000" && funct7 == "0100000")
                return 1;
            else if (funct3 == "000" && funct7 == "0000001")
                return 2;
            else if (funct3 == "101" && funct7 == "0000001")
                return 3;
            else if (funct3 == "110" && funct7 == "0000001")
                return 4;
        }else  if (opcode == "1100011"){//branch
            string funct3 = (Instruction.substr(17, 3));
            if (funct3 == "100")
                return 5;
            else if (funct3 == "101")
                return 6;
            else if (funct3 == "000")
                return 7; 
            else if (funct3 == "001")
                return 8; 
        }else if(opcode == "1101111"){
            return 9;
        }
        return -1;
    }

    bool ALUDo() {
        if(opcode == "0000000"){ //label
            return 0;
        }
        return 1;
    }

    bool MemDo() {
        if (opcode == "0000011" || opcode == "0100011")
            return 1;  // for load/store, memory unit will be active
        else
            return 0;
    }

    bool WritebackDo() {
        if (opcode == "0110011" || opcode == "0000011" || opcode == "0110111" || opcode == "0010011")
            return 1;  // for R-type/load/lui//addi, writeBack unit will be active
        else
            //sw/branch/label
            return 0;
    }

    void giveToRegister() {
        // giving the ID/EX registers the required data
        R_RegDst = RegDst();
        R_Branch = Branch();
        R_BranchOffset = BranchOffset();  // return offset -> new pc- pc+offset
        R_MemRead = MemRead();
        R_MemtoReg = MemtoReg();
        R_MemWrite = MemWrite();
        R_Jump = Jump(); 
        R_JumpOffset = JumpOffset(); /// return offset -> new pc- pc+offset
        R_ALUInput1 = ALUInput1();
        R_ALUInput2 = ALUInput2();
        R_RegWrite = RegWrite();
        R_dataForStore = dataForStore();
        R_ALUControl = ALUControl();
        R_ALUDo = ALUDo();
        R_MemDo = MemDo();
        R_WritebackDo = WritebackDo();
    }
};

class ALU {
   private:
    int ALUInput1;
    int ALUInput2;
    bool ALUdo;
    bool ALUBranch;
    int ALUBranchOffset;
    bool ALUMemRead;
    bool ALUMemToReg;
    bool ALUMemWrite;
    bool ALURegWrite;
    bool ALUMemDo;
    bool ALUWriteBackDo;
    bool ALUJump;
    int ALUJumpOffset;
    int passDataForSt;
    int regAddress;
    int ALUControl;

   public:
    void takeFromRegister() {
        // take the necessary data from the ID/EX registers
        regAddress = R_RegDst;
        ALUBranch = R_Branch;
        ALUBranchOffset = R_BranchOffset;
        ALUMemRead = R_MemRead;
        ALUMemToReg = R_MemtoReg;
        ALUMemWrite = R_MemWrite;
        ALUJump = R_Jump;
        ALUJumpOffset = R_JumpOffset;
        ALUInput1 = R_ALUInput1;
        ALUInput2 = R_ALUInput2;
        ALURegWrite = R_RegWrite;
        passDataForSt = R_dataForStore;
        ALUControl = R_ALUControl;
        ALUdo = R_ALUDo;
        ALUMemDo = R_MemDo;
        ALUWriteBackDo = R_WritebackDo;
        if (ALUdo) {
            if (ALUInput1 - ALUInput2 == 0 && ALUBranch) {
                ZeroFlag = true;
            }
            if (ALUBranch) {
                zeroFlagGenerated = true;
            }
            R_ALUDo = false;
        }
    }

    void giveToRegister() {
        if (ALUControl == 0) {
            R_ALUOutputReg = ALUInput1 + ALUInput2;  // funct add
        }

        else if (ALUControl == 1) {
            R_ALUOutputReg = ALUInput1 - ALUInput2;  // funct subtract
        }

        else if (ALUControl == 2) {
            R_ALUOutputReg = ALUInput1 * ALUInput2;  // funct mul
        }

        else if (ALUControl == 3) {
            R_ALUOutputReg = ALUInput1 / ALUInput2;  // funct mul
        }

        else if (ALUControl == 4) {
            R_ALUOutputReg = ALUInput1 % ALUInput2;  // funct mul
        }

        //branch
        else if (ALUControl == 5) {
            if(ALUInput1 < ALUInput2){
                PC += 4*ALUBranchOffset;
            }
        }

        else if (ALUControl == 6) {
            if(ALUInput1 >= ALUInput2){
                PC += 4*ALUBranchOffset;
            }
        }

        else if (ALUControl == 7) {
            if(ALUInput1 == ALUInput2){
                PC += 4*ALUBranchOffset;
            }
        }

        else if (ALUControl == 8) {
            if(ALUInput1 != ALUInput2){
                PC += 4*ALUBranchOffset;
            }
        }

        else if (ALUControl == 9) { //jal
            //R_ALUOutputReg = PC + 4;  // funct mul
            PC += 4*ALUJumpOffset;
        }
        debug(ALUInput1, ALUInput2, R_ALUOutputReg)

        // R_memRead1 = ALUMemRead;
        // R_writeBackDo1 = ALUWriteBackDo;
        // R_memDo1 = ALUMemDo;
        // R_regWrite1 = ALURegWrite;
        // R_memWrite1 = ALUMemWrite;
        // R_memToReg1 = ALUMemToReg;
        // R_memWriteData1 = passDataForSt;
        // R_regDst1 = regAddress;
    }
};

class Memory {
   private:
    bool memRead;
    bool memWrite;
    bool memDo;
    bool regWrite;
    bool writeBackDo;
    int ALUOutput;
    int dataToBeStored;
    int readData;
    int regDst;
    bool end;

   public:
    void takeFromRegister() {
        // takes the necessary data from EX/MEM registers
        memWrite = R_MemWrite;
        memDo = R_MemDo;
        ALUOutput = R_ALUOutputReg;
        dataToBeStored = R_dataForStore;
        memRead = R_MemRead;
        writeBackDo = R_WritebackDo;
        regWrite = R_RegWrite;
        regDst = R_RegDst;
        if (memWrite == 1 && memDo == 1) {
            int i = 0;
            while(i<4){
                MEM[ALUOutput+i] = dataToBeStored % (1<<8);
                dataToBeStored /=  (1<<8);
                i++;
            }
            R_MemDo = false;
            R_MemWrite = false;
        }
    }

    void giveToRegister() {
        
        readData = 0;
        if (memRead == 1 && memDo == 1) {
            int i = 0;
            while(i<4){
                readData *=  (1<<8);
                readData += MEM[ALUOutput+(4-i-1)];
                i++;
            }
            R_writeBackData = readData;    // data to write
            R_MemRead = false;
            R_MemDo = false;
        }else{
            R_writeBackData = R_ALUOutputReg;
            debug(R_ALUOutputReg, R_WritebackDo)
        }
    }
};

class Writeback {
   private:
    int dataToBeWritten;
    int regDst;
    bool writeDo;
    bool memdo;
   public:
    void takeFromRegister() {
        // taking the necessary data from MEM/WB registers
        writeDo = R_WritebackDo;
        regDst = R_RegDst;
        if (writeDo) {
            dataToBeWritten = R_writeBackData;
            R_WritebackDo = false;
        }
    }
    void writeBackToRegister() {
        if (writeDo == 1) {
            debug(regDst)
            R[regDst] = dataToBeWritten;
            R_writeBackData = 0;
        }
    }
};

void reset(){
    R_ALUOutputReg = 0;
    R_MEMoutdata = 0;
    R_writeBackData = 0;
    zeroFlagGenerated = 0;
    R_RegDst = -1;
    R_BranchOffset = 0;
    R_JumpOffset = 0;
    R_ALUInput1 = 0;
    R_ALUInput2 = 0;
    R_dataForStore = 0;
    R_ALUControl = -1;
    JA = 0;
    BA = 0;
    R_Branch = 0;
    R_MemRead = 0;
    R_MemtoReg = 0;
    R_MemWrite = 0;
    R_Jump = 0;
    R_RegWrite = 0;
    R_ALUDo = 0;
    R_MemDo = 0;
    R_WritebackDo = 0;
    R_End = 0;
    JumpFlag = 0;
    BranchPossible = 0;
    ZeroFlag = 0;
    EndFlag = 0;
}

ControlUnit cu;
ALU alu;
Memory mem;
Writeback wb;

class Fetch{
    string currInstruction;
    
    public:
    void fetchInstruction(string machineCode){
        currInstruction = machineCode;
        string opcode = (machineCode.substr(25, 7));
        if(opcode == "0000000"){
            PC += 4;
            return;
        }
        cu.setInstruction(currInstruction);
        cu.giveToRegister();
        alu.takeFromRegister();
        alu.giveToRegister();
        mem.takeFromRegister();
        mem.giveToRegister();
        wb.takeFromRegister();
        wb.writeBackToRegister();
        PC += 4;
    }

};

vector<string> machinecodes;

void ExecuteMachineCode() {
    Fetch f;
    
    int noOfInst = machinecodes.size();
    while(PC < (4*noOfInst)){
        cerr<<"************* "<<(PC)<<" *************"<<endl;
        f.fetchInstruction(machinecodes[(PC/4) + (PC % 4)]);
        //debug(MEM)
        debug(R[9], R[11], R[12],R[13],R[14],R[15],MEM[1000],MEM[1008])
        debug(R_ALUControl)
        reset();
        cerr<<endl;cerr<<endl;
    }
}

void setInitialValue(){
    //cout<<"Set Initial Value of Memory & Register"<<endl;
    //Ex:
        // R[10] = 5; R[15] = 12;
        // MEM[8] = 3;
}

void TakeMachineCode(){
   // cout<<"Enter Your MachineCode"<<endl;
    string machineCode;
    while (getline(cin, machineCode)) {
        machinecodes.push_back(machineCode);
    }
}

int main() {
    
    setInitialValue();
    TakeMachineCode();
    ExecuteMachineCode();

    return 0;
}
