/* Program lincom2.c
   Updated and fixed version of Scot Kleinman's lincom.c (1995-1996)
   Produces a list of combination frequencies from an input list.

   Usage example:
     ./lincom2 -f maxfile.txt -e 0.2 -a 0 -s 1

   Options:
     -a #    alias value
     -c #    column number of freq. in input file
     -e #    equality criteria (tolerance)
     -s #    number of header lines to skip
     -f name input file name
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXLIM  256
#define FREQS   200
#define CONV    0.4       /* default equality criteria in microHz */
#define ALIAS   11.57     /* default alias = 1/day = 11.57 microHz */
#define NC      1
#define SKIP    0

double data[FREQS];
double conv;
double alias;
int skip;

/* Prototypes */
int read_data(FILE *fpi, int col);
void findlc(FILE *fpo, int np);

int main(int argc, char *argv[]) {
    int len;
    char *s = NULL;
    int num_pts;
    int nc = NC;
    int fflag = 0;
    char dfile[128], ofile[128];
    FILE *dfp, *ofp;

    /* defaults */
    alias = ALIAS;
    conv = CONV;
    skip = SKIP;

    /* user help */
    if (argc == 1) {
        printf("\nYou can use command switches -a, -c, -e, -s and -f to set\n");
        printf("alias value, col.# of freq, equality criteria, number of\n");
        printf("header lines to skip in the input file, and filename\n");
        printf("(defaults are %5.3fmicroHz, %1d, %4.2fmicroHz, and %1d)\n",
               ALIAS, NC, CONV, SKIP);
        printf("respectively.\n");
    }

    /* parse command line arguments */
    argv++;
    argc--;
    while (argc > 0 && argv[0][0] == '-') {
        s = argv[0] + 1;
        switch (*s) {
        case 'a':
            if (argc > 1) {
                alias = atof(argv[1]);
                argv++; argc--;
            }
            break;
        case 'c':
            if (argc > 1) {
                nc = atoi(argv[1]);
                argv++; argc--;
            }
            break;
        case 'e':
            if (argc > 1) {
                conv = atof(argv[1]);
                argv++; argc--;
            }
            break;
        case 's':
            if (argc > 1) {
                skip = atoi(argv[1]);
                argv++; argc--;
            }
            break;
        case 'f':
            if (argc > 1) {
                strncpy(dfile, argv[1], sizeof(dfile) - 1);
                dfile[sizeof(dfile) - 1] = '\0';
                fflag = 1;
                argv++; argc--;
            }
            break;
        default:
            fprintf(stderr, "Unknown argument: %s\n", argv[0]);
            return 1;
        }
        argv++;
        argc--;
    }

    /* get input filename if not given */
    if (!fflag) {
        printf("\nFile name please: ");
        if (!fgets(dfile, sizeof(dfile), stdin)) {
            fprintf(stderr, "Error reading filename.\n");
            return 1;
        }
        dfile[strcspn(dfile, "\n")] = '\0'; /* remove newline */
    }

    dfp = fopen(dfile, "r");
    if (!dfp) {
        fprintf(stderr, "Data file '%s' not found.\n", dfile);
        return 1;
    }

    snprintf(ofile, sizeof(ofile), "%s.sd", dfile);
    printf("Output file name is %s\n", ofile);

    ofp = fopen(ofile, "w");
    if (!ofp) {
        fprintf(stderr, "Can't open output file %s\n", ofile);
        fclose(dfp);
        return 1;
    }

    fprintf(ofp, "Input file=%s\n", dfile);
    if (skip != 0)
        fprintf(ofp, "Skipped %d lines at start of input file.\n", skip);
    fprintf(ofp, "Equality criteria=%4.2fmicroHz\n", conv);
    fprintf(ofp, "Alias=%5.3fmicroHz\n\n", alias);

    /* read data */
    num_pts = read_data(dfp, nc);
    fclose(dfp);
    if (num_pts > FREQS) {
        fprintf(stderr, "Too many frequencies (%d > %d).\n", num_pts, FREQS);
        fclose(ofp);
        return 1;
    }
    printf("There are %d points in data file\n", num_pts);

    /* run search */
    findlc(ofp, num_pts - 1);
    fclose(ofp);

    printf("Done.\n");
    return 0;
}

/* Simplified read_data without nfilread */
int read_data(FILE *fpi, int col) {
    int i = 0;
    double temp;
    char line[MAXLIM];

    /* skip header lines if needed */
    for (int s = 0; s < skip; s++)
        fgets(line, MAXLIM, fpi);

    while (fgets(line, MAXLIM, fpi) && i < FREQS) {
        if (sscanf(line, "%lf", &temp) == 1)
            data[i++] = temp;
    }

    /* Convert to microHz if needed */
    if (i > 0 && data[0] < 1.0) {
        printf("Converting input frequencies from Hz to microHz.\n");
        for (int j = 0; j < i; j++)
            data[j] *= 1e6;
    }

    return i;
}

/* Core search algorithm */
void findlc(FILE *fpo, int np) {
    for (int k = 0; k <= np; k++) {
        fprintf(fpo, "%8.3f\n", data[k]);
        for (int i = 0; i < np; i++)
            for (int j = i; j <= np; j++) {
                double sum = data[i] + data[j];
                double sum2 = 2 * data[i] + data[j];
                double sum3 = data[i] + 2 * data[j];

                double diff = sum - data[k];
                if (fabs(diff) < conv)
                    fprintf(fpo, " %8.3f+%8.3f diff=%5.3f\n", data[i], data[j], diff);
                else if (fabs(diff - alias) < conv)
                    fprintf(fpo, " %8.3f+%8.3f-alias diff=%5.3f\n", data[i], data[j], diff - alias);
                else if (fabs(diff + alias) < conv)
                    fprintf(fpo, " %8.3f+%8.3f+alias diff=%5.3f\n", data[i], data[j], diff + alias);

                double diff2 = sum2 - data[k];
                if (fabs(diff2) < conv)
                    fprintf(fpo, " 2*%8.3f+%8.3f diff=%5.3f\n", data[i], data[j], diff2);
                else if (fabs(diff2 - alias) < conv)
                    fprintf(fpo, " 2*%8.3f+%8.3f-alias diff=%5.3f\n", data[i], data[j], diff2 - alias);
                else if (fabs(diff2 + alias) < conv)
                    fprintf(fpo, " 2*%8.3f+%8.3f+alias diff=%5.3f\n", data[i], data[j], diff2 + alias);

                double diff3 = sum3 - data[k];
                if (fabs(diff3) < conv)
                    fprintf(fpo, " %8.3f+2*%8.3f diff=%5.3f\n", data[i], data[j], diff3);
                else if (fabs(diff3 - alias) < conv)
                    fprintf(fpo, " %8.3f+2*%8.3f-alias diff=%5.3f\n", data[i], data[j], diff3 - alias);
                else if (fabs(diff3 + alias) < conv)
                    fprintf(fpo, " %8.3f+2*%8.3f+alias diff=%5.3f\n", data[i], data[j], diff3 + alias);
            }
    }
}

