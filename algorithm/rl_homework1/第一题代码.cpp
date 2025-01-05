#include<stdio.h>
#include<cstring>
#include<iostream>

#define numStates 25
#define gridLen 5
#define gamma  0.9

int state[gridLen][gridLen]={
    {0,1,2,3,4},
    {5,6,7,8,9},
    {10,11,12,13,14},
    {15,16,17,18,19},
    {20,21,22,23,24},
};

int dr[gridLen]={-1,0,1,0,0};
int dc[gridLen]={0,1,0,-1,0};


class Matrix
{
public:
    double **a;
    int m,n;
    Matrix(int m,int n)
    {
        this->m=m;
        this->n=n;
        a = new double*[m];
        for (int i = 0; i < m; ++i) 
            a[i] = new double[n];
    }
    ~Matrix()
    {
        for (int i = 0; i < m; ++i) 
            delete[] a[i];
        delete[] a;
    }
    void eye()//赋值为单位矩阵
    {
        for(int i=0;i<m;i++)
        {
            memset(a[i],0,sizeof(double)*n);
            a[i][i]=1;
        }
    }
    void kMultiply(double k)
    {
        for(int i=0;i<m;i++)
        {
            for(int j=0;j<n;j++)
            {
                a[i][j]*=k;
            }
        }
    }
    void zero()
    {
        for(int i=0;i<m;i++)
        {
            memset(a[i],0,sizeof(double)*n);
        }
    }
    void copy(Matrix *B)
    {
        for(int i=0;i<m;i++)
        {
            memcpy(a[i],B->a[i],sizeof(double)*n);
        }
    }
};

struct example
{
    Matrix *R;//R[s][0]表示s状态的即时收益，只有一列
    Matrix *P;//P[s1][s2]表示由s1转移到s2状态的概率
};

void matrixMul(Matrix *A,Matrix *B,Matrix *C)//计算矩阵乘法A*B 并将结果放在C中
{
    for (int i = 0; i < C->m; ++i) {
        for (int j = 0; j < C->n; ++j) {
            C->a[i][j] = 0;
            for (int k = 0; k < A->n; ++k) {
                C->a[i][j] += A->a[i][k] * B->a[k][j];
            }
        }
    }
}

void matrixSub(Matrix *A,Matrix *B,Matrix *C)
{
    for(int i=0;i<A->m;i++)
    {
        for(int j=0;j<B->n;j++)
        {
            C->a[i][j]=A->a[i][j]-B->a[i][j];
        }
    }
}

void matrixAdd(Matrix *A,Matrix *B,Matrix *C)
{
    for(int i=0;i<A->m;i++)
    {
        for(int j=0;j<B->n;j++)
        {
            C->a[i][j]=A->a[i][j]+B->a[i][j];
        }
    }
}

void matrixInverse(Matrix* matrix,Matrix* inverse) {
    int size = matrix->m;
    Matrix* augmented = new Matrix(size, 2 * size);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            augmented->a[i][j] = matrix->a[i][j];
        }
        augmented->a[i][i + size] = 1;
    }
    for (int i = 0; i < size; i++) {
        double pivot = augmented->a[i][i];
        for (int j = 0; j < 2 * size; j++) {
            augmented->a[i][j] /= pivot;
        }
        for (int k = 0; k < size; k++) {
            if (k!= i) {
                double factor = augmented->a[k][i];
                for (int j = 0; j < 2 * size; j++) {
                    augmented->a[k][j] -= factor * augmented->a[i][j];
                }
            }
        }
    }
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            inverse->a[i][j] = augmented->a[i][j + size];
        }
    }
    delete augmented;
    return;
}

void fillExample(int i,int j,double re[gridLen][gridLen],int ac[gridLen][gridLen],example *e)//借助reward矩阵与action矩阵得到当前状态s1的即时收益Rpi(s)，与从s1转移到s2的概率矩阵
{
    int nextr=i+dr[ac[i][j]],nextc=j+dc[ac[i][j]];
    
    int s1=state[i][j];

    memset(e->P->a[s1],0,numStates*sizeof(double));//全部赋值为零
    if(nextr<0||nextr>=gridLen||nextc<0||nextc>=gridLen){
        e->P->a[s1][s1]=1;//超出边界只能返回原地
        e->R->a[s1][0]=-1;//超出边界，收益为-1
        return;
    }
    int s2=state[nextr][nextc];
    e->P->a[s1][s2]=1;
    e->R->a[s1][0]=re[nextr][nextc];
    return;
}
// void getPpi(int i,int j)
void getExampleInfo(example *e,double re[gridLen][gridLen],int ac[gridLen][gridLen])//reward矩阵与action矩阵
{
    for(int i=0;i<gridLen;i++)
    {
        for(int j=0;j<gridLen;j++)
        {
            fillExample(i,j,re,ac,e);
        }
    }
}

void show(Matrix *A)
{
    for(int i=0;i<gridLen;i++)
    {
        for(int j=0;j<gridLen;j++)
        {
            printf("%10.4f ",A->a[state[i][j]][0]);
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

void show2(Matrix *A)
{
    for(int i=0;i<numStates;i++)
    {
        for(int j=0;j<numStates;j++)
        {
            std::cout<<A->a[i][j]<<' ';
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

void closedFormSolution(double reward[][gridLen],int action[][gridLen])
{
    example *e=new example;
    e->P=new Matrix(numStates,numStates);
    e->R=new Matrix(numStates,1);

    getExampleInfo(e,reward,action);

    Matrix V(numStates,1);
    Matrix inverse(numStates,numStates);
    Matrix I(numStates,numStates);//对角矩阵
    I.eye();
    Matrix temp(numStates,numStates);

    e->P->kMultiply(gamma);
    matrixSub(&I,e->P,&temp);
    
    // show2(&temp);
    matrixInverse(&temp,&inverse);

    matrixMul(&inverse,e->R,&V);

    show(&V);
    // show(e->R);
    // show2(e->P);

    delete e->P;
    delete e->R;
    delete e;
    return;
}

double getDiff(Matrix *V,Matrix *nextV)
{
    double maxDiff=0;
    for(int i=0;i<numStates;i++)
    {
        double diff=V->a[i][0]-nextV->a[i][0];
        double absDiff=diff>0?diff:-diff;
        maxDiff=maxDiff>absDiff?maxDiff:absDiff;
    }
    // printf("%f\n",maxDiff);
    return maxDiff;
}

void iterativeSolution(double reward[][gridLen],int action[][gridLen],int maxIterNum=100)
{
    example *e=new example;
    e->P=new Matrix(numStates,numStates);
    e->R=new Matrix(numStates,1);

    getExampleInfo(e,reward,action);

    Matrix V(numStates,1);
    Matrix nextV(numStates,1);//下一次迭代的V
    V.zero();
    Matrix temp(numStates,1);
    int i;
    for(i=0;i<maxIterNum;i++)
    {
        // V.kMultiply(gamma);
        matrixMul(e->P,&V,&temp);
        temp.kMultiply(gamma);
        matrixAdd(&temp,e->R,&nextV);
        if(getDiff(&V,&nextV)<0.005)
            break;
        V.copy(&nextV);
    }

    printf("iter num:%d\n",i);
    show(&V);
    delete e->P;
    delete e->R;
    delete e;
    return;
}


int main()
{
    double reward1[gridLen][gridLen]={
        {0,0,0,0,0},
        {0,-1,-1,0,0},
        {0,0,-1,0,0},
        {0,-1,1,-1,0},
        {0,-1,0,0,0},
    };
    int action1[gridLen][gridLen]={
        {1,1,1,2,2},
        {0,0,1,2,2},
        {0,3,2,1,2},
        {0,1,4,3,2},
        {0,1,0,3,3},
    };//0向上，1向右,2向下，3向左   4原地不动

    double reward2[gridLen][gridLen]={
        {0,0,0,0,0},
        {0,-1,-1,0,0},
        {0,0,-1,0,0},
        {0,-1,1,-1,0},
        {0,-1,0,0,0},
    };
    int action2[gridLen][gridLen]={
        {1,1,1,1,2},
        {0,0,1,1,2},
        {0,3,2,1,2},
        {0,1,4,3,2},
        {0,1,0,3,3},
    };//0向上，1向右,2向下，3向左   4原地不动

    double reward3[gridLen][gridLen]={
        {0,0,0,0,0},
        {0,-1,-1,0,0},
        {0,0,-1,0,0},
        {0,-1,1,-1,0},
        {0,-1,0,0,0},
    };
    int action3[gridLen][gridLen]={
        {1,1,1,1,1},
        {1,1,1,1,1},
        {1,1,1,1,1},
        {1,1,1,1,1},
        {1,1,1,1,1},
    };

    double reward4[gridLen][gridLen]={
        {0,0,0,0,0},
        {0,-1,-1,0,0},
        {0,0,-1,0,0},
        {0,-1,1,-1,0},
        {0,-1,0,0,0},
    };
    int action4[gridLen][gridLen]={
        {1,3,3,0,0},
        {2,4,1,2,1},
        {3,1,2,3,4},
        {4,2,0,0,1},
        {4,1,4,1,4},
    };//0向上，1向右,2向下，3向左  4原地不动
    printf("closed form solution result:\n");
    closedFormSolution(reward4,action4);
    printf("iterative solution result:\n");
    iterativeSolution(reward4,action4);
    return 0;
}