#include <iostream>

#include "Assembler.h"
#include "Emulator.h"
#include <stdlib.h>

int main(int argc, char* argv[]) {

    if(argc == 1 || argc == 2) {
        std::cout << "\n**** Nisu uneti ulazni fajlovi! ****\n" << std::endl;
        return 0;
    }

    int numOfIter = std::atoi(argv[1]);
    int cursor = 2;
    for(int i = 0; i < numOfIter; i++) {
        Assembler* a1 = new Assembler(argv[cursor], std::atoi(argv[cursor + 1]));
        cursor += 2;
        a1->execute();
    }

    const char* arr[MAX_NUM_OF_INPUT_FILES];
    for(int i = 0; i < numOfIter; i++) {
        arr[i] = argv[cursor++];
    }

    Emulator* e = new Emulator(arr, numOfIter);
    e->execute();

    std::cout << "Vucicu seceru!" << std::endl;
}
