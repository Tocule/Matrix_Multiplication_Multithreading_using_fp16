#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include<time.h>
#define MAX 3

typedef unsigned short ushort;
typedef unsigned int uint;

uint as_uint(const float x) 
{
    return *(uint*)&x;
}
float as_float(const uint x) {
    return *(float*)&x;
}

ushort float_to_half(const float x) { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
    const uint b = as_uint(x)+0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
    const uint e = (b&0x7F800000)>>23; // exponent
    const uint m = b&0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
    return (b&0x80000000)>>16 | (e>112)*((((e-112)<<10)&0x7C00)|m>>13) | ((e<113)&(e>101))*((((0x007FF000+m)>>(125-e))+1)>>1) | (e>143)*0x7FFF; // sign : normalized : denormalized : saturate
}
float half_to_float(const ushort x) { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
    const uint e = (x&0x7C00)>>10; // exponent
    const uint m = (x&0x03FF)<<13; // mantissa
    const uint v = as_uint((float)m)>>23; // evil log2 bit hack to count leading zeros in denormalized format
    return as_float((x&0x8000)<<16 | (e!=0)*((e+112)<<23|m) | ((e==0)&(m!=0))*((v-37)<<23|((m<<(150-v))&0x007FE000))); // sign : normalized : denormalized
}

void print_bits(const ushort x)
{
    ushort a;
    for(int i=15; i>=0; i--) {
        a=((x>>i)&1);
        printf("%u",a);
        if(i==15||i==10) 
            printf(" ");
        if(i==10)  
           printf("      ");
    }
    printf("\n");
}


float rand_float()
{
    return(((((float)rand())/((float)RAND_MAX))*5));
}

//Each thread computes single element in the resultant matrix
void *mult(void* arg)
{
	float *data = (float *)arg;
	int k = 0, i = 0;

	int x = data[0];
	for (i = 1; i <= x; i++)
		k += data[i]*data[i+x];

	k=k+data[2*x+1];

	float *p = (float*)malloc(sizeof(float));
		*p = k;

//Used to terminate a thread and the return value is passed as a pointer
	pthread_exit(p);
}

//Driver code
int main()
{
    srand(time(NULL));
	float matA[MAX][MAX];
	float matB[MAX][MAX];
	float matC[MAX][MAX];
    
    ushort MatrixA[MAX][MAX];
    ushort MatrixB[MAX][MAX];
    ushort MatrixC[MAX][MAX];

	int r1=MAX,c1=MAX,r2=MAX,c2=MAX,i,j,k;

    printf("Filled MatrixA with random values\n");
	// Generating random values in matA
	for (i = 0; i < r1; i++)
	{
			for (j = 0; j < c1; j++)
			{
				matA[i][j]=rand_float();
                printf("%f ",matA[i][j]);	
			}
			printf("\n");
	}
	 printf("Fill MatrixB with random values\n");
		// Generating random values in matB
	for (i = 0; i < r1; i++)
	{
			for (j = 0; j < c1; j++)
			{
				matB[i][j]=rand_float();
                printf("%f ",matB[i][j]);	
			}
			printf("\n");
	}

	 printf("Filled MatrixC with random values\n");
    for (i = 0; i < r1; i++)
	{
			for (j = 0; j < c1; j++)
			{
				matC[i][j]=rand_float();
                printf("%f ",matC[i][j]);	
			}
			printf("\n");
	}
	// Displaying matA
	 printf("Printed fp16 MatrixA\n");
	for (i = 0; i < r1; i++)
	{
		for(j = 0; j < c1; j++)
		{
		    MatrixA[i][j]=float_to_half(matA[i][j]);
			//printf("%f ",MatrixA[i][j]);
		    print_bits(MatrixA[i][j]);
			matA[i][j]=half_to_float(MatrixA[i][j]);
		}
		printf("\n");
	}

	 printf("Printed fp16 MatrixB\n");
	// Displaying matB
	for (i = 0; i < r2; i++)
	{
		for(j = 0; j < c2; j++)
		{
			MatrixB[i][j]=float_to_half(matB[i][j]);
			//printf("%u ",MatrixB[i][j]);
			 print_bits(MatrixB[i][j]);
			matB[i][j]=half_to_float(MatrixB[i][j]);
		}	
		printf("\n");
	}

	printf("Printed fp16 MatrixC\n");
    for (i = 0; i < r1; i++)
    {
		for(j = 0; j < c1; j++)
		{
			MatrixC[i][j]=float_to_half(matC[i][j]);
			//printf("%u ",MatrixC[i][j]);
			 print_bits(MatrixC[i][j]);
			matC[i][j]=half_to_float(MatrixC[i][j]);
		}	
		printf("\n");
	}


	int max = r1*c2;


	//declaring array of threads of size r1*c2
	pthread_t *threads;
	threads = (pthread_t*)malloc(max*sizeof(pthread_t));

	int count = 0;
	float* data = NULL;
	for (i = 0; i < r1; i++)
		for (j = 0; j < c2; j++)
			{

			//storing row and column elements in data
			data = (float *)malloc((20)*sizeof(float));
			data[0] = c1;
            //printf("OLLLA %f\n",data[0]);
			for (k = 0; k < c1; k++)
				data[k+1] = matA[i][k];

			for (k = 0; k < r2; k++)
				data[k+c1+1] = matB[k][j];

			data[2*c1+1]=matC[i][j];
			printf("OLLA %f",data[2*c1+1]);
			//creating threads
				pthread_create(&threads[count++], NULL,
							mult, (void*)(data));

					}

	printf("RESULTANT MATRIX IS :- \n");
	for (i = 0; i < max; i++)
	{
	void *k;

	//Joining all threads and collecting return value
	pthread_join(threads[i], &k);


		float *p = (float *)k;
	printf("%f ",*p);
	if ((i + 1) % c2 == 0)
		printf("\n");
	}



return 0;
}
