# LinCom

(.py or .c) Linear Combination

(PYTHON) Use this line of code to find your combinations:

```bash
python lincom.py -f example.txt -a 11.57 -c 1 -e 0.4 -s 0
```

(C) From the attached files, lincom.c is the original 1995 version, lincom3.c is the updated 2025 version

Use this line to generate the executable:

```bash
gcc lincom3.c -o lincom3 -lm -Wall -Wextra -O2
```
* gcc: GNU C Compiler
* -o: output
* -lm: libm (math library)
* -Wall: this activates all basic warnings of the compiler
* -Wextra: this activates all additional warnings
* -O2: the level of optimisation of the code during compilation
_*(-O3 is a more aggressive version of -O2)_

Use this line of code to find your combinations:

```bash
./lincom3 -f example.txt -a 11.57 -c 1 -e 0.4 -s 0
```

In this example (originally maxfile.txt), we have 75 frequencies read, and the output file will be example.txt.sdv (Python) or example.txt.sd (C).
