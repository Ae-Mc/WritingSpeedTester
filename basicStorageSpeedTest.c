#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

void enterSizesFromStdin(
        unsigned long long *filesize, unsigned long long *blocksize);
unsigned long long getSizeValue(
    char inputSuggestion[], unsigned long long defaultValue);
unsigned long long *getULLValue(const char inputSuggestion[]);
unsigned long long *getULLValueFromString(const char *str);
void CheckSizesMultiple(
        unsigned long long filesize, unsigned long long blocksize);
void enterSizesFromArgs(
        char **argv, unsigned long long *filesize, unsigned long long *blocksize);
char *trim(char *str);
char *GenerateBlockOfData(unsigned long long blocksize);
char *GetOutFileName(unsigned long long filesize);
FILE *createOutFile(unsigned long long filesize);
clock_t testWriteSpeed(unsigned long long filesize, unsigned long long blocksize);
static PyObject *TestWriteSpeed_Test(PyObject *self, PyObject *args);


static PyMethodDef TestWriteSpeedMethods[] = {
    {"Test", TestWriteSpeed_Test, METH_VARARGS,
        "Test disk writing speed.\n"
        "Args: filesize, blocksize.\n"
        "Reurns: writing time"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef TestWriteSpeedModule = {
    PyModuleDef_HEAD_INIT,
    "TestWriteSpeed",
        "Module for testing disk writing speed.",
    -1,
    TestWriteSpeedMethods
};

PyMODINIT_FUNC *PyInit_TestWriteSpeed(void) {
    return PyModule_Create(&TestWriteSpeedModule);
}

int main(int argc, char **argv) {
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }
       
    PyImport_AppendInittab("TestWriteSpeed", PyInit_TestWriteSpeed);
    Py_SetProgramName(program);
    Py_Initialize();
    PyImport_ImportModule("TestWriteSpeed");

    unsigned long long blocksize;
    unsigned long long filesize;
    if(argc != 3) {
        enterSizesFromStdin(&filesize, &blocksize);
    } else {
        enterSizesFromArgs(argv, &filesize, &blocksize);
    }
    clock_t took = testWriteSpeed(filesize, blocksize);
    printf("Took: %lf s\n", (double) took / CLOCKS_PER_SEC);
    PyMem_RawFree(program);
    return 0;
}

void enterSizesFromStdin(
        unsigned long long *filesize, unsigned long long *blocksize) {
    *filesize = 1024 * getSizeValue(
            "Enter filesize in KB (mutiple of blocksize, "
            "default: 4 GB): ", 4194304);

    *blocksize = 1024 * getSizeValue(
            "Enter blocksize in KB (default: 4096): ", 1024 * 4);
    CheckSizesMultiple(*filesize, *blocksize);
}

void CheckSizesMultiple(
        unsigned long long filesize, unsigned long long blocksize) {
    if(blocksize == 0) {
        fprintf(stderr, "%s\n", "Blocksize couldn't be 0!");
        exit(-1);
    }
    if (filesize % blocksize != 0) {
        fprintf(stderr, "%s\n", "Filesize is not a mutiple of blocksize!");
        exit(-2);
    }
}

unsigned long long getSizeValue(
    char inputSuggestion[], unsigned long long defaultValue) {
    unsigned long long *temp, result = defaultValue;

    if((temp = getULLValue(inputSuggestion)) != NULL) {
        result = *temp;
        free(temp);
    }
    return result;
}

unsigned long long *getULLValue(const char inputSuggestion[]) {
    unsigned long long *value;
    printf("%s", inputSuggestion);
    char *inputStr = NULL;
    size_t allocated;
    getline(&inputStr, &allocated, stdin);
    value = getULLValueFromString(inputStr);
    free(inputStr);
    return value;
}

unsigned long long *getULLValueFromString(const char *str) {
    unsigned long long *value = malloc(sizeof(unsigned long long));
    int scanned;
    char *strClone = malloc(strlen(str) + 1);
    strcpy(strClone, str);
    scanned = sscanf(strClone, "%llu", value);
    if(!scanned || scanned == EOF) {
        free(value);
        trim(strClone);
        if(strClone[0] == 0)
            value = NULL;
        else {
            free(strClone);
            fprintf(stderr, "Not a number exception!\n");
            exit(1);
        }
    }
    free(strClone);
    return value;
}

char *trim(char *str) {
    char trimmable[] = " \r\n\t";
    int trimmableLen = strlen(trimmable);
    char *start = str + strspn(str, trimmable);
    char *end = str + strlen(str) - 1;
    for(;end != start;end--) {
        int i = 0;
        for(; i < trimmableLen; i++)
            if (*end == trimmable[i])
                break;
        if(i == trimmableLen)
            break;
    }
    end[1] = 0;
    for(int i = 0; i < (end - start); i++)
        str[i] = start[i];
    str[end - start] = 0;
    return str;
}

void enterSizesFromArgs(char **argv, unsigned long long *filesize, unsigned long long *blocksize) {
    unsigned long long *temp;
    if((temp = getULLValueFromString(argv[1])) == NULL) {
            fprintf(stderr, "Not a number exception!\n");
            exit(1);
    }
    *filesize = *temp;
    free(temp);
    if((temp = getULLValueFromString(argv[2])) == NULL) {
            fprintf(stderr, "Not a number exception!\n");
            exit(1);
    }
    *blocksize = *temp;
    free(temp);
    CheckSizesMultiple(*filesize, *blocksize);
}

clock_t testWriteSpeed(unsigned long long filesize, unsigned long long blocksize) {
    char *filename = GetOutFileName(filesize);
    FILE *out = fopen(filename, "wb");
    unsigned long long blockCount = filesize / blocksize;
    char *block = GenerateBlockOfData(blocksize);
    clock_t before = clock();
    for(unsigned long long i = 0; i < blockCount; i++)
        fwrite(block, blocksize, 1, out);
    fclose(out);
    clock_t took = clock() - before;
    remove(filename);
    free(block);
    free(filename);
    return took;
}

char *GetOutFileName(unsigned long long filesize) {
    char *filename = malloc(1024);
    sprintf(filename, "%llua.txt", filesize);
    return filename;
}

char *GenerateBlockOfData(unsigned long long blocksize) {
    char *block = malloc(blocksize);
    for(unsigned long long i = 0; i < blocksize; i++)
        block[i] = 'a';
    return block;
}

static PyObject *TestWriteSpeed_Test(PyObject *self, PyObject *args) {
    unsigned long long blocksize;
    unsigned long long filesize;
    if(!PyArg_ParseTuple(args, "KK", &filesize, &blocksize)) {
        return NULL;
    }
    return PyFloat_FromDouble(
            (double) testWriteSpeed(filesize, blocksize) / CLOCKS_PER_SEC);
}
