#include <bits/stdc++.h>
using namespace std;

#ifndef ONLINE_JUDGE
#include<bits/debugger.h>
#else
#define graph(x,y)
#define debug(x...)
#endif

map<int, string> reverseRegisterMap;
map<int,string> Line;//line -> label
int label = 1;
int line = 1;
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

map<pair<int,int>, string> RFunc{
    {{0b000, 0b0000000}, "add"},{ {0b000, 0b0100000}, "sub"},
    { {0b000, 0b0000001}, "mul"},{ {0b101, 0b0000001}, "divu"},
    { {0b110, 0b0000001}, "rem"}
};

map<int, string> IFunc{
    {0b000, "addi"},{0b010, "lw"}
};

map<int, string> BFunc{
    {0b100, "blt"},{0b101, "bge"},
    {0b000, "beq"},{0b001, "bne"}
};

map<int, string> SFunc{
    {0b010, "sw"}
};

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

void initializeReverseMaps() {
    // Reverse register map
    for (const auto& entry : rntn) {
        reverseRegisterMap[entry.second] = entry.first;
    }
}

string disassembleRType(string machineCode) {
    string funct7 = toDecimal(machineCode.substr(0, 7)); //check here
    string rs2 = toDecimal(machineCode.substr(7, 5));
    string rs1 = toDecimal(machineCode.substr(12, 5));
    string funct3 = toDecimal(machineCode.substr(17, 3));
    string rd = toDecimal(machineCode.substr(20, 5));
    string opcode = toDecimal(machineCode.substr(25, 7));

    string mnemonic = RFunc[{stoi(funct3),stoi(funct7)}];
    return mnemonic + " " + reverseRegisterMap[stoi(rd)] + ", " +
           reverseRegisterMap[stoi(rs1)] + ", " + reverseRegisterMap[stoi(rs2)];
}

string disassembleIType(string machineCode) {
    string opcode = (machineCode.substr(25, 7));
    if(opcode == "0010011"){//addi
        string immediate = machineCode.substr(0, 12);
        string rs1 = toDecimal(machineCode.substr(12, 5));
        string funct3 = toDecimal(machineCode.substr(17, 3));
        string rd = toDecimal(machineCode.substr(20, 5));
        
        int imm = stoi(toDecimal(immediate));
        if(immediate.front() == '1'){
            twoscomplement(immediate);
            imm = - stoi(toDecimal(immediate));
        }

        string mnemonic = IFunc[stoi(funct3)];
        return mnemonic + " " + reverseRegisterMap[stoi(rd)] + ", " +
            reverseRegisterMap[stoi(rs1)] + ", " +to_string(imm);
    }
    if(opcode == "0000011"){//lw
        string immediate = machineCode.substr(0, 12);
        string rs1 = toDecimal(machineCode.substr(12, 5));
        string funct3 = toDecimal(machineCode.substr(17, 3));
        string rd = toDecimal(machineCode.substr(20, 5));
        
        string mnemonic = IFunc[stoi(funct3)];
        
        return mnemonic + " " + reverseRegisterMap[stoi(rd)] + ", " +toDecimal(immediate)
        +"("+reverseRegisterMap[stoi(rs1)]+")";
    }

   
    
}

string disassembleSType(string machineCode) {
    string immediate = machineCode.substr(0, 7) + machineCode.substr(20, 5);
    string rs2 = toDecimal(machineCode.substr(7, 5));
    string rs1 = toDecimal(machineCode.substr(12, 5));
    string funct3 = toDecimal(machineCode.substr(17, 3));

    string mnemonic = SFunc[stoi(funct3)];
    return mnemonic + " " + reverseRegisterMap[stoi(rs2)] + ", " +
           toDecimal(immediate) + "(" + reverseRegisterMap[stoi(rs1)] + ")";
}

string disassembleBType(string machineCode) {
    string immediate1 = machineCode.substr(0, 1) + machineCode.substr(20, 1) +
                             machineCode.substr(1, 6) + machineCode.substr(21, 4);
    string rs1= toDecimal(machineCode.substr(12, 5));
    string rs2 = toDecimal(machineCode.substr(7, 5));
    string funct3 = toDecimal(machineCode.substr(17, 3));
    string opcode = toDecimal(machineCode.substr(25, 7));
    
    debug(immediate1)
    string tmp;int lbline;
    if(immediate1.front()=='1'){
        twoscomplement(immediate1);
        tmp = toDecimal(immediate1);
        lbline = line - (stoi(tmp)/4);
    }else{
        tmp = toDecimal(immediate1);
        lbline = line + (stoi(tmp)/4);

    }
    string mnemonic = BFunc[stoi(funct3)];
    return mnemonic + " " + reverseRegisterMap[stoi(rs1)] + ", " +
           reverseRegisterMap[stoi(rs2)] + ", " +
           Line[lbline];
}

string disassembleJType(string machineCode) {
    string immediate =
        machineCode.substr(0, 1) + machineCode.substr(12, 8) + machineCode.substr(11, 1) +
        machineCode.substr(1, 10);
    string rd = toDecimal(machineCode.substr(20, 5));
    string opcode = toDecimal(machineCode.substr(25, 7));

    string tmp;int lbline;
    if(immediate.front()=='1'){
        twoscomplement(immediate);
        tmp = toDecimal(immediate);
        lbline = line - (stoi(tmp)/4);
    }else{
        tmp = toDecimal(immediate);
        lbline = line + (stoi(tmp)/4);

    }
    string mnemonic = "jal";
    return mnemonic + " " + reverseRegisterMap[stoi(rd)] + ", " +
           Line[lbline];
}
string disassembleUType(string machineCode) {
    string immediate = machineCode.substr(0, 20);
    string rd = toDecimal(machineCode.substr(20, 5));
    string opcode = toDecimal(machineCode.substr(25, 7));

    string mnemonic = "lui";
    return mnemonic + " " + reverseRegisterMap[stoi(rd)] + ", " +
           toDecimal(immediate);
}

string disassemble(string machineCode) {
    string opcode = (machineCode.substr(25, 7));
    if (opcode == "1101111") { // jal
        return disassembleJType(machineCode);
    } else if (opcode == "1100011") { // Branch Instructions
        return disassembleBType(machineCode);
    } else if (opcode == "0000011" || opcode == "0010011") { // lw and addi
        return disassembleIType(machineCode);
    } else if (opcode == "0100011") { // sw
        return disassembleSType(machineCode);
    } else if (opcode == "0110011") { //R-Type
        return disassembleRType(machineCode);
    } else if(opcode == "0110111"){ //lui
        return disassembleUType(machineCode);
    }
    
    else if(opcode == "0000000"){//label
        // Line[line] = "L"+to_string(label);
        // label++;
        return Line[line]+":";
    }

    return "Not Implemented"; // Default case
}

int main() {
    initializeReverseMaps();
    vector<string>machinecodes;
    string machineCode;
    while (getline(cin, machineCode)) {
        machinecodes.push_back(machineCode);
        string opcode = (machineCode.substr(25, 7));
        if(opcode == "0000000"){//label
            Line[line] = "L"+to_string(label);
            label++;
        }

        line++;
    }
    debug(Line)

    line = 1;
    for(auto machineCode : machinecodes){
        cout << disassemble(machineCode) << endl;
        line++;
    }

    return 0;
}

/*
 initializeReverseMaps();
    vector<string>machinecodes;
    string machineCode;
    while (getline(cin, machineCode)) {
        machinecodes.push_back(machineCode);
            

        
    }

*/
