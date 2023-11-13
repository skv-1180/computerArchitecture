#include<bits/stdc++.h>
using namespace std;

struct CacheBlock {
    int tag;
    bool valid;
    int lastUsed;
};

class CacheSimulator {
public:
    CacheSimulator(int cacheSize, int blockSize, int associativity) : cacheSize(cacheSize), blockSize(blockSize), associativity(associativity), time(0) {
        int numBlocks = cacheSize / blockSize;
        int numSets = numBlocks / associativity;

        cache.resize(numSets, vector<CacheBlock>(associativity, {-1, false, 0}));
    }

    bool access(int address) {
        int offset = address % blockSize;
        int numSets = cache.size(); 
        int index = (address / blockSize) % numSets;
        int tag = address / (numSets * blockSize); 

        for (int i = 0; i < associativity; ++i) {
            if (cache[index][i].valid && cache[index][i].tag == tag) {
                updateUsage(index, i);
                return true;
            }
        }

        replaceBlock(index, tag);
        return false;
    }

    void updateUsage(int index, int i) {
        cache[index][i].lastUsed = ++time; 
        cout << "Cache Hit!" << endl;
    }

    void replaceBlock(int index, int tag) {
        int way = findLeastRecentlyUsed(index);
        cache[index][way] = {tag, true, ++time};  
        cout << "Cache Miss!" << endl;
    }

private:
    int cacheSize;
    int blockSize;
    int associativity;
    vector<vector<CacheBlock>> cache; 
    int time = 0; 

    int findLeastRecentlyUsed(int index) {
        int lruWay = 0;
        for (int i = 1; i < associativity; ++i) {
            if (cache[index][i].lastUsed < cache[index][lruWay].lastUsed) {
                lruWay = i;
            }
        }
        return lruWay;
    }
};

class CoreSimulator {
public:
    CoreSimulator(CacheSimulator& cacheSim, int coreId) : cacheSim(cacheSim), coreId(coreId) {}

    void executeInstruction(int address, bool isWrite) {
        cout << "Core " << coreId << " executing instruction at address: " << address << endl;

        if (isWrite) {
            cacheSim.access(address);  
            cout << "Write access successful!" << endl;
        } else {
            if (cacheSim.access(address)) {
                cout << "Read access successful!" << endl;
            } else {
                cout << "Read access failed!" << endl;
            }
        }

    }

private:
    CacheSimulator& cacheSim;
    int coreId;
};

class IntegratedSystem {
public:
    IntegratedSystem(int cacheSize, int blockSize, int associativity, int numCores) : cacheSim(cacheSize, blockSize, associativity) {
        for (int i = 0; i < numCores; ++i) {
            coreSimulators.emplace_back(cacheSim, i);
        }
    }

    void simulateExecution(int numInstructions) {
        random_device rd;
        mt19937 eng(rd());
        uniform_int_distribution<> instrAddrDistr(0, 2047 * 64 - 1);

        for (int i = 0; i < numInstructions; ++i) {
            int instrAddress = instrAddrDistr(eng);

            bool isWrite = i % 2 == 0;

            for (auto& coreSimulator : coreSimulators) {
                coreSimulator.executeInstruction(instrAddress, isWrite);
            }

            cout << endl;
        }
    }

private:
    CacheSimulator cacheSim;
    vector<CoreSimulator> coreSimulators;
};

int main() {

    IntegratedSystem integratedSystem(2048, 64, 2, 2); 
    integratedSystem.simulateExecution(10);

    return 0;
}
