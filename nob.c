#define NOB_NO_ECHO
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOB_EXPERIMENTAL_DELETE_OLD
#define PARG_IMPLEMENTATION

#include <string.h>
#include <stdbool.h>
#include "include/builder/nob.h"
#include "include/parg.h"

#define PRINT_ERR(x) fprintf(stderr, "[" "\033[1;31m" "ERROR" "\033[0m" "] " "\033[1;31m" x "\033[0m" "\n")
#define PRINT_WARN(x) fprintf(stderr, "[" "\x1B[33m" "WARNING" "\x1B[0m" "]: " x "\n")
#define COLOR_BLUE "\033[1;34m"
#define COLOR_RESET "\033[0m"
#define PRINT_NOTE(x) fprintf(stderr, "[" "\033[1;34m" "NOTE" "\033[0m" "] " x "\n")
#define PRINT_DEFAULT(x) fprintf(stderr, x "\n")
#define SV_LIT(lit) sv_from_parts(lit, sizeof(lit) - 1)

#define CANIM_HEADERS_PATH "include/"

enum Target {TARGET_DEBUG, TARGET_DEBUG_SAN, TARGET_RELEASE, TARGET_RELEASE_NATIVE};
enum Target parseTargetOptions(int argc, char **argv);
void addIncludeFlags(Cmd *cmd, bool output_cmd);
void addSourceFiles(Cmd *cmd);
void addLinkFlags(Cmd *cmd);
void addTargetFlags(Cmd *cmd, enum Target);
void outputCompileFlags(void);

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Cmd cmd = {0};

    if(!mkdir_if_not_exists("bin") || !mkdir_if_not_exists("bin/include")) {
        PRINT_ERR("Couldn't create an output directory");
        return 0;
    }

    enum Target target = parseTargetOptions(argc, argv);

    cmd_append(&cmd, "gcc");
    addIncludeFlags(&cmd, true);
    addSourceFiles(&cmd);
    addLinkFlags(&cmd);
    addTargetFlags(&cmd, target);
    outputCompileFlags();

    cmd_append(&cmd, "-o", "bin/canim");
    cmd_run(&cmd, 0);

    if(needs_rebuild(CANIM_HEADERS_PATH "canim.h", (const char*[]){"bin/include/canim.h", CANIM_HEADERS_PATH "raylib"}, 2)) {
        copy_file(CANIM_HEADERS_PATH "canim.h", "bin/include/canim.h");
        copy_directory_recursively(CANIM_HEADERS_PATH "raylib", "bin/include/raylib");
    }
}

static String_Builder output_commands;

enum Target parseTargetOptions(int argc, char **argv)
{
    struct parg_state parg = {0};
    const struct parg_option opts[] = {
        {"target", PARG_REQARG, 0, 't'},
        {"help", PARG_NOARG, 0, 'h'}
    };

    int longindex, c;
    parg_init(&parg);

    while((c = parg_getopt_long(&parg, parg_reorder(argc, argv, "t:h", NULL), argv, "t:h", opts, &longindex)) >= 0) {
        if(c == '?')
            c = parg.optopt;
        switch(c) {
            case 1:
                printf("Nonoption: %s\n", parg.optarg);
                goto PARSE_ERROR;

            case 't':
                if(!parg.optarg) {
                    PRINT_ERR("Missing value for -t / --target");
                    goto PRINT_TARGETS_ERROR;
                }

                String_View val = sv_from_cstr(parg.optarg);

                if(sv_eq(val, SV_LIT("debug"))) return PRINT_NOTE("Selected target: " "debug"), TARGET_DEBUG;
                else if(sv_eq(val, SV_LIT("debug-san"))) return PRINT_NOTE("Selected target: " "debug-san"), TARGET_DEBUG_SAN;
                else if(sv_eq(val, SV_LIT("release"))) return PRINT_NOTE("Selected target: " "release"), TARGET_RELEASE;
                else if(sv_eq(val, SV_LIT("release-native"))) return PRINT_NOTE("Selected target: " "release-native"), TARGET_RELEASE_NATIVE;
                PRINT_ERR("Invalid value for -t / --target");
            PRINT_TARGETS_ERROR:
                 PRINT_DEFAULT(
                    COLOR_BLUE "Possible options:" "\n"
                                "  " "debug" COLOR_RESET "          - Compile in debug mode, no optimizations, no sanitizers" "\n"
                    COLOR_BLUE  "  " "debug-san" COLOR_RESET "      - Compile in debug mode, use ASan and UBSan" "\n"
                    COLOR_BLUE  "  " "release" COLOR_RESET "        - Compile in release mode, O2 optimizations, strips symbols" "\n"
                    COLOR_BLUE  "  " "release-native" COLOR_RESET " - Compile in release mode, O3 optimizations, targets native architecture" "\n");
                goto FATAL_ERROR;
            case 'h':
                PRINT_NOTE(
                "Usage: \n"
                "    -h / --help" COLOR_BLUE " - Print helpful options" COLOR_RESET "\n"
                "    -t / --target=" COLOR_BLUE
                        "<" COLOR_RESET
                        "debug" COLOR_BLUE ":" COLOR_RESET
                        "debug-san" COLOR_BLUE ":" COLOR_RESET
                        "release" COLOR_BLUE ":" COLOR_RESET
                        "release-native" COLOR_BLUE
                        "> - Choose the target" COLOR_RESET);
                goto FATAL_ERROR;
            default:
                printf("What the fuck: %c : %c\n", c, parg.optopt);
                goto PARSE_ERROR;
        }
    }

PARSE_ERROR:
    PRINT_WARN("No -t argument specified, defaulting to debug.");
    PRINT_NOTE(COLOR_BLUE "See -h/--help for usage" COLOR_RESET);
	return false;
FATAL_ERROR:
    exit(0);
}

void parseDirectory(const char* dir_name, Nob_File_Paths ret[static 1])
{
    Nob_File_Paths paths = {0};
    nob_read_entire_dir(dir_name, &paths);
    for(int i = 0; i < paths.count; ++i)
        if(!strcmp(paths.items[i], ".") || !strcmp(paths.items[i], ".."))
            continue;
        else {
            String_Builder sb = {0};
            sb_append_cstr(&sb, dir_name);
            sb_append_buf(&sb, "/", 1);
            sb_append_cstr(&sb, paths.items[i]);
            sb_append_null(&sb);
            da_append(ret, sb_to_sv(sb).data);
        }

    da_free(paths);
}

void addIncludeFlags(Nob_Cmd *cmd, bool output_cmd)
{
    const char* include_paths[] = { "-I" "include" };
    for(int i = 0; i < NOB_ARRAY_LEN(include_paths); ++i) {
        cmd_append(cmd, include_paths[i]);

        if(output_cmd) {
            sb_append_cstr(&output_commands, include_paths[i]);
            sb_append_buf(&output_commands, "\n", 1);
        }
    }
}

void addSourceFiles(Cmd *cmd)
{
    Nob_File_Paths paths = {0};
    parseDirectory("src", &paths);
    parseDirectory("src/core", &paths);
    parseDirectory("src/extra", &paths);

    for(int i = 0; i < paths.count; ++i) {
        if(!sv_end_with(sv_from_cstr(paths.items[i]), ".c"))
            continue;

        cmd_append(cmd, paths.items[i]);
    }

    da_free(paths);
}

void addLinkFlags(Cmd *cmd)
{
    cmd_append(cmd, "lib/libraylib.a", "-lm", "-ldl", "-rdynamic", "-l:libX11.so.6");
}

void addTargetFlags(Cmd *cmd, enum Target target)
{
    switch (target) {
        case TARGET_DEBUG_SAN: cmd_append(cmd, "-fsanitize=address", "-fsanitize=undefined");
        case TARGET_DEBUG: cmd_append(cmd, "-g", "-O0"); break;
        case TARGET_RELEASE: cmd_append(cmd, "-s", "-O2"); break;
        case TARGET_RELEASE_NATIVE: cmd_append(cmd, "-s", "-O3", "-march=native"); break;
    }
}

void outputCompileFlags()
{
    if(file_exists("compile_flags.txt"))
        return;

    write_entire_file("compile_flags.txt", output_commands.items, output_commands.count);
    da_free(output_commands);
}
