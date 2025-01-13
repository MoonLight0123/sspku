#include<stdio.h>
#include<cstring>
#include<iostream>
// 第二题
// #define gridR 1//格子的行数
// #define gridC 3//格子的列数
// #define numStates 3//gridR*gridC

// 第三题
#define gridR 5//格子的行数
#define gridC 5//格子的列数
#define numStates 25//gridR*gridC


#define gamma  0.9
#define actionNum 5
#define iterThreshold  0.001

#define rBoundry -1
#define rForbidden -10
#define rTarget 1
#define rOtherstep 0

// 第二题
// int state[gridR][gridC]={
//     {0,1,2},
// };

// 第三题
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


void fillExample(int i,int j,double re[gridR][gridC],int ac[gridR][gridC],example *e)//借助reward矩阵与action矩阵得到当前状态s1的即时收益Rpi(s)，与从s1转移到s2的概率矩阵
{
    int nextr=i+dr[ac[i][j]],nextc=j+dc[ac[i][j]];
    
    int s1=state[i][j];

    memset(e->P->a[s1],0,numStates*sizeof(double));//全部赋值为零
    if(nextr<0||nextr>=gridR||nextc<0||nextc>=gridC){
        e->P->a[s1][s1]=1;//超出边界只能返回原地
        e->R->a[s1][0]=rBoundry;//超出边界，收益为-1
        return;
    }
    int s2=state[nextr][nextc];
    e->P->a[s1][s2]=1;
    e->R->a[s1][0]=re[nextr][nextc];
    return;
}

void getExampleInfo(example *e,double re[gridR][gridC],int ac[gridR][gridC])//reward矩阵与action矩阵
{
    for(int i=0;i<gridR;i++)
    {
        for(int j=0;j<gridC;j++)
        {
            fillExample(i,j,re,ac,e);
        }
    }
}

void show(Matrix *A)
{
    for(int i=0;i<gridR;i++)
    {
        for(int j=0;j<gridC;j++)
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

void showAction(int a[][gridC])
{
    // char dir[actionNum]="↑→↓←.\0";
    for(int i=0;i<gridR;i++)
    {
        for(int j=0;j<gridC;j++)
        {
            if(a[i][j]==0)std::cout<<"up    ";
            else if(a[i][j]==1)std::cout<<"right ";
            else if(a[i][j]==2)std::cout<<"down  ";
            else if(a[i][j]==3)std::cout<<"left  ";
            else if(a[i][j]==4)std::cout<<"stay  ";
            std::cout<<" ";
        }
        std::cout<<std::endl;
    }
}

void closedFormSolution(double reward[][gridC],int action[][gridC])
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

void iterativeSolution(double reward[][gridC],int action[][gridC],int maxIterNum=100)
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
        if(getDiff(&V,&nextV)<0.01)
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

void updateActionOfS(int i,int j,double reward[][gridC],int action[][gridC],Matrix *V)
{
    int s=state[i][j],s2,maxAxtionDir;
    double maxActionVal=-99999;//最大动作值，最大动作值的方向
    for(int k=0;k<actionNum;k++)
    {
        int nextr=i+dr[k],nextc=j+dc[k];//检查采取第k种动作的动作值
        double curActionVal;
        if(nextr<0||nextr>=gridR||nextc<0||nextc>=gridC)//撞墙返回
        {
            // s2=s;
            curActionVal=rBoundry+gamma*V->a[s][0];
        }
        else 
        {
            s2=state[nextr][nextc];
            curActionVal=reward[nextr][nextc]+gamma*V->a[s2][0];
        }
        if(curActionVal>=maxActionVal)
        {
            maxActionVal=curActionVal;
            maxAxtionDir=k;
        }
    }
    action[i][j]=maxAxtionDir;
    return;
}

void actionImprovement(double reward[][gridC],int action[][gridC],Matrix* V)
{
    for(int i=0;i<gridR;i++)
    {
        for(int j=0;j<gridC;j++)
        {
            updateActionOfS(i,j,reward,action,V);
        }
    }
    showAction(action);
}

void actionEvaluate(example *e,Matrix *V,double reward[][gridC],int action[][gridC],int maxIterNum)
{
    Matrix nextV(numStates,1);
    Matrix temp(numStates,1);

    getExampleInfo(e,reward,action);
    // V->zero();//初始猜测V0为全零
    // V->one();
    int i;
    for(i=0;i<maxIterNum;i++)
    {
        // V.kMultiply(gamma);
        matrixMul(e->P,V,&temp);
        temp.kMultiply(gamma);
        matrixAdd(&temp,e->R,&nextV);
        if(getDiff(V,&nextV)<iterThreshold || i==maxIterNum-1){
            // std::cout<<"111";
            V->copy(&nextV);
            break;
        }
        else V->copy(&nextV);
    }
    printf("policy evaluate iter num:%d\n",i);
    show(V);
}

void BOE(double reward[][gridC],int maxIterNum)
{
    int action[gridR][gridC];
    // memset(action,0,sizeof(int)*gridR*gridC);//初始猜测策略为全向上走
    for(int i=0;i<gridR;i++)
    {
        for(int j=0;j<gridC;j++)
        {
            action[i][j]=actionNum-1;//初始策略全为原地不动
        }
    }

    example *e=new example;
    e->P=new Matrix(numStates,numStates);
    e->R=new Matrix(numStates,1);


    Matrix V(numStates,1);
    V.zero();
    Matrix prevV(numStates,1);

    int iterNum;

    for(iterNum=0;iterNum<100;iterNum++)
    {
        printf("\nBOE iter Num %d start!\n",iterNum);
        actionEvaluate(e,&V,reward,action,maxIterNum);
        actionImprovement(reward,action,&V);
        if(getDiff(&V,&prevV)<0.001&&iterNum!=0)
            break;
        prevV.copy(&V);
        printf("\nBOE iter Num %d end!\n",iterNum);
    }
    printf("\nBOE iter Num:%d\n",iterNum);
    showAction(action);
    show(&V);
    delete e->P;
    delete e->R;
    delete e;
}

int main()
{
    double reward1[gridR][gridC]={
        {rOtherstep,rOtherstep,rOtherstep,rOtherstep,rOtherstep},
        {rOtherstep,rForbidden,rForbidden,rOtherstep,rOtherstep},
        {rOtherstep,rOtherstep,rForbidden,rOtherstep,rOtherstep},
        {rOtherstep,rForbidden,rTarget,rForbidden,rOtherstep},   
        {rOtherstep,rForbidden,rOtherstep,rOtherstep,rOtherstep},
    };

    // BOE(reward1,1<<30);//策略迭代,策略评估时迭代无穷次直到每个状态的状态值收敛
    BOE(reward1,3);
    // BOE(reward1,1);//值迭代
    return 0;
}