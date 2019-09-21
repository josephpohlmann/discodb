import discodb


c = discodb.DiscoDBConstructor()


with open("src/tests/CJK.txt", "r") as f:
    data = f.read().split("\n")


for line in data:
    if " " in line:
        k,v = line.split(maxsplit=1)
        print(k.encode(), v.encode().decode())
        c.add(k.encode(), v.encode())

o = c.finalize()
with open("cjk.ddb", "wb") as f:
    o.dump(f)
