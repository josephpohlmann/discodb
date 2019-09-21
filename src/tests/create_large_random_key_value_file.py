from sys import argv
from random import randint
output_name = argv[1]
loops = int(argv[2])

with open(output_name, "w") as f:
    for i in range(loops):
        r1 = str(randint(1, 80000))
        r2 = str(randint(1, 80000))
        f.write(f"{r1} {r2}")
        f.write("\n")

