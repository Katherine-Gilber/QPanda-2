#include <map>
#include <cstdlib>
#include <sstream>
#include <string>
#include <complex>
#include <algorithm>
#include <regex>
#include <ctime>
#include "Core/Core.h"
#include "gtest/gtest.h"
#include "Core/QuantumCircuit/QNodeDeepCopy.h"
#include "Core/Utilities/Tools/MatrixDecomposition.h"
#include "Core/Utilities/QProgTransform/QProgToDAG/GraphMatch.h"
#include "Core/Utilities/Tools/QCircuitGenerator.h"
#include "Core/Utilities/QProgInfo/QCircuitInfo.h"
#include "Core/Variational/var.h"
#include "Variational/expression.h"
#include "Variational/utils.h"
#include "Variational/Optimizer.h"
#include "Variational/VarFermionOperator.h"
#include "Variational/VarPauliOperator.h"

#include "Extensions/Extensions.h"
#ifdef USE_EXTENSION

using namespace std;
USING_QPANDA

prob_dict precision_test(bool precison)
{
    int deepth = 9;
    for (int i = 10; i < 11; i++)
    {
        auto qvm = CPUQVM();
        qvm.init(precison);
        auto qubits = qvm.qAllocMany(i);
        auto c = qvm.cAllocMany(i);

        auto circuit = QCircuit();
        auto prog = QProg();
        for (auto &qbit : qubits) {
            circuit << RX(qbit, 25);
        }
        for (auto &qbit : qubits) {
            circuit << RY(qbit, 25);
        }
        for (size_t j = 0; j < i; ++j) {
            circuit << CNOT(qubits[j], qubits[(j + 1) % i]);
        }
        for (size_t k = 0; k < deepth; ++k) {
            for (auto &qbit : qubits) {
                circuit << RZ(qbit, 25).dagger();
            }
            for (auto &qbit : qubits) {
                circuit << RX(qbit, 25).dagger();
            }
            for (auto &qbit : qubits) {
                circuit << RZ(qbit, 25).dagger();
            }
            for (size_t j = 0; j < i; ++j) {
                circuit << CNOT(qubits[j], qubits[(j + 1) % i]);
            }
        }
        for (auto &qbit : qubits) {
            circuit << RZ(qbit, 25);
        }
        for (auto &qbit : qubits) {
            circuit << RX(qbit, 25);
        }
        prog << circuit;
        Fusion fuser;
        fuser.aggregate_operations(prog, &qvm);
        auto res = qvm.probRunDict(prog, qubits);
        return res;
    }

}


static bool test_vf1_0()
{
    //precision test
    auto double_res = precision_test(true);
    auto float_res = precision_test(false);
    double result = 0.0;
    double MSE = 0.0;
    for (auto &r : double_res)
    {
        MSE += (r.second - float_res[r.first])*(r.second - float_res[r.first]);
    }
    MSE = MSE / (1 << 10);
    // MSE Result
    std::cout << "MSE: " << MSE << std::endl;

    // Peak Signal to Noise Ratio Result
    for (auto &r : double_res)
    {
        result += (r.second - float_res[r.first])*(r.second - float_res[r.first]);
    }
    result = result / (1 << 10);
    double PSNR = 10 * log10((1 << 10 - 1)*(1 << 10 - 1) / result);
    std::cout << "result " << ": " << PSNR << setprecision(8) << std::endl;

    return true;
}

static bool test_vf1_1()
{
    // time test
    int deepth = 9;
    for (int i = 5; i < 26; i++)
    {
        auto qvm = CPUQVM();
        qvm.init(false);
        auto qubits = qvm.qAllocMany(i);
        auto c = qvm.cAllocMany(i);

        auto circuit = QCircuit();
        auto prog = QProg();
        for (auto &qbit : qubits) {
            circuit << RX(qbit, rand());
        }
        for (auto &qbit : qubits) {
            circuit << RY(qbit, rand());
        }
        for (size_t j = 0; j < i; ++j) {
            circuit << CNOT(qubits[j], qubits[(j + 1) % i]);
        }
        for (size_t k = 0; k < deepth; ++k) {
            for (auto &qbit : qubits) {
                circuit << RZ(qbit, rand()).dagger();
            }
            for (auto &qbit : qubits) {
                circuit << RX(qbit, rand()).dagger();
            }
            for (auto &qbit : qubits) {
                circuit << RZ(qbit, rand()).dagger();
            }
            for (size_t j = 0; j < i; ++j) {
                circuit << CNOT(qubits[j], qubits[(j + 1) % i]);
            }
        }
        for (auto &qbit : qubits) {
            circuit << RZ(qbit, rand());
        }
        for (auto &qbit : qubits) {
            circuit << RX(qbit, rand());
        }
        prog << circuit;
        Fusion fuser;
        fuser.aggregate_operations(prog, &qvm);

        std::cout << "===============================" << std::endl;
        auto start = std::chrono::system_clock::now();
        qvm.directlyRun(prog);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double>elapsed_seconds = end - start;
        std::cout << "qbit: " << i << ", Time used:  " << elapsed_seconds.count() << std::endl;

    }
    return true;
}

TEST(CPU_Precison, test1)
{
    bool test_val = false;
    try
    {
        test_val = test_vf1_0();
        test_val = test_vf1_1();
    }
    catch (const std::exception& e)
    {
        cout << "Got a exception: " << e.what() << endl;
    }
    catch (...)
    {
        cout << "Got an unknow exception: " << endl;
    }

    //ASSERT_TRUE(test_val);

    //cout << "VF2 test over, press Enter to continue." << endl;
    //getchar();
}

#endif