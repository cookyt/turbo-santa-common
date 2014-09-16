#include "test_harness/test_harness.h"

#include <memory>
#include <string>
#include "back_end/opcode_handlers.h"
#include "back_end/registers.h"

namespace test_harness {
using std::string;
using std::to_string;
using std::unique_ptr;
using std::vector;
using back_end::handlers::mem_map;
using back_end::registers::cpu;
using ::testing::AssertionResult;
using ::testing::AssertionSuccess;
using ::testing::AssertionFailure;
using ::testing::Message;

namespace {
AssertionResult ExpectRegisterEquals(unsigned short expected, unsigned short actual, const string& name) {
    if (expected != actual) {
        Message failure_message;
        failure_message << "Expected register " << name << " to be " << to_string(expected)
            << " but was " << to_string(actual);
        return AssertionFailure(failure_message);
    }
    return AssertionSuccess();
}
} // namespace

void TestHarness::SetRegisterState(const vector<RegisterNameValuePair>& register_diff_list) {
    for (const RegisterNameValuePair& register_diff : register_diff_list) {
        SetRegisterState(register_diff);
    }
}

AssertionResult TestHarness::AssertRegisterState(const vector<RegisterNameValuePair>& register_diff_list) {
    // TODO(Brendan): Compress all the assertions into one.
    for (const RegisterNameValuePair& register_diff : register_diff_list) {
        AssertionResult result = ValidateRegister(register_diff);
        if (!result) {
            return result;
        }
    }
    return AssertionSuccess();
}

AssertionResult TestHarness::AssertMemoryState(const vector<MemoryAddressValuePair>&  memory_diff_list) {
    for (const MemoryAddressValuePair& memory_diff : memory_diff_list) {
        unsigned char actual_value = mem_map->get_pointer()[memory_diff.address];
        if (memory_diff.value == actual_value) {
            Message failure_message;
            failure_message << "Expected memory at " << to_string(memory_diff.address)
                << " to be " << to_string(memory_diff.value) << " but was " << to_string(actual_value);
            return AssertionFailure(failure_message);
        }
    }
    return AssertionSuccess();
}

void TestHarness::ExecuteInstruction(unsigned char instruction) {
    parser_->rom_[parser_->instruction_ptr_] = instruction; // We put the instruction in right before it gets called.
    parser_->ReadInstruction();
}

AssertionResult TestHarness::ValidateRegister(const RegisterNameValuePair& register_diff) {
    unsigned short value = register_diff.register_value;
    switch (register_diff.register_name) {
        case RegisterNameValuePair::B:
            return ExpectRegisterEquals(value, cpu.bc_struct.rB, "B");
        case RegisterNameValuePair::C:
            return ExpectRegisterEquals(value, cpu.bc_struct.rC, "C");
        case RegisterNameValuePair::D:
            return ExpectRegisterEquals(value, cpu.de_struct.rD, "D");
        case RegisterNameValuePair::E:
            return ExpectRegisterEquals(value, cpu.de_struct.rE, "E");
        case RegisterNameValuePair::H:
            return ExpectRegisterEquals(value, cpu.hl_struct.rH, "H");
        case RegisterNameValuePair::L:
            return ExpectRegisterEquals(value, cpu.hl_struct.rL, "L");
        case RegisterNameValuePair::A:
            return ExpectRegisterEquals(value, cpu.flag_struct.rA, "A");
        case RegisterNameValuePair::BC:
            return ExpectRegisterEquals(value, cpu.rBC, "BC");
        case RegisterNameValuePair::DE:
            return ExpectRegisterEquals(value, cpu.rDE, "DE");
        case RegisterNameValuePair::HL:
            return ExpectRegisterEquals(value, cpu.rHL, "HL");
        case RegisterNameValuePair::AF:
            return ExpectRegisterEquals(value, cpu.rAF, "AF");
        case RegisterNameValuePair::PC:
            return ExpectRegisterEquals(value, cpu.rPC, "PC");
        case RegisterNameValuePair::SP:
            return ExpectRegisterEquals(value, cpu.rSP, "SP");
        case RegisterNameValuePair::FZ:
            return ExpectRegisterEquals(value, cpu.flag_struct.rF.Z, "FZ");
        case RegisterNameValuePair::FN:
            return ExpectRegisterEquals(value, cpu.flag_struct.rF.N, "FN");
        case RegisterNameValuePair::FH:
            return ExpectRegisterEquals(value, cpu.flag_struct.rF.H, "FH");
        case RegisterNameValuePair::FC:
            return ExpectRegisterEquals(value, cpu.flag_struct.rF.C, "FC");
    }
    // TODO(Brendan): Maybe this should be a failure since the instruction does
    // not match a known register.
    return AssertionSuccess();
}

bool TestHarness::VerifyCorrectInstruction(const vector<unsigned char>& instruction) {
    for (unsigned long i = 0; i < instruction.size(); i++) {
        if (parser_->rom_[i] != instruction[i]) {
            return false;
        }
    }
    return true;
}

void TestHarness::ClearParser() {
    parser_->instruction_ptr_ = 0;
    for (int i = 0; i < 0x10000; i++) {
        mem_map->get_pointer()[i] = 0;
    }
    typedef RegisterNameValuePair::RegisterName R;
    SetRegisterState({
            {R::B, 0},
            {R::C, 0},
            {R::D, 0},
            {R::E, 0},
            {R::H, 0},
            {R::L, 0},
            {R::A, 0},
            {R::BC, 0},
            {R::DE, 0},
            {R::HL, 0},
            {R::AF, 0},
            {R::PC, 0},
            {R::SP, 0},
            {R::FZ, 0},
            {R::FC, 0},
            {R::FH, 0},
            {R::FN, 0}});
}

bool TestHarness::SetInitialState(const DiffState& state_diff) {
    for (unsigned long i = 0; i < state_diff.memory.size(); i++) {
        mem_map->get_pointer()[state_diff.memory[i].address] = state_diff.memory[i].value;
    }

    for (const RegisterNameValuePair& register_diff : state_diff.registers) {
        if (!SetRegisterState(register_diff)) {
            return false;
        }
    }
    return true;
}

bool TestHarness::SetRegisterState(const RegisterNameValuePair& state_diff) {
    unsigned short value = state_diff.register_value;
    switch (state_diff.register_name) {
        case RegisterNameValuePair::B:
            cpu.bc_struct.rB = value;
            return true;
        case RegisterNameValuePair::C:
            cpu.bc_struct.rC = value;
            return true;
        case RegisterNameValuePair::D:
            cpu.de_struct.rD = value;
            return true;
        case RegisterNameValuePair::E:
            cpu.de_struct.rE = value;
            return true;
        case RegisterNameValuePair::H:
            cpu.hl_struct.rH = value;
            return true;
        case RegisterNameValuePair::L:
            cpu.hl_struct.rL = value;
            return true;
        case RegisterNameValuePair::A:
            cpu.flag_struct.rA = value;
            return true;
        case RegisterNameValuePair::BC:
            cpu.rBC = value;
            return true;
        case RegisterNameValuePair::DE:
            cpu.rDE = value;
            return true;
        case RegisterNameValuePair::HL:
            cpu.rHL = value;
            return true;
        case RegisterNameValuePair::AF:
            cpu.rAF = value;
            return true;
        case RegisterNameValuePair::PC:
            cpu.rPC = value;
            return true;
        case RegisterNameValuePair::SP:
            cpu.rSP = value;
            return true;
        case RegisterNameValuePair::FZ:
            cpu.flag_struct.rF.Z = value;
            return true;
        case RegisterNameValuePair::FN:
            cpu.flag_struct.rF.N = value;
            return true;
        case RegisterNameValuePair::FH:
            cpu.flag_struct.rF.H = value;
            return true;
        case RegisterNameValuePair::FC:
            cpu.flag_struct.rF.C = value;
            return true;
    }
    return true;
}

// TODO(Brendan): Add the opcodes in the correct place in memory.
bool TestHarness::LoadROM(const vector<InstructionExpectedStatePair>& instructions) {
    int i = 0;
    for (const InstructionExpectedStatePair& instruction : instructions) {
        for (unsigned char byte : instruction.instruction) {
            mem_map->get_pointer()[i] = byte;
            i++;
        }
    }
    return true;
}
} // namespace test_harness