/*Program lincom.c will produce a list of combination frequencies from
an input list of frequencies.  That is, given a file of frequencies, it
will search for entries which are sums of other entries.  If A,B, and C
are input frequencies, it will search for combinations such that A+B=C,
2A+B=C, A+2B=C (where the second and third cases are similar but
included becuase the algorithm to avoid repeat entires - A+B and B+A -
demands it). Each test is subject to some equality criteria (conv) and
will look for matches that are plus or minus one alias (alias) away.
The input need not be, but will give a more pleasing ouutput if they
are, in numerically increasing order.  The output is a similar list,
but with combinations for each mode noted. The input frequencies can be
either in Hz or microHz; the output will be microHz.

conv, alias, column number, and skip are set to CONV, ALIAS, NC, ans SKIP
unless specified with command line switches. The switches are:

-a #    alias value
-c #    column number of freq. in input file
-e #    conv value (equality criteria)
-s #    number of header lines to skip in data file
-f name input file name - will be asked for if not provided

Written 22Feb95 by Scot Kleinman.
6Feb96 -added -s switch - sjk
Needs to be linked with ncolread.o (my generic read data routines).*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAXLIM 80
#define MAXSTRING 81
#define FREQS 200        /*Max freqs. allowed in input file*/
#define CONV 0.4         /*default equality criteria = 0.4microHz*/
#define ALIAS 11.57      /*default alias check = 1/day = 11.57microHz*/
#define NC 1             /*default frequencies are read from column 1*/
#define SKIP 0           /*default => no header lines to skip in input file*/

char *gets();
char *strcpy();
double atof();
double fabs();

int nfilread();

int read_data();
void findlc();

double data[FREQS];                 /*the data array of input Freqs*/
double conv;                        /*convergence criteria*/
double alias;                       /*dominant alias spacing in FT*/
int skip;                           /*input file header lines to skip*/

main(argc,argv)
    int argc;
    char *argv[];

    {
    int len;
    char *s;
    int num_pts;                            /*# Freqs in input file*/
    int nc;                                 /*column # of Freq in input file*/
                                            /*By default ALIAS = 1/day*/
    int fflag;                              /*flag if -f used on command line*/
    char dfile[80], ofile[80];              /*name of data & output files*/
    FILE *dfp, *ofp;                        /*ptr to data and output files*/
    
    /*Set up defaults if no command arguments given*/

    nc=NC;
    alias=ALIAS;
    conv=CONV;
    skip=SKIP;
    fflag=0;

    /* Process input options */

    if (argc==1)
        {
        printf("\nYou can use command switches -a, -c, -e, -s and -f to set\n");
        printf("alias value, col.# of freq, equality criteria, number of\n");
        printf("header lines to skip in the input file, and filename\n");
        printf("(defaults are %5.3fmicroHz, %1d, %4.2fmicroHz, and %1d)\n",
                ALIAS,NC,CONV,SKIP);
        printf("respectively.\n");
        }

    while(argc >1 && (*++argv)[0] == '-')
        {
        for(s = argv[0]+1; *s != '\0'; s++)
            {
            switch(*s) 
            {
            case 'a':                       /* use alias other than ALIAS */
                if(*(s+1) != '\0')          /* no space between opt & value */
                    s++;
                else
                    {
                    s = (++argv)[0];        /* space between opt & value */
                    argc--;
                    }
                alias = atof(s);
                s += strlen(s)-1;
                argc--;
                break;
            case 'c':                       /* Column # of freq. in input file*/
                if(*(s+1) != '\0')          /* no space between opt & value */
                    s++;
                else
                    {
                    s = (++argv)[0];        /* space between opt & value */
                    argc--;
                    }
                nc = atoi(s);
                s += strlen(s)-1;
                argc--;
                break;
            case 'e':                       /* use equal. crit other than CONV*/
                if(*(s+1) != '\0')          /* no space between opt & value */
                    s++;
                else
                    {
                    s = (++argv)[0];        /* space between opt & value */
                    argc--;
                    }
                conv = atof(s);
                s += strlen(s)-1;
                argc--;
                break;
            case 's':                       /* # headerlines to skip past*/
                if(*(s+1) != '\0')          /* no space between opt & value */
                    s++;
                else
                    {
                    s = (++argv)[0];        /* space between opt & value */
                    argc--;
                    }
                skip = atoi(s);
                s += strlen(s)-1;
                argc--;
                break;
            case 'f':                       /* set input file name*/
                if(*(s+1) != '\0')          /* no space between opt & value */
                    s++;
                else
                    {
                    s = (++argv)[0];        /* space between opt & value */
                    argc--;
                    }
                strcpy(dfile, s);
                fflag=1;                    /* don't ask for filename later*/
                s += strlen(s)-1;
                argc--;
                break;
            default:
                fprintf(stderr, "unknown arg %c\n", *s);
                exit(1);
            }
            }
        }


    /*Get input and setup output file.*/
    if (fflag==0)
        {    
        printf("\nFile name please: ");
        gets(dfile);
        }
    dfp = fopen(dfile,"r");
    if (dfp == NULL)
        {
        printf("Data file not found\n");
        return;
        }
    strcpy(ofile,dfile);
    len = strlen(dfile);
    ofile[len++] = '.';
    ofile[len++] = 's';
    ofile[len++] = 'd';
    ofile[len] = '\0';
    printf("output file name is %s\n",ofile);
    ofp = fopen(ofile,"w");
    if (ofp == NULL)
        {
        printf("Can't open output file %s. Don't rightly know why not\n",ofile);
        return;
        }
    fprintf(ofp,"Input file=%s\n",dfile);
    if (skip != 0)
        fprintf(ofp,"Skipped %d lines at strt of input file.\n",skip);
    fprintf(ofp,"Equality criteria=%4.2fmicroHz\n",conv);
    fprintf(ofp,"Alias=%5.3fmicroHz\n\n",alias);
    /*Read data and make sure we have not exceeded arrays*/
    num_pts = read_data(dfp,nc);	
    fclose(dfp);
    if (num_pts > FREQS)
        {
        printf("Current max. # Freqs is %d.\nYou have %d.\n",FREQS,num_pts);
        return;
        }
    printf("There are %d points in data file\n",num_pts+1);

    /*Do it*/
    findlc(ofp,num_pts);
    }


int read_data(fpi,col)

    FILE *fpi;                  /*input file pointers*/
    int col;                    /*column number of Freqs in data file*/
    
    {
    int i;
    int collist[2];
    char datarow[2][MAXLIM];
    int nflag;
    int np;
    char s[MAXSTRING+1];

    if (skip !=0)              /*skip past any header lines if specified*/
        for (i=1;i<=skip;i++)  /*on command line*/
            fgets(s,MAXSTRING,fpi);
    i=0;
    collist[0] = col;
    collist[1] = 99;
    do
        {
        nflag = nfilread(fpi,collist,datarow);
        data[i++] = atof(datarow[0]);
        }
    while (nflag != EOF);
    np=i-1;

    /*If freq. in Hz, convert to microHz*/
    if (data[0]<1)
        {
        printf("Converting input frequencies in Hz to microHz.\n");
        for(i=0;i<=np;i++)
            data[i]*=1e6;
        }

    return np;      
    }

void findlc(fpo,np)

/*Algorithm used here is stupid and inefficient, but it works and should be
easily modified.*/

    FILE *fpo;                   /*output file pointer*/
    int np;                      /*Number of pts (from 0)*/

    {
    int i,j,k;
    double sum,sum2,sum3;
    double diff,diff2,diff3;
    
    for (k=0;k<=np;k++)          /*for each mode in list successively*/
        {
        fprintf(fpo,"%8.3f\n",data[k]);
        for (i=0;i<np;i++)       /*start looking for sums that make data[k]*/
            for (j=i;j<=np;j++)
                {
                sum=data[i]+data[j];
                sum2=2*data[i]+data[j];
                sum3=data[i]+2*data[j];
                 
                /*Check for A+B=C+/-Alias*/
                diff = sum-data[k];
                if (fabs(diff)<conv) 
                    fprintf(fpo," %8.3f+%8.3f diff=%5.3f\n",data[i],
                            data[j],diff);
                else if (fabs(diff-alias)<conv)
                    fprintf(fpo," %8.3f+%8.3f-alias diff=%5.3f\n",data[i],
                            data[j],diff-alias);
                else if (fabs(diff+alias)<conv)
                    fprintf(fpo," %8.3f+%8.3f+alias diff=%5.3f\n",data[i],
                            data[j],diff+alias);

                /*Check for 2A+B=C+/-Alias*/
                diff2=sum2-data[k];
                if (fabs(diff2)<conv) 
                    fprintf(fpo," 2*%8.3f+%8.3f diff=%5.3f\n",
                            data[i],data[j],diff2);
                else if (fabs(diff2-alias)<conv)
                    fprintf(fpo," 2*%8.3f+%8.3f-alias diff=%5.3f\n",
                            data[i],data[j],diff2-alias);
                else if (fabs(diff2+alias)<conv)
                    fprintf(fpo," 2*%8.3f+%8.3f+alias diff=%5.3f\n",
                            data[i],data[j],diff2+alias);

                /*Check for A+2B=C+/-Alias*/
                diff3=sum3-data[k];
                if (fabs(diff3)<conv) 
                    fprintf(fpo," %8.3f+2*%8.3f diff=%5.3f\n",
                            data[i],data[j],diff3);
                else if (fabs(diff3-alias)<conv)
                    fprintf(fpo," %8.3f+2*%8.3f-alias diff=%5.3f\n",
                            data[i],data[j],diff3-alias);
                else if (fabs(diff3+alias)<conv)
                    fprintf(fpo," %8.3f+2*%8.3f+alias diff=%5.3f\n",
                            data[i],data[j],diff3+alias);
                }
        }
    }

