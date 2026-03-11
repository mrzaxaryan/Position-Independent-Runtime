//===-- PICTransformOpt.cpp - Standalone PIC transform tool ----------------===//
//
// Reads LLVM bitcode or textual IR, runs PICTransformPass to eliminate
// data sections, and writes the transformed result.
//
// Usage:
//   clang++ -emit-llvm -c -O2 input.cpp -o input.bc
//   pic-transform input.bc -o output.bc
//   clang++ output.bc -o output.exe
//
//===----------------------------------------------------------------------===//

#define PIC_TRANSFORM_STANDALONE

#include "PICTransformPass.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

static cl::opt<std::string>
    InputFilename(cl::Positional, cl::desc("<input bitcode/IR file>"),
                  cl::Required);

static cl::opt<std::string>
    OutputFilename("o", cl::desc("Output filename"),
                   cl::value_desc("filename"), cl::init("-"));

static cl::opt<bool>
    OutputAssembly("S", cl::desc("Write output as LLVM assembly (.ll)"));

int main(int argc, char **argv) {
  InitLLVM X(argc, argv);

  cl::ParseCommandLineOptions(argc, argv,
      "pic-transform - Eliminate data sections from LLVM IR\n\n"
      "Transforms global constants (strings, floats, arrays) into\n"
      "stack-local allocations with immediate-value stores.\n"
      "Produces binaries with only a .text section.\n");

  LLVMContext Context;
  SMDiagnostic Err;
  std::unique_ptr<Module> M = parseIRFile(InputFilename, Err, Context);
  if (!M) {
    Err.print(argv[0], errs());
    return 1;
  }

  ModuleAnalysisManager MAM;
  PICTransformPass Pass;
  Pass.run(*M, MAM);

  std::error_code EC;
  ToolOutputFile Out(OutputFilename, EC, sys::fs::OF_None);
  if (EC) {
    errs() << "Error opening output file: " << EC.message() << "\n";
    return 1;
  }

  if (OutputAssembly)
    M->print(Out.os(), nullptr);
  else
    WriteBitcodeToFile(*M, Out.os());

  Out.keep();
  return 0;
}
