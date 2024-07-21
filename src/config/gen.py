import sys
import random 

MAX_ATTEMPTS = 1_000_000

random.seed(0)
f = open('tokens.txt', 'r')
tokens = list(map(lambda x: x.split(' '), filter(lambda x: x != "", f.read().split('\n')[1:-1])))
f.close()

hashtable_len = 1
while hashtable_len < len(tokens):
    hashtable_len <<= 1
hashtable_len <<= 1
bits = hashtable_len - 1


print("Number of items: ", len(tokens))
print("Hash table len: ", hashtable_len)


for _ in range(MAX_ATTEMPTS):
    obtained_hashes = {}
    is_perfect = True
    coef = random.randint(2, bits)
    offset = random.randint(2, bits)
    overflow_limit = (1 << 63) - 1
    for w in tokens:
        hash = offset
        for l in w[0]:
            hash = ((hash * coef) + ord(l)) & bits
        if hash in obtained_hashes:
            is_perfect = False
            break
        else:
            obtained_hashes[hash] = w[2]
    if is_perfect:
        print("Perfect coef:", coef)
        print("Perfect offset:", offset)
        print(obtained_hashes)
        break
else:
    print("No perfect hashes found")
    sys.exit(1)

f = open("map.c", "w")
f.write(
"""#include "c.h"

static usize hashtable_size = %d;
""" % (hashtable_len))

for w in tokens:
    f.write("""static TokenData %s = {
\t.string = "%s",
\t.token = {
\t\t.type = %s,
\t\t.value = %s,
\t}
};
""" % (w[2], w[0], w[1], w[3]))


f.write("static TokenData *hashtable[] = {\n")
for i in range(hashtable_len):
    if i in obtained_hashes:
        f.write(f"\t&{obtained_hashes[i]}")
    else:
        f.write("\tNULL")
    if i != hashtable_len - 1:
        f.write(',')
    f.write('\n')

f.write("""};

static u16 hash(char *s) {
\tu16 h = %d;
\twhile (*s != '\\0') {
\t\th = h * %d + *s;
\t\ts++;
\t}
\treturn h;
}

Token getToken(char *s) {
\tu16 h = hash(s);
\th = h & (hashtable_size - 1);
\tif (hashtable[h] && (strcmp(s, hashtable[h]->string) == 0)) {
\t\treturn hashtable[h]->token;
\t}
\treturn (Token) {
\t\t.type = TOKEN_INVALID,
\t\t.value = (uptr) s
\t};
}
""" % (offset, coef))

f.close()
