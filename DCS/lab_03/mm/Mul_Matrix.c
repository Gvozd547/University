#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <strings.h>

#include <time.h>

#include <inttypes.h>


uint64_t get_time (void)
{
    uint32_t low, high;

    __asm__ __volatile__ ( "rdtsc\n" : "=a" (low), "=d" (high) );

    return ((uint64_t)high << 32) | low;
}


int **matrix_init (int x, int y)
{
    int **tmp = NULL;

    tmp = (int **) malloc (sizeof (int *) * x);
    for (int i = 0; i < x; ++i) {
        tmp[i] = (int *) malloc (sizeof (int) * y);
    }


    for (int i = 0; i < x; ++i) {
        for (int j = 0; j < y; ++j) {
            tmp[i][j] = rand () % 5;
        }
    }
    
    return tmp;
}


void matrix_free (int **M, int x, int y)
{
    for (int i = 0; i < x; ++i) {
        free (M[i]);
    }

    free (M);
}


void matrix_print (int **M, int x, int y, FILE *fp)
{
    for (int i = 0; i < x; ++i) {
        for (int j = 0; j < y; ++j) {
            fprintf (fp, " %4d", M[i][j]);
        }
        fprintf (fp, "\n");
    }
}


/*
 * A - rows (main)
 * B - rows (second)
 */
void compute_Rr (int **A, int **B, int **C, int N)
{
    for (int i = 0; i < N; ++i)
        bzero (C[i], N * sizeof (int));
#pragma omp parallel 
{
    //printf ( "Hello, multithreaded world: thread %d of %d\n",
    //         omp_get_thread_num(), omp_get_num_threads()      );
    int i, j, k;
    #pragma omp for private (j, k) //collapse (3)
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            for (k = 0; k < N; ++k) {
                //printf ( "C[%d][%d] += A[%d][%d] * B[%d][%d]\n",
                //         i, k, i, j, j, k                        );      
                C[i][k] += A[i][j] * B[j][k];
            }
        }
    }
}

}


/*
 * A - columns (main)
 * B - columns (second)
 */
void compute_Cc (int **A, int **B, int **C, int N)
{
    for (int i = 0; i < N; ++i)
        bzero (C[i], N * sizeof (int));

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                //printf ( "C[%d][%d] += A[%d][%d] * B[%d][%d]\n",
                //         k, j, k, i, i, j                        );     
                C[k][j] += A[k][i] * B[i][j];
            }
        }
    }
}



/*
 * A - rows (main)
 * B - columns (second)
 */
void compute_Rc (int **A, int **B, int **C, int N)
{
    for (int i = 0; i < N; ++i)
        bzero (C[i], N * sizeof (int));

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                //printf ( "C[%d][%d] += A[%d][%d] * B[%d][%d]\n",
                //         i, j, i, k, k, j                        );     
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

/*
 * A - columns (main)
 * B - rows (second)
 */
void compute_Cr (int **A, int **B, int **C, int N)
{
    for (int i = 0; i < N; ++i)
        bzero (C[i], N * sizeof (int));

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                //printf ( "C[%d][%d] += A[%d][%d] * B[%d][%d]\n",
                //         j, k, j, i, i, k                        );     
                C[j][k] += A[j][i] * B[i][k];
            }
        }
    }
}


/*
 * A - columns (second)
 * B - columns (main)
 */
void compute_cC (int **A, int **B, int **C, int N)
{
    for (int i = 0; i < N; ++i)
        bzero (C[i], N * sizeof (int));

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                //printf ( "C[%d][%d] += A[%d][%d] * B[%d][%d]\n",
                //         k, i, k, j, j, i                        );     
                C[k][i] += A[k][j] * B[j][i];
            }
        }
    }
}



/*
 * A - rows (second)
 * B - columns (main)
 */
void compute_rC (int **A, int **B, int **C, int N)
{
    for (int i = 0; i < N; ++i)
        bzero (C[i], N * sizeof (int));

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                //printf ( "C[%d][%d] += A[%d][%d] * B[%d][%d]\n",
                //         j, i, j, k, k, i                        );     
                C[j][i] += A[j][k] * B[k][i];
            }
        }
    }
}


int XA = 16, YA = 16, XB = 16, YB = 16;

int main (int argc, char *argv[])
{
    int opt, save = 0, compute = 0;
    char *fname = NULL;
    char *fout = NULL;
    char *res = NULL;

    while ((opt = getopt (argc, argv, "sc:n:f:o:r:")) != -1) {
        switch (opt) {
        case 'c':
            compute = atoi (optarg);
            break;
        case 's':
            save = 1;
            break;
        case 'n':
            XA = YA = XB = YB = atoi (optarg);
            break;
        case 'f':
            fname = (char *) malloc (strlen (optarg));
            memcpy (fname, optarg, strlen (optarg));
            break;
        case 'o':
            fout = (char *) malloc (strlen (optarg));
            memcpy (fout, optarg, strlen (optarg));
            break;
        case 'r':
            res = (char *) malloc (strlen (optarg));
            memcpy (res, optarg, strlen (optarg));
            break;
        default:
            printf ("Incorrect option\n");
            exit (1);
        }
    }

    int **A = NULL, **B = NULL, **C = NULL;

    srand (time (NULL));

    A = matrix_init (XA, YA);
    B = matrix_init (XB, YB);

    if (save && fname != NULL) {
        FILE *fp = fopen (fname, "wb");

        for (int i = 0; i < XA; ++i) {
            for (int j = 0; j < YA; ++j) {
                fprintf (fp, "%d ", A[i][j]);
            }
            fprintf (fp, "\n");
        }
        for (int i = 0; i < XB; ++i) {
            for (int j = 0; j < YB; ++j) {
                fprintf (fp, "%d ", B[i][j]);
            }
            fprintf (fp, "\n");
        }

        fclose (fp);
    } else if (fname != NULL) {
        FILE *fp = fopen (fname, "rb");

        for (int i = 0; i < XA; ++i) {
            for (int j = 0; j < YA; ++j) {
                fscanf (fp, "%d", &A[i][j]);
            }
        }
        for (int i = 0; i < XB; ++i) {
            for (int j = 0; j < YB; ++j) {
                fscanf (fp, "%d", &B[i][j]);
            }
        }

        fclose (fp);
    }

    if (compute) {
        C = matrix_init (XA, YB);

        uint64_t start, end;
    //    matrix_print (A, XA, YA);
    //    matrix_print (B, XB, YB);
        switch (compute) {
        case 1:
            compute_Rr (A, B, C, XA);
            start = get_time ();
            for (int i = 0; i < 5; ++i) {
                compute_Rr (A, B, C, XA);
            } 
            end = get_time ();
            break;
        case 2:
            compute_Cc (A, B, C, XA);
            start = get_time ();
            for (int i = 0; i < 5; ++i) {
                compute_Cc (A, B, C, XA);
            } 
            end = get_time ();
            break;
        case 3:
            compute_Rc (A, B, C, XA);
            start = get_time ();
            for (int i = 0; i < 5; ++i) {
                compute_Rc (A, B, C, XA);
            } 
            end = get_time ();
            break;
        case 4:
            compute_Cr (A, B, C, XA);
            start = get_time ();
            for (int i = 0; i < 5; ++i) {
                compute_Cr (A, B, C, XA);
            } 
            end = get_time ();
            break;
        case 5:
            compute_cC (A, B, C, XA);
            start = get_time ();
            for (int i = 0; i < 5; ++i) {
                compute_cC (A, B, C, XA);
            } 
            end = get_time ();
            break;
        case 6:
            compute_rC (A, B, C, XA);
            start = get_time ();
            for (int i = 0; i < 5; ++i) {
                compute_rC (A, B, C, XA);
            } 
            end = get_time ();
            break;
        default:
            compute_Rr (A, B, C, XA);
            start = get_time ();
            for (int i = 0; i < 5; ++i) {
                compute_Rr (A, B, C, XA);
            } 
            end = get_time ();
        }
	    if (fout) {
            FILE *out = fopen (fout, "wb");
            fprintf (out, "SIZE\t\t\tCYCLES\n");
            fprintf (out, "%d\t\t\t%ld\n", XA, (end - start) / 5);
            fclose (out);
    	} else {
            printf ("SIZE\t\t\tCYCLES\n");
            printf ("%d\t\t\t%ld\n", XA, (end - start) / 5);
        }
        if (res) {
            FILE *fres = fopen (res, "wb");
            matrix_print (C, XA, YB, fres);
            fclose (fres);
        }
        matrix_free (C, XA, YB);
    }

    matrix_free (A, XA, YA);
    matrix_free (B, XB, YB);

    if (fname) free (fname);
    if (fout) free (fout);
    if (res) free (res);

    return 0;
}
