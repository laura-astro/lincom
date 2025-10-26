# LinCom

(.c or .py) Linear Combination

(PYTHON) Use this line of code to find your combinations:

```bash
python lincom.py -f example.txt -a 11.57 -c 1 -e 0.4 -s 0
```

(C) Use this line to generate the executable:

```bash
gcc lincom.c -o lincom -lm -Wall -Wextra -O2
```
* gcc: GNU C Compiler
* -o: output
* -lm: libm (math library)
* -Wall: this activates all basic warnings of the compiler
* -Wextra: this activated all addiotional warnings
* -O2: the level of optimisation of the code during compilation
_* (-O3 is a more aggressive version of -O2)_

Use this line of code to find your combinations:

```bash
./lincom -f example.txt -a 11.57 -c 1 -e 0.4 -s 0
```

In this example (originally maxfile.txt), we have 75 frequencies read, and the output file will be example.txt.sdv (Python) or example.txt.sd (C).
