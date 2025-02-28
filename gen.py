#!/usr/bin/python3

import sys

seen = set()
states = []
# rule layout, (STATE, [CONDITION], [HOOK], NEWSTATE)
rules = []
start = None

infile = sys.argv[1]
header = sys.argv[2]
source = sys.argv[3]

def maybe_add_state(state):
    if state not in seen:
        seen.add(state)
        states.append(state)

with open(infile, 'r') as file:
    current = None

    for line in file.readlines():
        line = line.strip()

        if len(line) == 0:
            continue

        if line.startswith("START"):
            split = line.split()
            start = split[1]
            continue
        if line.startswith("STATE"):
            split = line.split()
            current = split[1]
            maybe_add_state(current)
            continue
        if line.startswith("END"):
            current = None
            continue

        assert current is not None

        split = line.split("->")

        conditions = list(map(str.strip, split[0].split(",")))

        parts = list(map(str.strip, split[1].split(",")))

        to_state = parts[-1]
        maybe_add_state(to_state)

        hook = parts[0]

        rules.append((current, conditions, hook, to_state))

with open(header, 'w') as hfile, open(source, 'w') as sfile:
    def printh(string):
        hfile.write(string + "\n")
    def prints(string):
        sfile.write(string + "\n")

    printh("#ifndef PARSER_H")
    printh("#define PARSER_H")

    printh("")
    printh("#include <stdint.h>")
    printh("")

    printh("enum parser_state {")
    for state in states:
        if state != "ANY":
            printh("\tSTATE_{},".format(state))
    printh("};\n")

    printh("struct parser {")
    printh("\tenum parser_state state;")
    printh("\tvoid *priv;")
    printh("};\n")
    printh("void parser_init(struct parser *ctx, void *priv);")
    printh("void parser_push(struct parser *ctx, uint32_t cp);\n")
    printh("#endif")

    prints("#include \"{}\"".format(header))

    prints("")
    for hook in set([hook for _, _, hook, _ in rules]):
        if hook == "NOP":
            continue
        prints("extern void {}(struct parser *ctx, uint32_t cp);".format(hook))
    prints("")

    prints("void parser_init(struct parser *ctx, void *priv)")
    prints("{")
    prints("\tctx->state = STATE_{};".format(start))
    prints("\tctx->priv = priv;")
    prints("}\n")

    prints("void parser_push(struct parser *ctx, uint32_t cp)")
    prints("{")

    assert start is not None

    for from_state, conditions, hook, to_state in rules:
        if from_state != "ANY":
            prints("\tif (ctx->state == STATE_{}) {{".format(from_state))
        else:
            prints("\tif (1) {")
        prints("\t\tswitch (cp) {")
        for condition in conditions:
            if condition == "DEFAULT":
                prints("\t\tdefault:")
                pass
            else:
                split = condition.split("-")

                a = int(split[0], 16)
                b = a + 1

                if len(split) > 1:
                    b = int(split[1], 16) + 1

                for i in range(a, b):
                    prints("\t\tcase 0x{:02x}:".format(i))

        if hook != "NOP":
            prints("\t\t\t{}(ctx, cp);".format(hook))
        prints("\t\t\tctx->state = STATE_{};".format(to_state))
        prints("\t\t\treturn;")
        prints("\t\t}")
        prints("\t}")

    prints("}")
