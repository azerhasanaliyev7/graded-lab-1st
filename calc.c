// Azar Hasanaliyev 251ADB034
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: I asked Google AI for syntax help a few times, ran the code locally then put in repo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#define STUDENT_FIRST "Azar"
#define STUDENT_LAST  "Hasanaliyev"
#define STUDENT_ID    "251ADB034"

typedef struct {
    const char *text;
    size_t length;
    size_t index;
    size_t line_end_position;
    size_t error_position;
    int failed;
} Parser;

static void skip_spaces(Parser *p)
{
    while (p->index < p->length &&
           (p->text[p->index] == ' ' || p->text[p->index] == '\t' ||
            p->text[p->index] == '\r')) {
        p->index++;
    }
}

static long long parse_expression(Parser *p);

static long long parse_number(Parser *p)
{
    size_t start;
    long long value = 0;

    skip_spaces(p);

    if (p->index >= p->length) {
        p->failed = 1;
        p->error_position = p->line_end_position;
        return 0;
    }

    if (!isdigit((unsigned char)p->text[p->index])) {
        p->failed = 1;
        p->error_position = p->index + 1;
        return 0;
    }

    start = p->index;
    while (p->index < p->length &&
           isdigit((unsigned char)p->text[p->index])) {
        int digit = p->text[p->index] - '0';
        value = value * 10 + digit;
        p->index++;
    }

    (void)start;
    return value;
}
static long long parse_term(Parser *p)
{
    long long value = parse_number(p);

    while (!p->failed) {
        size_t operator_position;
        char op;
        long long right;
        size_t right_start;

        skip_spaces(p);
        if (p->index >= p->length ||
            (p->text[p->index] != '*' && p->text[p->index] != '/')) {
            break;
        }

        operator_position = p->index;
        op = p->text[p->index++];
        skip_spaces(p);
        right_start = p->index;
        right = parse_number(p);
        if (p->failed) {
            return 0;
        }

        if (op == '*') {
            value *= right;
        } else {
            if (right == 0) {
                p->failed = 1;
                p->error_position = right_start < p->length
                    ? right_start + 1 : p->line_end_position;
                return 0;
            }
            value /= right;
        }

        (void)operator_position;
    }

    return value;
}

static long long parse_expression(Parser *p)
{
    long long value = parse_term(p);

    while (!p->failed) {
        char op;
        long long right;

        skip_spaces(p);
        if (p->index >= p->length ||
            (p->text[p->index] != '+' && p->text[p->index] != '-')) {
            break;
        }

        op = p->text[p->index++];
        right = parse_term(p);
        if (p->failed) {
            return 0;
        }

        if (op == '+') {
            value += right;
        } else {
            value -= right;
        }
    }
    return value;
}

    p.line_end_position = line_end_position;
    p.error_position = 0;
    p.failed = 0;

    *result = parse_expression(&p);
    skip_spaces(&p);

    if (!p.failed && p.index < p.length) {
        p.failed = 1;
        p.error_position = p.index + 1;
    }

    if (p.failed) {
        *error_position = absolute_start + p.error_position - 1;
        return -1;
    }

    return 1;
}

static char *read_entire_file(const char *filename, size_t *length)
{
    FILE *file = fopen(filename, "rb");
    char *buffer = NULL;
    size_t used = 0;
    size_t capacity = 4096;

    if (file == NULL) {
        return NULL;
    }

    buffer = malloc(capacity + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    for (;;) {
        size_t amount;

        if (used == capacity) {
            size_t new_capacity = capacity * 2;
            char *new_buffer = realloc(buffer, new_capacity + 1);
            if (new_buffer == NULL) {
                free(buffer);
                fclose(file);
                return NULL;
            }
            buffer = new_buffer;
            capacity = new_capacity;
        }

        amount = fread(buffer + used, 1, capacity - used, file);
        used += amount;

        if (amount == 0) {
            if (ferror(file)) {
                free(buffer);
                fclose(file);
                return NULL;
            }
            break;
        }
    }

    if (fclose(file) != 0) {
        free(buffer);
        return NULL;
    }

    buffer[used] = '\0';
    *length = used;
    return buffer;
}
static int make_directory(const char *path)
{
    struct stat info;

    if (stat(path, &info) == 0) {
        return S_ISDIR(info.st_mode);
    }

    return mkdir(path, 0777) == 0;
}

static void basename_without_txt(const char *path, char *out, size_t out_size)
{
    const char *base = strrchr(path, '/');
    size_t length;

    base = (base == NULL) ? path : base + 1;
    length = strlen(base);

    if (length >= 4 && strcmp(base + length - 4, ".txt") == 0) {
        length -= 4;
    }

    if (length >= out_size) {
        length = out_size - 1;
    }

    memcpy(out, base, length);
    out[length] = '\0';
}

int main(int argc, char **argv)
{
    const char *input_name;
    const char *output_directory = NULL;
    char default_directory[512];
    char output_name[2048];
    char input_base[256];
    char first_lower[128];
    char last_lower[128];
    size_t input_length = 0;
    char *contents;
    FILE *output;
    size_t position = 0;
    size_t output_used = 0;
    long long *results = NULL;
    size_t results_count = 0;
    size_t results_capacity = 0;
    size_t first_error = 0;
    size_t i;
if (argc == 4 && (strcmp(argv[1], "-o") == 0 ||
                      strcmp(argv[1], "--output-dir") == 0)) {
        output_directory = argv[2];
        input_name = argv[3];
    } else if (argc == 2) {
        input_name = argv[1];
    } else {
        fprintf(stderr, "Usage: %s [-o OUTDIR] input.txt\n", argv[0]);
        return 1;
    }

    contents = read_entire_file(input_name, &input_length);
    if (contents == NULL) {
        fprintf(stderr, "Error: cannot read input file '%s'\n", input_name);
        return 1;
    }

    if (output_directory == NULL) {
        basename_without_txt(input_name, input_base, sizeof(input_base));
        (void)snprintf(default_directory, sizeof(default_directory),
                       "%s_%s_%s", input_base, STUDENT_FIRST, STUDENT_ID);
        output_directory = default_directory;
    }

    if (!make_directory(output_directory)) {
        fprintf(stderr, "Error: cannot create output directory '%s'\n",
                output_directory);
        free(contents);
        return 1;
    }

    (void)snprintf(first_lower, sizeof(first_lower), "%s", STUDENT_FIRST);
    (void)snprintf(last_lower, sizeof(last_lower), "%s", STUDENT_LAST);
    for (i = 0; first_lower[i] != '\0'; i++) {
        first_lower[i] = (char)tolower((unsigned char)first_lower[i]);
    }
    for (i = 0; last_lower[i] != '\0'; i++) {
        last_lower[i] = (char)tolower((unsigned char)last_lower[i]);
    }

    basename_without_txt(input_name, input_base, sizeof(input_base));
    (void)snprintf(output_name, sizeof(output_name), "%.900s/%s_%s_%s_%s.txt",
                   output_directory, input_base, first_lower, last_lower,
                   STUDENT_ID);

                    while (position < input_length) {
        size_t line_start = position;
        size_t line_length;
        size_t end_position;
        long long result = 0;
        size_t error_position = 0;
        int status;

        while (position < input_length && contents[position] != '\n') {
            position++;
        }

        line_length = position - line_start;
        end_position = (position < input_length) ? position + 1 : input_length + 1;

        status = evaluate_line(contents + line_start, line_length,
                               line_start + 1, end_position,
                               &result, &error_position);

        if (status == -1) {
            first_error = error_position;
            break;
        }

        if (status == 1) {
            if (results_count == results_capacity) {
                size_t new_capacity = (results_capacity == 0) ? 16 : results_capacity * 2;
                long long *new_results = realloc(results,
                                                 new_capacity * sizeof(*results));
                if (new_results == NULL) {
                    fprintf(stderr, "Error: out of memory\n");
                    free(results);
                    free(contents);
                    return 1;
                }
                results = new_results;
                results_capacity = new_capacity;
            }
            results[results_count++] = result;
        }

        if (position < input_length && contents[position] == '\n') {
            position++;
        }
    }

    output = fopen(output_name, "w");
    if (output == NULL) {
        fprintf(stderr, "Error: cannot write output file '%s'\n", output_name);
        free(results);
        free(contents);
        return 1;
    }
 if (first_error != 0) {
        (void)fprintf(output, "ERROR:%zu\n", first_error);
    } else {
        for (output_used = 0; output_used < results_count; output_used++) {
            (void)fprintf(output, "%lld\n", results[output_used]);
        }
    }

    if (fclose(output) != 0) {
        fprintf(stderr, "Error: failed to finish output file\n");
        free(results);
        free(contents);
        return 1;
    }

    free(results);
    free(contents);
    return 0;
}
