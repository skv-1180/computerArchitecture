/* Write an assembler for conversion of the assembly code to machine encodings using the instructions*/

#include <bits/stdc++.h>
using namespace std;


#ifndef ONLINE_JUDGE
#include<bits/debugger.h>
#else
#define graph(x,y)
#define debug(x...)
#endif

/* Assumptions 
comment -> ignore
label -> 00000000000000000*/

map<string, int> rntn{//register name to number
    {"zero", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3},
    {"tp", 4}, {"t0", 5}, {"t1", 6}, {"t2", 7},
    {"s0", 8}, {"s1", 9}, {"a0", 10}, {"a1", 11},
    {"a2", 12}, {"a3", 13}, {"a4", 14}, {"a5", 15},
    {"a6", 16}, {"a7", 17}, {"s2", 18}, {"s3", 19},
    {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23},
    {"s8", 24}, {"s9", 25}, {"s10", 26}, {"s11", 27},
    {"t3", 28}, {"t4", 29}, {"t5", 30}, {"t6", 31}
};

unordered_map<string, int> opcodes{
    {"jal", 0b1101111}, {"jalr", 0b1100111},
    {"blt", 0b1100011}, {"bge", 0b1100011},
    {"beq", 0b1100011}, {"bne", 0b1100011},
    {"lw", 0b0000011}, {"sw", 0b0100011},
    {"addi", 0b0010011}, {"add", 0b0110011},
    {"sub", 0b0110011}, {"lui", 0b0110111},
    {"mul", 0b0110011}, {"divu", 0b0110011},
    {"rem", 0b0110011}, {"blez", 0b1100011},
    {"bnez", 0b1100011}, {"beqz", 0b1100011}

};

unordered_map<string, int> func3{
    {"addi", 0b000}, {"add", 0b000},
    {"blt", 0b100}, {"bge", 0b101},
    {"beq", 0b000}, {"bne", 0b001},
    {"lw", 0b010}, {"sw", 0b010},
    {"sub", 0b000},{"mul", 0b000}, 
    {"divu", 0b101},{"rem", 0b110}, 
    {"blez", 0b000},{"bnez", 0b001}, 
    {"beqz", 0b000}
};

unordered_map<string, int> func7{
    {"add", 0b0000000},{"sub", 0b0100000},
    {"mul", 0b00000001},{"divu", 0b0000001},
    {"rem", 0b0000001},
};

map<string, int> Label;
int line = 1;

/* function */
string toBinary(int value, int width) {
    bitset<32> bits(value);
    return bits.to_string().substr(32 - width);
}

string assembleRType(istringstream& iss, string func) {
    string rd, rs1, rs2;
    iss >> rd >> rs1 >> rs2;

    if(rd.back() == ',')rd.pop_back();
    if(rs1.back() == ',')rs1.pop_back();
    if(rs2.back() == ',')rs2.pop_back();
    
    int opcode = opcodes[func];
    int funct3 = func3[func];
    int funct7 = func7[func];

    return toBinary(funct7, 7) + toBinary(rntn[rs2], 5) + toBinary(rntn[rs1], 5) +
           toBinary(funct3, 3) + toBinary(rntn[rd], 5) + toBinary(opcode, 7);
}

string assembleIType(istringstream& iss, string func) {
    if(func == "addi"){
        string rd, rs1, imm;
        iss >> rd >> rs1 >> imm;
        if(rd.back() == ',')rd.pop_back();
        if(rs1.back() == ',')rs1.pop_back();
        int opcode = opcodes[func];
        int funct3 = func3[func];
        int immediate = stoi(imm);
        debug(rd,rs1)
        return toBinary(immediate, 12) + toBinary(rntn[rs1], 5) +
            toBinary(funct3, 3) + toBinary(rntn[rd], 5) + toBinary(opcode, 7);
    }
    if(func == "lw"){
        string rd, rs1, imm, tmp;
        iss >> rd >> tmp;

        if(rd.back() == ',')rd.pop_back();

        int i1 = tmp.find('('), i2 = tmp.find(')');
        imm = tmp.substr(0, i1);
        rs1 = tmp.substr(i1+1, i2-i1-1);
        debug(i1,i2,rs1,imm,rd)

        int opcode = opcodes[func];
        int funct3 = func3[func];
        int immediate = stoi(imm);
        
        return toBinary(immediate, 12) + toBinary(rntn[rs1], 5) +
            toBinary(funct3, 3) + toBinary(rntn[rd], 5) + toBinary(opcode, 7);

    }
}

string assembleSType(istringstream& iss, string func){
    string rs2, rs1, imm, tmp;
    iss >> rs2 >> tmp;

    if(rs2.back() == ',')rs2.pop_back();
    
    int i1 = tmp.find('('), i2 = tmp.find(')');
    imm = tmp.substr(0, i1);
    rs1 = tmp.substr(i1+1, i2-i1-1);

    int opcode = opcodes[func];
    int funct3 = func3[func];
    string immediate = toBinary(stoi(imm), 12);

    return immediate.substr(0,7) + toBinary(rntn[rs2], 5) + toBinary(rntn[rs1], 5) +
        toBinary(funct3, 3) + immediate.substr(7,5)  + toBinary(opcode, 7);

}

string assembleBType(istringstream& iss, string func) {
    string rs1, rs2, label;
    iss >> rs1 >> rs2 >> label;

    if(rs1.back() == ',')rs1.pop_back();
    if(rs2.back() == ',')rs2.pop_back();
    
    int level = Label[label];
    int imm = (level - line)*4;
    int opcode = opcodes[func];
    int funct3 = func3[func]; 

    string immediate =  toBinary(imm, 12);
    string imm1 = immediate[0] + immediate.substr(2,6);
    string imm2 = immediate[1] + immediate.substr(8,4);
    debug(line,level,imm,immediate,imm1,imm2)
    return imm1 + toBinary(rntn[rs2], 5) + toBinary(rntn[rs1], 5) +
           toBinary(funct3, 3) + imm2 + toBinary(opcode, 7);
}

string assembleJType(istringstream& iss, string func) {
    string rd,label;
    if(func == "j"){
        rd = "zero";
        iss>>label;
        func = "jal";
    }else{
        iss >> rd>> label;
    }

    if(rd.back() == ',')rd.pop_back();
    
    int level = Label[label];
    int imm = (level - line)*4;
    int opcode = opcodes[func];
    debug(line,level,imm)
    string tmp =  toBinary(imm, 20);
    string immediate = tmp.substr(0,1) + tmp.substr(10,10) + tmp.substr(9,1) + tmp.substr(1,8);
    debug(tmp,immediate)

    return immediate + toBinary(rntn[rd], 5) + toBinary(opcode, 7);
}

string assembleUType(istringstream& iss, string func) {
    string rd,imm;
    iss >> rd>> imm;

    if(rd.back() == ',')rd.pop_back();

    int opcode = opcodes[func];
 

    return toBinary(stoi(imm), 20) + toBinary(rntn[rd], 5) + toBinary(opcode, 7);
}

string assemblePType(istringstream& iss, string func) {
    string rd,imm;
    iss >> rd>> imm;
    
    if(rd.back() == ',')rd.pop_back();

    //lui rd, imm1
    int imm1 = stoi(imm)>>12;
    
    //addi rd, rd, imm2
    int imm2 = stoi(imm)%(1<<12);
    
    string s1 = "lui "+rd+", "+ to_string(imm1);
    string s2 = "addi "+rd+", "+rd+", "+to_string(imm2);
    istringstream iss1(s1);
    string mnemonic1; iss1 >> mnemonic1;
    istringstream iss2(s2);
    string mnemonic2; iss2 >> mnemonic2;
    cout<<assembleUType(iss1,mnemonic1)<<endl;
    return assembleIType(iss2,mnemonic2);
}

string assemble(string instruction) {
    istringstream iss(instruction);
    string mnemonic; iss >> mnemonic;

    if (mnemonic == "add" || mnemonic == "sub" || mnemonic == "mul" || mnemonic == "divu" || mnemonic == "rem") {
        return assembleRType(iss,mnemonic);
    }else if(mnemonic == "beq" || mnemonic == "bne" || mnemonic == "blt" || mnemonic == "bge" ){
        return assembleBType(iss,mnemonic);
    }else if (mnemonic == "addi" || mnemonic == "lw") {
        return assembleIType(iss,mnemonic);
    }else if(mnemonic == "sw"){
        return assembleSType(iss,mnemonic);
    }else if(mnemonic == "jal" || mnemonic == "j"){
        return assembleJType(iss,mnemonic);
    }else if(mnemonic == "lui"){
        return assembleUType(iss,mnemonic);
    }else if(mnemonic == "li"){
         line++;//2 line -> lui + addi
        return assemblePType(iss,mnemonic);
    }
    return "00000000000000000000000000000000"; // Default case, return all zeros

}

signed main()
{
    for(int i=0;i<32;i++){
        rntn["x"+to_string(i)] = i;
    }
    vector<string>instructions;
    string instruction;
    while (getline(cin, instruction))
    {
        if(instruction[0] == '#')continue;
        if(instruction == "nop")break;

        if(instruction.back() == ':'){
            instruction.pop_back();
            Label[instruction] = line;
        }
        instructions.push_back(instruction);


        if(instruction.substr(0,2) == "li"){
            line++;
        }
        line ++;
    }
    line = 1;
    for(auto instruction : instructions){
        debug(instruction)
        cout<<assemble(instruction)<<endl;
        cout<<flush;
        line++;
    }
    return 0;
}