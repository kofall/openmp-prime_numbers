#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <omp.h>

int M, N, threads, sieveSize, primeRangeToSearch, primeSize = 0;

void setSieve(int *sieve) {
    int j = 0;
    for(int i = M; i <= N;) {
        if(i == 2) {
            sieve[j++] = i++;
            continue;
        }
        if(i % 2 == 0)
            i++;
        sieve[j++] = i;
        sieveSize--;
        i += 2;
    }
}

void getPrimes(int *primes) {
    bool select[primeRangeToSearch];
    memset(select, true, primeRangeToSearch);
    for(int p = 2; p * p <= sqrt(N); p++) {
        if(select[p]) {
            for(int i = p * p; i <= sqrt(N); i += p) {
                select[i] = false;
            }
        }
    }
    
    for(int p = 2; p < primeRangeToSearch; p++) {
        if(select[p]) {
            primes[primeSize++] = p;
        }
    }
}

int findFirstOccurrenceOfMultiple(int *primes, int i) {
    int v = primes[i];
    if(v < M) {
        v = M - M % primes[i];
        while(v % 2 == 0 || v < M)
            v += primes[i];
    }
    int id = ceil((v - M) / 2.0f);
    if(M != 2 && M % 2 == 0)
        id -= 1;
    return id;
}

void siftSieve(int *sieve, int *primes) {
    #pragma omp parallel for schedule(dynamic)
    for(int i = 1; i < primeSize; i++) {
        int start = findFirstOccurrenceOfMultiple(primes, i);
        for(int j = start; j <= sieveSize; j += primes[i]) {
            if(sieve[j] != primes[i])
                sieve[j] = 0;
        }
    }
}

int main(int argc, char* argv[]){
    threads = omp_get_max_threads();
    switch(argc) {
        case 3:
            sscanf(argv[1], "%d", &threads);
            M = 2;
            sscanf(argv[2], "%d", &N);
            break;
        case 4:
            sscanf(argv[1], "%d", &threads);
            sscanf(argv[2], "%d", &M);
            sscanf(argv[3], "%d", &N);
            break;
        default:
            printf("Wrong parameters. Closing program...\n");
            return 0;
    }
    if(threads < 1 || M < 2 || N < 2 || M > N) {
        printf("Wrong parameters. Closing program...\n");
        return 0;
    }

    omp_set_dynamic(0);
    omp_set_num_threads(threads);

    printf("INITIALISATION\n");
    printf("\tNumber of processors available = %d\n", omp_get_num_procs());
    printf("\tNumber of threads =              %d\n\n", omp_get_max_threads());

    sieveSize = N - M + 1;
    primeRangeToSearch = sqrt(N) + 1;
    primeSize = 0;
                
    double wtime;
    wtime = omp_get_wtime();

    int* sieve = (int*)malloc(sieveSize*sizeof(int));
    setSieve(sieve);
    int* primes = (int*)malloc(primeRangeToSearch*sizeof(int));
    getPrimes(primes);
    siftSieve(sieve, primes);

    int counter = 0;
    #pragma omp parallel for reduction(+:counter)
    for(int i = 0; i < sieveSize; i++)
        if(sieve[i] > 0) {
            counter++;
            // printf("%d\n", sieve[i]);
        }

    wtime = omp_get_wtime() - wtime;
    free(sieve);
    free(primes);
    printf("RANGE: <%d; %d>\nFOUND: %d\nTIME: %f\n", M, N, counter, wtime);
}