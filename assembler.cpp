//
//  assembler.cpp: Written by Grayson Arthur
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <string.h>
#include <vector>
#include <fstream>
#include <map>
#include <queue>

std::vector<std::string> tokenize_instruction(std::string);

int instr_type(std::string);

std::string machine_instruction(std::vector<std::string>);

std::string register_to_address(std::string);
std::string op_to_opcode(std::string);
std::string decimal_to_binary(std::string);
std::string decimal_to_binary(int);
std::string decimal_to_binary_12bit(std::string);

std::string hex_to_decimal(char);

enum formats {R, J, Ia, Ib, Ic};

int main(int argc, char* argv[])
{
    //unit testing
    /*std::cout << "jal op: " << op_to_opcode("jal") << std::endl;
    std::cout << "add op:" << op_to_opcode("add") << std::endl;
    std::cout << "or type" << instr_type("or") << std::endl;
    std::cout << "beq type" << instr_type("beq") << std::endl;

    std::cout << "15: " << decimal_to_binary(15) << std::endl;
    std::cout << "6: " << decimal_to_binary("6") << std::endl;

    std::cout << "addi $t1, $t2, $t3: " << std::endl;
    std::vector<std::string> t;// = tokenize_instruction("addi $t1, $t2, $t3");
    for(auto it = t.begin(); it != t.end(); it++)
        std::cout << *it << std::endl;

    //return 0;*/

    std::ifstream asm_file;
    asm_file.open(argv[1]);
    if(!asm_file.is_open()) {
        std::cout << "Usage: ./a.out filename.txt" << std::endl;
        return 0;
    }

    //construct instruction queue
    std::string line;
    std::map<std::string, int> labeled_instructions;
    std::vector<std::string> unpositioned_labels;
    std::map<std::string, int> labels;
    std::vector<std::string> t;
    std::vector<std::string> program;
    //std::vector<std::string> t;
    int index = 0;
    char hex[4];
    while(std::getline(asm_file, line))
    {
        if(line.find("li ") != std::string::npos) { //load immediate instruction conversion
            t = tokenize_instruction(line);
            //for(auto it = t.begin(); it != t.end(); it++)
            //    std::cout << *it << std::endl;
            sscanf(t.at(2).c_str(), "0x%c%c%c%c", &hex[0], &hex[1], &hex[2], &hex[3]);
            for(int i = 0; i < 4; i++)
            {
                program.push_back("addi " + t[1] + ", " + t[1] + ", " + hex_to_decimal(hex[i]));index++;
                if(i < 3) {
                    program.push_back("sll " + t[1] + ", " + t[1] + ", " + "4");index++;
                }
            }
            //program.push(line);index++;
        } else if(line.find(":") != std::string::npos) { //label collection
            std::string l = line.substr(0, line.find(":"));
            labels[l] = index;
            int pos = line.find(":");
            program.push_back(line.substr(pos + 2, line.length() - pos));index++;
        } else if(line.find("beq") != std::string::npos || line.find("j") != std::string::npos || line.find("bne") != std::string::npos) { //label instruction collection
            labeled_instructions[line] = index;
            program.push_back(line);index++;
        } else { //regular legal instruction no labels
            if(!line.empty()) program.push_back(line);index++;
        }
        
    }

    //return 0;

    /*index = 0;
    for(auto it = program.begin(); it != program.end(); it++)
    {
        std::cout << index++ << ": " << *it << std::endl;
    }*/

    //std::cout << std::endl << std::endl;

    //fuck with the labels and shit here
    for(auto it = labeled_instructions.begin(); it != labeled_instructions.end(); it++) {
        line = it->first;
        int i = it->second;
        //std::cout << line << ": " << i << std::endl;
        t = tokenize_instruction(line);
        int x;
        switch (instr_type(t.at(0)))
        {
        case Ic: //PC Relative addressing
            index = labels[t.at(3)];
            x = index - i - 1;
            program[i] = t.at(0) + ", " + t.at(1) + ", " + t.at(2) + ", " + std::to_string(x);
            break;
        
        case J: //Pseudo Direct Addressing
            program[i] = t.at(0) + " " + std::to_string(labels[t.at(1)]);
            break;

        default:
            break;
        }
    }

    //std::cout << std::endl << std::endl;

    /*index = 0;
    for(auto it = program.begin(); it != program.end(); it++)
    {
        std::cout << index++ << ": " << *it << std::endl;
    }*/
    
    //return 0;

    //convert into machine code
    //std::string instr;
    std::vector<std::string> tokens;
    for(auto it = program.begin(); it != program.end(); it++)
    {
        //instr = program.front();
        //program.pop();
        tokens = tokenize_instruction(*it);
        std::string instruction_bin = machine_instruction(tokens);
        std::cout << "\"" << instruction_bin.substr(0,8) << "\"" << ", \"" << instruction_bin.substr(8,8) << "\"," << std::endl;
    }
    
    return 0;
}

//simple lookup table
int instr_type(std::string instruction)
{
    if(instruction == "add" || instruction == "sub" || instruction == "and" || instruction == "or" || instruction == "xor" || instruction == "slt" || instruction == "jr") {
        return R;
    } else if (instruction == "jal" || instruction == "j") {
        return J;
    } else if (instruction == "addi" || instruction == "sll" || instruction == "srl") { // ins $rs, $rt, i
        return Ia;
    } else if (instruction == "lh" || instruction == "sh") { // ins $rs, i($rt)
        return Ib;
    } else if (instruction == "beq" || instruction == "bne") { // ins $rs, $rt, Label
        return Ic;
    } else {
        return -1;
    }
}

//break a line up into tokens
std::vector<std::string> tokenize_instruction(std::string instruction)
{
    std::vector<std::string> result;
    
    const char delims[] = " ,()";
    char instruction_line[40];
    strcpy(instruction_line, instruction.c_str());
    char * token = strtok(instruction_line, delims);

    while (token != NULL)
    {
        result.push_back(std::string(token));
        token = strtok(NULL, delims);
    }

    return result;
}

//concatenates the machine code numbers of the tokens into a single large instruction
std::string machine_instruction(std::vector<std::string> token_list)
{

    std::string return_string;

    switch (instr_type(token_list.at(0)))
    {
    case R:
        return_string = op_to_opcode(token_list.at(0)) + register_to_address(token_list.at(2)) + register_to_address(token_list.at(3)) + register_to_address(token_list.at(1));
        break;

    case J:
        return_string = op_to_opcode(token_list.at(0)) + decimal_to_binary_12bit(token_list.at(1));
        break;

    case Ia:
    case Ic:
        return_string = op_to_opcode(token_list.at(0)) + register_to_address(token_list.at(1)) + register_to_address(token_list.at(2)) + decimal_to_binary(token_list.at(3));
        break;

    case Ib:
        return_string = op_to_opcode(token_list.at(0)) + register_to_address(token_list.at(1)) + register_to_address(token_list.at(3)) + decimal_to_binary(token_list.at(2));
    
    default:
        break;
    }
    return return_string;
}

//another lookup table which takes the letter as a base and then adds the number next to it
std::string register_to_address(std::string reg)
{
    if(reg == "$zero") {
        return "0000";
    } else if("$ra") {
        return "0001";
    } else if("sp")  {
        return "1111";
    } else {
        char base;
        int base_addr, offset;
        sscanf(reg.c_str(), "$%c%d", &base, &offset);
        switch (base)
        {
        case 'a':
            base_addr = 2;
            break;

        case 't':
            base_addr = 4;
            break;
        
        case 'v':
            base_addr = 8;
            break;

        default:
            break;
        }
        return decimal_to_binary(base_addr + offset);
    }
}

//simple lookup table
std::string op_to_opcode(std::string operation)
{
    if(operation == "addi") {
        return "0000";
    } else if(operation == "add") {
        return "0001";
    } else if(operation == "sub") {
        return "0010";
    } else if(operation == "and") {
        return "0011";
    } else if(operation == "or") {
        return "0100";
    } else if(operation == "xor") {
        return "0101";
    } else if(operation == "sll") {
        return "0110";
    } else if(operation == "srl") {
        return "0111";
    } else if(operation == "lh") {
        return "1000";
    } else if(operation == "sh") {
        return "1001";
    } else if(operation == "j") {
        return "1010";
    } else if(operation == "beq") {
        return "1011";
    } else if(operation == "bne") {
        return "1100";
    } else if(operation == "slt") {
        return "1101";
    } else if(operation == "jal") {
        return "1111";
    } else {
        return "";
    }
}

//input the number in the string into an int and then turn that int into binary
std::string decimal_to_binary(std::string decimal)
{
    int num;
    char return_string[4];
    sscanf(decimal.c_str(), "%d", &num);
    for(int i = 0; i < 4; i++) {
        if(num % 2 == 1) {
            return_string[i] = '1';
        } else {
            return_string[i] = '0';
        }
        num /= 2;
    }
    return std::string(return_string);
}

std::string decimal_to_binary(int decimal)
{
    char return_string[4];
    for(int i = 0; i < 4; i++) {
        if(decimal % 2 == 1) {
            return_string[i] = '1';
        } else {
            return_string[i] = '0';
        }
        decimal /= 2;
    }
    return std::string(return_string);
}

std::string decimal_to_binary_12bit(std::string decimal)
{
    int num;
    char return_string[12];
    sscanf(decimal.c_str(), "%d", &num);
    for(int i = 0; i < 12; i++) {
        if(num % 2 == 1) {
            return_string[i] = '1';
        } else {
            return_string[i] = '0';
        }
        num /= 2;
    }
    return std::string(return_string);
}

std::string hex_to_decimal(char digit)
{
    switch (digit)
    {
    case '0':
        return "0";
        break;

    case '1':
        return "1";
        break;

    case '2':
        return "2";
        break;

    case '3':
        return "3";
        break;

    case '4':
        return "4";
        break;

    case '5':
        return "5";
        break;

    case '6':
        return "6";
        break;

    case '7':
        return "7";
        break;

    case '8':
        return "8";
        break;

    case '9':
        return "9";
        break;
        
    case 'A':
    case 'a':
        return "10";
        break;

    case 'B':
    case 'b':
        return "11";
        break;

    case 'C':
    case 'c':
        return "12";
        break;

    case 'D':
    case 'd':
        return "13";
        break;

    case 'E':
    case 'e':
        return "14";
        break;
    
    case 'F':
    case 'f':
        return "15";
        break;
    
    default:
        return "";
        break;
    }
}