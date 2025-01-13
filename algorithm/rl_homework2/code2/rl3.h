
#include<stdio.h>
#include<cstring>
#include<iostream>
#include<vector>
#include<cstdlib>
#include<ctime>
// 情况1
// #define gridR 1//格子的行数
// #define gridC 3//格子的列数
// #define numStates 3//gridR*gridC

// 情况2
#define gridR 5//格子的行数
#define gridC 5//格子的列数
#define numStates 25//gridR*gridC


#define gamma  0.9
#define actionNum 5
#define iterThreshold  0.00001
#define alpha 0.8


#define rBoundry -1.0
#define rForbidden -10.0
#define rTarget 1.0
#define rOtherstep 0.0

// 情况1
// int state[gridR][gridC]={
//     {0,1,2},
// };

// 情况2
int state[gridR][gridC]={
    {0,1,2,3,4},
    {5,6,7,8,9},
    {10,11,12,13,14},
    {15,16,17,18,19},
    {20,21,22,23,24},
};

int dr[actionNum]={-1,0,1,0,0};
int dc[actionNum]={0,1,0,-1,0};

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
    void one()
    {
        for(int i=0;i<m;i++)
        {
            for(int j=0;j<n;j++)
            {
                a[i][j]=1.0;
            }
        }
    }
    void randomInit()
    {
        for(int i=0;i<m;i++)
        {
            for(int j=0;j<n;j++)
            {
                a[i][j]=static_cast<double>(rand()) / RAND_MAX;
            }
        }
    }
    void uniformInit()
    {
        for(int i=0;i<m;i++)
        {
            for(int j=0;j<n;j++)
            {
                a[i][j]=1.0/actionNum;
            }
        }
    }
    void show()
    {
        for(int i=0;i<m;i++)
        {
            for(int j=0;j<n;j++)
            {
                std::cout<<a[i][j]<<' ';
            }
            std::cout<<std::endl;
        }
    }
};

struct example
{
    Matrix *R;//R[s][0]表示s状态的即时收益，只有一列
    Matrix *P;//P[s1][s2]表示由s1转移到s2状态的概率
};

// class mySaPair
// {
// public:
//     int s,a;
//     mySaPair(int s,int a)
//     {
//         this->s=s;
//         this->a=a;
//     }
//     mySaPair(const mySaPair &b){
//         this->s=b.s;
//         this->a=b.a;
//     }
// };

struct saPair
{
    int s,a;
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

void showRoad(std::vector<saPair>* road)
{
    for(int i=0;i<30&&i<road->size();i++)
    {
        printf("s:%d a:%d \n",(*road)[i].s,(*road)[i].a);
        // printf("s:%d a:%d \n",road.at(i).s,road.at(i).a);
    }
}

void showMaxPolicyInGrid(Matrix *A)
{

    double maxqVal=-9999,maxqIndex;
    for(int i=0;i<gridR;i++)
    {
        for(int j=0;j<gridC;j++)
        {
            int s=state[i][j];
            maxqVal=-9999,maxqIndex;
            for(int k=0;k<actionNum;k++)
            {
                if(A->a[s][k]>maxqVal)
                {
                    maxqVal=A->a[s][k];
                    maxqIndex=k;
                }
            }
            if(maxqIndex==0)std::cout<<"up    ";
            else if(maxqIndex==1)std::cout<<"right ";
            else if(maxqIndex==2)std::cout<<"down  ";
            else if(maxqIndex==3)std::cout<<"left  ";
            else if(maxqIndex==4)std::cout<<"stay  ";
            std::cout<<" ";
        }
        std::cout<<std::endl;
    }
    std::cout<<"       maxProb:"<<maxqVal<<std::endl;
}

void showAllPolicy(Matrix *A)
{
    for(int i=0;i<numStates;i++)
    {
        std::cout<<"state :"<<i<<"  "; 
        for(int j=0;j<actionNum;j++)
        {
            std::cout<<A->a[i][j]<<" ";
        }
        std::cout<<std::endl; 
    }
}

void showVByPolicy(Matrix *A,Matrix *B)
{
    for(int i=0;i<gridR;i++)
    {
        for(int j=0;j<gridC;j++)
        {
            int s=state[i][j];
            double sum=0.0;
            for(int k=0;k<actionNum;k++)
            {
                sum+=A->a[s][k]*B->a[s][k];
            }
            std::cout<<sum<<' ';
        }
        std::cout<<std::endl;
    }
}

void getVByPolicy(Matrix *A,Matrix *B,Matrix *C)
{
    for(int i=0;i<gridR;i++)
    {
        for(int j=0;j<gridC;j++)
        {
            int s=state[i][j];
            double sum=0.0;
            for(int k=0;k<actionNum;k++)
            {
                sum+=A->a[s][k]*B->a[s][k];
            }
            C->a[s][0]=sum;
        }
    }
}

void showCount(int a[][actionNum])
{
        for(int i=0;i<numStates;i++)
        {
            std::cout<<"state :"<<i<<"  "; 
            for(int j=0;j<actionNum;j++)
            {
                std::cout<<a[i][j]<<" ";
            }
            std::cout<<std::endl; 
        }
}