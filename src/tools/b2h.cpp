#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <cstdio>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        return EXIT_FAILURE;
    }

    FILE* input = fopen(argv[1], "rb");
    if (!input) {
        return EXIT_FAILURE;
    }

    FILE* output = fopen(argv[2], "w");
    if (!output) {
        fclose(input);
        return EXIT_FAILURE;
    }

    fseek(input, 0, SEEK_END);
    const long fileSize = ftell(input);
    fseek(input, 0, SEEK_SET);

    const char* variable_name = argv[3];
    int column = 0;

    fprintf(output, "const unsigned char %s[] = {\n", variable_name);

    int index = 0;

    int byte;
    while ((byte = fgetc(input)) != EOF) {
        if (column == 0) {
            fprintf(output, "    ");
        }

        fprintf(output, (column == 15) ? "0x%02X,\n" : "0x%02X, ", unsigned char(byte));

        column = (column + 1) % 16;
        ++index;
    }

    for (int i = 0; i < (index % 4); ++i) {
        fprintf(output, "0x00, ");
    }

    if (column == 0) {
        fprintf(output, "    ");
    }

    fprintf(output, (column == 0) ? "};" : "\n};");

    fclose(output);
    fclose(input);
    return EXIT_SUCCESS;
}
